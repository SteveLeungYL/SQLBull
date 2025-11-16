//
// Created by XXX on 10/21/24.
//

#ifndef SQLITE_FUZZER_CONFIGURATION_H
#define SQLITE_FUZZER_CONFIGURATION_H

#include "../../headers/ir_types_common.h"
#include <vector>

namespace SQLiteFuzzerConfigurations {
constexpr int pct_mutate_ratio = 80; // percentage of chances that uses query sequence mutation.
constexpr int pct_insert_new_in_mut = 70; // percentage of chances that insert new randomly generated statements when mutating existing query sequence.

static const std::vector<IRTYPE> sqlite_interesting_ir_types = {
    IRTypeWindow,
    IRTypeSortlist,
    IRTypeSeltablist,
    IRTypeGroupbyOpt,
    IRTypeCaseOperand,
    IRTypeMultiselectOp,
    IRTypeOneselect,
    IRTypeJoinop
};

}

#endif // SQLITE_FUZZER_CONFIGURATION_H