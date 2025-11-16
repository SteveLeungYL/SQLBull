//
// Created by XXX on 10/21/24.
//

#include "sqlite_ir_wrapper.h"

IRTYPE SQLiteIRWrapper::get_cur_stmt_type_from_sub_ir(IR* cur_ir)
{
    if (cur_ir == nullptr) {
        cerr << "Error: Getting nullptr from SQLiteIRWrapper::get_cur_stmt_type_from_sub_ir;\n\n\n";
        abort();
    }
    while (cur_ir->get_parent_node() && cur_ir->get_parent_node()->get_ir_type() != IRTypeCmd 
        && cur_ir->get_parent_node()->get_ir_type() != IRTypeEcmd
        && cur_ir->get_parent_node()->get_ir_type() != IRTypeSelect 
        && cur_ir->get_parent_node()->get_ir_type() != IRTypeSelectStmt
        ) {
        cur_ir = cur_ir->get_parent_node();
    }

    if (cur_ir->get_parent_node() != nullptr) {
        return cur_ir->get_ir_type();
    } else {
        cerr << "Error: Cannot find the stmt type from the function "
                "SQLiteIRWrapper::get_cur_stmt_type_from_sub_ir. \n\n\n";
        abort();
    }
}

bool SQLiteIRWrapper::is_in_subquery(IR* cur_stmt, IR* check_node, bool output_debug)
{
    IR* cur_iter = check_node;
    while (1) {
        if (cur_iter == NULL) { // Iter to the parent node. This is Not a subquery.
            return false;
        } else if (cur_iter == cur_stmt) { // Iter to the cur_stmt node already.
            // Not in a  subquery.
            return false;
        } else if (cur_iter->get_ir_type() == IRTypeCmd || cur_iter->get_ir_type() == IRTypeEcmd) { // Iter to the parent stmt node. This is Not a subquery.
            return false;
        } else if (cur_iter->get_ir_type() == IRTypeSelect || cur_iter->get_ir_type() == IRTypeSelectStmt) {
            if (cur_iter->get_parent_node() != NULL && cur_iter->get_parent_node()->get_ir_type() != IRTypeCmd) {
                return true; // In a subquery.
            } else {
                return false;
            }
        }
        cur_iter = cur_iter->get_parent_node(); // Assuming cur_iter->get_parent() will always get
        // to kStatementList. Otherwise, it would be error.
        continue;
    }
}

bool SQLiteIRWrapper::is_ir_statement_typed(IRTYPE ir_type)
{
    // TODO::FIXME:: Yu, Myself. More efficient implementation please...
    if (ir_type == IRTypeCmd || ir_type == IRTypeEcmd || ir_type == IRTypeCmdx || ir_type == IRTypeSelect || ir_type == IRTypeSelectStmt) {
        return true;
    } else {
        return false;
    }
}