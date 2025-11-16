//
// Created by XXX on 12/10/24.
//

#ifndef MYSQL_EXPR_FILTER_H
#define MYSQL_EXPR_FILTER_H

#include "../../headers/expr_filter_common.h"

vector<ProductionNode*> remove_unimpl_mysql(vector<ProductionNode*>& prods);
bool mysql_comp_expr_filter(const string& node_val);

#endif // MYSQL_EXPR_FILTER_H
