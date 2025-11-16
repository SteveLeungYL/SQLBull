//
// Created by XXX on 10/21/24.
//

#include "postgresql_ir_wrapper.h"
#include "../../headers/utils.h"


static const vector<IRTYPE> v_stmts_types {
    IRTypeAlterEventTrigStmt,
    IRTypeAlterCollationStmt,
    IRTypeAlterDatabaseStmt,
    IRTypeAlterDatabaseSetStmt,
    IRTypeAlterDefaultPrivilegesStmt,
    IRTypeAlterDomainStmt,
    IRTypeAlterEnumStmt,
    IRTypeAlterExtensionStmt,
    IRTypeAlterExtensionContentsStmt,
    IRTypeAlterFdwStmt,
    IRTypeAlterForeignServerStmt,
    IRTypeAlterFunctionStmt,
    IRTypeAlterGroupStmt,
    IRTypeAlterObjectDependsStmt,
    IRTypeAlterObjectSchemaStmt,
    IRTypeAlterOwnerStmt,
    IRTypeAlterOperatorStmt,
    IRTypeAlterTypeStmt,
    IRTypeAlterPolicyStmt,
    IRTypeAlterSeqStmt,
    IRTypeAlterSystemStmt,
    IRTypeAlterTableStmt,
    IRTypeAlterTblSpcStmt,
    IRTypeAlterCompositeTypeStmt,
    IRTypeAlterPublicationStmt,
    IRTypeAlterRoleSetStmt,
    IRTypeAlterRoleStmt,
    IRTypeAlterSubscriptionStmt,
    IRTypeAlterStatsStmt,
    IRTypeAlterTSConfigurationStmt,
    IRTypeAlterTSDictionaryStmt,
    IRTypeAlterUserMappingStmt,
    IRTypeAnalyzeStmt,
    IRTypeCallStmt,
    IRTypeCheckPointStmt,
    IRTypeClosePortalStmt,
    IRTypeClusterStmt,
    IRTypeCommentStmt,
    IRTypeConstraintsSetStmt,
    IRTypeCopyStmt,
    IRTypeCreateAmStmt,
    IRTypeCreateAsStmt,
    IRTypeCreateAssertionStmt,
    IRTypeCreateCastStmt,
    IRTypeCreateConversionStmt,
    IRTypeCreateDomainStmt,
    IRTypeCreateExtensionStmt,
    IRTypeCreateFdwStmt,
    IRTypeCreateForeignServerStmt,
    IRTypeCreateForeignTableStmt,
    IRTypeCreateFunctionStmt,
    IRTypeCreateGroupStmt,
    IRTypeCreateMatViewStmt,
    IRTypeCreateOpClassStmt,
    IRTypeCreateOpFamilyStmt,
    IRTypeCreatePublicationStmt,
    IRTypeAlterOpFamilyStmt,
    IRTypeCreatePolicyStmt,
    IRTypeCreatePLangStmt,
    IRTypeCreateSchemaStmt,
    IRTypeCreateSeqStmt,
    IRTypeCreateStmt,
    IRTypeCreateSubscriptionStmt,
    IRTypeCreateStatsStmt,
    IRTypeCreateTableSpaceStmt,
    IRTypeCreateTransformStmt,
    IRTypeCreateTrigStmt,
    IRTypeCreateEventTrigStmt,
    IRTypeCreateRoleStmt,
    IRTypeCreateUserStmt,
    IRTypeCreateUserMappingStmt,
    IRTypeCreatedbStmt,
    IRTypeDeallocateStmt,
    IRTypeDeclareCursorStmt,
    IRTypeDefineStmt,
    IRTypeDeleteStmt,
    IRTypeDiscardStmt,
    IRTypeDoStmt,
    IRTypeDropCastStmt,
    IRTypeDropOpClassStmt,
    IRTypeDropOpFamilyStmt,
    IRTypeDropOwnedStmt,
    IRTypeDropStmt,
    IRTypeDropSubscriptionStmt,
    IRTypeDropTableSpaceStmt,
    IRTypeDropTransformStmt,
    IRTypeDropRoleStmt,
    IRTypeDropUserMappingStmt,
    IRTypeDropdbStmt,
    IRTypeExecuteStmt,
    IRTypeExplainStmt,
    IRTypeFetchStmt,
    IRTypeGrantStmt,
    IRTypeGrantRoleStmt,
    IRTypeImportForeignSchemaStmt,
    IRTypeIndexStmt,
    IRTypeInsertStmt,
    IRTypeListenStmt,
    IRTypeRefreshMatViewStmt,
    IRTypeLoadStmt,
    IRTypeLockStmt,
    IRTypeMergeStmt,
    IRTypeNotifyStmt,
    IRTypePrepareStmt,
    IRTypeReassignOwnedStmt,
    IRTypeReindexStmt,
    IRTypeRemoveAggrStmt,
    IRTypeRemoveFuncStmt,
    IRTypeRemoveOperStmt,
    IRTypeRenameStmt,
    IRTypeRevokeStmt,
    IRTypeRevokeRoleStmt,
    IRTypeRuleStmt,
    IRTypeSecLabelStmt,
    IRTypeSelectStmt,
    IRTypeTransactionStmt,
    IRTypeTruncateStmt,
    IRTypeUnlistenStmt,
    IRTypeUpdateStmt,
    IRTypeVacuumStmt,
    IRTypeVariableResetStmt,
    IRTypeVariableSetStmt,
    IRTypeVariableShowStmt,
    IRTypeViewStmt
};


IRTYPE PostgreSQLIRWrapper::get_cur_stmt_type_from_sub_ir(IR* cur_ir)
{
    if (cur_ir == nullptr) {
        cerr << "Error: Getting nullptr from MySQLIRWrapper::get_cur_stmt_type_from_sub_ir;\n\n\n";
        abort();
    }
    while (cur_ir->get_parent_node() && cur_ir->get_parent_node()->get_ir_type() != IRTypeStmt || cur_ir->get_parent_node()->get_ir_type() != IRTypeTransactionStmtLegacy || cur_ir->get_parent_node()->get_ir_type() != IRTypeStmtmulti) {
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

bool PostgreSQLIRWrapper::is_in_subquery(IR* cur_stmt, IR* check_node, bool output_debug)
{
    if (check_node->get_ir_type() == IRTypeSelectWithParens && check_node->get_parent_node() != nullptr && 
        !(
            check_node->get_parent_node()->get_ir_type() == IRTypeSelectWithParens ||
            check_node->get_parent_node()->get_ir_type() == IRTypeSelectStmt
        )
    ) {
        return true;
    } else if (check_node->get_ir_type() == IRTypeSelectStmt && check_node->get_parent_node() != nullptr && 
        !(
            check_node->get_parent_node()->get_ir_type() == IRTypeExplainStmt ||
            check_node->get_parent_node()->get_ir_type() == IRTypePrepareStmt ||
            check_node->get_parent_node()->get_ir_type() == IRTypeStmt
        )
    ) {
        return true;
    } else {
        return false;
    }
}

bool PostgreSQLIRWrapper::is_ir_statement_typed(IRTYPE ir_type)
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