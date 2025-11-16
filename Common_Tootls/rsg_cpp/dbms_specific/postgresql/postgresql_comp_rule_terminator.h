//
// Created by XXX on 10/14/24.
//

#ifndef POSTGRESQL_COMP_RULE_TERMINATOR_H
#define POSTGRESQL_COMP_RULE_TERMINATOR_H

#include "../../headers/rsg.h"

// return true if successfully handled. otherwise, return false.
bool postgresql_comp_rule_terminator(RSG*, ProductionNode*&, IR*&);

#endif // POSTGRESQL_COMP_RULE_TERMINATOR_H
