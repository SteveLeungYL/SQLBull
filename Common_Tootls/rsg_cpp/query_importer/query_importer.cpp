//
// Created by XXX on 5/17/24.
//

#include "../headers/query_importer.h"
#include "../headers/rsg.h"

#include <algorithm>

vector<IR*> QueryImporter::import_grammar(const string file_name)
{
    string content = read_file_to_str(file_name);
    return this->parse_grammar_dump(content);
}

vector<IR*> QueryImporter::parse_grammar_dump(const string in)
{
    vector<string> v_all_stmts = string_splitter(in, "\n");
    vector<string> v_single_stmt;
    vector<IR*> v_res;

    for (string& cur_in : v_all_stmts) {
        v_single_stmt.push_back(cur_in);
        if (cur_in.find("stmt_block") != string::npos) {
            v_res.push_back(this->parse_one_stmt(v_single_stmt));
            v_single_stmt.clear();
        }
    }

    return v_res;
}

IR* QueryImporter::parse_one_stmt(vector<string> in)
{

    vector<pair<string, pair<int, string>>> v_pair; // save the top keyword to rule idx mapping
    vector<IR*> v_ir;

    for (string& cur_line : in) {
        string cur_keyword = string_splitter(cur_line, "(")[1];
        cur_keyword = string_splitter(cur_keyword, ",")[0];

        int gram_rule_idx = stoi(string_splitter(cur_line, ",")[1]);

        string gram_rule_str = string_splitter(cur_line, ",")[2];
        gram_rule_str = string_splitter(gram_rule_str, "[")[1];
        gram_rule_str = string_splitter(gram_rule_str, "]")[0];

        v_pair.push_back(make_pair(cur_keyword, make_pair(gram_rule_idx, gram_rule_str)));

#ifdef DEBUG
        do {
            // DEBUG::keyword to rule mapping
            if (gram_rule_idx == -1) {
                cerr << "Getting terminating keyword: " << cur_keyword << "\n\n\n";
                break;
            }
            string cur_token_seq = string_splitter(cur_line, "[")[1];
            cur_token_seq = string_splitter(cur_token_seq, "]").front();

            ProductionNode* cur_debug_prod = rsg->get_prod_from_string(cur_keyword);

            cerr << "From production node: " << cur_keyword << ", getting rule idx: "
                 << gram_rule_idx << ", extracted rule: " << cur_token_seq
                 << ", saved expression: ";
            const vector<TokenNode*>& v_debug_token = cur_debug_prod->get_exprs()[gram_rule_idx]->get_tokens();
            for (const auto& cur_debug_token : v_debug_token) {
                cerr << cur_debug_token->get_string() << ",";
            }
            cerr << "\n\n\n";
        } while (0);
#endif
    }

#ifdef DEBUG
    cerr << "v_pair is: \n";
    for (const pair<string, pair<int, string>>& cur_iter : v_pair) {
        cerr << cur_iter.first << ", " << cur_iter.second.first << ", " << cur_iter.second.second << "\n";
    }
#endif

    // construct the IR from the query grammar log.
    // directly use the root node to construct all the nodes.
    ProductionNode* root_prod = rsg->get_prod_from_string(v_pair.back().first);
    if (root_prod == nullptr) {
        cerr << "Error: cannot find the Production node from keyword: " << v_pair.back().first << "\n\n\n";
        abort();
    }

    IR* res_root_ir = this->generate_ir_from_prod(root_prod, v_pair, v_pair.size() - 1);

#ifdef DEBUG
    res_root_ir->debug(cerr);
#endif

    return res_root_ir;
}

