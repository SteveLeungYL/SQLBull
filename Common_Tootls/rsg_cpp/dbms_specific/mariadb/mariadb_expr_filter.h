//
// Created by XXX on 12/10/24.
//

#ifndef MARIADB_EXPR_FILTER_H
#define MARIADB_EXPR_FILTER_H

#include "../../headers/expr_filter_common.h"

vector<ProductionNode*> remove_unimpl_mariadb(vector<ProductionNode*>& prods);
bool mariadb_comp_expr_filter(const string& node_val);

#endif // MARIADB_EXPR_FILTER_H
