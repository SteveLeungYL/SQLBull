//
// Created by XXX on 2/26/24.
//

#include "../headers/rsg.h"
#include "../headers/expr_filter_common.h"
#include "../headers/fuzzer_configurations.h"
#include "../headers/query_importer.h"
#include "../headers/utils.h"

#if defined(cockroachdb)
#include "../dbms_specific/cockroachdb/cockroachdb_fuzzer_configurations.h"
#elif defined(duckdb)
#include "../dbms_specific/duckdb/duckdb_fuzzer_configurations.h"
#elif defined(sqlite)
#include "../dbms_specific/sqlite/sqlite_fuzzer_configurations.h"
#elif defined(mysqldb)
#include "../dbms_specific/mysqldb/mysql_fuzzer_configurations.h"
#elif defined(mariadb)
#include "../dbms_specific/mariadb/mariadb_fuzzer_configurations.h"
#elif defined(postgresql)
#include "../dbms_specific/postgresql/postgresql_fuzzer_configurations.h"
#endif

#include <algorithm>
#include <array>
#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>

using namespace std;

RSG* get_new_rsg(const string& grammar_text, const string dbms_name, const string root_keyword_str, double epsilon,
    FuzzingMode fuzzing_mode,
    QueryImporter* query_importer,
    bool (*comp_expr_filter_in)(const string&),
    void (*grammar_keyword_handling_in)(TokenNode*&),
    vector<ProductionNode*> (*rov_unimpl_func_in)(vector<ProductionNode*>&),
    bool (*comp_rule_terminator_in)(RSG*, ProductionNode*&, IR*&),
    void (*ir_context_setup_in)(RSG*, IR*&),
    IRWrapper* ir_wrapper_in)
{
    // The entry function for the RSG construction. also set up the rule filtering and classification etc.
    Tree* grammar_tree = new Tree("sql", dbms_name);
    grammar_tree->parse(grammar_text);

    vector<ProductionNode*> parsed_prods = grammar_tree->transfer_parsed_productions();
    delete grammar_tree;

    RSG* rsg = new RSG(dbms_name, root_keyword_str, epsilon, fuzzing_mode,
        query_importer,
        grammar_keyword_handling_in,
        rov_unimpl_func_in, comp_expr_filter_in, comp_rule_terminator_in,
        ir_context_setup_in, ir_wrapper_in);

    // init and setup all the required data structures for RSG.
    rsg->setup_rsg_with_parsed_grammar(parsed_prods);

    return rsg;
}

RSG::RSG(const string& dbms_name_in, const string& root_keyword_str_in, double& epsilon_in, FuzzingMode& fuzzing_mode_in,
    QueryImporter* query_importer_in,
    void (*grammar_keyword_handling_in)(TokenNode*&),
    vector<ProductionNode*> (*rov_unimpl_func_in)(vector<ProductionNode*>&),
    bool (*comp_expr_filter_in)(const string&),
    bool (*comp_rule_terminator_in)(RSG*, ProductionNode*&, IR*&),
    void (*ir_context_setup_in)(RSG*, IR*&),
    IRWrapper* ir_wrapper_in)
    : dbms_name(dbms_name_in)
      , root_keyword_str(root_keyword_str_in)
      , epsilon(epsilon_in)
      , cur_path_id(0)
      , fuzzing_mode(fuzzing_mode_in)
      , query_importer(query_importer_in)
      , p_grammar_keyword_handl_func(grammar_keyword_handling_in)
      , p_rov_unimpl_func(rov_unimpl_func_in)
      , p_comp_expr_filter(comp_expr_filter_in)
      , p_comp_rule_terminator(comp_rule_terminator_in)
      , p_ir_context_setup(ir_context_setup_in)
      , p_ir_wrapper(ir_wrapper_in)
{
    srand((unsigned int)time(nullptr));
    // set up the shadow map for edge coverage on heap.
    edge_cov_map = new unsigned char[1 >> 18]();
    fill(edge_cov_map, edge_cov_map + (1 >> 18), 0);

    query_importer->set_rsg(this);
}

RSG::~RSG()
{
    delete[] edge_cov_map;
    for (IR* cur_cached_ir : this->v_all_cached_irs) {
        // could have children, thus deep drop.
        cur_cached_ir->deep_drop();
    }
    for (auto& cur_prod : this->all_parsed_prods) {
        delete cur_prod;
    }
    // Clear all interesting irs.
    for (auto& cur_ir : this->v_interesting_irs_root) {
        cur_ir->deep_drop();
    }

    for (auto iter : this->v_p_cached_reverse_tree) {
        delete iter;
    }
}

void RSG::setup_rsg_with_parsed_grammar(vector<ProductionNode*>& parsed_prods)
{
    this->all_parsed_prods = std::move(parsed_prods); // NOT DEEP COPY.

    // Cache the grammar keyword to IRType keyword mapping.
    this->setup_m_keyword_2_ir_type();

    // Remove the unimplemented or doomed error grammar rules.
    remove_err_grammar(this->all_parsed_prods);

    for (ProductionNode* cur_prod : this->all_parsed_prods) {
        // It is possible that one non-terminal keyword has multiple grammar rule clusters.
        this->m_all_prods_str[cur_prod->name].push_back(cur_prod);
    }

    if (this->m_all_prods_str.count(this->root_keyword_str) == 0) {
        cerr << "Error: cannot find the root keyword: " << this->root_keyword_str << ". \nAbort. \n\n\n";
        abort();
    }
    this->root_prod = this->m_all_prods_str.find(this->root_keyword_str)->second.front();

    this->connect_grammar_prods();
    this->cache_ir_in_grammar_node();

    // categorize grammar rules. only need to run once.
    this->classify_grammar_exprs();
}

void RSG::connect_grammar_prods() const
{
    // Iterate all the saved prods.
    if (this->m_all_prods_str.empty()) {
        cerr << "Error: RSG::all_prods_str not setup before calling connect_grammar_prods. Abort(). \n\n\n";
        abort();
    }

    for (const auto& m_prods : this->m_all_prods_str) {
        const vector<ProductionNode*>& v_prods = m_prods.second;
        for (ProductionNode* cur_prod : v_prods) {
            for (ExpressionNode* cur_expr : cur_prod->all_exprs) {
                cur_expr->set_parent_production_node(cur_prod); // set up the reverse mapping
#ifdef DEBUG
                cerr << "Reverse Mapping cur_expr: " << cur_expr->get_command() << " to production node: " << cur_prod->get_name() << endl;
#endif
                int token_idx = -1;
                for (TokenNode* cur_token : cur_expr->get_tokens()) {
                    token_idx++;
                    cur_token->set_parent_expression_node(cur_expr); // setup the reverse mapping
#ifdef DEBUG
                    cerr << "Reverse Mapping cur_token: " << cur_token->get_string() << " to expression: ";
                    for (const TokenNode* tmp_plotting_token : cur_expr->get_tokens()) {
                        cerr << tmp_plotting_token->get_string() << " ";
                    }
                    cerr << endl;
#endif
                    if (!(cur_token->mapped_children_prods.empty())) {
                        // already setup. continue;
                        continue;
                    }
                    string cur_token_str = cur_token->get_string();
                    if (this->m_all_prods_str.count(cur_token_str)) {
                        // auto set the current token type to TypNonTermKeyword.
                        cur_token->set_mapped_children_prods(this->m_all_prods_str.find(cur_token_str)->second);

                        // set up the reverse mapping.
                        for (ProductionNode* cur_referenced_production_node : this->m_all_prods_str.find(cur_token_str)->second) {
                            cur_referenced_production_node->append_v_parent_expression_nodes(cur_expr, token_idx);
#ifdef DEBUG
                            cerr << "Reverse Mapping: " << cur_referenced_production_node->get_name() << " to expression: ";
                            for (const TokenNode* tmp_plotting_token : cur_expr->get_tokens()) {
                                cerr << tmp_plotting_token->get_string() << " ";
                            }
                            cerr << " from production node: " << cur_prod->get_name();
                            cerr << endl;
#endif
                        }
                    }
                }
            }
#ifdef DEBUG
            cerr << "Debug: For Prod: " << cur_prod->get_name() << ", getting expr: \n";
            for (int debug_idx = 0; debug_idx < cur_prod->get_exprs().size(); debug_idx++) {
                const ExpressionNode* cur_expr = cur_prod->get_exprs()[debug_idx];
                cerr << "Expr idx: " << debug_idx << ": ";
                for (const TokenNode* cur_token : cur_expr->get_tokens()) {
                    cerr << cur_token->get_string();
                    if (cur_token->get_type() == TypNonTermKeyword) {
                        cerr << "(mapped " << cur_token->get_mapped_children_prods().front()->get_name() << ")";
                    }
                    cerr << " ";
                }
                cerr << "\n";
            }
#endif
        }
    }
}

void RSG::remove_err_grammar(vector<ProductionNode*>& all_prods_in)
{
    if (this->p_rov_unimpl_func) {
        all_prods_in = p_rov_unimpl_func(all_prods_in);
    }
}

void RSG::classify_grammar_exprs()
{
    if (this->p_comp_expr_filter) {
        GrammarExprFilter::classify_grammar_exprs_helper(this, p_comp_expr_filter);
    } else {
        cerr << "Warning: Does not detect any Complex Grammar manual filtering function. "
            "Use dump filtering function instead. \n";
        GrammarExprFilter::classify_grammar_exprs_helper(this, [](const string&) -> bool { return false; });
    }
}

