#ifndef COCKROACHDB_FUZZER_CONFIGURATIONS_H
#define COCKROACHDB_FUZZER_CONFIGURATIONS_H

#include "../../headers/ir_types_common.h"
#include <vector>

namespace CockroachDBFuzzerConfigurations {
constexpr int pct_mutate_ratio = 70; // percentage of chances that uses query sequence mutation.
constexpr int pct_insert_new_in_mut = 20; // percentage of chances that uses query sequence mutation.

static const std::vector<IRTYPE> cockroachdb_interesting_ir_types = {
    IRTypeWindowClause,
    IRTypeSortClause,
    IRTypeJoinedTable,
    IRTypeGroupClause,
    IRTypeOverClause,
    IRTypeLimitClause,
    IRTypeCaseExpr,
    IRTypeWhenClause,
    IRTypeSelectStmt,
    // Repeat again for the second time.
    IRTypeWindowClause,
    IRTypeSortClause,
    IRTypeJoinedTable,
    IRTypeGroupClause,
    IRTypeOverClause,
    IRTypeLimitClause,
    IRTypeCaseExpr,
    IRTypeWhenClause,
    IRTypeSelectStmt,
};

}

#endif // COCKROACHDB_FUZZER_CONFIGURATIONS_H