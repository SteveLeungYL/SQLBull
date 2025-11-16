//
// Created by XXX on 10/14/24.
//

#ifndef DUCKDB_COMPL_RULE_TERMINATOR_H
#define DUCKDB_COMPL_RULE_TERMINATOR_H

#include "../../headers/rsg.h"

// return true if successfully handled. otherwise, return false.
bool duckdb_comp_rule_terminator(RSG*, ProductionNode*&, IR*&);

#endif // DUCKDB_COMPL_RULE_TERMINATOR_H