void RSG::setup_m_keyword_2_ir_type()
{
    if (!filesystem::exists(FuzzerConfigurations::keyword_mapping_file_path)) {
        // the keyword mapping file not exist.
        cerr << "Warning: does not find keyword mapping file. \n";
        abort();
    }
    ifstream in(FuzzerConfigurations::keyword_mapping_file_path);
    string cur_line;
    while (getline(in, cur_line)) {
        if (cur_line.empty()) {
            continue;
        }
        if (cur_line.back() == '\n') {
            // remove the new line.
            cur_line = cur_line.substr(0, cur_line.size() - 1);
        }
        if (cur_line.find(',') == string::npos) {
            continue;
        }
        vector<string> v_split = string_splitter(cur_line, ',');
        if (v_split.size() != 2) {
            cerr << "Error: Wrong format from the keyword mapping file: " << cur_line << "\n\n\n";
            abort();
        }
        IRTYPE mapped_ir_type = get_ir_type_by_string(v_split.back());
        if (mapped_ir_type == IRTypeUnknownType) {
            cerr << "Error: Does not detect IR type for node: " << v_split.back() << "\n\n\n";
            abort();
        }
        this->m_keyword_2_ir_type[v_split.front()] = mapped_ir_type;
    }
}

// Helper function for void (RSG::cache_ir_in_grammar_node)();
IR* RSG::build_ir_cache_from_grammar_node(RSG* rsg, TokenNode*& cur_grammar_node)
{

    string mapped_sql_str = cur_grammar_node->get_string();
    switch (cur_grammar_node->get_type()) {
    case (TypNonTermKeyword): {
        if (rsg->m_keyword_2_ir_type.count(cur_grammar_node->get_string()) == 0) {
            cerr << "Error: Does not detect IR type for node: " << cur_grammar_node->print_info() << "\n\n\n";
            abort();
        }
        IRTYPE ir_type = rsg->m_keyword_2_ir_type[cur_grammar_node->get_string()];

        IR* cur_ir = new IR(SymbolNonTerm, ir_type, mapped_sql_str, cur_grammar_node, nullptr, rsg->get_prod_from_string(cur_grammar_node->get_string()));
        this->v_all_cached_irs.push_back(cur_ir);
        return cur_ir;
    }
    case (TypLiteral): {
        // by default, unknown literal
        // default as Normal Term. Change later by DBMS specific code.
        mapped_sql_str = this->remove_literal_quoting(mapped_sql_str); // remove the bounded quoting.
        IR* cur_ir = new IR(SymbolTerm, IRTypeUnknownType, mapped_sql_str, cur_grammar_node);
        this->v_all_cached_irs.push_back(cur_ir);
        return cur_ir;
    }
    case (TypTermKeyword): {
        // default as Normal Term. Change later by DBMS specific code.
        IRTYPE cur_ir_type = this->m_keyword_2_ir_type[cur_grammar_node->get_string()];
        IR* cur_ir = new IR(SymbolTerm, cur_ir_type, mapped_sql_str, cur_grammar_node);
        this->v_all_cached_irs.push_back(cur_ir);
        return cur_ir;
    }
    default: {
        cerr << "Error: getting cur_token_node->get_type(): " << to_string(cur_grammar_node->get_type()) << "\n\n\n";
        abort();
    }
    }
}

// Helper function for void (RSG::cache_ir_in_grammar_node)();
IR* RSG::build_ir_cache_from_grammar_node(RSG* rsg, ProductionNode*& cur_grammar_node)
{
    // handling non-terminal keyword that didn't show up in the sub-rules.
    // most likely the root or the entry point of the grammar tree.
    // no TokenNode provided.
    string mapped_sql_str = cur_grammar_node->get_name();
    if (rsg->m_keyword_2_ir_type.count(cur_grammar_node->get_name()) == 0) {
        cerr << "Error: Does not detect IR type for Production node: " << cur_grammar_node->get_name() << "\n\n\n";
        abort();
    }
    IRTYPE ir_type = rsg->m_keyword_2_ir_type[cur_grammar_node->get_name()];

    // no TokenNode mapping, since this should be the root.
    IR* cur_ir = new IR(SymbolNonTerm, ir_type, cur_grammar_node->get_name(), nullptr, nullptr, cur_grammar_node);
    this->v_all_cached_irs.push_back(cur_ir);
    return cur_ir;
}

void RSG::cache_ir_in_grammar_node()
{
    // use the all_prods_str as entry. Most easy to access
    // must be called after this->setup_mapped_keywords();

    for (auto& m_prod : this->m_all_prods_str) {
        vector<ProductionNode*>& v_prod = m_prod.second;
        for (ProductionNode*& cur_prod : v_prod) {
            for (ExpressionNode* cur_expr : cur_prod->all_exprs) {
                for (TokenNode* cur_token : cur_expr->get_tokens()) {
                    IR* cur_ir = build_ir_cache_from_grammar_node(this, cur_token);
                    cur_token->set_cached_ir(cur_ir);
                    ProductionNode* target_prod = nullptr;
                    if (cur_ir->get_symbol_type() == SymbolNonTerm) {
                        target_prod = this->get_prod_from_string(cur_token->get_string());
                        if (target_prod == nullptr || cur_ir->get_ir_type() == IRTypeUnknownType) {
                            cerr << "Error: cannot find correct IR Type for Non-Term token: " << cur_token->get_string()
                                << "\n\n\n";
                            abort();
                        }
                        target_prod->set_cached_ir(cur_ir);
                    }

                    // modify the cached IR and the mapped grammar token to match the generation needs.
                    // call the user-defined DBMS specific keyword handling function.
                    if (this->p_grammar_keyword_handl_func != nullptr) {
                        this->p_grammar_keyword_handl_func(cur_token);
                    }

                    // save the IR type to Production mapping.
                    if (cur_token->get_type() == TypNonTermKeyword && !this->m_ir_types_2_prods.count(cur_ir->get_ir_type())) {
                        // only valid for SymbolNonTerm grammar nodes. empty for others.
                        this->m_ir_types_2_prods[cur_ir->get_ir_type()] = cur_token->get_mapped_children_prods();
                    } else if (cur_token->get_type() != TypNonTermKeyword) {
                        // For non-Symbol or NonTerm, no children, no need to setup.
                    }
                    if (!this->m_all_tokens_ir_type.count(cur_ir->get_ir_type())) {
                        this->m_all_tokens_ir_type[cur_ir->get_ir_type()] = cur_token;
                    }
#ifdef DEBUG
                    // DEBUG all the cached IR and it mapping Prods.
                    cerr << "For cur_token: " << cur_token->get_string();
                    if (target_prod != nullptr) {
                        cerr << ", target_prod: " << target_prod->get_name();
                    }
                    cerr << ", getting cached_ir: "
                         << get_string_by_ir_type(cur_token->get_cached_ir()->get_ir_type()) << ", "
                         << cur_token->get_cached_ir()->to_string() << ", "
                         << cur_token->get_cached_ir()->print_info_short()
                         << "\n\n\n";
#endif
                }
            }
        } // cur_prod loop
    }

    for (auto& m_prod : this->m_all_prods_str) {
        vector<ProductionNode*>& v_prod = m_prod.second;
        for (ProductionNode*& cur_prod : v_prod) {
            if (cur_prod->get_cached_ir() == nullptr) {
                // The left behind prod. Not used anywhere in the sub-rules.
                // Could be the entry note.
                IR* cur_ir = build_ir_cache_from_grammar_node(this, cur_prod);
                cur_prod->set_cached_ir(cur_ir);
#ifdef DEBUG
                // DEBUG all the cached IR and it mapping Prods.
                cerr << "For root_prod: " << cur_prod->get_name() << ", getting cached_ir: "
                     << get_string_by_ir_type(cur_prod->get_cached_ir()->get_ir_type()) << ", "
                     << cur_prod->get_cached_ir()->to_string() << ", "
                     << cur_prod->get_cached_ir()->print_info_short()
                     << "\n\n\n";
#endif
            }
        }
    }
}

void RSG::_cache_reverse_tree_helper(ReverseTree* p_reverse_tree) {

    if (p_reverse_tree->size() >= FuzzerConfigurations::reverse_maximum_depth) {
        delete p_reverse_tree;
        return;
    }

    for (int j = 0; j < p_reverse_tree->size(); j++) {
        auto cur_sub_node = p_reverse_tree->get_reverse_node_on_idx(j);
        IRTYPE cur_sub_node_type = p_reverse_tree->get_ir_type_on_idx(j);
#ifdef DEBUG
                cerr << "In RSG::cache_reverse_generated_ir_nodes(), saving IRTYPE: " << get_string_by_ir_type(cur_ir_type)
                     << " with idx:" << j << " in " << v_p_sub_trees->at(j).first->get_name() << "\n";
#endif
        this->m_cached_reverse_tree[cur_sub_node_type].emplace_back(p_reverse_tree, j, cur_sub_node_type);
    }

    // Don't use emplace_back, will trigger copy constructor and cause memory leak.
    this->v_p_cached_reverse_tree.push_back(p_reverse_tree);
}

