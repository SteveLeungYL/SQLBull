//
// Created by XXX on 10/21/24.
//

#ifndef SQLITE_IR_WRAPPER_H
#define SQLITE_IR_WRAPPER_H

#include "../../headers/ir_wrapper.h"

class SQLiteIRWrapper : public IRWrapper {
    virtual IRTYPE get_cur_stmt_type_from_sub_ir(IR* cur_ir) override;
    virtual bool is_in_subquery(IR* cur_stmt, IR* check_node, bool output_debug = false) override;

    virtual bool is_ir_statement_typed(IRTYPE ir_type) override;
};

#endif // SQLITE_IR_WRAPPER_H
