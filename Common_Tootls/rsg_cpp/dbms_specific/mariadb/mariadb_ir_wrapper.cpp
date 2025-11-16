//
// Created by XXX on 10/21/24.
//

#include "mariadb_ir_wrapper.h"
#include "../../headers/utils.h"


static const vector<IRTYPE> v_stmts_types {
    IRTypeAlter,
    IRTypeAnalyze,
    IRTypeAnalyzeStmtCommand,
    IRTypeBackup,
    IRTypeBinlogBase64Event,
    IRTypeCall,
    IRTypeChange,
    IRTypeCheck,
    IRTypeChecksum,
    IRTypeCommit,
    IRTypeCreate,
    IRTypeDeallocate,
    IRTypeDelete,
    IRTypeDescribe,
    IRTypeDo,
    IRTypeDrop,
    IRTypeExecute,
    IRTypeExplainForConnection,
    IRTypeFlush,
    IRTypeGetDiagnostics,
    IRTypeGrant,
    IRTypeHandler,
    IRTypeHelp,
    IRTypeInsert,
    IRTypeInstall,
    IRTypeKeepGccHappy,
    IRTypeKeycache,
    IRTypeKill,
    IRTypeLoad,
    IRTypeLock,
    IRTypeOptimize,
    IRTypeParseVcolExpr,
    IRTypePartitionEntry,
    IRTypePreload,
    IRTypePrepare,
    IRTypePurge,
    IRTypeRaiseStmtOracle,
    IRTypeRelease,
    IRTypeRename,
    IRTypeRepair,
    IRTypeReplace,
    IRTypeReset,
    IRTypeResignalStmt,
    IRTypeRevoke,
    IRTypeRollback,
    IRTypeSavepoint,
    IRTypeSelect,
    IRTypeSelectInto,
    IRTypeSet,
    IRTypeSignalStmt,
    IRTypeShow,
    IRTypeShutdown,
    IRTypeSlave,
    IRTypeStart,
    IRTypeTruncate,
    IRTypeUninstall,
    IRTypeUnlock,
    IRTypeUpdate,
    IRTypeUse,
    IRTypeXa
};


IRTYPE MariaDBIRWrapper::get_cur_stmt_type_from_sub_ir(IR* cur_ir)
{
    if (cur_ir == nullptr) {
        cerr << "Error: Getting nullptr from MySQLIRWrapper::get_cur_stmt_type_from_sub_ir;\n\n\n";
        abort();
    }
    while (cur_ir->get_parent_node() && cur_ir->get_parent_node()->get_ir_type() != IRTypeStatement) {
        cur_ir = cur_ir->get_parent_node();
    }

    if (cur_ir->get_parent_node() != nullptr) {
        return cur_ir->get_ir_type();
    } else {
        cerr << "Error: Cannot find the stmt type from the function "
                "MySQLIRWrapper::get_cur_stmt_type_from_sub_ir. \n\n\n";
        abort();
    }
}

bool MariaDBIRWrapper::is_in_subquery(IR* cur_stmt, IR* check_node, bool output_debug)
{
    if (is_ir_in(check_node, IRTypeSubquery)) {
        return true;
    } else {
        return false;
    }
}

bool MariaDBIRWrapper::is_ir_statement_typed(IRTYPE ir_type)
{
    bool is_matched = false;
    for (const auto& stmt_type : v_stmts_types) {
        if (ir_type == stmt_type) {
            is_matched = true;
            break;
        }
    }
    return is_matched;
}