void RSG::cache_reverse_tree()
{

#if defined(cockroachdb)
    std::vector<IRTYPE> v_interesting_ir_types = CockroachDBFuzzerConfigurations::cockroachdb_interesting_ir_types;
#elif defined(duckdb)
    std::vector<IRTYPE> v_interesting_ir_types = DuckDBFuzzerConfigurations::duckdb_interesting_ir_types;
#elif defined(sqlite)
    std::vector<IRTYPE> v_interesting_ir_types = SQLiteFuzzerConfigurations::sqlite_interesting_ir_types;
#elif defined(mysqldb)
    std::vector<IRTYPE> v_interesting_ir_types = MySQLFuzzerConfigurations::mysql_interesting_ir_types;
#elif defined(mariadb)
    std::vector<IRTYPE> v_interesting_ir_types = MariaDBFuzzerConfigurations::mariadb_interesting_ir_types;
#elif defined(postgresql)
    std::vector<IRTYPE> v_interesting_ir_types = PostgreSQLFuzzerConfigurations::postgresql_interesting_ir_types;
#endif

    for (IRTYPE cur_ir_type : v_interesting_ir_types) {
        ProductionNode* cur_prod = this->get_prod_from_ir_type(cur_ir_type);
        for (int i = 0; i < 100; i++) {
            ReverseTree tmp_rev_tree = this->reverse_traverse_prod_node_to_stmt(cur_prod);
            if (tmp_rev_tree.is_empty()) {
                continue;
            }
            auto* p_reverse_tree = new ReverseTree(tmp_rev_tree);

            if (p_reverse_tree == nullptr) {
                cerr << "FatalError: new vector with p_reverse_tree returns null. \n";
                abort();
            }
            if (p_reverse_tree->size() == 0) {
#ifdef DEBUG
                cerr << "v_p_sub_trees is empty. Is getting duplicated reverse tree?";
#endif
                delete p_reverse_tree;
                continue;
            }

            _cache_reverse_tree_helper(p_reverse_tree);
        }
    }

    // further mutate the generated Reverse tree.
    vector<ReverseTree*> tmp_append_reverse_trees; // for not modifying the v_p_cached_reverse_tree while iterating.
    for (auto* cur_tree: this->v_p_cached_reverse_tree) {
        vector<ReverseTree*> v_mutated_tress = this->mutate_reverse_tree(cur_tree);
        for (auto* cur_mutated_tree: v_mutated_tress) {
            tmp_append_reverse_trees.push_back(cur_mutated_tree);
        }
    }
    for (auto* tmp_append_tree: tmp_append_reverse_trees) {
        _cache_reverse_tree_helper(tmp_append_tree);
    }

#ifdef DEBUG
    cerr << "Getting num of IRTypeSelectStmt:" << this->m_cached_reverse_tree[IRTypeSelectStmt].size() << "\n\n";
#endif
}

IR* RSG::reverse_generate_ir_helper(IR* cur_sub_node, int remaining_depth, ReverseTree* cached_sub_tree, int node_idx_in_tree, int cur_iter_idx)
{
    // Helper function for RSG::reverse_generate_ir_from_prod;

#ifdef DEBUG
    cerr << "For cur_sub_node: " << get_string_by_ir_type(cur_sub_node->get_ir_type()) << "\n";
#endif // DEBUG

    pair<ExpressionNode*, int> cur_choosen_parent_expr_pair = {};
    if (cached_sub_tree != nullptr) {
        if (cur_iter_idx >= cached_sub_tree->size()) {
#ifdef DEBUG
            cerr << "Reverse generating ir from current type: " << get_string_by_ir_type(cur_sub_node->get_ir_type()) << ", reaching given tree's root. \n";
#endif
            return cur_sub_node;
        }
        // reverse generating based on given path
        cur_choosen_parent_expr_pair = make_pair<ExpressionNode*, int>(cached_sub_tree->get_expr_node_on_idx(cur_iter_idx), static_cast<int>(cached_sub_tree->get_expr_idx_on_tree_idx(cur_iter_idx)));
#ifdef DEBUG
        cerr << "Reverse generating ir from current type: " << get_string_by_ir_type(cur_sub_node->get_ir_type()) << ", getting provided expression: " << cur_choosen_parent_expr_pair.first->to_string() << "\n";
#endif
    } else {
        // random reverse generating.
        ProductionNode* cur_prod_node = cur_sub_node->get_mapped_prod_node();
        const vector<pair<ExpressionNode*, int> >& parent_exprs = cur_prod_node->get_parent_exprs();
        if (parent_exprs.empty() || this->p_ir_wrapper->is_ir_statement_typed(cur_sub_node->get_ir_type())) {
            // Reaching root or reaching statement types?
#ifdef DEBUG
            cerr << "For cur_prod_node: " << get_string_by_ir_type(cur_sub_node->get_ir_type()) << ", reaching root. \n\n\n";
#endif // DEBUG
            return cur_sub_node;
        }

        // TODO, FIXME: A better way to handle this.
        // auto cur_choosen_parent_expr_pair = vector_rand_ele(parent_exprs);
        // FIXME: Alternative way, prioritize simple expression when reverse constructing IR tree.
        cur_choosen_parent_expr_pair = this->prioritize_term_expr(parent_exprs);
    }

#ifdef DEBUG
    cerr << "Choosing parent expression: " << cur_choosen_parent_expr_pair.first->to_string() << "\n";
#endif // DEBUG

    vector<IR*> v_children = this->generate_ir_vec_from_expr(cur_choosen_parent_expr_pair.first, remaining_depth,
        cur_sub_node, cur_choosen_parent_expr_pair.second);

    ProductionNode* cur_parent_prod_node = cur_choosen_parent_expr_pair.first->get_parent_production_node();
    IR* cur_parent_ir = cur_parent_prod_node->get_cached_ir()->deep_copy();

    cur_parent_ir->free_children(); // free children, if any
    cur_parent_ir->set_children_nodes(v_children);
    cur_parent_ir->set_mapped_expr_node(cur_choosen_parent_expr_pair.first);

    p_ir_context_setup(this, cur_parent_ir);

#ifdef DEBUG
    cerr << "Getting parent_ir node: " << cur_parent_ir->to_string() << "\n";
#endif // DEBUG

    if (cached_sub_tree != nullptr && node_idx_in_tree != -1 && cur_iter_idx != -1 && node_idx_in_tree == cur_iter_idx) {
        // Already meets the sub-node tree top ir.
#ifdef DEBUG
        cerr << "Reverse generating IR type. Reaching node_idx_in_tree == cur_iter_idx. \n";
#endif
        return cur_parent_ir;
    }

    return this->reverse_generate_ir_helper(cur_parent_ir, remaining_depth, cached_sub_tree, node_idx_in_tree, cur_iter_idx + 1);
}

IR* RSG::reverse_generate_ir_from_prod(ProductionNode* cur_prod, int remaining_depth, ReverseTree* cached_sub_tree, int node_idx_in_tree)
{

#ifdef DEBUG
    cerr << "Reverse generating IRs from production node: " << cur_prod->get_name() << "\n";
#endif // DEBUG

    // Forward generate the subtree first.
    IR* cur_sub_node_ir = this->generate_ir_from_prod(cur_prod, remaining_depth);

    if (node_idx_in_tree == 0) {
        return cur_sub_node_ir;
    }

#ifdef DEBUG
    cerr << "Getting forward generation first: " << cur_sub_node_ir->to_string() << "\n";
#endif // DEBUG

    // cur_iter_idx do not start from 0. The idx 0 represents the cur_sub_node_ir.
    IR* cur_root_node = this->reverse_generate_ir_helper(cur_sub_node_ir, remaining_depth, cached_sub_tree, node_idx_in_tree, 1);

#ifdef DEBUG
    cerr << "Getting full IR: " << cur_root_node->to_string() << "\n";
#endif // DEBUG

    return cur_root_node;
}

bool RSG::is_cached_reverse_node_contain_type(IRTYPE type_in)
{
    return static_cast<bool>(this->m_cached_reverse_tree.count(type_in)) && (this->m_cached_reverse_tree[type_in].size() != 0);
}

IR* RSG::get_random_cached_reverse_generated_ir(IRTYPE type_in, bool is_get_full_stmt, int remaining_depth)
{
    ReverseTreeMatch& cur_cached_reverse_tree_match = vector_rand_ele(this->m_cached_reverse_tree[type_in]);
#ifdef DEBUG
    cerr << "In get random reverse generated ir, getting IRTYPE: " << get_string_by_ir_type(type_in) << ", getting tree size: " << cur_cached_reverse_tree.first->size() << " with index: " << cur_cached_reverse_tree.second << "\n\n\n";
#endif

    this->v_ref_reverse_trees_.push_back(cur_cached_reverse_tree_match.tree);

    ProductionNode* tmp_leaf_production_node = cur_cached_reverse_tree_match.tree->get_prod_node_on_idx( 0 /* Get the most leaf node */);
    if (is_get_full_stmt) {
        return this->reverse_generate_ir_from_prod(tmp_leaf_production_node, remaining_depth, cur_cached_reverse_tree_match.tree,
            -1);
    } else {
        return this->reverse_generate_ir_from_prod(tmp_leaf_production_node, remaining_depth, cur_cached_reverse_tree_match.tree,
            static_cast<int>(cur_cached_reverse_tree_match.matched_tree_idx));
    }
}

