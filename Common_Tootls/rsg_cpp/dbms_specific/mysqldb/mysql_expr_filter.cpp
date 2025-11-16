//
// Created by XXX on 12/10/24.
//

#include "mysql_expr_filter.h"
#include "../../headers/expr_filter_common.h"
#include "../../headers/utils.h"

using namespace GrammarExprFilter;

vector<ProductionNode*> remove_unimpl_mysql(vector<ProductionNode*>& prods)
{

    vector<ProductionNode*> res_prods;
    res_prods.reserve(prods.size());

    for (auto& prod : prods) {
        remove_expression_with_token(prod, "ident_keyword");
        remove_expression_with_token(prod, "role_keyword");
        remove_expression_with_token(prod, "label_keyword");
        remove_expression_with_token(prod, "label_keyword");
        remove_expression_with_token(prod, "shutdown_stmt");
        remove_expression_with_token(prod, "restart_server_stmt");
        remove_expression_with_token(prod, "param_marker");
        remove_expression_with_token(prod, "DUMPFILE");
        
        // Experimental. 
        remove_expression_with_token(prod, "table_value_constructor");
        remove_expression_with_token(prod, "explicit_table");

        if (prod->get_name() == "ident_keyword") {
            delete prod;
        } else if (prod->get_name() == "ident_keywords_ambiguous_1_roles_and_labels") {
            delete prod;
        } else if (prod->get_name() == "ident_keywords_ambiguous_2_labels") {
            delete prod;
        } else if (prod->get_name() == "ident_keywords_ambiguous_3_roles") {
            delete prod;
        } else if (prod->get_name() == "ident_keywords_ambiguous_4_system_variables") {
            delete prod;
        } else if (prod->get_name() == "ident_keywords_unambiguous") {
            delete prod;
        } else if (prod->get_name() == "role_keyword") {
            delete prod;
        } else if (prod->get_name() == "lvalue_keyword") {
            delete prod;
        } else if (prod->get_name() == "label_keyword") {
            delete prod;
        } else if (prod->get_name() == "shutdown_stmt") {
            delete prod;
        } else if (prod->get_name() == "restart_server_stmt") {
            delete prod;
        } else if (prod->get_name() == "param_marker") {
            delete prod;
        } else if (prod->get_name() == "opt_from_clause") {
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

        } else if (prod->get_name() == "from_tables") {
            remove_expression_with_token(prod, "DUAL_SYM");
            res_prods.push_back(prod);
        } else if (prod->get_name() == "opt_table_alias") {
            remove_expression_with_token(prod, "opt_as");
            res_prods.push_back(prod);
        } else {
            res_prods.push_back(prod);
        }
    }

    return std::move(res_prods);
}

bool mysql_comp_expr_filter(const string& node_val)
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
        "partition_clause",
        "references",
        // "table_factor",
        "joined_table",
    };

    return find_substr_from_vec_helper(node_val, filter_list);
}