//
// Created by XXX on 10/17/24.
//

#ifndef DUCKDB_EXPR_FILTER_H
#define DUCKDB_EXPR_FILTER_H

#include "../../headers/expr_filter_common.h"

vector<ProductionNode*> remove_unimpl_duckdb(vector<ProductionNode*>& prods);
bool duckdb_comp_expr_filter(const string& node_val);

#endif // DUCKDB_EXPR_FILTER_H
