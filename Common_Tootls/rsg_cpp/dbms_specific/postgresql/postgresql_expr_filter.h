//
// Created by XXX on 12/10/24.
//

#ifndef POSTGRESQL_EXPR_FILTER_H
#define POSTGRESQL_EXPR_FILTER_H

#include "../../headers/expr_filter_common.h"

vector<ProductionNode*> remove_unimpl_postgresql(vector<ProductionNode*>& prods);
bool postgresql_comp_expr_filter(const string& node_val);

#endif // POSTGRESQL_EXPR_FILTER_H