inline ReverseTree remove_recursive_in_reverse_path(ReverseTree path_in)
{
#ifdef DEBUG
    cerr << "Before trimming, path: \n";
    for (auto& cur_item : path_in) {
        cerr << cur_item.first->get_name() << ", ";
        if (cur_item.second.first) {
            cerr << cur_item.second.first->to_string();
        }
        cerr << "\n";
    }
    cerr << "END\n\n\n";
#endif

    ReverseTree trim_path;
    trim_path.reserve(path_in.size());

    bool is_finished = false;
    while (!is_finished) {
        int j = static_cast<int>(path_in.size()) - 1;
        is_finished = true;
        for (; j >= 0; j--) {
            const IRTYPE cur_ir_type = path_in.get_ir_type_on_idx(j);
            for (int i = 0; i < j; i++) {
                if (path_in.get_ir_type_on_idx(i) == cur_ir_type) {
                    // Find recursion, remove the gap.
                    for (int trim_idx = 0; trim_idx < path_in.size(); trim_idx++) {
                        if (trim_idx <= i || trim_idx > j) {
                            trim_path.append_node(path_in.get_reverse_node_on_idx(trim_idx));
                        } else if (trim_idx > i && trim_idx <= j) {
                            // empty. Skipped.
                        } else {
                            cerr << "Logic Error: Should not happen. Getting unexpected condition in remove_recursive_in_reverse_path();\n";
                            abort();
                        }
                    }
                    path_in = trim_path;
                    trim_path.clear();
                    trim_path.reserve(path_in.size());
                    is_finished = false;
                    break;
                }
            }
            if (!is_finished) {
                // Already done recursive trimming, jump out and RESTART/REDO scanning.
                // Required since the size of the trimmed tree has changed.
                break;
            }
        }
    }

#ifdef DEBUG
    cerr << "After trimming, path: \n";
    for (auto& cur_item : path_in) {
        cerr << cur_item.first->get_name() << ", ";
        if (cur_item.second.first) {
            cerr << cur_item.second.first->to_string();
        }
        cerr << "\n";
    }
    cerr << "END\n\n\n";
#endif

    return std::move(path_in);
}

void RSG::_reverse_traverse_prod_node_to_stmt_helper(ProductionNode* cur_prod, ReverseTree* res_tree) const {

    assert(res_tree != nullptr);

    pair<ExpressionNode*, int> cur_expr_pair = {};
    vector<pair<ExpressionNode*, int> > parent_exprs = cur_prod->get_parent_exprs();

    int maximum_depth = FuzzerConfigurations::reverse_maximum_depth;

    while (!parent_exprs.empty()) {

        bool is_chosen = false;
        maximum_depth--;

        if (maximum_depth < 0) {
            res_tree->clear();
            return;
        }

        for (auto& cur_tmp_parent_expr: parent_exprs) {
            IRTYPE parent_prod_type = cur_tmp_parent_expr.first->get_parent_production_node()->get_cached_ir()->get_ir_type();
            // In pct_reverse_prioritize_term_stmt chance, prioritize terminating with stmt type in reverse generation.
            if ( this->p_ir_wrapper->is_ir_statement_typed(parent_prod_type)  && get_pct_hit(FuzzerConfigurations::pct_reverse_prioritize_term_stmt) ) {
                cur_expr_pair = cur_tmp_parent_expr;
                is_chosen = true;
                break;
            }
        }

        if (!is_chosen) {
            cur_expr_pair = vector_rand_ele(parent_exprs);
            // is_chosen = true;
        }

        cur_prod = cur_expr_pair.first->get_parent_production_node();
        res_tree->append_node(cur_prod, cur_prod->get_expr_idx(cur_expr_pair.first), cur_expr_pair.first, cur_expr_pair.second);

        parent_exprs = cur_prod->get_parent_exprs();
        if (this->p_ir_wrapper->is_ir_statement_typed(cur_prod->get_cached_ir()->get_ir_type())) {
            return;
        }
    }

#ifdef DEBUG
    cerr << "For cur_prod_node: " << cur_prod->get_name() << ", reaching root. \n\n\n";
#endif // DEBUG
}

[[nodiscard]] vector<ReverseTree*> RSG::mutate_reverse_tree(const ReverseTree* const tree_in) {

    vector<ReverseTree*> v_res_mutated_reverse_trees;

#ifdef DEBUG
    cerr << "Mutating from original reverse tree: \n" << tree_in->debug_print() << "\n\n\n";
#endif

    for (int mut_num_idx = 0; mut_num_idx < FuzzerConfigurations::num_reverse_tree_mutation; mut_num_idx++) {
        // Iterate all nodes in the reverse tree, force mutaing to a different choice.
        for (int mut_idx = 1 /* Cannot to mutate the first one */; mut_idx < tree_in->size(); mut_idx++) {
            ProductionNode* target_prod_node = tree_in->get_prod_node_on_idx(mut_idx);
            ExpressionNode* target_expr_node = tree_in->get_expr_node_on_idx(mut_idx);
            ProductionNode* parent_production_node = tree_in->get_prod_node_on_idx(mut_idx- 1);
            const uint64_t parent_expr_size = parent_production_node->get_parent_exprs().size();

            if (parent_expr_size <= 1) {
                // This tree_node cannot be mutated at this location.
                continue;
            }

            ReverseTree res_new_tree = ReverseTree();

            // Fill till the mutation point.
            for (int iter_idx = 0; iter_idx < mut_idx; iter_idx++) {
                res_new_tree.append_node(tree_in->get_reverse_node_on_idx(iter_idx));
            }

            uint64_t ori_chosen_rule_idx = 0;
            for (; ori_chosen_rule_idx < parent_expr_size; ori_chosen_rule_idx++) {
                if (parent_production_node->get_parent_exprs()[ori_chosen_rule_idx].first == target_expr_node) {
                    break;
                }
            }

            uint64_t mutated_parent_expr_idx = ori_chosen_rule_idx;
            // Force choosing a different node.
            while (mutated_parent_expr_idx == ori_chosen_rule_idx) {
                mutated_parent_expr_idx = static_cast<uint64_t>(get_rand_int(static_cast<int>(parent_expr_size)));
            }

            pair<ExpressionNode*, int> mutated_res_expr_pair = parent_production_node->get_parent_exprs()[mutated_parent_expr_idx];

            ProductionNode* new_res_production_node = mutated_res_expr_pair.first->get_parent_production_node();

            res_new_tree.append_node(new_res_production_node, new_res_production_node->get_expr_idx(mutated_res_expr_pair.first), mutated_res_expr_pair.first, mutated_res_expr_pair.second);

            this->_reverse_traverse_prod_node_to_stmt_helper(new_res_production_node, &res_new_tree);

            if (res_new_tree.is_empty()) {
                continue;
            }

            res_new_tree = remove_recursive_in_reverse_path(res_new_tree);
            if (this->is_reverse_tree_existed(res_new_tree)) {
#ifdef DEBUG
                cerr << "Mutated to reverse tree: \n" << tree_in->debug_print() << "\nDUPLICATED.\n\n\n";
#endif
                continue;
            }

#ifdef DEBUG
            cerr << "Mutated to reverse tree: \n" << tree_in->debug_print() << "\nSAVED.\n\n\n";
#endif
            v_res_mutated_reverse_trees.push_back(new ReverseTree(res_new_tree));
        }
    }

    return v_res_mutated_reverse_trees;
}

ReverseTree RSG::reverse_traverse_prod_node_to_stmt(ProductionNode* cur_prod)
{
#ifdef DEBUG
    cerr << "Reverse traversing IRs from production node: " << cur_prod->get_name() << "\n";
#endif // DEBUG

    ReverseTree res_tree;

    // The first node in the reverse tree, start from the production node, therefore, no chosen rule(expression).
    res_tree.append_node(cur_prod, 0, nullptr, 0);

    this->_reverse_traverse_prod_node_to_stmt_helper(cur_prod, &res_tree);

    if (res_tree.is_empty()) {
        return res_tree;
    }

    res_tree = remove_recursive_in_reverse_path(res_tree);
    if (this->is_reverse_tree_existed(res_tree)) {
#ifdef DEBUG
        cerr << "Res_tree duplicated: " << res_tree.debug_print() << "\n\n";
#endif

        res_tree.clear();
    }
    return res_tree;
}

