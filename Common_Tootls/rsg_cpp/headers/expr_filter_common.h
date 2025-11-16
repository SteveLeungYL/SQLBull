//
// Created by XXX on 3/1/24.
//

#ifndef RSG_CPP_EXPR_FILTER_COMMON_H
#define RSG_CPP_EXPR_FILTER_COMMON_H

#include <string>
#include <vector>

#include "node.h"

using std::string;
using std::vector;

class RSG;

inline void remove_expression_with_token(ProductionNode*& prod, const string token_str)
{
    bool is_rov = false;
    ssize_t idx = 0;
    for (; idx < prod->all_exprs.size(); idx++) {
        for (auto& cur_token : prod->all_exprs[idx]->get_tokens()) {
            if (cur_token->str_value == token_str) {
                is_rov = true;
                break;
            }
        }
        if (is_rov) {
            break;
        }
    }
    if (is_rov) {
        auto expr_to_rov = prod->all_exprs[idx];
        prod->all_exprs.erase(prod->all_exprs.begin() + idx);
        delete expr_to_rov;
    }
}

namespace GrammarExprFilter {
// CockroachDB
bool cockroachdb_expr_filter(const std::string&);
vector<ProductionNode*> remove_unimpl_cockroachdb(vector<ProductionNode*>&);

// Common
bool find_substr_from_vec_helper(const std::string& str, std::vector<std::string>& v_sub);

/*
 * A classifier to categorize the observed grammar rules into different types.
 * Terminal Rules:
 *      The rules that doomed to be terminated in N steps. Most preferred because of
 *      immediately termination.
 * Normal Preferred Rules:
 *      Cannot determine whether the current rule would lead to termination
 *      or complex, or somewhere in-between. However, did detect term rule in the subtree.
 *      Less preferred than Terminal Rules because delayed termination.
 * Normal Rules:
 *      Cannot determine whether the current rule would lead to termination
 *      or complex, or somewhere in-between. Did not detect term rules in the subtree.
 * Complex Rules: Grammar Rules that doomed to lead to more complex expressions.
 *      Recursive Complex Rules:
 *          recursively referencing the current root keyword.
 *      Non-recursive Complex Rules:
 *          rules that the user defined to be complex.
 *
 * Design:
 * Recursively scan through all the non-terminal keywords under each grammar expressions.
 * Label them in the ProductionNode structure.
 *
 * */
void classify_grammar_exprs_helper(RSG* rsg, bool (*comp_filter_func)(const string&));
}

#endif // RSG_CPP_EXPR_FILTER_COMMON_H
