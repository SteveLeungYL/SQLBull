//
// Created by XXX on 10/14/24.
//

#ifndef MYSQL_COMP_RULE_TERMINATOR_H
#define MYSQL_COMP_RULE_TERMINATOR_H

#include "../../headers/rsg.h"

// return true if successfully handled. otherwise, return false.
bool mysql_comp_rule_terminator(RSG*, ProductionNode*&, IR*&);

#endif // MYSQL_COMP_RULE_TERMINATOR_H
