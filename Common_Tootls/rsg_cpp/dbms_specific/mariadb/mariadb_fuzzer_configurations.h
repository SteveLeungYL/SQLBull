//
// Created by XXX on 10/21/24.
//

#ifndef MYSQL_FUZZER_CONFIGURATION_H
#define MYSQL_FUZZER_CONFIGURATION_H

#include "../../headers/ir_types_common.h"
#include <vector>

namespace MariaDBFuzzerConfigurations {
constexpr int pct_mutate_ratio = 80; // percentage of chances that uses query sequence mutation.
constexpr int pct_insert_new_in_mut = 30; // percentage of chances that insert new randomly generated statements when mutating existing query sequence.

static const std::vector<IRTYPE> mariadb_interesting_ir_types = {
    // IRTypeWindowingClause,
    // IRTypeWindowNameOrSpec,
    // IRTypeOrderExpr,
    IRTypeLimitClause,
    IRTypeWindowDef,
    IRTypeAlterOrderItem,
    // IRTypeOrderList, // Handled by order_expr
    IRTypeGroupList,
    IRTypeCaseStmtSpecification,
    // IRTypeSearchedCaseStmt,
    IRTypeWhenList,
    IRTypeJoinTable,
    IRTypeSumExpr,
    IRTypeSubquery,
    IRTypeInSumExpr,
    IRTypeInSumExpr,
    IRTypeInSumExpr,
    IRTypeSumExpr,
    IRTypeSumExpr,
    IRTypeSumExpr,
};

}

#endif // MYSQL_FUZZER_CONFIGURATION_H