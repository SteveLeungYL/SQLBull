//
// Created by XXX on 3/14/24.
//

#ifndef RSG_CPP_COCKROACHDB_COMP_RULE_TERMINATOR_H
#define RSG_CPP_COCKROACHDB_COMP_RULE_TERMINATOR_H

#include "../../headers/rsg.h"

// return true if successfully handled. otherwise, return false.
bool cockroachdb_comp_rule_terminator(RSG*, ProductionNode*&, IR*&);

#endif // RSG_CPP_COCKROACHDB_COMP_RULE_TERMINATOR_H
