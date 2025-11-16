//
// Created by XXX on 10/21/24.
//

#ifndef DUCKDB_IR_WRAPPER_H
#define DUCKDB_IR_WRAPPER_H

#include "../../headers/ir_wrapper.h"

class DuckDBIRWrapper : public IRWrapper {
    virtual IRTYPE get_cur_stmt_type_from_sub_ir(IR* cur_ir) override;
    virtual bool is_in_subquery(IR* cur_stmt, IR* check_node, bool output_debug = false) override;

    virtual bool is_ir_statement_typed(IRTYPE ir_type) override;
};

#endif // DUCKDB_IR_WRAPPER_H
