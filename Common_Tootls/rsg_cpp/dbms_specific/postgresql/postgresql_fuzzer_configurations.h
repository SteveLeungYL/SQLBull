//
// Created by XXX on 10/21/24.
//

#ifndef POSTGRESQL_FUZZER_CONFIGURATION_H
#define POSTGRESQL_FUZZER_CONFIGURATION_H

#include "../../headers/ir_types_common.h"
#include <vector>

namespace PostgreSQLFuzzerConfigurations {
constexpr int pct_mutate_ratio = 80; // percentage of chances that uses query sequence mutation.
constexpr int pct_insert_new_in_mut = 30; // percentage of chances that insert new randomly generated statements when mutating existing query sequence.

static const std::vector<IRTYPE> postgresql_interesting_ir_types = {
    IRTypeWindowDefinition,
    IRTypeOverClause,
    IRTypeLimitClause,
    IRTypeGroupByList,
    IRTypeCaseExpr,
    IRTypeJoinedTable,
    IRTypeFuncApplication,
};

}

#endif // POSTGRESQL_FUZZER_CONFIGURATION_H