vector<IR*> RSG::generate_ir_vec_from_expr(ExpressionNode* cur_expr, const int& remaining_depth, IR* replace_ir, int replace_idx)
{
    vector<IR*> v_children;
    vector<TokenNode*> v_tokens = cur_expr->get_tokens();
    for (int idx = 0; idx < v_tokens.size(); idx++) {
        TokenNode*& cur_token = v_tokens[idx];
        if (replace_ir != nullptr && replace_idx == idx) {
            v_children.push_back(replace_ir);
        } else if (cur_token->get_type() == TypNonTermKeyword) {
            vector<ProductionNode*> v_child_mapped_prod = cur_token->get_mapped_children_prods();
            ProductionNode* cur_child_mapped_prod = vector_rand_ele(v_child_mapped_prod);

            IRTYPE cur_child_mapped_prod_type = cur_child_mapped_prod->get_cached_ir()->get_ir_type();
            if (this->is_cached_reverse_node_contain_type(cur_child_mapped_prod_type) && remaining_depth >= 0 && get_pct_hit(FuzzerConfigurations::pct_gen_from_expr_using_reversed_cached_tree) ) {
#if defined(cockroachdb)
                IR* cur_child_ir = this->get_random_cached_reverse_generated_ir(cur_child_mapped_prod_type, false, remaining_depth - 1);
#else
                IR* cur_child_ir = this->get_random_cached_reverse_generated_ir(cur_child_mapped_prod_type, false, -1);
#endif
                assert(cur_child_ir->get_ir_type() == cur_child_mapped_prod_type);
#ifdef DEBUG
                cerr << "Prioritizing reverse cached trees, from top node: " << get_string_by_ir_type(cur_child_mapped_prod_type)
                     // << ", prioritizing interesting node: " << get_string_by_ir_type(tmp_production_node->cached_ir->get_ir_type())
                     << ", getting " << cur_child_ir->to_string() << "\n\n\n";
#endif

                v_children.push_back(cur_child_ir);
            } else {
                IR* cur_child_ir = this->generate_ir_from_prod(cur_child_mapped_prod, remaining_depth - 1);
                v_children.push_back(cur_child_ir);
            }
        } else {
            // TypLiteral or TypTermKeyword.
            IR* cur_child_ir = cur_token->get_cached_ir()->deep_copy();
            v_children.push_back(cur_child_ir);
        }
    }
    return v_children;
}

IR* RSG::generate_ir_from_prod(ProductionNode* cur_prod, const int remaining_depth)
{
    // TODO::is prioritizing unseen rules necessary?
    assert(cur_prod && cur_prod->get_cached_ir() && "RSG::generate_ir_from_prod getting NULL cur_prod. \n");
    assert(this->p_ir_context_setup && "Error: this->p_ir_context_setup is NULL!!!\n\n\n");
#ifdef DEBUG
    if (!this->p_comp_rule_terminator) {
        cerr << "Warning: this->p_comp_rule_terminator is NULL. \n\n\n";
    }
    cerr << "Handling IR from prod: " << cur_prod->get_name() << "\n\n";
#endif

    IR* cur_ir = cur_prod->get_cached_ir()->deep_copy();

    if (remaining_depth < 0) {
        // cur_prod and cur_ir could be modified inside p_comp_rule_terminator.
        // cur_prod and cur_ir are passed by reference.
        if (this->p_comp_rule_terminator && this->p_comp_rule_terminator(this, cur_prod, cur_ir)) {
            // the terminator function has handled the IR. Do not continue further.
        } else {
            // the terminator failed to handle the current IR. continue forward to the
            // subtree.
            ExpressionNode* cur_expr = this->prioritize_term_expr(cur_prod);
            vector<IR*> v_children = this->generate_ir_vec_from_expr(cur_expr, remaining_depth);
            cur_ir->free_children(); // free children, if any
            cur_ir->set_children_nodes(v_children);
            cur_ir->set_mapped_expr_node(cur_expr);
        }
        // finish production node handling for remaining_depth < 0.
    } else {
        // remaining_depth >= 0
        // randomly choose expression node from the production node.
        ExpressionNode* cur_expr = nullptr;
        if (get_pct_hit(FuzzerConfigurations::pct_random_choose_rule) ) {
#ifdef DEBUG
            cerr << "Randomly choosing expression from prod: " << cur_prod->get_name() << endl;
#endif
            cur_expr = vector_rand_ele(cur_prod->all_exprs);
        } else {
#ifdef DEBUG
            cerr << "Prioritize non-term rules from prod: " << cur_prod->get_name() << endl;
#endif
#if defined(cockroachdb)
            cur_expr = this->pick_random_non_term_expr_from_prod(cur_prod);
#elif defined(duckdb)
            // cur_expr = this->MAB_choose_expr(cur_prod);
            cur_expr = this->pick_random_non_term_expr_from_prod(cur_prod);
#else
            cur_expr = this->pick_random_non_term_expr_from_prod(cur_prod);
#endif
        }
        vector<IR*> v_children = this->generate_ir_vec_from_expr(cur_expr, remaining_depth);
        cur_ir->free_children();
        cur_ir->set_children_nodes(v_children);
        cur_ir->set_mapped_expr_node(cur_expr);
    }
#ifdef DEBUG
    cerr << "From prod: " << cur_prod->get_name() << ", getting ir: " << cur_ir->to_string() << "\n";
    cur_ir->debug(cerr);
    cerr << "\n";
#endif
    this->p_ir_context_setup(this, cur_ir);
#ifdef DEBUG
    cerr << "after setup, getting ir: " << cur_ir->to_string() << "\n\n\n\n\n";
#endif

    return cur_ir;
}

#define rand_float (static_cast<float>(rand()) / static_cast<float>(RAND_MAX))

ExpressionNode* RSG::MAB_choose_expr(ProductionNode* cur_prod)
{
    if (rand_float < FuzzerConfigurations::epsilon) {
        // Random Action.
        int res_idx = rand() % (cur_prod->all_exprs.size());
        return cur_prod->all_exprs[res_idx];
    } else {
        // choose the highest reward.
        vector<double> v_reward;
        v_reward.reserve(cur_prod->all_exprs.size());
        for (ExpressionNode*& cur_expr : cur_prod->all_exprs) {
            v_reward.push_back(cur_expr->get_reward_score());
        }
        auto max_iter = max_element(v_reward.begin(), v_reward.end());
        int argmax_idx = std::distance(v_reward.begin(), max_iter); // absolute index of max reward.
        return cur_prod->all_exprs[argmax_idx];
    }
}

ExpressionNode* RSG::pick_random_non_term_expr_from_prod(ProductionNode* cur_prod)
{
#ifdef DEBUG
    if (cur_prod->all_exprs.empty()) {
        cerr << "Error: The given prod: " << cur_prod->name << " does not contain any expressions. \n";
        abort();
    }
#endif

    unsigned non_term_expr_size = (cur_prod->comp_exprs.size() + cur_prod->norm_exprs.size() + cur_prod->norm_preferred_exprs.size());
    if (non_term_expr_size != 0) {
        int chosen_idx = get_rand_int(non_term_expr_size);
        if (chosen_idx < cur_prod->comp_exprs.size()) {
            return cur_prod->comp_exprs[chosen_idx];
        }
        chosen_idx -= cur_prod->comp_exprs.size();
        if (chosen_idx < cur_prod->norm_exprs.size()) {
            return cur_prod->norm_exprs[chosen_idx];
        }
        chosen_idx -= cur_prod->norm_exprs.size();
        return cur_prod->norm_preferred_exprs[chosen_idx];
    }

#ifdef DEBUG
    cerr << "Cannot find non-terminating expression in prod: " << cur_prod->name << endl;
#endif

    return vector_rand_ele(cur_prod->all_exprs);
}

ExpressionNode* RSG::prioritize_term_expr(ProductionNode* cur_prod)
{
    // when remaining_depth < 0. Prioritize terminating the current rule-based generation.
    if (!(cur_prod->term_exprs.empty())) {
        return vector_rand_ele(cur_prod->term_exprs);
    } else if (!(cur_prod->norm_preferred_exprs.empty())) {
        return vector_rand_ele(cur_prod->norm_preferred_exprs);
    } else if (!(cur_prod->norm_exprs.empty())) {
        return vector_rand_ele(cur_prod->norm_exprs);
    } else if (!(cur_prod->non_recur_comp_exprs.empty())) {
        return vector_rand_ele(cur_prod->non_recur_comp_exprs);
    } else if (!(cur_prod->recur_comp_exprs.empty())) {
        return vector_rand_ele(cur_prod->recur_comp_exprs);
    } else {
        cerr << "Error: Cannot find any expressions in the Production Node: " << cur_prod->get_name() << "\n\n\n";
        abort();
    }
}

pair<ExpressionNode*, int> RSG::prioritize_term_expr(vector<pair<ExpressionNode*, int> > v_expression_node)
{
    vector<pair<ExpressionNode*, int> > term_exprs;
    vector<pair<ExpressionNode*, int> > norm_preferred_exprs;
    vector<pair<ExpressionNode*, int> > norm_exprs;
    vector<pair<ExpressionNode*, int> > non_recur_comp_exprs;
    vector<pair<ExpressionNode*, int> > recur_comp_exprs;
    int is_saved = false;

    for (auto& cur_expr_pair : v_expression_node) {
        switch (cur_expr_pair.first->get_expression_type()) {
        case ExpTypTerm:
            term_exprs.push_back(cur_expr_pair);
            is_saved = true;
            break;
        case ExpTypNormPrefered:
            norm_preferred_exprs.push_back(cur_expr_pair);
            is_saved = true;
            break;
        case ExpTypNorm:
            norm_exprs.push_back(cur_expr_pair);
            is_saved = true;
            break;
        case ExpTypNonRecurComp:
            non_recur_comp_exprs.push_back(cur_expr_pair);
            is_saved = true;
            break;
        case ExpTypRecComp:
            recur_comp_exprs.push_back(cur_expr_pair);
            is_saved = true;
            break;
        default:
            cerr << "Error: Unknown expression type in prioritize_term_expr. \n";
            abort();
        }
    }

    assert(is_saved && "Error: Do not find any rules matching in the rule list. Logic Error. \n\n\n");

    // when remaining_depth < 0. Prioritize terminating the current rule-based generation.
    if (!(term_exprs.empty())) {
        return vector_rand_ele(term_exprs);
    }
    if (!(norm_preferred_exprs.empty())) {
        return vector_rand_ele(norm_preferred_exprs);
    }
    if (!(norm_exprs.empty())) {
        return vector_rand_ele(norm_exprs);
    }
    if (!(non_recur_comp_exprs.empty())) {
        return vector_rand_ele(non_recur_comp_exprs);
    }
    if (!(recur_comp_exprs.empty())) {
        return vector_rand_ele(recur_comp_exprs);
    }

    cerr << "Error: Logic Error in prioritize_term_expr. \n\n\n";
    abort();
}

