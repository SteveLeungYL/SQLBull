//
// Created by XXX on 3/1/24.
//

#include "../../headers/expr_filter_common.h"

using namespace GrammarExprFilter;

bool cockroachdb_comp_expr_filter(const string& node_val)
{
    vector<string> escape_list = {
        "opt_"
    };
    if (find_substr_from_vec_helper(node_val, escape_list)) {
        // escaped, do not check.
        return false;
    }

    vector<string> filter_list = {
        "expr",
        "select_",
        "table_ref"
    };

    return find_substr_from_vec_helper(node_val, filter_list);
}

vector<ProductionNode*> remove_unimpl_cockroachdb(vector<ProductionNode*>& prods)
{
    vector<ProductionNode*> trimmed_prods;
    for (ProductionNode* cur_prod : prods) {
        vector<ExpressionNode*> trimmed_exprs;
        vector<ExpressionNode*> rov_exprs;
        for (ExpressionNode* cur_expr : cur_prod->all_exprs) {
            bool is_err = false;
            for (TokenNode*& cur_token : cur_expr->tokens) {
                const string& cur_token_str = cur_token->get_string();
                if (cur_token_str == "error" || cur_token_str.find("keyword") != string::npos
                    || cur_token_str.find("unsupported") != string::npos) {
                    is_err = true;
                    break;
                }
            }

            // Also, remove the empty rule for opt_alias_clause.
            if (cur_prod->get_name() == "opt_alias_clause") {
                if (cur_expr->get_tokens().empty()) {
                    is_err = true;
                    cur_expr->set_parent_production_node(nullptr);
                }
            }

            // Also, remove the '@' iconst64 rule from d_expr
            if (cur_prod->get_name() == "d_expr") {
                if (!cur_expr->get_tokens().empty() && cur_expr->get_tokens().front()->get_string() == "'@'") {
                    is_err = true;
                    cur_expr->set_parent_production_node(nullptr);
                }
            }

            if (!is_err) {
                trimmed_exprs.push_back(cur_expr);
            } else {
                rov_exprs.push_back(cur_expr);
            }
        }
        if (!trimmed_exprs.empty()) {
            cur_prod->all_exprs = trimmed_exprs;
            trimmed_prods.push_back(cur_prod);
            if (!rov_exprs.empty()) {
                for (ExpressionNode*& rov_expr : rov_exprs) {
                    // Free the unnecessary memory.
                    // only free the memory after the trimmed_prods overrides the original.
                    delete rov_expr;
                }
            }
        } else {
            delete cur_prod;
        }
    }

    return trimmed_prods;
}