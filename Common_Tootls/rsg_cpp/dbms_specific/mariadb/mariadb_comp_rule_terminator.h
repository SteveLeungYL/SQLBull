//
// Created by XXX on 10/14/24.
//

#ifndef MARIADB_COMP_RULE_TERMINATOR_H
#define MARIADB_COMP_RULE_TERMINATOR_H

#include "../../headers/rsg.h"

// return true if successfully handled. otherwise, return false.
bool mariadb_comp_rule_terminator(RSG*, ProductionNode*&, IR*&);

#endif // MARIADB_COMP_RULE_TERMINATOR_H