#undef rand_float

string RSG::remove_literal_quoting(string in)
{
    if (in.front() == '\'' || in.front() == '"') {
        in = in.substr(1, in.size() - 2);
    }

    return in;
}

// feedback related helper functions.

static void rsg_succeed_with_reward_helper(IR* cur_ir, void* var_1, void* dump_1)
{
    if (cur_ir->get_symbol_type() != SymbolNonTerm) {
        // no need to handle the Non-terminating node.
        return;
    }

    if (cur_ir->get_mapped_expr_node() == nullptr) {
#ifdef DEBUG
        cerr << "Warning: From non-terminating keyword: " << get_string_by_ir_type(cur_ir->get_ir_type())
             << ", failed to find its mapped expression. IR has been modified by user? \n\n\n";
#endif
        return;
    }

    ExpressionNode* rewarding_expr = cur_ir->get_mapped_expr_node();

    // the modifications would be persistent.
    // there is only one ExpressionNode that mapped to the specific
    // grammar rules at one fuzzing instance.
    rewarding_expr->HitCount++;

    int hc = rewarding_expr->get_hit_count();
    double rc = rewarding_expr->get_reward_score();
    rewarding_expr->set_reward_score( // reward equation. ends with 1.0
        (double(hc - 1) / double(hc)) * rc + (1.0 / double(hc)) * 1.0);

#ifdef DEBUG
    cerr << "For expression: " << rewarding_expr->get_command() << ", getting reward: " << rewarding_expr->get_reward_score()
         << ", with hit count: " << rewarding_expr->get_hit_count() << "\n\n\n";
#endif

    // Do not copy the m_interesting_irs. Use reference.
    map<IRTYPE, vector<IR*> >& m_interesting_irs = *(static_cast<map<IRTYPE, vector<IR*> >*>(var_1));
    // DO NOT DEEP_COPY to save memory.
    // The root is saved in m_interesting_ir_roots.
    IRTYPE ir_type = cur_ir->get_ir_type();
    if (m_interesting_irs.count(ir_type) == 0) {
        m_interesting_irs[ir_type] = { cur_ir };
    } else {
        m_interesting_irs[ir_type].push_back(cur_ir);
    }

#ifdef DEBUG
    cerr << "DEBUG: Saving IR: " << cur_ir->to_string() << " with type: " << get_string_by_ir_type(cur_ir->get_ir_type()) << "\n\n\n";
#endif

#ifdef DEBUG
    cerr << "DEBUG: For all_type in m_interesting_irs: " << endl;
    for (auto it = m_interesting_irs.begin(); it != m_interesting_irs.end(); it++) {
        cerr << "Type: " << get_string_by_ir_type(it->first) << endl;
        cerr << "Saved Size: " << it->second.size() << endl;
    }
    cerr << "Finished \n\n\n";
#endif
}

static void rsg_failed_with_penalty_helper(IR* cur_ir, void* dump_1, void* dump_2)
{

    if (cur_ir->get_symbol_type() != SymbolNonTerm) {
        // no need to handle the Non-terminating node.
        return;
    }

    if (cur_ir->get_mapped_expr_node() == nullptr) {
#ifdef DEBUG
        cerr << "Warning: From non-terminating keyword: " << get_string_by_ir_type(cur_ir->get_ir_type())
             << ", failed to find its mapped expression. IR has been modified by user? \n\n\n";
#endif
        return;
    }

    ExpressionNode* rewarding_expr = cur_ir->get_mapped_expr_node();

    // the modifications would be persistent.
    // there is only one ExpressionNode that mapped to the specific
    // grammar rules at one fuzzing instance.
    rewarding_expr->HitCount++;

    int hc = rewarding_expr->get_hit_count();
    double rc = rewarding_expr->get_reward_score();
    rewarding_expr->set_reward_score( // penalty equation, ends with 0.0
        (double(hc - 1) / double(hc)) * rc + (1.0 / double(hc)) * 0.0);

#ifdef DEBUG
    cerr << "For expression: " << rewarding_expr->get_command() << ", getting penalty: " << rewarding_expr->get_reward_score()
         << ", with hit count: " << rewarding_expr->get_hit_count() << "\n\n\n";
#endif
}

static void rsg_remove_is_instantiated_false_helper(IR* cur_ir, void* dump_1, void* dump_2)
{
    cur_ir->set_is_instantiated(false);
}

void RSG::rsg_succeed_with_reward(QuerySequenceGenerator* p_query_sequence_gen)
{
    if (p_query_sequence_gen->p_query_sequence->v_good_query_stmts.empty()) {
        cerr << "Logic Error: getting reward from empty v_good_query_stmts. \n\n\n";
        abort();
    }

    QueryStmt* latest_stmt = p_query_sequence_gen->p_query_sequence->v_all_query_stmts.back();

    // Do not reward or save to interesting set if the statement has been rewarded before.
    uint64_t latest_stmt_ir_type_hash = latest_stmt->stmt_ir->hash_tree();

    if (this->s_interesting_irs_hash.count(latest_stmt_ir_type_hash)) {
#ifdef DEBUG
        cerr << "Getting duplicated interesting stmt in rsg_succeed_with_reward: " << get_string_by_ir_type(latest_stmt->stmt_ir->get_ir_type())
             << ", string: " << latest_stmt->to_string()
             << "\n\n\n";
#endif
        this->v_ref_reverse_trees_.clear();
        return;
    } else {
        this->s_interesting_irs_hash.insert(latest_stmt_ir_type_hash);
    }

#ifdef DEBUG
    cerr << "Rewarding IR tree with root type: " << get_string_by_ir_type(latest_stmt->stmt_ir->get_ir_type())
         << ", string: " << latest_stmt->to_string()
         << "\n\n\n";
#endif

    IR *interesting_ir = nullptr, *interesting_stmt = nullptr;
    if (latest_stmt->gen_method == GenAllFromNew) {
        interesting_stmt = latest_stmt->stmt_ir->deep_copy();
        interesting_ir = interesting_stmt->deep_copy(); // FIXME:: not optimal. Deep copy two times to the same data.
    } else if (latest_stmt->gen_method == GenFallBackUseOriginalSequence) {
        this->v_ref_reverse_trees_.clear();
        return;
    } else {
        // GenMut or GenMutFromOtherSequence
        interesting_stmt = latest_stmt->stmt_ir->deep_copy();
        interesting_ir = latest_stmt->mutating_irs.front()->deep_copy();
    }

#ifdef DEBUG
    cerr << "Rewarding interesting_ir: " << get_string_by_ir_type(interesting_ir->get_ir_type())
         << ", string: " << interesting_ir->to_string()
         << "\n\n\n";
#endif

    this->p_ir_wrapper->iter_sub_nodes_with_handler(interesting_ir, (void*)&this->m_interesting_irs, nullptr, rsg_succeed_with_reward_helper);

    // Save and Promote whole mutated statement.
    if (interesting_stmt->get_ir_type() != interesting_ir->get_ir_type()) {
        this->m_interesting_irs[interesting_stmt->get_ir_type()].emplace_back(interesting_stmt);
#if defined(sqlite)
        this->m_interesting_irs[IRTypeCmd].emplace_back(interesting_stmt);
#elif defined(mysqldb)
        this->m_interesting_irs[IRTypeSimpleStatement].emplace_back(interesting_stmt);
#elif defined(mariadb)
        this->m_interesting_irs[IRTypeVerbClause].emplace_back(interesting_stmt);
#elif defined(postgresql)
        this->m_interesting_irs[IRTypeStmt].emplace_back(interesting_stmt);
#else
        this->m_interesting_irs[IRTypeStmt].emplace_back(interesting_stmt);
#endif
    }

    assert(!this->m_interesting_irs.empty());
    this->v_interesting_irs_root.push_back(interesting_ir);
    this->v_interesting_irs_root.push_back(interesting_stmt);

    for (auto* cur_ref_reverse_tree: this->v_ref_reverse_trees_) {
#ifdef DEBUG
        cerr << "While rsg_succeed, rewarding reverse tree: \n" << cur_ref_reverse_tree->debug_print() << "\n\n\n";
#endif
        vector<ReverseTree*> new_mutated_trees = this->mutate_reverse_tree(cur_ref_reverse_tree);
        for (auto* cur_new_mutated_tree: new_mutated_trees) {
            this->_cache_reverse_tree_helper(cur_new_mutated_tree);
        }
    }
    this->v_ref_reverse_trees_.clear();
}

