//
// Created by XXX on 10/17/24.
//

#include "duckdb_expr_filter.h"
#include "../../headers/expr_filter_common.h"

using namespace GrammarExprFilter;

vector<ProductionNode*> remove_unimpl_duckdb(vector<ProductionNode*>& prods)
{
    vector<ProductionNode*> res_prods;
    res_prods.reserve(prods.size());
    for (auto& prod : prods) {
        // This is a recursive keyword that used by NOBODY!!!
        // Delete it to make sane with reverse traversal.
        if (prod->get_name() == "extended_indirection") {
            delete prod;
        } else if (prod->get_name() == "over_clause") {
            /*
             * The OVER window_name is causing far too many errors.
             * The parserfuzz is not handling the window names well.
             */
            remove_expression_with_token(prod, "ColId");
            res_prods.push_back(prod);
        } else if (prod->get_name() == "opt_existing_window_name") {
            /*
             * The OVER window_name is causing far too many errors.
             * The parserfuzz is not handling the window names well.
             */
            remove_expression_with_token(prod, "ColId");
            res_prods.push_back(prod);
        } else if (prod->get_name() == "select_with_parens") {
            /*
             * Avoid too simple SELECTs.
             */
            remove_expression_with_token(prod, "VariableShowStmt");
            res_prods.push_back(prod);
        } else if (prod->get_name() == "type_name_list") {
            // ignored.
            delete prod;
        } else {
            res_prods.push_back(prod);
        }
    }
    prods.clear();
    return std::move(res_prods);
}

bool duckdb_comp_expr_filter(const string& node_val)
{
    vector<string> filter_list = {
        "expr",
        "select_",
        "table_ref",
        "opt_sort_clause",
        "with_clause",
        "joined_table",
        "case_expr"
    };

    return find_substr_from_vec_helper(node_val, filter_list);
}