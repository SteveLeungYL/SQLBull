//
// Created by XXX on 10/21/24.
//

#include "mysql_ir_wrapper.h"
#include "../../headers/utils.h"


static const vector<IRTYPE> v_stmts_types {
    IRTypeAlterDatabaseStmt,
    IRTypeAlterEventStmt,
    IRTypeAlterFunctionStmt,
    IRTypeAlterInstanceStmt,
    IRTypeAlterLogfileStmt,
    IRTypeAlterProcedureStmt,
    IRTypeAlterResourceGroupStmt,
    IRTypeAlterServerStmt,
    IRTypeAlterTablespaceStmt,
    IRTypeAlterUndoTablespaceStmt,
    IRTypeAlterTableStmt,
    IRTypeAlterUserStmt,
    IRTypeAlterViewStmt,
    IRTypeAnalyzeTableStmt,
    IRTypeBinlogBase64Event,
    IRTypeCallStmt,
    IRTypeChange,
    IRTypeCheckTableStmt,
    IRTypeChecksum,
    IRTypeCloneStmt,
    IRTypeCommit,
    IRTypeCreate,
    IRTypeCreateIndexStmt,
    IRTypeCreateResourceGroupStmt,
    IRTypeCreateRoleStmt,
    IRTypeCreateSrsStmt,
    IRTypeCreateTableStmt,
    IRTypeDeallocate,
    IRTypeDeleteStmt,
    IRTypeDescribeStmt,
    IRTypeDoStmt,
    IRTypeDropDatabaseStmt,
    IRTypeDropEventStmt,
    IRTypeDropFunctionStmt,
    IRTypeDropIndexStmt,
    IRTypeDropLogfileStmt,
    IRTypeDropProcedureStmt,
    IRTypeDropResourceGroupStmt,
    IRTypeDropRoleStmt,
    IRTypeDropServerStmt,
    IRTypeDropSrsStmt,
    IRTypeDropTablespaceStmt,
    IRTypeDropUndoTablespaceStmt,
    IRTypeDropTableStmt,
    IRTypeDropTriggerStmt,
    IRTypeDropUserStmt,
    IRTypeDropViewStmt,
    IRTypeExecute,
    IRTypeExplainStmt,
    IRTypeFlush,
    IRTypeGetDiagnostics,
    IRTypeGroupReplication,
    IRTypeGrant,
    IRTypeHandlerStmt,
    IRTypeHelp,
    IRTypeImportStmt,
    IRTypeInsertStmt,
    IRTypeInstallStmt,
    IRTypeKill,
    IRTypeLoadStmt,
    IRTypeLock,
    IRTypeOptimizeTableStmt,
    IRTypeKeycacheStmt,
    IRTypePreloadStmt,
    IRTypePrepare,
    IRTypePurge,
    IRTypeRelease,
    IRTypeRename,
    IRTypeRepairTableStmt,
    IRTypeReplaceStmt,
    IRTypeReset,
    IRTypeResignalStmt,
    IRTypeRestartServerStmt,
    IRTypeRevoke,
    IRTypeRollback,
    IRTypeSavepoint,
    IRTypeSelectStmt,
    IRTypeSet,
    IRTypeSetResourceGroupStmt,
    IRTypeSetRoleStmt,
    IRTypeShowBinaryLogsStmt,
    IRTypeShowBinlogEventsStmt,
    IRTypeShowCharacterSetStmt,
    IRTypeShowCollationStmt,
    IRTypeShowColumnsStmt,
    IRTypeShowCountErrorsStmt,
    IRTypeShowCountWarningsStmt,
    IRTypeShowCreateDatabaseStmt,
    IRTypeShowCreateEventStmt,
    IRTypeShowCreateFunctionStmt,
    IRTypeShowCreateProcedureStmt,
    IRTypeShowCreateTableStmt,
    IRTypeShowCreateTriggerStmt,
    IRTypeShowCreateUserStmt,
    IRTypeShowCreateViewStmt,
    IRTypeShowDatabasesStmt,
    IRTypeShowEngineLogsStmt,
    IRTypeShowEngineMutexStmt,
    IRTypeShowEngineStatusStmt,
    IRTypeShowEnginesStmt,
    IRTypeShowErrorsStmt,
    IRTypeShowEventsStmt,
    IRTypeShowFunctionCodeStmt,
    IRTypeShowFunctionStatusStmt,
    IRTypeShowGrantsStmt,
    IRTypeShowKeysStmt,
    IRTypeShowMasterStatusStmt,
    IRTypeShowOpenTablesStmt,
    IRTypeShowPluginsStmt,
    IRTypeShowPrivilegesStmt,
    IRTypeShowProcedureCodeStmt,
    IRTypeShowProcedureStatusStmt,
    IRTypeShowProcesslistStmt,
    IRTypeShowProfileStmt,
    IRTypeShowProfilesStmt,
    IRTypeShowRelaylogEventsStmt,
    IRTypeShowReplicaStatusStmt,
    IRTypeShowReplicasStmt,
    IRTypeShowStatusStmt,
    IRTypeShowTableStatusStmt,
    IRTypeShowTablesStmt,
    IRTypeShowTriggersStmt,
    IRTypeShowVariablesStmt,
    IRTypeShowWarningsStmt,
    IRTypeShutdownStmt,
    IRTypeSignalStmt,
    IRTypeStart,
    IRTypeStartReplicaStmt,
    IRTypeStopReplicaStmt,
    IRTypeTruncateStmt,
    IRTypeUninstall,
    IRTypeUnlock,
    IRTypeUpdateStmt,
    IRTypeUse,
    IRTypeXa
};


IRTYPE MySQLIRWrapper::get_cur_stmt_type_from_sub_ir(IR* cur_ir)
{
    if (cur_ir == nullptr) {
        cerr << "Error: Getting nullptr from MySQLIRWrapper::get_cur_stmt_type_from_sub_ir;\n\n\n";
        abort();
    }
    while (cur_ir->get_parent_node() && cur_ir->get_parent_node()->get_ir_type() != IRTypeSimpleStatement || cur_ir->get_parent_node()->get_ir_type() != IRTypeSimpleStatementOrBegin || cur_ir->get_parent_node()->get_ir_type() != IRTypeOptEndOfInput || cur_ir->get_parent_node()->get_ir_type() != IRTypeSqlStatement) {
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

bool MySQLIRWrapper::is_in_subquery(IR* cur_stmt, IR* check_node, bool output_debug)
{
    if (is_ir_in(check_node, IRTypeSubquery)) {
        return true;
    } else {
        return false;
    }
}

bool MySQLIRWrapper::is_ir_statement_typed(IRTYPE ir_type)
{
    // TODO::FIXME:: Yu, Myself. More efficient implementation please...
    // string ir_type_str = get_string_by_ir_type(ir_type);

    if (ir_type == IRTypeSimpleCaseStmt) {
        return false;
    }

    bool is_matched = false;
    for (const auto& stmt_type : v_stmts_types) {
        if (ir_type == stmt_type) {
            is_matched = true;
            break;
        }
    }
    return is_matched;
}