void RSG::rsg_failed_with_penalty(QuerySequenceGenerator* p_query_sequence_gen)
{
    if (p_query_sequence_gen->p_query_sequence->v_good_query_stmts.empty()) {
        cerr << "Logic Error: getting reward from empty v_good_query_stmts. \n\n\n";
        abort();
    }

    QueryStmt* latest_stmt = p_query_sequence_gen->p_query_sequence->v_all_query_stmts.back();

    IR* interesting_ir = nullptr;
    if (latest_stmt->gen_method == GenAllFromNew) {
        interesting_ir = latest_stmt->stmt_ir->deep_copy();
    } else if (latest_stmt->gen_method == GenFallBackUseOriginalSequence) {
        return;
    } else {
        // GenMut or GenMutFromOtherSequence
        interesting_ir = latest_stmt->mutating_irs.front()->deep_copy();
    }

    this->p_ir_wrapper->iter_sub_nodes_with_handler(interesting_ir, nullptr, nullptr, rsg_failed_with_penalty_helper);
    interesting_ir->deep_drop();
    this->v_ref_reverse_trees_.clear();

    // no need to save into the interesting ir list.
}

vector<IR*> RSG::get_additive_mutation_target_node(IR* ir_root) const
{
#ifdef DEBUG
    cerr << "Using additive mutation. \n";
#endif

    /* Additive mutation method. Mutate on either non_term node that uses ExpTypTerm, or node that
     * contains both simple or complex nodes, but choosing the simple one. */
    vector<IR*> v_irs = p_ir_wrapper->get_all_non_term_ir_node(ir_root, ExpTypNormPrefered);

    if (v_irs.empty() || get_pct_hit(100 - FuzzerConfigurations::pct_use_additive_mutation_preferred) ) {
        // return empty vector to indicate additive target finding failed.
        // Or directly ignore additive_mutation_preferred filtering.

#ifdef DEBUG
        cerr << "From get_additive_mutation_target_node, without perferred filtering, getting node to mutate: \n";
        for (IR*& debug_ir: v_irs) {
            cerr << get_string_by_ir_type(debug_ir->get_ir_type()) << ", ";
        }
        cerr << "END; \n\n\n";
#endif

        return v_irs;
    }

    // Use is_additive_preferred nodes.
#ifdef DEBUG
    cerr << "Using additive mutation preferred. \n";
#endif
    vector<IR*> res_irs;
    res_irs.reserve(v_irs.size());
    for (auto*& cur_ir : v_irs) {
        if (cur_ir->get_mapped_prod_node()->is_additive_mutation_preferred) {
            res_irs.push_back(cur_ir);
        }
    }

    // TODO:: FIXME:: Necessary?

    if (!res_irs.empty()) {
#ifdef DEBUG
        cerr << "get_additive_mutation_target_node, getting favor and terminating node: \n";
        for (auto*& cur_ir : res_irs) {
            cerr << cur_ir->to_string() << "\n";
        }
        cerr << "End\n\n\n";
#endif
        return res_irs;
    } else {
#ifdef DEBUG
        cerr << "get_additive_mutation_target_node, getting favor and non-terminating node: \n";
        for (auto*& cur_ir : v_irs) {
            cerr << cur_ir->to_string() << "\n";
        }
        cerr << "End\n\n\n";
#endif
        return v_irs;
    }
}

[[nodiscard]] QueryStmt* RSG::mutate_on_input_stmt_ir(IR* ir_root, const int gen_complex_depth)
{
    this->v_ref_reverse_trees_.clear();

    vector<IR*> v_irs;

    if (get_pct_hit(FuzzerConfigurations::pct_use_favor_node)) {
        // Prioritize node that marked as favored.
        vector<IR*> v_favor_node = p_ir_wrapper->get_all_non_term_ir_node(ir_root, true);

        #ifdef DEBUG
        cerr << "Using favor node. \n";
        cerr << "Getting size of favor node: " << v_favor_node.size() << "\n";
        for (IR*& cur_ir: v_favor_node) {
            cerr << cur_ir->to_string() << "\n";
        }
        cerr << "End\n\n\n";
        #endif

        if (get_pct_hit(FuzzerConfigurations::pct_use_additive_mutation)) {
            // Prioritize using terminating node.

            #ifdef DEBUG
            cerr << "Using additive mutation inside the favor node. \n";
            #endif

            for (IR*& cur_favor_node: v_favor_node) {
                vector<IR*> v_tmp = get_additive_mutation_target_node(cur_favor_node);
            #ifdef DEBUG
            cerr << "Gettting size of additive mutation v_tmp: " << v_tmp.size() << "\n";
            for (IR*& cur_ir: v_tmp) {
                cerr << cur_ir->to_string() << "\n";
            }
            cerr << "End\n\n\n";
            #endif
                v_irs.insert(v_irs.end(), v_tmp.begin(), v_tmp.end());
            }
        }

        if (v_irs.empty()) {
            #ifdef DEBUG
            cerr << "Using favor node, but no node for additive mutation. \n";
            #endif
            v_irs = v_favor_node;
        }
    }

    if (v_irs.empty() && get_pct_hit(FuzzerConfigurations::pct_use_additive_mutation)) {
        v_irs = get_additive_mutation_target_node(ir_root);
        #ifdef DEBUG
        cerr << "Using pure additive mutation. \n";
        cerr << "Getting size of v_irs: " << v_irs.size() << "\n";
        for (IR*& cur_ir: v_irs) {
            cerr << cur_ir->to_string() << "\n";
        }
        cerr << "End\n\n\n";
        #endif
    }

    if (!v_irs.empty() && v_irs.back() == ir_root) {
        v_irs.pop_back();
    }

    if (v_irs.empty()) {
        // Additive mutation failed or not enabled, mutate
        // on all possible non_terminating nodes.
        v_irs = p_ir_wrapper->get_all_non_term_ir_node(ir_root);
        #ifdef DEBUG
        cerr << "Using pure additive mutation failed, mutate on all possible non_terminating nodes. \n";
        cerr << "Getting size of v_irs: " << v_irs.size() << "\n";
        for (IR*& cur_ir: v_irs) {
            cerr << cur_ir->to_string() << "\n";
        }
        cerr << "End\n\n\n";
        #endif
    }

    if (!v_irs.empty() && v_irs.back() == ir_root) {
        v_irs.pop_back();
    }

#ifdef DEBUG
    cerr << "IN IR mutation: before mutate: " << ir_root->to_string() << ", address: " << ir_root << "\n\n\n";
#endif

    if (v_irs.empty()) {
#ifdef DEBUG
        cerr << "Error: Getting super simple statement: " << ir_root->to_string()
             << ". Fall back to generation mode. \n\n\n";
#endif
        // ir_root->deep_drop();
        // abort();
        auto* res_stmt = new QueryStmt(ir_root);
        res_stmt->gen_method = GenAllFromNew;
        return res_stmt;
    }

    int rand_idx = get_rand_int(v_irs.size());
    IR* node_to_mutate = v_irs[rand_idx];
    v_irs.clear();

#ifdef DEBUG
    cerr << "DEBUG: Choosing ir_type: " << get_string_by_ir_type(node_to_mutate->get_ir_type()) << " to mutate, getting saved m_interesting_irs size: "
         << this->m_interesting_irs.count(node_to_mutate->get_ir_type()) << ", reversed cached size: " << this->m_cached_reverse_tree.count(node_to_mutate->get_ir_type()) << "\n";
#endif

    IR* mutated_node = nullptr;
    if (
        (
            this->m_interesting_irs.count(node_to_mutate->get_ir_type()) || this->is_cached_reverse_node_contain_type(node_to_mutate->get_ir_type()))
        && get_pct_hit(FuzzerConfigurations::pct_mutate_from_saved_interesting_irs) ) {

        if (this->is_cached_reverse_node_contain_type(node_to_mutate->get_ir_type()) && ( get_pct_hit(FuzzerConfigurations::pct_mutate_existing_sample_from_reversed_cached_tree) || this->m_interesting_irs.count(node_to_mutate->get_ir_type()) == 0)) {
#if defined(cockroachdb)
            mutated_node = this->get_random_cached_reverse_generated_ir(node_to_mutate->get_ir_type(), false, 1);
#elif defined(duckdb)
            mutated_node = this->get_random_cached_reverse_generated_ir(node_to_mutate->get_ir_type(), false, -1);
#elif defined(mysqldb)
            mutated_node = this->get_random_cached_reverse_generated_ir(node_to_mutate->get_ir_type(), false, -1);
#elif defined(mariadb)
            mutated_node = this->get_random_cached_reverse_generated_ir(node_to_mutate->get_ir_type(), false, -1);
#elif defined(postgresql)
            mutated_node = this->get_random_cached_reverse_generated_ir(node_to_mutate->get_ir_type(), false, -1);
#else
            // SQLite
            mutated_node = this->get_random_cached_reverse_generated_ir(node_to_mutate->get_ir_type(), false, -1);
#endif

#ifdef DEBUG
            cerr << "Using mutation from reverse cached ir, getting: " << mutated_node->to_string() << "\n\n\n";
#endif

        } else {
            // Use random existing saved subtree.
            mutated_node = vector_rand_ele(this->m_interesting_irs[node_to_mutate->get_ir_type()])->deep_copy();

#ifdef DEBUG
            cerr << "Using mutation from m_interesting_irs, getting: " << mutated_node->to_string() << "\n\n\n";
#endif
        }

        // The mutated_node type might not be the same to the node_to_mutate type! This is expected.
    } else {
        // Use pure generation based approach.
        mutated_node = generate_ir_with_type(node_to_mutate->get_ir_type(), gen_complex_depth);

#ifdef DEBUG
        cerr << "Using pure mutation node-based generation, getting: " << mutated_node->to_string() << "\n\n\n";
#endif
    }

    if (mutated_node == nullptr) {
        // mutation failed. fall back to gen mode.
#ifdef DEBUG
        cerr << "Warning: Failed to generating subtree from node: " << mutated_node->get_ir_type()
             << ", does the user modify the original cached IR for this type? \n";
        cerr << "Fall back to generation mode. \n\n\n";
#endif
        ir_root->deep_drop();
        abort();
    }
    IR* parent_node = node_to_mutate->get_parent_node();
    parent_node->swap_one_child(node_to_mutate, mutated_node, true);

#ifdef DEBUG
    cerr << "For ir type: " << get_string_by_ir_type(mutated_node->get_ir_type()) << ", use mut mode. \n";
    cerr << "Getting " << ir_root->to_string() << "\n\n\n\n";
#endif

    auto* res_stmt = new QueryStmt(ir_root);
    res_stmt->mutating_irs.push_back(mutated_node);
    res_stmt->gen_method = GenMut; // Assumed GenMut. But could be overwritten by parent function to GenMutFromOtherSeq.

    return res_stmt;
}

