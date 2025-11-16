//
// Created by XXX on 3/19/24.
//

#ifndef RSG_CPP_COCKROACHDB_EXPR_FILTER_H
#define RSG_CPP_COCKROACHDB_EXPR_FILTER_H

#include "../../headers/expr_filter_common.h"

vector<ProductionNode*> remove_unimpl_cockroachdb(vector<ProductionNode*>& prods);
bool cockroachdb_comp_expr_filter(const string& node_val);

#endif // RSG_CPP_COCKROACHDB_EXPR_FILTER_H
