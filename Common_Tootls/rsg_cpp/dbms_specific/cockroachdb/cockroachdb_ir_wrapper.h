//
// Created by XXX on 3/18/24.
//

#ifndef RSG_CPP_COCKROACHDB_IR_WRAPPER_H
#define RSG_CPP_COCKROACHDB_IR_WRAPPER_H

#include "../../headers/ir_wrapper.h"

class CockroachDBIRWrapper : public IRWrapper {
    virtual IRTYPE get_cur_stmt_type_from_sub_ir(IR* cur_ir) override;
    virtual bool is_in_subquery(IR* cur_stmt, IR* check_node, bool output_debug = false) override;

    virtual bool is_ir_statement_typed(IRTYPE ir_type) override;
};

#endif // RSG_CPP_COCKROACHDB_IR_WRAPPER_H
