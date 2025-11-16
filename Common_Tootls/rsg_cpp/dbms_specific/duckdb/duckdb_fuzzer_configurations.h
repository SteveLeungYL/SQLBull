//
// Created by XXX on 10/21/24.
//

#ifndef DUCKDB_FUZZER_CONFIGURATION_H
#define DUCKDB_FUZZER_CONFIGURATION_H

#include "../../headers/ir_types_common.h"
#include <vector>

namespace DuckDBFuzzerConfigurations {
constexpr int pct_mutate_ratio = 70; // percentage of chances that uses query sequence mutation.
constexpr int pct_insert_new_in_mut = 20; // percentage of chances that uses query sequence mutation.

static const std::vector<IRTYPE> duckdb_interesting_ir_types = {
    IRTypeWindowClause,
    IRTypeSortClause,
    IRTypeJoinedTable,
    IRTypeGroupClause,
    IRTypeWindowSpecification,
    IRTypeCaseExpr,
    IRTypeSelectStmt,
    IRTypeSelectWithParens,
    IRTypeSelectNoParens
};

}

#endif // DUCKDB_FUZZER_CONFIGURATION_H
