//
// Created by XXX on 12/10/24.
//

#include "sqlite_expr_filter.h"
#include "../../headers/expr_filter_common.h"

using namespace GrammarExprFilter;

vector<ProductionNode*> remove_unimpl_sqlite(vector<ProductionNode*>& prods)
{
    vector<ProductionNode*> res_prods;
    res_prods.reserve(prods.size());
    for (auto& prod : prods) {
        // FIXME:: Maybe we should remove the unimpl exprs here. Should add back later. 
        if (prod->get_name() == "from") {
            vector<ExpressionNode*> expr_list;
            for (auto& expr : prod->all_exprs) {
                // from::= nm COMMA seltablist .
                if (expr->get_tokens().size() != 4) {
                    delete expr;
                } else {
                    expr_list.push_back(expr);
                }
            }
            prod->set_exprs(expr_list);
        }
        res_prods.push_back(prod);
    }
    prods.clear();
    return std::move(res_prods);
}

bool sqlite_comp_expr_filter(const string& node_val)
{
    // Too much of the expression that limits the generation?
    vector<string> filter_list = {
        "expr",
        "select"
        // "selectnowith",
        // "seltablist",
        // "case_exprlist",
    };

    return find_substr_from_vec_helper(node_val, filter_list);
}