IR* QueryImporter::generate_ir_from_prod(ProductionNode* cur_prod, vector<pair<string, pair<int, string>>>& v_pair, unsigned long cur_idx)
{

    // TODO::is prioritizing unseen rules necessary?
    assert(cur_prod && cur_prod->get_cached_ir() && "RSG::generate_ir_from_prod getting NULL cur_prod. \n");
    assert(this->rsg->p_ir_context_setup && "Error: rsg->p_ir_context_setup is NULL!!!\n\n\n");

    IR* cur_prod_ir = cur_prod->get_cached_ir()->deep_copy();

    int chosen_rule_idx = v_pair[cur_idx].second.first;
    if (chosen_rule_idx < 0 || chosen_rule_idx >= cur_prod->get_exprs().size()) {
        cerr << "Error: Cannot find rule idx for idx: " << cur_idx
             << ", production node: " << v_pair[cur_idx].first << "\n\n\n";
        abort();
    }

    ExpressionNode* cur_chosen_expr = cur_prod->get_exprs()[chosen_rule_idx];
    vector<IR*> v_children = this->generate_ir_vec_from_expr(cur_chosen_expr, v_pair, cur_idx);
    cur_prod_ir->free_children(); // free children, if any
    cur_prod_ir->set_children_nodes(v_children);
    cur_prod_ir->set_mapped_expr_node(cur_chosen_expr);

#ifdef DEBUG
    cerr << "From prod: " << cur_prod->get_name() << ", getting ir: " << cur_prod_ir->to_string() << "\n";
    cur_prod_ir->debug(cerr);
    cerr << "\n";
#endif
    this->rsg->p_ir_context_setup(this->rsg, cur_prod_ir);
#ifdef DEBUG
    cerr << "after setup, getting ir: " << cur_prod_ir->to_string() << "\n\n\n\n\n";
#endif

    // invalidate the current v_pair, avoid the same type of IR appear in the nested structure.
    v_pair[cur_idx].first.clear();

    return cur_prod_ir;
}

vector<IR*> QueryImporter::generate_ir_vec_from_expr(ExpressionNode* cur_expr, vector<pair<string, pair<int, string>>>& v_pair, unsigned long cur_idx)
{

    vector<IR*> v_children;
    const vector<TokenNode*>& token_seq = cur_expr->get_tokens();

    if (token_seq.size() != 0) {
        for (auto token_idx = token_seq.size() - 1; /* iterate to 0, then break */; token_idx--) {
            TokenNode* cur_token = token_seq[token_idx];
            if (cur_token->get_type() == TypNonTermKeyword) {
                vector<ProductionNode*> v_child_mapped_prod = cur_token->get_mapped_children_prods();
                ProductionNode* cur_child_mapped_prod = vector_rand_ele(v_child_mapped_prod);
                int child_prod_idx = this->find_mapped_v_pair_idx(v_pair, cur_token->get_string(), (int)cur_idx, false);

#ifdef DEBUG
                cerr << "For cur non-term token: " << cur_token->get_string() << ", getting mapped SQL dump sequence idx: "
                     << child_prod_idx << "\n\n\n";
#endif

                IR* cur_child_ir = this->generate_ir_from_prod(cur_child_mapped_prod, v_pair, child_prod_idx);
                v_children.push_back(cur_child_ir);
            } else {
                // TypLiteral or TypTermKeyword.
                IR* cur_child_ir = cur_token->get_cached_ir()->deep_copy();
                int child_prod_idx = this->find_mapped_v_pair_idx(v_pair, cur_token->get_string(), (int)cur_idx, true);
                if (child_prod_idx != -1) {
                    cur_child_ir->set_str_val(v_pair[child_prod_idx].second.second);
                    v_pair[child_prod_idx].first.clear();
#ifdef DEBUG
                    cerr << "For cur term token: " << cur_token->get_string() << ", getting mapped string: "
                         << cur_child_ir->get_str_val() << "\n\n\n";
#endif
                } else {
#ifdef DEBUG
                    cerr << "Error: Cannot find the mapped string for Literal or Term Keyword: "
                         << cur_token->get_string() << "\n\n\n";
#endif
                }
                v_children.push_back(cur_child_ir);
            }
            if (token_idx == 0) {
                break;
            }
        }
    }

    std::reverse(v_children.begin(), v_children.end());

    return v_children;
}

int QueryImporter::find_mapped_v_pair_idx(const vector<pair<string, pair<int, string>>>& v_pair, string str, int tail_idx, bool is_literal)
{
    for (int idx = tail_idx - 1; idx >= 0; idx--) {
        if (!is_literal && v_pair[idx].first == str) {
            return idx;
        } else if (is_literal && v_pair[idx].first == str) {
            return idx;
        }
    }
    return -1;
}