//
// Created by XXX on 2/26/24.
//

#ifndef RSG_CPP_RSG_H
#define RSG_CPP_RSG_H

// For random number generator
#include <cstdlib>
#include <ctime>
#include <map>
#include <unordered_set>

#include "reverse_tree.h"
#include "fuzzer_configurations.h"
#include "ir_wrapper.h"
#include "node.h"
#include "parse.h"
#include "query_sequence.h"
#include "results_handler.h"
#include "utils.h"

using namespace std;

constexpr int EARLYTERM = -1;

class QueryImporter;

// TODO: move FuzzingMode related to main fuzzer?
enum FuzzingMode {
    FuzzingModeNormal = 0, // Enable all ParserFuzz features
    FuzzingModeNoFav = 1, // Disable unseen rule prioritization
    FuzzingModeNoFavNoMAB = 2, // Further disable MAB based rule prioritization
    FuzzingModeNoFavNoMABNoAcc = 3, // Further disable Categorization-based rule prioritization
    FuzzingModeNoFavNoMABNoAccNoCat = 4 // Further disable the accumulative mutations
};
const map<string, FuzzingMode> fuzzing_mode_map = {
    { "normal", FuzzingModeNormal },
    { "noFav", FuzzingModeNoFav },
    { "noFavNoMAB", FuzzingModeNoFavNoMAB },
    { "noFavNoMABNoAcc", FuzzingModeNoFavNoMABNoAcc },
    { "noFavNoMABNoAccNoCat", FuzzingModeNoFavNoMABNoAccNoCat }
};

class RSG {
public:
    bool is_use_additive_mutation = 1; // enable additive mutation or not.
    vector<ProductionNode*> all_parsed_prods; // all parsed Production Node, used for managing Production struct.
    map<string, vector<ProductionNode*>> m_all_prods_str; // mapping from grammar string to ProductionNode.
    map<IRTYPE, vector<ProductionNode*>> m_ir_types_2_prods; // mapping from IR type to ProductionNode.
    map<IRTYPE, TokenNode*> m_all_tokens_ir_type; // mapping from IR type to TokenNode.

    string root_keyword_str;
    ProductionNode* root_prod;

    map<string, IRTYPE> m_keyword_2_ir_type; // The mapping from grammar token to SQL token.

    vector<IR*> v_all_cached_irs; // just for debugging and memory management purpose.

    string cur_mutating_token_name;
    const string dbms_name;

    double epsilon; // Init as 0.3
    unsigned int cur_path_id;
    unsigned char* edge_cov_map;

    // TODO:: Implementing the fuzzing_mode.
    FuzzingMode fuzzing_mode;

    // TODO:: Implementing the QueryImporter.
    QueryImporter* query_importer;

    // user-defined function.
    void (*p_grammar_keyword_handl_func)(TokenNode*&);
    vector<ProductionNode*> (*p_rov_unimpl_func)(vector<ProductionNode*>&);
    bool (*p_comp_expr_filter)(const string&);
    bool (*p_comp_rule_terminator)(RSG*, ProductionNode*&, IR*&);
    void (*p_ir_context_setup)(RSG*, IR*&);

    IRWrapper* p_ir_wrapper;

    // save all the interesting IR trees that used for
    // accumulative mutations.
    vector<IR*> v_interesting_irs_root;
    unordered_set<uint64_t> s_interesting_irs_hash;
    map<IRTYPE, vector<IR*>> m_interesting_irs;

    // Reverse tree related.
    map<IRTYPE, vector<ReverseTreeMatch> > m_cached_reverse_tree;
    vector<ReverseTree*> v_p_cached_reverse_tree;
    unordered_set<u32> s_reverse_tree_hash_set;

    RSG(const string& dbms_name_in, const string& root_keyword_str_in, double& epsilon_in, FuzzingMode& fuzzing_mode_in,
        QueryImporter* query_importer_in,
        void (*grammar_keyword_handling_in)(TokenNode*&),
        vector<ProductionNode*> (*rov_unimpl_func_in)(vector<ProductionNode*>&),
        bool (*comp_expr_filter_in)(const string&),
        bool (*comp_rule_terminator_in)(RSG*, ProductionNode*&, IR*&),
        void (*ir_context_setup_in)(RSG*, IR*&),
        IRWrapper* ir_wrapper_in);
    ~RSG();

    // RSG generation function entry.
    [[nodiscard]] QueryStmt* generate_stmt_with_stmt_type(IRTYPE ir_type, const int gen_complex_depth =                                                                  FuzzerConfigurations::rsg_generator_comp_depth);
    [[nodiscard]] QueryStmt* mutate_on_input_stmt_ir(IR* ir_root, const int gen_complex_depth = FuzzerConfigurations::rsg_generator_comp_depth);

    // Called by generate_stmt_with_stmt_type.
    [[nodiscard]] QueryStmt* mutate_on_saved_stmt_with_stmt_type(IRTYPE ir_type, const int gen_complex_depth = FuzzerConfigurations::rsg_generator_comp_depth); // call mutate_on_input_stmt_ir.
    [[nodiscard]] QueryStmt* generate_complete_new_stmt_with_stmt_type(IRTYPE ir_type, const int gen_complex_depth =                                                                               FuzzerConfigurations::rsg_generator_comp_depth);

    // Init
    void setup_rsg_with_parsed_grammar(vector<ProductionNode*>&);
    void cache_reverse_tree();

