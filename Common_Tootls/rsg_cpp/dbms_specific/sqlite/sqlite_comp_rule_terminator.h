//
// Created by XXX on 10/14/24.
//

#ifndef SQLITE_COMP_RULE_TERMINATOR_H
#define SQLITE_COMP_RULE_TERMINATOR_H

#include "../../headers/rsg.h"

// return true if successfully handled. otherwise, return false.
bool sqlite_comp_rule_terminator(RSG*, ProductionNode*&, IR*&);

#endif // SQLITE_COMP_RULE_TERMINATOR_H
