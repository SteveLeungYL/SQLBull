//
// Created by XXX on 12/10/24.
//

#include "mariadb_expr_filter.h"
#include "../../headers/expr_filter_common.h"
#include "../../headers/utils.h"

using namespace GrammarExprFilter;

vector<ProductionNode*> remove_unimpl_mariadb(vector<ProductionNode*>& prods)
{

    vector<ProductionNode*> res_prods;
    res_prods.reserve(prods.size());

    for (auto& prod : prods) {
        // remove_expression_with_token(prod, "ident_keyword");
        // remove_expression_with_token(prod, "role_keyword");
        remove_expression_with_token(prod, "shutdown");
        remove_expression_with_token(prod, "param_marker");
        remove_expression_with_token(prod, "DUMPFILE");

        remove_expression_with_token(prod, "help");

        remove_expression_with_token(prod, "reserved_keyword_udt");
        remove_expression_with_token(prod, "non_reserved_keyword_udt");
        remove_expression_with_token(prod, "DUAL_SYM");

        remove_expression_with_token(prod, "table_value_constructor");
        // remove_expression_with_token(prod, "explicit_table");
        remove_expression_with_token(prod, "keyword_sp_var_not_label");
        remove_expression_with_token(prod, "keyword_func_sp_var_and_label");
        remove_expression_with_token(prod, "reserved_keyword_udt_not_param_type");
        remove_expression_with_token(prod, "keyword_table_alias");
        remove_expression_with_token(prod, "keyword_ident");
        remove_expression_with_token(prod, "keyword_sysvar_name");
        remove_expression_with_token(prod, "keyword_set_usual_case");
        remove_expression_with_token(prod, "keyword_sp_var_not_label");
        remove_expression_with_token(prod, "keyword_sp_head");
        remove_expression_with_token(prod, "keyword_verb_clause");
        remove_expression_with_token(prod, "keyword_sp_var_and_label");
        remove_expression_with_token(prod, "ROLE_SYM");
        remove_expression_with_token(prod, "USER_SYM");
        remove_expression_with_token(prod, "load");

        if (prod->get_name() == "opt_from_clause") {
            vector<ExpressionNode*> new_exprs;
            for (auto& expr : prod->all_exprs) {
                if (expr->get_tokens().size() == 0) {
                    delete expr;
                } else {
                    new_exprs.push_back(expr);
                }
            }
            prod->set_exprs(new_exprs);

            res_prods.push_back(prod);
        } else if (prod->get_name() == "shutdown") {
            delete prod;
        } else if (prod->get_name() == "param_marker") {
            delete prod;
        } else if (prod->get_name() == "help") {
            delete prod;
        } else if (prod->get_name() == "reserved_keyword_udt") {
            delete prod;
        } else if (prod->get_name() == "non_reserved_keyword_udt") {
            delete prod;
        } else if (prod->get_name() == "table_value_constructor") {
            delete prod;
        } else if (prod->get_name() == "keyword_sp_var_not_label") {
            delete prod;
        } else if (prod->get_name() == "keyword_func_sp_var_and_label") {
            delete prod;
        } else if (prod->get_name() == "reserved_keyword_udt_not_param_type") {
            delete prod;
        } else if (prod->get_name() == "keyword_table_alias") {
            delete prod;
        } else if (prod->get_name() == "keyword_ident") {
            delete prod;
        } else if (prod->get_name() == "keyword_sysvar_name") {
            delete prod;
        } else if (prod->get_name() == "keyword_set_usual_case") {
            delete prod;
        } else if (prod->get_name() == "keyword_sp_var_not_label") {
            delete prod;
        } else if (prod->get_name() == "keyword_sp_head") {
            delete prod;
        } else if (prod->get_name() == "keyword_verb_clause") {
            delete prod;
        } else if (prod->get_name() == "keyword_func_sp_var_and_label") {
            delete prod;
        } else if (prod->get_name() == "keyword_sp_var_and_label") {
            delete prod;
        } else if (prod->get_name() == "load") {
            delete prod;
        } else if (prod->get_name() == "limit_option") {
            remove_expression_with_token(prod, "ident_cli");
            res_prods.push_back(prod);
        } else if (prod->get_name() == "limit_option") {
            remove_expression_with_token(prod, "ident_cli");
            remove_expression_with_token(prod, "param_marker");
            res_prods.push_back(prod);
        } else {
            res_prods.push_back(prod);
        }
    }

    return std::move(res_prods);
}

bool mariadb_comp_expr_filter(const string& node_val)
{

    if (node_val == "opt_equal") {
        return false;
    }

    if (findStringIn(node_val, "opt_")) {
        return true;
    }


    // Too much of the expression that limits the generation?
    vector<string> filter_list = {
        "expr",
        "simple_expr",
        "in_sum_expr",
        "bit_expr",
        "subquery",
        "opt_window_partition_clause",
        "references",
        // "table_factor",
        "join_table",
        "join_table_parens"
    };

    return find_substr_from_vec_helper(node_val, filter_list);
}