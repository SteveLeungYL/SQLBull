//
// Created by XXX on 12/10/24.
//

#include "postgresql_expr_filter.h"
#include "../../headers/expr_filter_common.h"
#include "../../headers/utils.h"

using namespace GrammarExprFilter;

vector<ProductionNode*> remove_unimpl_postgresql(vector<ProductionNode*>& prods)
{

    vector<ProductionNode*> res_prods;
    res_prods.reserve(prods.size());

    vector<string> filter_list = {
        "reserved_keyword",
        "unreserved_keyword",
        "col_name_keyword",
        "type_func_name_keyword",
        "bare_label_keyword",
    };

    for (auto& prod : prods) {
        bool is_filtered = false;
        for (auto& filter_str : filter_list) {
            if (prod->get_name() == filter_str) {
                delete prod;
                is_filtered = true;
                break;
            }
            remove_expression_with_token(prod, filter_str);
        }
        if (is_filtered) {
            continue;
        }
        res_prods.push_back(prod);
    }

    return std::move(res_prods);
}

bool postgresql_comp_expr_filter(const string& node_val)
{

    if (node_val == "opt_equal") {
        return false;
    }

    if (findStringIn(node_val, "opt_")) {
        return true;
    }


    // Too much of the expression that limits the generation?
    vector<string> filter_list = {
        "a_expr",
        "b_expr",
        "c_expr",
        "table_ref"
    };

    return find_substr_from_vec_helper(node_val, filter_list);
}