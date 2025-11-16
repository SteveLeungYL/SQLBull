//
// Created by XXX on 12/10/24.
//

#ifndef SQLITE_EXPR_FILTER_H
#define SQLITE_EXPR_FILTER_H

#include "../../headers/expr_filter_common.h"

vector<ProductionNode*> remove_unimpl_sqlite(vector<ProductionNode*>& prods);
bool sqlite_comp_expr_filter(const string& node_val);

#endif // SQLITE_EXPR_FILTER_H