[[nodiscard]] QueryStmt* RSG::mutate_on_saved_stmt_with_stmt_type(IRTYPE ir_type, const int gen_complex_depth)
{

#ifdef DEBUG
    cerr << "Running mutate_on_saved_stmt_with_stmt_type: " << get_string_by_ir_type(ir_type) << "\n\n\n";
#endif

    if (this->m_interesting_irs.count(ir_type) == 0) {
        // no previous saved query with the same type.
#ifdef DEBUG
        cerr << "Revert to generate_complete_new_stmt_with_stmt_type because no existing saved stmt. \n\n\n";
#endif
        return this->generate_complete_new_stmt_with_stmt_type(ir_type, gen_complex_depth);
    }

    IR* saved_root = get_rand_interesting_ir_from_ir_type(ir_type);
    auto* res_stmt = this->mutate_on_input_stmt_ir(saved_root->deep_copy(), gen_complex_depth);

    /* Do not save the statement to mutating_original_stmt, this statement is newly appended, does not belong
     * to this query sequence before. */
    if (res_stmt->gen_method == GenMut) {
        res_stmt->gen_method = GenMutFromOtherSequence;
    }

    // Remove all marked is_instantiated to false.
    this->p_ir_wrapper->iter_sub_nodes_with_handler(res_stmt->stmt_ir, nullptr, nullptr, rsg_remove_is_instantiated_false_helper);

    return res_stmt;
}

[[nodiscard]] QueryStmt* RSG::generate_complete_new_stmt_with_stmt_type(IRTYPE ir_type, const int gen_complex_depth)
{
#ifdef DEBUG
    cerr << "Running generate_complete_new_stmt_with_stmt_type. ";
    cerr << "For ir type: " << get_string_by_ir_type(ir_type) << ", no saved interesting irs. \n\n\n";
#endif

    IR* res_ir = nullptr;
    if (this->is_cached_reverse_node_contain_type(ir_type) && get_pct_hit(FuzzerConfigurations::pct_gen_new_from_reversed_cached_tree) ) {
#if defined(cockroachdb)
        res_ir = this->get_random_cached_reverse_generated_ir(ir_type, true, 1);
#elif defined(duckdb)
        res_ir = this->get_random_cached_reverse_generated_ir(ir_type, true, -1);
#elif defined(mysqldb)
        res_ir = this->get_random_cached_reverse_generated_ir(ir_type, true, -1);
#elif defined(mariadb)
        res_ir = this->get_random_cached_reverse_generated_ir(ir_type, true, -1);
#elif defined(postgresql)
        res_ir = this->get_random_cached_reverse_generated_ir(ir_type, true, -1);
#else
        res_ir = this->get_random_cached_reverse_generated_ir(ir_type, true, -1);
#endif
    } else {
        res_ir = generate_ir_with_type(ir_type, gen_complex_depth);
    }

#ifdef DEBUG
    cerr << "For ir type: " << get_string_by_ir_type(ir_type) << ", forced to gen mode: "
         << res_ir->to_string()
         << " \n\n\n";
#endif
    auto* res_stmt = new QueryStmt(res_ir);
    res_stmt->gen_method = GenAllFromNew;
    return res_stmt;
}

[[nodiscard]] QueryStmt* RSG::generate_stmt_with_stmt_type(IRTYPE ir_type, const int gen_complex_depth)
{
    this->v_ref_reverse_trees_.clear();

    if (get_pct_hit(FuzzerConfigurations::pct_mutate_from_existing_saved_query) ) {
        return this->mutate_on_saved_stmt_with_stmt_type(ir_type, gen_complex_depth);
    } else {
        return this->generate_complete_new_stmt_with_stmt_type(ir_type, gen_complex_depth);
    }
}

[[nodiscard]] ProductionNode* RSG::get_prod_from_string(string in)
{
    if (this->m_all_prods_str.count(in)) {
        return this->m_all_prods_str[in].front();
    } else {
#ifdef DEBUG
        cerr << "Error: Cannot find production node from: " << in << "\n\n\n";
#endif
        //            abort();
        return nullptr;
    }
}

[[nodiscard]] ProductionNode* RSG::get_prod_from_ir_type(IRTYPE ir_type)
{
    if (this->m_ir_types_2_prods.count(ir_type)) {
        return this->m_ir_types_2_prods[ir_type].front();
    } else {
        cerr << "Error: Cannot find production node from: " << get_string_by_ir_type(ir_type) << "\n\n\n";
        abort();
    }
}

[[nodiscard]] TokenNode* RSG::get_token_from_ir_type(IRTYPE ir_type)
{
    if (this->m_all_tokens_ir_type.count(ir_type)) {
        return this->m_all_tokens_ir_type[ir_type];
    } else {
        cerr << "Error: Cannot find production node from: " << get_string_by_ir_type(ir_type) << "\n\n\n";
        abort();
    }
}

[[nodiscard]] IR* RSG::get_rand_interesting_ir_from_ir_type(IRTYPE ir_type)
{
    if (this->m_interesting_irs.count(ir_type) == 0 || this->m_interesting_irs[ir_type].empty()) {
        // do not have any saved irs.
#ifdef DEBUG
        cerr << "DEBUG: Does not find any saved IRs stmts with ir type: " << get_string_by_ir_type(ir_type) << "\n\n\n";
#endif
        return nullptr;
    }

    const vector<IR*>& v_interesting_irs = this->m_interesting_irs[ir_type];

    IR* chosen_one = vector_rand_ele(v_interesting_irs);

#ifdef DEBUG
    cerr << "DEBUG: Choosing IR node: " << chosen_one->to_string() << "\n\n\n";
#endif

    return chosen_one;
}

// RSG generation function entry.
IR* RSG::generate_ir_with_type(IRTYPE ir_type, const int gen_complex_depth)
{
    if (!this->m_ir_types_2_prods.count(ir_type)) {
#ifdef DEBUG
        cerr << "Warning: RSG::generate_ir_with_type, cannot find Prod mapping for ir_type: " << get_string_by_ir_type(ir_type) << "\n\n\n";
#endif
        return nullptr;
    }
    vector<ProductionNode*> v_prods = this->m_ir_types_2_prods[ir_type];
    return this->generate_ir_from_prod(v_prods.front(), gen_complex_depth);
}

IR* RSG::generate_ir_with_type(const string& in, const bool is_use_term)
{
    IRTYPE ir_type = get_ir_type_by_string(in);
    if (ir_type == IRTypeUnknownType) {
        cerr << "Error: RSG::generate_ir_with_type, cannot recognize type: " << get_string_by_ir_type(ir_type) << "\n\n\n";
        abort();
    }
    return generate_ir_with_type(ir_type, is_use_term);
}

[[nodiscard]] bool RSG::is_reverse_tree_existed(ReverseTree v_sub_tree_in, bool is_save)
{
    string tmp_str;
    for (size_t idx = 0; idx < v_sub_tree_in.size(); idx++) {
        tmp_str += v_sub_tree_in.get_prod_node_on_idx(idx)->get_name() + "_";
    }

    u32 cur_hash = hash32(tmp_str.c_str(), tmp_str.size(), 100);

    if (this->s_reverse_tree_hash_set.count(cur_hash) == 0) {
        if (is_save) {
            this->s_reverse_tree_hash_set.insert(cur_hash);
        }
#ifdef DEBUG
        cerr << "Saving reverse path: " << tmp_str << "\n";
#endif
        return false;
    } else {
#ifdef DEBUG
        cerr << "Skipping reverse path because of duplication: " << tmp_str << "\n";
#endif
        return true;
    }
}