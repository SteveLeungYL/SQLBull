//
// Created by XXX on 3/18/24.
//

#include "cockroachdb_ir_wrapper.h"

IRTYPE CockroachDBIRWrapper::get_cur_stmt_type_from_sub_ir(IR* cur_ir)
{
    if (cur_ir == nullptr) {
        cerr << "Error: Getting nullptr from CockroachDBIRWrapper::get_cur_stmt_type_from_sub_ir;\n\n\n";
        abort();
    }
    while (cur_ir->get_parent_node() && cur_ir->get_parent_node()->get_ir_type() != IRTypeStmt
        && cur_ir->get_parent_node()->get_ir_type() != IRTypeStmtWithoutLegacyTransaction
        && cur_ir->get_parent_node()->get_ir_type() != IRTypeExplainableStmt
        && cur_ir->get_parent_node()->get_ir_type() != IRTypePreparableStmt) {
        cur_ir = cur_ir->get_parent_node();
    }

    if (cur_ir->get_parent_node() != nullptr) {
        return cur_ir->get_ir_type();
    } else {
        cerr << "Error: Cannot find the stmt type from the function "
                "CockroachDBIRWrapper::get_cur_stmt_type_from_sub_ir. \n\n\n";
        abort();
    }
}

bool CockroachDBIRWrapper::is_in_subquery(IR* cur_stmt, IR* check_node, bool output_debug)
{
    IR* cur_iter = check_node;
    while (1) {
        if (cur_iter == NULL) { // Iter to the parent node. This is Not a subquery.
            return false;
        } else if (cur_iter == cur_stmt) { // Iter to the cur_stmt node already.
            // Not in a  subquery.
            return false;
        } else if (cur_iter->get_ir_type() == IRTypeStmt) { // Iter to the parent stmt node. This is Not a subquery.
            return false;
        } else if (cur_iter->get_ir_type() == IRTypeSelectStmt && cur_iter->get_parent_node() != NULL && cur_iter->get_parent_node()->get_ir_type() != IRTypePreparableStmt) {
            return true; // In a subquery.
        } else if (cur_iter->get_ir_type() == IRTypeSelectNoParens && cur_iter->get_parent_node() != NULL && cur_iter->get_parent_node()->get_ir_type() != IRTypeSelectStmt) {
            // doesn't seem to be possible
            return true; // In a subquery.
        } else if (cur_iter->get_ir_type() == IRTypeSelectWithParens && cur_iter->get_parent_node() != NULL && cur_iter->get_parent_node()->get_ir_type() != IRTypeSelectStmt && cur_iter->get_parent_node()->get_ir_type() != IRTypeSelectClause) {
            return true; // In a subquery.
        }
        cur_iter = cur_iter->get_parent_node(); // Assuming cur_iter->get_parent() will always get
        // to kStatementList. Otherwise, it would be error.
        continue;
    }
}

bool CockroachDBIRWrapper::is_ir_statement_typed(IRTYPE ir_type)
{
    // switch (ir_type) {
    //     case (IRTypeStmt):
    //     case (IRTypeAlterStmt):
    //     case (IRTypeAlterChangefeedStmt):
    //     case (IRTypeAlterBackupStmt):
    //     case (IRTypeAlterDdlStmt):
    //     case (IRTypeAlterTableStmt):
    //     case (IRTypeAlterIndexStmt):
    //     case (IRTypeAlterViewStmt):
    //     case (IRTypeAlterSequenceStmt):
    //     case (IRTypeAlterDatabaseStmt):
    //     case (IRTypeAlterRangeStmt):
    //     case (IRTypeAlterPartitionStmt):
    //     case (IRTypeAlter):
    // }
    // TODO::FIXME:: Yu, Myself. More efficient implementation please...
    if (const string ir_str = get_string_by_ir_type(ir_type);
        ir_str.size() > 4 && (ir_str.substr(ir_str.size() - 4, 4) == "Stmt")) {
        return true;
    } else {
        return false;
    }
}