    [[nodiscard]] IR* get_random_cached_reverse_generated_ir(IRTYPE type_in, bool is_get_full_stmt = false, int remaining_depth = -1);
    [[nodiscard]] bool is_cached_reverse_node_contain_type(IRTYPE type_in);

    IR* reverse_generate_ir_from_prod(ProductionNode* cur_prod, int remaining_depth, ReverseTree* reverse_tree, int node_idx_in_tree = -1);
    IR* reverse_generate_ir_helper(IR* cur_sub_node, int remaining_depth, ReverseTree* reverse_tree, int node_idx_in_tree = -1, int cur_iter_idx = -1);
    ReverseTree reverse_traverse_prod_node_to_stmt(ProductionNode* cur_prod);
    [[nodiscard]] bool is_reverse_tree_existed(ReverseTree v_sub_tree_in, bool is_save = true);
    [[nodiscard]] vector<ReverseTree*> mutate_reverse_tree(const ReverseTree* const tree_in);

    [[nodiscard]] ProductionNode* get_prod_from_string(string in);
    [[nodiscard]] ProductionNode* get_prod_from_ir_type(IRTYPE ir_type);
    [[nodiscard]] TokenNode* get_token_from_ir_type(IRTYPE ir_type);
    [[nodiscard]] IR* get_rand_interesting_ir_from_ir_type(IRTYPE ir_type);
    [[nodiscard]] vector<IR*> get_additive_mutation_target_node(IR* ir_root) const;

    // Feedback related interface
    virtual void rsg_succeed_with_reward(QuerySequenceGenerator* p_query_sequence_gen);
    virtual void rsg_failed_with_penalty(QuerySequenceGenerator* p_query_sequence_gen);

    void clear_v_ref_reverse_trees() {this->v_ref_reverse_trees_.clear();}

private:

    vector<ReverseTree*> v_ref_reverse_trees_;

    void connect_grammar_prods() const;
    void setup_m_keyword_2_ir_type();
    void cache_ir_in_grammar_node();

    void _cache_reverse_tree_helper(ReverseTree *p_reverse_tree);

    void classify_grammar_exprs();
    void remove_err_grammar(vector<ProductionNode*>&);
    IR* build_ir_cache_from_grammar_node(RSG* rsg, TokenNode*& cur_grammar_node);
    IR* build_ir_cache_from_grammar_node(RSG* rsg, ProductionNode*& cur_grammar_node);

    IR* generate_ir_from_prod(ProductionNode*, const int);
    vector<IR*> generate_ir_vec_from_expr(ExpressionNode* cur_expr, const int& remaining_depth, IR* replace_ir = nullptr, int replace_idx = -1);

    // helper function for choosing the target expression from production node.
    // helper function for generate_ir_from_prod.
    ExpressionNode* MAB_choose_expr(ProductionNode*);
    ExpressionNode* pick_random_non_term_expr_from_prod(ProductionNode*);
    static ExpressionNode* prioritize_term_expr(ProductionNode*);
    static pair<ExpressionNode*, int> prioritize_term_expr(vector<pair<ExpressionNode*, int>> v_expression_node);

    // string handling helper function
    string remove_literal_quoting(string in);

    IR* generate_ir_with_type(IRTYPE ir_type, const int gen_complex_depth = FuzzerConfigurations::rsg_generator_comp_depth);
    IR* generate_ir_with_type(const string& in, const bool is_use_term = false);

    void _reverse_traverse_prod_node_to_stmt_helper(ProductionNode* cur_prod, ReverseTree* res_tree) const;
};

/* RSG construction function. RSG init.
 *
 * Passed-in Functions (if optional, can pass in nullptr):
 * comp_expr_filter (optional):
 *      take referenced string as input, return true if the passed in grammar token is user-defined complex rule,
 *      false if not.
 *
 * grammar_keyword_handling_in (optional):
 *      mapping function that help the random statement generator to correctly map the grammar token to SQL token.
 *      Takes ref-TokenNode as input, modify the TokenNode.str_val, and return void.
 *
 * rov_unimpl_func_in (optional):
 *      given ref_ProductionNode, trim all the error-related expressions from the ProductionNode.
 *      return the all ProductionNode vector with error-related expressions removed.
 *
 * comp_rule_terminator (optional):
 *      Given the RSG as helper class, ref_ProductionNode and ref_IRCache as input, write user-defined terminating
 *      function from the current generated IR Tree.
 *      Return tree if the function successfully handle the current generated node, and ABORT generating the subtree.
 *      Return false if we wish to continue generating the subtree. The ProductionNode and IR can be directly modified
 *      inside this function to alter the grammar-based generation, but the user is responsible for freeing the original
 *      IR.
 *
 * ir_context_setup (required):
 *      Set up the Identifier types in the parsed IR node.
 *      set up the DataType, DataFlag, DataAffinity or even p_custom_type if necessary.
 *      Used to label the IR tree to guide the query instantiation later.
 *      Passed in RSG as helper function, and ref_IR as input.
 *      Attention: The ref_IR has all subtree generated, but no parent node available.
 */
RSG* get_new_rsg(const string& grammar_text, const string dbms_name, const string root_keyword_str, double epsilon,
    FuzzingMode fuzzing_mode,
    QueryImporter* query_importer,
    bool (*)(const string&),
    void (*)(TokenNode*&),
    vector<ProductionNode*> (*)(vector<ProductionNode*>&),
    bool (*)(RSG*, ProductionNode*&, IR*&),
    void (*)(RSG*, IR*&),
    IRWrapper*);

#endif // RSG_CPP_RSG_H
