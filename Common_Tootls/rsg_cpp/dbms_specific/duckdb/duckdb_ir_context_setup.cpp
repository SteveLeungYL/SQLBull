//
// Created by XXX on 10/17/24.
//

#include "duckdb_ir_context_setup.h"
#include "duckdb_fuzzer_configurations.h"

#define BEGIN vector<IR*> children = cur_ir->get_children();

inline void handle_relation_expr(IR*& cur_ir, DATAFLAG data_flag = ContextUse)
{
    BEGIN;

    for (int idx = 0; idx < children.size(); idx++) {
        IR* cur_child = children[idx];
        if (cur_child->get_ir_type() == IRTypeQualifiedName) {
            IR* new_name = new IR(IRTypeIDENT, "v00", DataTableName, data_flag, children[idx]->get_mapped_token_node());
            cur_ir->swap_one_child(children[idx], new_name);
            break;
        }
    }
}

inline void handle_relation_expr_opt_alias(IR*& cur_ir, DATAFLAG data_flag = ContextDefine)
{
    BEGIN;

    for (int idx = 0; idx < children.size(); idx++) {
        if (children[idx]->get_ir_type() == IRTypeColId) {
            IR* new_name = new IR(IRTypeIDENT, "a00", DataTableAliasName, ContextDefine, children[idx]->get_mapped_token_node());
            cur_ir->swap_one_child(children[idx], new_name);
        } else if (children[idx]->get_ir_type() == IRTypeRelationExpr) {
            handle_relation_expr(children[idx], data_flag);
        }
    }
}

inline void handle_alter_table_stmt(IR*& cur_ir)
{
    BEGIN;

    DATATYPE data_type = DataTableName;
    if (children[1]->get_ir_type() == IRTypeINDEX) {
        data_type = DataIndexName;
    } else if (children[1]->get_ir_type() == IRTypeSEQUENCE) {
        data_type = DataSequenceName;
    } else if (children[1]->get_ir_type() == IRTypeVIEW) {
        data_type = DataViewName;
    } else {
        // skip
    }

    for (int idx = 0; idx < children.size(); idx++) {
        if (children[idx]->get_ir_type() == IRTypeQualifiedName) {
            IR* new_name = new IR(IRTypeIDENT, "x00", data_type, ContextUse, children[idx]->get_mapped_token_node());
            cur_ir->swap_one_child(children[idx], new_name);
            break;
        }
    }
}

inline void handle_column_def(IR*& cur_ir, DATAFLAG data_flag = ContextDefine)
{
    BEGIN;

    IR* new_name = new IR(IRTypeIDENT, "c01", DataColumnName, data_flag, children.front()->get_mapped_token_node());
    cur_ir->swap_one_child(children.front(), new_name);
}

inline void handle_table_constraint(IR*& cur_ir, DATAFLAG data_flag = ContextUse)
{
    BEGIN;

    if (children.size() == 3 && children[1]->get_ir_type() == IRTypeName) {
        IR* new_name = new IR(IRTypeIDENT, "constraint_name_0", DataConstraintName, data_flag, children[1]->get_mapped_token_node());
        cur_ir->swap_one_child(children[1], new_name);
    }
}

inline void handle_alter_table_cmd(IR*& cur_ir)
{
    BEGIN;

    for (int idx = 0; idx < children.size(); idx++) {
        if (children[idx]->get_ir_type() == IRTypeColId) {
            IR* new_name = new IR(IRTypeIDENT, "c01", DataColumnName, ContextUse, children[idx]->get_mapped_token_node());
            cur_ir->swap_one_child(children[idx], new_name);
            break;
        }
    }

    if (children.back()->get_ir_type() == IRTypeColId) {
        if (children.size() > 5 && children[4]->get_ir_type() == IRTypeSTORAGE) {
            IR* new_name = new IR(IRTypeIDENT, "storage_name_0", DataStorageName, ContextUse, children[4]->get_mapped_token_node());
            cur_ir->swap_one_child(children.back(), new_name);
        }
    }

    if (children.size() > 2 && children[1]->get_ir_type() == IRTypeCONSTRAINT) {
        DATAFLAG data_flag = ContextUse;
        if (children.front()->get_ir_type() == IRTypeDROP) {
            data_flag = ContextUndefine;
        }
        for (int idx = 0; idx < children.size(); idx++) {
            if (children[idx]->get_ir_type() == IRTypeName) {
                IR* new_name = new IR(IRTypeIDENT, "constraint_name_0", DataConstraintName, data_flag, children[idx]->get_mapped_token_node());
                cur_ir->swap_one_child(children[idx], new_name);
                break;
            }
        }
    }

    if (children.size() == 2 && children[0]->get_ir_type() == IRTypeCONSTRAINT) {
        if (children[0]->get_ir_type() == IRTypeADDP)
            handle_table_constraint(children[1], ContextDefine);
    }
}

inline void handle_generic_option_elem(IR*& cur_ir, DATAFLAG data_flag = ContextDefine)
{
    BEGIN;

    IR* new_name = new IR(IRTypeIDENT, "generic_option_0", DataGenericOptionName, data_flag, children.front()->get_mapped_token_node());
    cur_ir->swap_one_child(children.front(), new_name);

    new_name = new IR(IRTypeIDENT, "generic_option_args_0", DataGenericOptionArgs, data_flag, children.back()->get_mapped_token_node());
    cur_ir->swap_one_child(children.back(), new_name);
}

inline void handle_alter_generic_option_elem(IR*& cur_ir)
{
    BEGIN;

    DATAFLAG data_flag = ContextDefine;
    if (children.size() == 2) {
        if (children.front()->get_ir_type() == IRTypeADDP || children.front()->get_ir_type() == IRTypeSET) {
            // pass
        } else {
            if (children.front()->get_ir_type() == IRTypeDROP) {
                data_flag = ContextUndefine;
                IR* new_name = new IR(IRTypeIDENT, "generic_option_0", DataGenericOptionName, ContextUndefine, children.back()->get_mapped_token_node());
                cur_ir->swap_one_child(children.back(), new_name);
                return;
            }
        }
    }

    handle_generic_option_elem(children.back(), data_flag);
}

inline void handle_deallocate_stmt(IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeName) {
            IR* new_name = new IR(IRTypeIDENT, "parpared_stmt_0", DataStatementPreparedName, ContextUndefine, child->get_mapped_token_node());
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_qualified_name(IR*& cur_ir, DATATYPE data_type, DATAFLAG data_flag)
{

    BEGIN;

    IR* new_name = new IR(IRTypeIDENT, "v00", data_type, data_flag, children.front()->get_mapped_token_node());
    cur_ir->swap_one_child(children.front(), new_name);

    /* Experimental, remove the indirection */
    if (children.size() == 2) {
        auto node_to_rov = children.back();
        cur_ir->detach_one_child(children.back());
        node_to_rov->deep_drop();
    }
}

inline void handle_col_id(IR*& cur_ir, DATATYPE data_type, DATAFLAG data_flag)
{

    BEGIN;

    IR* new_name = new IR(IRTypeIDENT, "v00", data_type, data_flag, children.front()->get_mapped_token_node());
    cur_ir->free_children();
    cur_ir->add_one_child(new_name, 0);
}

inline void handle_col_id_or_string(IR*& cur_ir, DATATYPE data_type, DATAFLAG data_flag)
{

    BEGIN;

    if (children.front()->get_ir_type() == IRTypeColId) {
        IR* new_name = new IR(IRTypeIDENT, "v00", data_type, data_flag, children.front()->get_mapped_token_node());
        cur_ir->free_children();
        cur_ir->add_one_child(new_name, 0);
    } else {
        // SCONST, don't care?
    }
}

inline void handle_col_label(IR*& cur_ir, DATATYPE data_type, DATAFLAG data_flag)
{

    BEGIN;

    IR* new_name = new IR(IRTypeIDENT, "v00", data_type, data_flag, children.front()->get_mapped_token_node());
    cur_ir->free_children();
    cur_ir->add_one_child(new_name, 0);
}

inline void handle_rename_stmt(IR*& cur_ir)
{

    BEGIN;

    DATATYPE data_type = DataTableName;

    if (children[1]->get_ir_type() == IRTypeSCHEMA) {
        data_type = DataSchemaName;
    } else if (children[1]->get_ir_type() == IRTypeTABLE) {
        for (auto& child : children) {
            if (child->get_ir_type() == IRTypeOptColumn) {
                data_type = DataColumnName;
                break;
            } else if (child->get_ir_type() == IRTypeCONSTRAINT) {
                data_type = DataConstraintName;
                break;
            }
        }
        // otherwise, just DataTableName.
    } else if (children[1]->get_ir_type() == IRTypeSEQUENCE) {
        data_type = DataSequenceName;
    } else if (children[1]->get_ir_type() == IRTypeVIEW) {
        data_type = DataSequenceName;
    } else if (children[1]->get_ir_type() == IRTypeINDEX) {
        data_type = DataIndexName;
    }

    unsigned int counted_name = 0;
    for (auto& child : children) {
        IRTYPE tmp_type = child->get_ir_type();
        if (tmp_type == IRTypeRelationExpr || tmp_type == IRTypeName || tmp_type == IRTypeQualifiedName) {
            counted_name++;
        }
    }

    DATAFLAG flag_array[2] = { ContextReplaceUndefine, ContextReplaceDefine };
    if (data_type != DataTableName && data_type != DataColumnName) {
        // For nontable and non-column, there is no handling for replace*.
        flag_array[0] = ContextUndefine;
        flag_array[1] = ContextDefine;
    }
    unsigned flag_idx = 0;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeRelationExpr) {
            // If counted_name == 3, the first relationExpr is for context, not for rename.
            if (counted_name == 3) {
                handle_relation_expr(child, ContextUse);
            } else {
                handle_relation_expr(child, flag_array[flag_idx]);
                flag_idx++;
            }
        } else if (child->get_ir_type() == IRTypeQualifiedName) {
            handle_qualified_name(child, data_type, flag_array[flag_idx]);
            flag_idx++;
        } else if (child->get_ir_type() == IRTypeName) {
            IR* new_name = new IR(IRTypeIDENT, "v00", data_type, flag_array[flag_idx], child->get_mapped_token_node());
            cur_ir->swap_one_child(child, new_name);
            flag_idx++;
        }
    }
}

inline void handle_insert_stmt(IR*& cur_ir)
{
    // Nothing we need to do.
}

inline void handle_insert_target(IR*& cur_ir)
{
    BEGIN;

    IR* new_name = new IR(IRTypeIDENT, "v00", DataTableName, ContextUse, children.front()->get_mapped_token_node());
    cur_ir->swap_one_child(children.front(), new_name);

    if (children.size() == 3) {
        IR* new_alias_name = new IR(IRTypeIDENT, "a03", DataTableAliasName, ContextDefine, children.back()->get_mapped_token_node());
        cur_ir->swap_one_child(children.back(), new_alias_name);
    }
}

inline void handle_index_elem(IR*& cur_ir)
{
    BEGIN;

    if (children.front()->get_ir_type() == IRTypeColId) {
        IR* new_name = new IR(IRTypeIDENT, "c01", DataColumnName, ContextUse, children.front()->get_mapped_token_node());
        cur_ir->swap_one_child(children.front(), new_name);
    }
}

inline void handle_opt_conf_expr(IR*& cur_ir)
{
    BEGIN;

    // index_params is handled in its own rule.

    if (!children.empty() && children.back()->get_ir_type() == IRTypeName) {
        IR* new_name = new IR(IRTypeIDENT, "constraint_0", DataConstraintName, ContextUse, children.back()->get_mapped_token_node());
        cur_ir->swap_one_child(children.back(), new_name);
    }
}

inline void handle_insert_column_item(IR*& cur_ir)
{
    BEGIN;

    if (children.front()->get_ir_type() == IRTypeColId) {
        IR* new_name = new IR(IRTypeIDENT, "c01", DataColumnName, ContextUse, children.front()->get_mapped_token_node());
        cur_ir->swap_one_child(children.front(), new_name);
    }
}

inline void handle_set_target(IR*& cur_ir)
{
    BEGIN;

    if (children.front()->get_ir_type() == IRTypeColId) {
        IR* new_name = new IR(IRTypeIDENT, "c01", DataColumnName, ContextUse, children.front()->get_mapped_token_node());
        cur_ir->swap_one_child(children.front(), new_name);
    }
}

inline void handle_target_el(IR*& cur_ir, DATATYPE data_type = DataColumnName, DATAFLAG data_flag = ContextDefine)
{
    BEGIN;

    if (children.back()->get_ir_type() == IRTypeColLabelOrString || children.back()->get_ir_type() == IRTypeIDENT) {
        IR* new_name = new IR(IRTypeIDENT, "column_alias_1", data_type, data_flag, children.back()->get_mapped_token_node());
        cur_ir->swap_one_child(children.back(), new_name);
    }
}

inline void handle_opt_collate(IR*& cur_ir)
{
    BEGIN;

    if (!children.empty()) {
        IR* new_name = new IR(IRTypeIDENT, "collate_0", DataCollationName, ContextUse, children.back()->get_mapped_token_node());
        cur_ir->swap_one_child(children.back(), new_name);
    }
}

inline void handle_opt_class(IR*& cur_ir)
{
    BEGIN;

    if (!children.empty()) {
        IR* new_name = new IR(IRTypeIDENT, "class_name_0", DataClassName, ContextUse, children.back()->get_mapped_token_node());
        cur_ir->swap_one_child(children.back(), new_name);
    }
}

inline void handle_create_type_stmt(IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeQualifiedName) {
            IR* new_name = new IR(IRTypeIDENT, "STRING", DataTypeName, ContextDefine, child->get_mapped_token_node());
            cur_ir->swap_one_child(child, new_name);
            break;
        }
    }

    if (children.back()->get_ir_type() == IRTypeTypename) {

        IR* new_name = new IR(IRTypeIDENT, "INT", DataTypeName, ContextUse, children.back()->get_mapped_token_node());
        cur_ir->swap_one_child(children.back(), new_name);
    }
}

inline void handle_pragma_stmt(IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeColId) {
            IR* new_name = new IR(IRTypeIDENT, "pragma_0", DataPragmaName, ContextUse, child->get_mapped_token_node());
            cur_ir->swap_one_child(child, new_name);
            break;
        }
    }
}

inline void handle_create_seq_stmt(IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeQualifiedName) {
            IR* new_name = new IR(IRTypeIDENT, "sequance_name_0", DataSequenceName, ContextDefine, child->get_mapped_token_node());
            cur_ir->swap_one_child(child, new_name);
            break;
        }
    }
}

inline void handle_opt_secret_name(IR*& cur_ir, DATAFLAG data_flag)
{
    BEGIN;

    if (!children.empty()) {
        IR* new_name = new IR(IRTypeIDENT, "secret_name_0", DataSecretName, data_flag, children.front()->get_mapped_token_node());
        cur_ir->swap_one_child(children.front(), new_name);
    }
}

inline void handle_create_secret_stmt(IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeOptSecretName) {
            handle_opt_secret_name(child, ContextDefine);
        }
    }
}

inline void handle_opt_storage_specifier(IR*& cur_ir)
{
    BEGIN;

    if (!children.empty()) {
        IR* new_name = new IR(IRTypeIDENT, "storage_name_0", DataStorageName, ContextUse, children.back()->get_mapped_token_node());
        cur_ir->swap_one_child(children.back(), new_name);
    }
}

inline void handle_column_elem(IR*& cur_ir, DATAFLAG data_flag = ContextUse)
{
    BEGIN;

    IR* new_name = new IR(IRTypeIDENT, "c01", DataColumnName, data_flag, children.front()->get_mapped_token_node());
    cur_ir->swap_one_child(children.front(), new_name);
}

inline void handle_column_list(IR*& cur_ir, DATAFLAG data_flag = ContextUse)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeColumnElem) {
            handle_column_elem(child, data_flag);
        } else if (child->get_ir_type() == IRTypeColumnList) {
            handle_column_list(child, data_flag);
        }
    }
}

inline void handle_opt_column_list(IR*& cur_ir, DATAFLAG data_flag = ContextUse)
{
    BEGIN;

    if (!children.empty()) {
        handle_column_list(children.front(), data_flag);
    }
}

inline void handle_update_extensions_stmt(IR*& cur_ir)
{
    // the opt_column_list is handled by itself.
    // empty
}

inline void handle_execute_stmt(IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeName) {
            IR* new_name = new IR(IRTypeIDENT, "v00", DataStatementPreparedName, ContextUse, child->get_mapped_token_node());
            cur_ir->swap_one_child(child, new_name);
            break;
        }
    }
}

inline void handle_create_as_target(IR*& cur_ir)
{
    BEGIN;

    // ONLY ONE RULE

    assert(children.size() == 4);

    if (children[0]->get_ir_type() == IRTypeQualifiedName) {
        IR* new_name = new IR(IRTypeIDENT, "v00", DataTableName, ContextDefine, children[0]->get_mapped_token_node());
        cur_ir->swap_one_child(children[0], new_name);
    }

    if (children[1]->get_ir_type() == IRTypeOptColumnList) {
        handle_opt_column_list(children[1], ContextDefine);
    }
}

inline void handle_execute_param_expr(IR*& cur_ir)
{
    BEGIN;

    if (children[0]->get_ir_type() == IRTypeParamName) {
        IR* new_name = new IR(IRTypeIDENT, "param_name_0", DataStorageParams, ContextUse, children[0]->get_mapped_token_node());
        cur_ir->swap_one_child(children[0], new_name);
    }
}

inline void handle_alter_seq_stmt(IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeQualifiedName) {
            IR* new_name = new IR(IRTypeIDENT, "sequence_name_0", DataSequenceName, ContextUse, child->get_mapped_token_node());
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_seq_opt_elem(IR*& cur_ir)
{
    BEGIN;

    if (children.back()->get_ir_type() == IRTypeName) {
        if (children.front()->get_ir_type() == IRTypeOWNED) {
            IR* new_name = new IR(IRTypeIDENT, "role_0", DataRoleName, ContextUse, children.back()->get_mapped_token_node());
            cur_ir->swap_one_child(children.back(), new_name);
        } else if (children.front()->get_ir_type() == IRTypeSEQUENCE) {
            IR* new_name = new IR(IRTypeIDENT, "sequence_0", DataSequenceName, ContextUse, children.back()->get_mapped_token_node());
            cur_ir->swap_one_child(children.back(), new_name);
        }
    }
}

inline void handle_drop_secret_stmt(IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeColId) {
            IR* new_name = new IR(IRTypeIDENT, "secret_name_0", DataSecretName, ContextUndefine, child->get_mapped_token_node());
            cur_ir->swap_one_child(child, new_name);
            break;
        }
    }
}

inline void handle_transaction_stmt(IR*& cur_ir)
{
    // NOTHING we need to do.
}

inline void handle_use_stmt(IR*& cur_ir)
{
    BEGIN;

    IR* new_name = new IR(IRTypeIDENT, "role_0", DataRoleName, ContextUse, children.back()->get_mapped_token_node());
    cur_ir->swap_one_child(children.back(), new_name);
}

inline void handle_create_stmt(IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeQualifiedName) {
            IR* new_name = new IR(IRTypeIDENT, "v0", DataTableName, ContextDefine, child->get_mapped_token_node());
            cur_ir->swap_one_child(child, new_name);
            break;
        }
    }
}

inline void handle_table_element(IR*& cur_ir, DATAFLAG data_flag)
{
    BEGIN;

    if (children.front()->get_ir_type() == IRTypeColumnDef) {
        handle_column_def(children.front(), ContextDefine);
    }
}

inline void handle_table_element_list(IR*& cur_ir, DATAFLAG data_flag)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeTableElement) {
            handle_table_element(child, data_flag);
            break;
        }
    }
}

inline void handle_opt_table_element_list(IR*& cur_ir, DATAFLAG data_flag)
{
    BEGIN;

    if (!children.empty()) {
        handle_table_element_list(children.front(), data_flag);
    }
}

inline void handle_col_constraint(IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeName) {
            IR* new_name = new IR(IRTypeIDENT, "constraint_0", DataConstraintName, ContextDefine, child->get_mapped_token_node());
            cur_ir->swap_one_child(child, new_name);
            break;
        }
        if (child->get_ir_type() == IRTypeAnyName) {
            IR* new_name = new IR(IRTypeIDENT, "en", DataCollationName, ContextUse, child->get_mapped_token_node());
            cur_ir->swap_one_child(child, new_name);
            break;
        }
    }
}

inline void handle_col_constraint_elem(IR*& cur_ir)
{
    BEGIN;

    if (children.back()->get_ir_type() == IRTypeName) {
        IR* new_name = new IR(IRTypeIDENT, "v00", DataCompressionName, ContextUse, children.back()->get_mapped_token_node());
        cur_ir->swap_one_child(children.back(), new_name);
    } else if (children.front()->get_ir_type() == IRTypeREFERENCES) {
        IR* new_name = new IR(IRTypeIDENT, "v00", DataTableName, ContextUse, children[1]->get_mapped_token_node());
        cur_ir->swap_one_child(children[1], new_name);
    }
}

inline void handle_existing_index(IR*& cur_ir)
{
    BEGIN;
    IR* new_name = new IR(IRTypeIDENT, "v00", DataIndexName, ContextUse, children.back()->get_mapped_token_node());
    cur_ir->swap_one_child(children.back(), new_name);
}

inline void handle_reloption_elem(IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeColLabel) {
            IR* new_name = new IR(IRTypeIDENT, "reloption_name", DataRelOptionName, ContextUse, child->get_mapped_token_node());
            cur_ir->swap_one_child(child, new_name);
        }
    }

    if (children.back()->get_ir_type() == IRTypeDefArg) {
        IR* new_name = new IR(IRTypeIDENT, "reloption_args", DataRelOptionArgs, ContextUse, children.back()->get_mapped_token_node());
        cur_ir->swap_one_child(children.back(), new_name);
    }
}

inline void handle_def_elem(IR*& cur_ir)
{
    BEGIN;

    IR* new_name = new IR(IRTypeIDENT, "c01", DataColumnName, ContextUse, children.front()->get_mapped_token_node());
    cur_ir->swap_one_child(children.front(), new_name);
}

inline void handle_func_type(IR*& cur_ir)
{
    // BEGIN;
    // TODO: ignore for now.
}

inline void handle_table_like_clause(IR*& cur_ir)
{
    BEGIN;

    // ONLY ONE RULE.
    IR* new_name = new IR(IRTypeIDENT, "v00", DataTableName, ContextUse, children[1]->get_mapped_token_node());
    cur_ir->swap_one_child(children[1], new_name);
}

inline void handle_any_name(IR*& cur_ir, DATATYPE data_type, DATAFLAG data_flag)
{
    BEGIN;

    // remove the '.' attrs
    if (children.size() == 2) {
        auto child_to_rov = children.back();
        cur_ir->detach_one_child(children.back());
        child_to_rov->deep_drop();
    }

    IR* new_name = new IR(IRTypeIDENT, "v00", data_type, data_flag, children.front()->get_mapped_token_node());
    cur_ir->swap_one_child(children.front(), new_name);
}

inline void handle_any_name_list(IR*& cur_ir, DATATYPE datatype, DATAFLAG data_flag)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeAnyName) {
            handle_any_name(child, datatype, data_flag);
        }
    }
}

inline void handle_name(IR*& cur_ir, DATATYPE data_type, DATAFLAG data_flag)
{
    BEGIN;

    IR* new_name = new IR(IRTypeIDENT, "v00", data_type, data_flag, children.front()->get_mapped_token_node());
    cur_ir->swap_one_child(children.front(), new_name);
}

inline void handle_name_list(IR*& cur_ir, DATATYPE datatype, DATAFLAG data_flag)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeName) {
            handle_name(child, datatype, data_flag);
        }
    }
}

inline void handle_drop_stmt(IR*& cur_ir)
{
    BEGIN;

    DATATYPE data_type = DataTableName;

    if (children[1]->get_ir_type() == IRTypeDropTypeAnyName) {
        switch (children[1]->get_children().front()->get_ir_type()) {
        case (IRTypeTABLE):
            data_type = DataTableName;
            break;
        case (IRTypeSEQUENCE):
            data_type = DataSequenceName;
            break;
        case (IRTypeFUNCTION):
            data_type = DataFunctionName;
            break;
        case (IRTypeMACRO):
            // FIXME:: MACRO and MACRO TABLE
            data_type = DataTypeName;
            break;
        case (IRTypeVIEW):
            data_type = DataViewName;
            break;
        case (IRTypeMATERIALIZED):
            data_type = DataViewName;
            break;
        case (IRTypeINDEX):
            data_type = DataIndexName;
            break;
        case (IRTypeFOREIGN):
            data_type = DataTableName;
            break;
        case (IRTypeCOLLATION):
            data_type = DataCollationName;
            break;
        case (IRTypeCONVERSIONP):
            // FIXME
            break;
        case (IRTypeSCHEMA):
            data_type = DataSchemaName;
            break;
        case (IRTypeSTATISTICS):
            data_type = DataStatisticsName;
            break;
        case (IRTypeTEXTP):
            // FIXME
            break;
        case (IRTypeTYPEP):
            data_type = DataTypeName;
            break;
        default:
            break;
        }
    } else if (children[1]->get_ir_type() == IRTypeDropTypeName) {
        switch (children[1]->get_children().front()->get_ir_type()) {
        case (IRTypeACCESS):
            data_type = DataAccessMethodName;
            break;
        case (IRTypeEVENT):
            data_type = DataEventName;
            break;
        case (IRTypeEXTENSION):
            data_type = DataExtensionName;
            break;
        case (IRTypeFOREIGN):
            // FIXME
            break;
        case (IRTypePUBLICATION):
            // FIXME
            break;
        case (IRTypeSERVER):
            data_type = DataServerName;
            break;
        default:
            break;
        }
    } else if (children[1]->get_ir_type() == IRTypeDropTypeNameOnAnyName) {
        switch (children[1]->get_children().front()->get_ir_type()) {
        case (IRTypePOLICY):
            data_type = DataPolicyName;
            break;
        case (IRTypeRULE):
            data_type = DataRuleName;
            break;
        case (IRTypeTRIGGER):
            data_type = DataTriggerName;
            break;
        default:
            break;
        }
    }

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeAnyNameList) {
            handle_any_name_list(child, data_type, ContextUndefine);
            break;
        } else if (child->get_ir_type() == IRTypeNameList) {
            handle_name_list(child, data_type, ContextUndefine);
            break;
        } else if (child->get_ir_type() == IRTypeName) {
            handle_name(child, data_type, ContextUndefine);
            // Can happen together with IRTypeAnyName
            continue;
        } else if (child->get_ir_type() == IRTypeAnyName) {
            handle_any_name(child, DataDatabaseName, ContextUse);
            continue;
        }
    }
}

inline void handle_create_function_stmt(IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeQualifiedName) {
            IR* new_name = new IR(IRTypeIDENT, "v00", DataFunctionName, ContextDefine, child->get_mapped_token_node());
            cur_ir->swap_one_child(child, new_name);
            break;
        }
    }
}

inline void handle_name_list_opt_comma(IR*& cur_ir, DATATYPE data_type, DATAFLAG data_flag)
{
    BEGIN;
    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeNameList) {
            handle_name_list(child, data_type, data_flag);
            break;
        }
    }
}

inline void handle_opt_name_list(IR*& cur_ir, DATATYPE data_type, DATAFLAG data_flag)
{
    BEGIN;
    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeNameListOptComma) {
            handle_name_list_opt_comma(child, data_type, data_flag);
            break;
        }
    }
}

inline void handle_common_table_expr(IR*& cur_ir)
{
    BEGIN;

    IR* new_name = new IR(IRTypeIDENT, "a01", DataTableAliasName, ContextDefine, children.front()->get_mapped_token_node());
    cur_ir->swap_one_child(children.front(), new_name);

    handle_opt_name_list(children[1], DataColumnAliasName, ContextDefine);
}

inline void handle_update_stmt(IR*& cur_ir)
{
    BEGIN;

    // opt_with_clause is handled by common_table_expr();

    handle_relation_expr_opt_alias(children[2], ContextUse);
}

inline void handle_target_list(IR*& cur_ir, DATATYPE data_type, DATAFLAG data_flag)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeTargetEl) {
            handle_target_el(child, data_type, data_flag);
        } else {
            handle_target_list(child, data_type, data_flag);
        }
    }
}

inline void handle_target_list_opt_comma(IR*& cur_ir, DATATYPE data_type, DATAFLAG data_flag)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeTargetList) {
            handle_target_list(child, data_type, data_flag);
        }
    }
}

inline void handle_opt_target_list_opt_comma(IR*& cur_ir, DATATYPE data_type, DATAFLAG data_flag)
{
    BEGIN;

    if (!children.empty()) {
        handle_target_list_opt_comma(children.front(), data_type, data_flag);
    }
}

inline void handle_table_ref(IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeRelationExpr) {
            handle_relation_expr(child, ContextUse);
        } else if (child->get_ir_type() == IRTypeTargetListOptComma) {
            handle_target_list_opt_comma(child, DataColumnName, ContextUse);
        }
    }
}

inline void handle_returning_clause(IR*& cur_ir)
{
    BEGIN;

    if (!children.empty()) {
        handle_target_list(children[1], DataColumnName, ContextUse);
    }
}

inline void handle_copy_stmt(IR*& cur_ir)
{
    BEGIN;

    if (children[2]->get_ir_type() == IRTypeQualifiedName) {
        handle_qualified_name(children[2], DataTableName, ContextUse);
    }
    if (children[3]->get_ir_type() == IRTypeOptColumnList) {
        handle_opt_column_list(children[3], ContextUse);
    }

    if (children[3]->get_ir_type() == IRTypeColId && children[5]->get_ir_type() == IRTypeColId) {
        IR* new_name = new IR(IRTypeIDENT, "database_name_0", DataDatabaseName, ContextUse, children[3]->get_mapped_token_node());
        cur_ir->swap_one_child(children[3], new_name);

        new_name = new IR(IRTypeIDENT, "database_name_1", DataDatabaseName, ContextUse, children[5]->get_mapped_token_node());
        cur_ir->swap_one_child(children[5], new_name);
    }
}

inline void handle_copy_file_name(IR*& cur_ir)
{
    BEGIN;

    if (children.front()->get_ir_type() == IRTypeIDENT) {
        IR* new_name = new IR(IRTypeIDENT, "v00", DataTableName, ContextUse, children.front()->get_mapped_token_node());
        cur_ir->swap_one_child(children.front(), new_name);
    }

    if (children.size() == 3) {
        IR* new_name = new IR(IRTypeIDENT, "v00", DataTableName, ContextUse, children.back()->get_mapped_token_node());
        cur_ir->swap_one_child(children.back(), new_name);
    }
}

inline void handle_copy_generic_opt_elem(IR*& cur_ir)
{
    BEGIN;

    // ONLY ONE RULE

    IR* new_name = new IR(IRTypeIDENT, "v00", DataGenericOptionName, ContextUse, children.front()->get_mapped_token_node());
    cur_ir->swap_one_child(children.front(), new_name);

    children.back()->set_data_type(DataGenericOptionArgs);
}

inline void handle_copy_opt_item(IR*& cur_ir)
{
    BEGIN;

    // ONLY ONE RULE

    if (children.back()->get_ir_type() == IRTypeColumnList) {
        handle_column_list(children.back(), ContextUse);
    }
}

inline void handle_opt_select(IR*& cur_ir)
{
    BEGIN;

    if (!children.empty()) {
        handle_opt_target_list_opt_comma(children.back(), DataColumnName, ContextUse);
    }
}

inline void handle_simple_select(IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeOptTargetListOptComma) {
            handle_opt_target_list_opt_comma(child, DataColumnName, ContextUse);
        } else if (child->get_ir_type() == IRTypeTargetListOptComma) {
            handle_target_list_opt_comma(child, DataColumnName, ContextUse);
        }

        // IRTypeFromClause is handled by table_ref.
        // group_by_clause doesn't need to be handled?
        // into_clause is handled by itself.

        else if (child->get_ir_type() == IRTypeRelationExpr) {
            handle_relation_expr(child, ContextUse);
        }

        // table_ref is handled by itself.
        // handle_name_list_opt_comma_opt_bracket is handled by itself.

        else if (child->get_ir_type() == IRTypeName && children.front()->get_ir_type() == IRTypeUnpivotKeyword) {
            IR* new_name = new IR(IRTypeIDENT, "v00", DataTableName, ContextDefine, child->get_mapped_token_node());
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_name_list_opt_comma_opt_bracket(IR*& cur_ir, DATATYPE data_type = DataColumnName, DATAFLAG data_flag = ContextUse)
{
    BEGIN;
    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeNameListOptComma) {
            handle_name_list_opt_comma(child, data_type, data_flag);
        }
    }
}

inline void handle_single_pivot_value(IR*& cur_ir)
{
    BEGIN;

    if (children.back()->get_ir_type() == IRTypeColIdOrString) {
        IR* new_name = new IR(IRTypeIDENT, "c01", DataColumnName, ContextUse, children.back()->get_mapped_token_node());
        cur_ir->swap_one_child(children.back(), new_name);
    }
}

inline void handle_opt_temp_table_name(IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeQualifiedName) {
            handle_qualified_name(child, DataTableName, ContextDefine);
        }
    }
}

inline void handle_opt_sample_func(IR*& cur_ir)
{
    BEGIN;

    if (!children.empty()) {
        IR* new_name = new IR(IRTypeIDENT, "func_name_0", DataSamplingFuncName, ContextUse, children.front()->get_mapped_token_node());
        cur_ir->swap_one_child(children.front(), new_name);
    }
}

inline void handle_tablesample_entry(IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeColId) {
            IR* new_name = new IR(IRTypeIDENT, "func_name_0", DataSamplingFuncName, ContextUse, child->get_mapped_token_node());
            cur_ir->swap_one_child(child, new_name);
            break;
        }
    }
}

inline void handle_qualified_name_list(IR*& cur_ir, DATATYPE data_type = DataTableName, DATAFLAG data_flag = ContextUse)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeQualifiedName) {
            handle_qualified_name(child, data_type, data_flag);
        } else {
            handle_qualified_name_list(child, data_type, data_flag);
        }
    }
}

inline void handle_locked_rels_list(IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeQualifiedNameList) {
            handle_qualified_name_list(child, DataTableName, ContextUse);
        }
    }
}

inline void handle_alias_clause(IR*& cur_ir, DATATYPE data_type = DataTableAliasName, DATAFLAG data_flag = ContextDefine)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeColIdOrString || child->get_ir_type() == IRTypeColId) {
            IR* new_name = new IR(IRTypeIDENT, "v00", data_type, data_flag, child->get_mapped_token_node());
            cur_ir->swap_one_child(child, new_name);
        } else if (child->get_ir_type() == IRTypeNameListOptComma) {
            handle_name_list_opt_comma(child, data_type, data_flag);
        }
    }
}

inline void handle_opt_alias_clause(IR*& cur_ir, DATATYPE data_type, DATAFLAG data_flag)
{
    BEGIN;

    if (!children.empty()) {
        handle_alias_clause(children.front(), data_type, data_flag);
    }
}

inline void handle_func_application(IR*& cur_ir)
{
    BEGIN;

    // Are these operations necessary?
    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeFuncName) {
            IR* new_name = new IR(IRTypeIDENT, "func_name", DataFunctionName, ContextUse, child->get_mapped_token_node());
            cur_ir->swap_one_child(child, new_name);
        } else if (child->get_ir_type() == IRTypeFuncArgList) {
            // handled by func_arg_expr.
        }
    }

    cur_ir->set_data_type(DataFunctionExpr);
}

inline void handle_func_arg_expr(IR*& cur_ir)
{
    BEGIN;

    if (children.front()->get_ir_type() == IRTypeParamName) {
        IR* new_name = new IR(IRTypeIDENT, "v00", DataParamName, ContextUse, children.front()->get_mapped_token_node());
        cur_ir->swap_one_child(children.front(), new_name);
    }
}

inline void handle_join_qual(IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeNameListOptComma) {
            handle_name_list_opt_comma(child, DataColumnName, ContextUse);
        }
    }
}

inline void handle_table_func_element(IR*& cur_ir, DATATYPE data_type = DataColumnName, DATAFLAG data_flag = ContextUse)
{
    BEGIN;

    // ONLY ONE RULE
    IR* new_name = new IR(IRTypeIDENT, "v00", data_type, data_flag, children.front()->get_mapped_token_node());
    cur_ir->swap_one_child(children.front(), new_name);
}

inline void handle_table_func_element_list(IR*& cur_ir, DATATYPE data_type = DataColumnName, DATAFLAG data_flag = ContextUse)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeTableFuncElement) {
            handle_table_func_element(child, data_type, data_flag);
        } else {
            handle_table_func_element_list(child, data_type, data_flag);
        }
    }
}

inline void handle_opt_collate_clause(IR*& cur_ir)
{
    BEGIN;

    if (!children.empty()) {
        IR* new_name = new IR(IRTypeIDENT, "en", DataCollationName, ContextUse, children.back()->get_mapped_token_node());
        cur_ir->swap_one_child(children.back(), new_name);
    }
}

inline void handle_func_alias_clause(IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeAliasClause) {
            handle_alias_clause(child, DataTableAliasName, ContextUse);
        } else if (child->get_ir_type() == IRTypeTableFuncElementList) {
            handle_table_func_element_list(child, DataColumnAliasName, ContextDefine);
        } else if (child->get_ir_type() == IRTypeColIdOrString || child->get_ir_type() == IRTypeColId) {
            IR* new_name = new IR(IRTypeIDENT, "alias_0", DataTableAliasName, ContextDefine, child->get_mapped_token_node());
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_pivot_value(IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeTargetListOptComma) {
            handle_target_list_opt_comma(child, DataColumnName, ContextUse);
        } else if (child->get_ir_type() == IRTypeColIdOrString) {
            IR* new_name = new IR(IRTypeIDENT, "c01", DataColumnName, ContextUse, child->get_mapped_token_node());
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_unpivot_header(IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeColIdOrString) {
            IR* new_name = new IR(IRTypeIDENT, "c01", DataColumnName, ContextUse, child->get_mapped_token_node());
            cur_ir->swap_one_child(child, new_name);
        } else if (child->get_ir_type() == IRTypeNameListOptComma) {
            handle_name_list_opt_comma(child, DataColumnName, ContextUse);
        }
    }
}

inline void handle_unpivot_value(IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeTargetListOptComma) {
            handle_target_list_opt_comma(child, DataColumnName, ContextUse);
        }
    }
}

inline void handle_joined_table(IR*& cur_ir)
{
    // BEGIN;
    // Nothing we need to do.
}

inline void handle_func_table(IR*& cur_ir)
{
    // BEGIN;

    // Nothing we need to do.
}

inline void handle_opt_col_def_list(IR*& cur_ir)
{
    BEGIN;

    if (!children.empty()) {
        // FIXME:: ROWS FROM AS (??? Column_Alias_Define?)
        handle_table_func_element_list(children.back(), DataColumnAliasName, ContextDefine);
    }
}

inline void handle_colid_type_list(IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeColId) {
            IR* new_name = new IR(IRTypeIDENT, "c01", DataColumnName, ContextUse, child->get_mapped_token_node());
            cur_ir->swap_one_child(child, new_name);
        } else if (child->get_ir_type() == IRTypeColidTypeList) {
            // do not run recursively.
        }
    }
}

inline void handle_qualified_type_name(IR*& cur_ir)
{
    BEGIN;

    // Force rewrite the whole syntax.
    IR* new_name = new IR(IRTypeIDENT, "INT", DataTypeName, ContextUse, cur_ir->get_mapped_token_node());
    cur_ir->free_children();
    cur_ir->add_one_child(new_name, 0);
}

inline void handle_generic_type(IR*& cur_ir)
{
    BEGIN;

    // ONLY ONE RULE
    // But we still force rewrite the rule since we don't want to use custom types in the query generator (too complicated)

    IR* new_name = new IR(IRTypeIDENT, "INT", DataTypeName, ContextUse, cur_ir->get_mapped_token_node());
    cur_ir->free_children();
    cur_ir->add_one_child(new_name, 0);
}

inline void handle_a_expr(IR*& cur_ir)
{
    BEGIN;

    if (children.back()->get_ir_type() == IRTypeAnyName && children.size() == 3 && children[1]->get_ir_type() == IRTypeCOLLATE) {
        IR* new_name = new IR(IRTypeIDENT, "c01", DataCollationName, ContextUse, children.back()->get_mapped_token_node());
        cur_ir->swap_one_child(children.back(), new_name);
    }

    if (children.front()->get_ir_type() == IRTypeColId) {
        if (children.size() > 5 && children[3]->get_ir_type() == IRTypeOptExceptList) {
            IR* new_name = new IR(IRTypeIDENT, "v00", DataTableName, ContextUse, children.front()->get_mapped_token_node());
            cur_ir->swap_one_child(children.front(), new_name);
        } else {
            // This is introduced by force IR rewrite in duckdb_comp_rule_terminator.
            IR* new_name = new IR(IRTypeIDENT, "c01", DataColumnName, ContextUse, children.front()->get_mapped_token_node());
            cur_ir->swap_one_child(children.front(), new_name);
        }
    }
}

inline void handle_columnref_opt_indirection(IR*& cur_ir)
{
    BEGIN;

    IR* new_name = new IR(IRTypeIDENT, "c01", DataColumnName, ContextUse, children.front()->get_mapped_token_node());
    cur_ir->swap_one_child(children.front(), new_name);
}

inline void handle_columnref(IR*& cur_ir)
{
    BEGIN;

    IR* new_name = new IR(IRTypeIDENT, "c01", DataColumnName, ContextUse, children.front()->get_mapped_token_node());
    cur_ir->swap_one_child(children.front(), new_name);
}

inline void handle_indirection_expr(IR*& cur_ir)
{
    BEGIN;

    if (children.back()->get_ir_type() == IRTypeColLabel) {
        IR* new_name = new IR(IRTypeIDENT, "c01", DataColumnName, ContextUse, children.back()->get_mapped_token_node());
        cur_ir->swap_one_child(children.back(), new_name);
    }
}

inline void handle_extract_arg(IR*& cur_ir)
{
    BEGIN;

    if (!children.empty() && children.front()->get_ir_type() == IRTypeIDENT) {
        IR* new_name = new IR(SymbolTerm, IRTypeUnknownType, string("YEAR"), nullptr, nullptr, nullptr);
        cur_ir->free_children();
        cur_ir->add_one_child(new_name, 0);
    }
}

inline void handle_window_definition(IR*& cur_ir)
{
    BEGIN;

    // ONLY ONE RULE
    IR* new_name = new IR(IRTypeIDENT, "window_0", DataWindowName, ContextDefine, children.front()->get_mapped_token_node());
    cur_ir->swap_one_child(children.front(), new_name);
}

inline void handle_over_clause(IR*& cur_ir)
{
    BEGIN;

    if (!children.empty() && children.back()->get_ir_type() == IRTypeColId) {
        IR* new_name = new IR(IRTypeIDENT, "c01", DataWindowName, ContextUse, children.back()->get_mapped_token_node());
        cur_ir->swap_one_child(children.back(), new_name);
    }
}

inline void handle_opt_existing_window_name(IR*& cur_ir)
{
    BEGIN;

    if (!children.empty() && children.back()->get_ir_type() == IRTypeColId) {
        IR* new_name = new IR(IRTypeIDENT, "c01", DataWindowName, ContextUse, children.back()->get_mapped_token_node());
        cur_ir->swap_one_child(children.back(), new_name);
    }
}

inline void handle_dict_arg(IR*& cur_ir)
{
    // BEGIN;
    //  FIXME:: Not handled yet.
}

inline void handle_any_operator(IR*& cur_ir)
{
    BEGIN;

    if (children.size() == 3) {
        // Deprecate the rule of "ColId . any_operator".
        IR* new_name = new IR(SymbolTerm, IRTypeUnknownType, "+", nullptr, nullptr, nullptr);
        cur_ir->free_children();
        cur_ir->add_one_child(new_name, 0);
    }
}

inline void handle_extended_indirection_el(IR*& cur_ir)
{
    BEGIN;

    if (children.size() > 2 && children[1]->get_ir_type() == IRTypeAttrName) {
        IR* new_name = new IR(IRTypeIDENT, "c01", DataAttrName, ContextUse, children[1]->get_mapped_token_node());
        cur_ir->swap_one_child(children[1], new_name);
    }
}

inline void handle_except_list(IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeNameListOptComma) {
            handle_name_list_opt_comma(child, DataColumnName, ContextUse);
        } else if (child->get_ir_type() == IRTypeColId) {
            IR* new_name = new IR(IRTypeIDENT, "c01", DataColumnName, ContextUse, child->get_mapped_token_node());
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_replace_list_el(IR*& cur_ir)
{
    BEGIN;

    // ONLY ONE RULE
    IR* new_name = new IR(IRTypeIDENT, "c01", DataColumnAliasName, ContextDefine, children.back()->get_mapped_token_node());
    cur_ir->swap_one_child(children.back(), new_name);
}

inline void handle_func_name(IR*& cur_ir)
{
    BEGIN;

    IR* new_name = new IR(IRTypeIDENT, "sum", DataFunctionName, ContextUse, cur_ir->get_mapped_token_node());
    cur_ir->free_children();
    cur_ir->add_one_child(new_name, 0);
}

inline void handle_type_function_name(IR*& cur_ir)
{
    BEGIN;

    IR* new_name = new IR(IRTypeIDENT, "sum", DataFunctionName, ContextUse, cur_ir->get_mapped_token_node());
    cur_ir->free_children();
    cur_ir->add_one_child(new_name, 0);
}

inline void handle_function_name_token(IR*& cur_ir)
{
    BEGIN;

    IR* new_name = new IR(IRTypeIDENT, "sum", DataFunctionName, ContextUse, cur_ir->get_mapped_token_node());
    cur_ir->free_children();
    cur_ir->add_one_child(new_name, 0);
}

inline void handle_type_name_token(IR*& cur_ir)
{
    BEGIN;

    IR* new_name = new IR(IRTypeIDENT, "INT", DataTypeName, ContextUse, cur_ir->get_mapped_token_node());
    cur_ir->free_children();
    cur_ir->add_one_child(new_name, 0);
}

inline void handle_any_name(IR*& cur_ir)
{
    // Placeholder, highly inaccurate.
    BEGIN;

    IR* new_name = new IR(IRTypeIDENT, "v00", DataTableName, ContextUse, cur_ir->get_mapped_token_node());
    cur_ir->free_children();
    cur_ir->add_one_child(new_name, 0);
}

inline void handle_attrs(IR*& cur_ir)
{
    // Placeholder, highly inaccurate.
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeAttrName) {
            IR* new_name = new IR(IRTypeIDENT, "c01", DataAttrName, ContextUse, child->get_mapped_token_node());
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_prepare_stmt(IR*& cur_ir)
{
    BEGIN;

    // ONLY ONE RULE
    IR* new_name = new IR(IRTypeIDENT, "prepareable_name_0", DataStatementPreparedName, ContextDefine, children[1]->get_mapped_token_node());
    cur_ir->swap_one_child(children[1], new_name);
}

inline void handle_create_schema_stmt(IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeQualifiedName) {
            IR* new_name = new IR(IRTypeIDENT, "schema_0", DataSchemaName, ContextDefine, child->get_mapped_token_node());
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_index_name(IR*& cur_ir, DATAFLAG data_flag)
{
    BEGIN;

    // ONLY ONE RULE
    IR* new_name = new IR(IRTypeIDENT, "idx_0", DataIndexName, data_flag, children.front()->get_mapped_token_node());
    cur_ir->swap_one_child(children.front(), new_name);
}

inline void handle_opt_index_name(IR*& cur_ir, DATAFLAG data_flag)
{
    BEGIN;

    if (!children.empty()) {
        handle_index_name(children.front(), data_flag);
    }
}

inline void handle_index_elem(IR*& cur_ir, DATATYPE data_type, DATAFLAG data_flag)
{
    BEGIN;

    if (!children.empty() && children.front()->get_ir_type() == IRTypeColId) {
        IR* new_name = new IR(IRTypeIDENT, "index_0", data_type, data_flag, children.front()->get_mapped_token_node());
        cur_ir->swap_one_child(children.front(), new_name);
    }
}

inline void handle_index_params(IR*& cur_ir, DATATYPE data_type, DATAFLAG data_flag)
{
    BEGIN;
    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIndexElem) {
            handle_index_elem(child, data_type, data_flag);
        } else if (child->get_ir_type() == IRTypeIndexParams) {
            handle_index_params(child, data_type, data_flag);
        }
    }
}

inline void handle_index_stmt(IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIndexName) {
            handle_index_name(child, ContextDefine);
        } else if (child->get_ir_type() == IRTypeOptIndexName) {
            handle_opt_index_name(child, ContextDefine);
        } else if (child->get_ir_type() == IRTypeQualifiedName) {
            handle_qualified_name(child, DataTableName, ContextUse);
        } else if (child->get_ir_type() == IRTypeIndexParams) {
            handle_index_params(child, DataColumnName, ContextUse);
        }
    }
}

inline void handle_access_method(IR*& cur_ir)
{
    BEGIN;

    IR* new_name = new IR(SymbolTerm, IRTypeUnknownType, "ART", nullptr, nullptr, nullptr);
    cur_ir->swap_one_child(children.front(), new_name);
}

inline void handle_opt_col_id(IR*& cur_ir)
{
    BEGIN;

    if (!children.empty()) {
        IR* new_name = new IR(IRTypeIDENT, "checkpoint_name_0", DataCheckPointName, ContextDefine, children.front()->get_mapped_token_node());
        cur_ir->swap_one_child(children.front(), new_name);
    }
}

inline void handle_alter_object_schema_stmt(IR*& cur_ir)
{
    BEGIN;

    assert(children.size() > 3);

    DATATYPE data_type = DataTableName;

    switch (children[1]->get_ir_type()) {
    case (IRTypeTABLE):
        break;
    case (IRTypeSEQUENCE):
        data_type = DataSequenceName;
        break;
    case (IRTypeVIEW):
        data_type = DataViewName;
        break;
    default:
        cerr << "Error: Getting unknown name in handle_alter_object_schema_stmt: " << get_string_by_ir_type(children[1]->get_ir_type()) << endl;
        abort();
    }

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeQualifiedName) {
            handle_qualified_name(child, data_type, ContextUse);
        } else if (child->get_ir_type() == IRTypeRelationExpr) {
            handle_relation_expr(child, ContextUse);
        }
    }

    if (children.back()->get_ir_type() == IRTypeName) {
        IR* new_name = new IR(IRTypeIDENT, "schema_0", DataSchemaName, ContextUse, children.back()->get_mapped_token_node());
        cur_ir->swap_one_child(children.back(), new_name);
    }
}

inline void handle_var_value(IR*& cur_ir)
{
    // BEGIN;

    // NOTHING WE NEED TO DO.
}

inline void handle_comment_on_stmt(IR*& cur_ir)
{
    BEGIN;

    if (children[2]->get_ir_type() == IRTypeCommentOnTypeAnyName) {

        DATATYPE data_type = DataTableName;
        switch (children[2]->get_children().front()->get_ir_type()) {
        case (IRTypeTABLE):
            break;
        case (IRTypeSEQUENCE):
            data_type = DataSequenceName;
            break;
        case (IRTypeFUNCTION):
            data_type = DataFunctionName;
            break;
        case (IRTypeMACRO):
            // FIXME:: Not accurate.
            data_type = DataTypeName;
            break;
        // FIXME::Skipped macro table... not sure how to handle it :-(
        case (IRTypeVIEW):
            data_type = DataViewName;
            break;
        case (IRTypeDATABASE):
            data_type = DataDatabaseName;
            break;
        case (IRTypeINDEX):
            data_type = DataIndexName;
            break;
        case (IRTypeSCHEMA):
            data_type = DataSchemaName;
            break;
        case (IRTypeTYPEP):
            data_type = DataTypeName;
            break;
        default:
            cerr << "ERROR: Getting unexpected comment_on_type_any_name: " << get_string_by_ir_type(children[2]->get_ir_type()) << endl;
        }

        for (auto& child : children) {
            if (child->get_ir_type() == IRTypeQualifiedName) {
                handle_qualified_name(child, data_type, ContextUse);
            }
        }
    }
}

inline void handle_export_stmt(IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeColId || child->get_ir_type() == IRTypeSconst) {
            IR* new_name = new IR(IRTypeIDENT, "database_0", DataDatabaseName, ContextUse, child->get_mapped_token_node());
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_import_stmt(IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeColId || child->get_ir_type() == IRTypeSconst) {
            IR* new_name = new IR(IRTypeIDENT, "database_0", DataDatabaseName, ContextUse, child->get_mapped_token_node());
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_set_rest(IR*& cur_ir)
{
    BEGIN;

    if (children.front()->get_ir_type() == IRTypeVarName) {
        IR* new_name = new IR(IRTypeIDENT, "var_name_0", DataSettingName, ContextUse, children.front()->get_mapped_token_node());
        cur_ir->swap_one_child(children.front(), new_name);
    }
}

inline void handle_generic_set(IR*& cur_ir)
{
    BEGIN;

    if (children.front()->get_ir_type() == IRTypeVarName) {
        IR* new_name = new IR(IRTypeIDENT, "var_name_0", DataSettingName, ContextUse, children.front()->get_mapped_token_node());
        cur_ir->swap_one_child(children.front(), new_name);
    }
}

inline void handle_zone_value(IR*& cur_ir)
{
    BEGIN;

    if (children.front()->get_ir_type() == IRTypeIDENT || children.front()->get_ir_type() == IRTypeSconst) {
        IR* new_name = new IR(SymbolTerm, IRTypeLOCAL, string("LOCAL"), nullptr, nullptr, nullptr);
        cur_ir->swap_one_child(children.front(), new_name);
    }
}

inline void handle_load_stmt(IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeFileName) {
            IR* new_name = new IR(IRTypeIDENT, "local_file_0", DataLocalFileName, ContextUse, child->get_mapped_token_node());
            cur_ir->swap_one_child(child, new_name);
        } else if (child->get_ir_type() == IRTypeColId || child->get_ir_type() == IRTypeSconst) {
            IR* new_name = new IR(IRTypeIDENT, "v00", DataTableName, ContextUse, child->get_mapped_token_node());
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_opt_ext_version(IR*& cur_ir)
{
    // FIXME:: Don't know how to fix this.
    cur_ir->free_children();
    // bye bye :-)
}

inline void handle_vacuum_stmt(IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeQualifiedName) {
            handle_qualified_name(child, DataTableName, ContextUse);
        } else if (child->get_ir_type() == IRTypeOptNameList) {
            handle_opt_name_list(child, DataColumnName, ContextUse);
        }
    }
}

inline void handle_delete_stmt(IR*& cur_ir)
{
    // BEGIN;

    // NOTHING WE NEED TO DO.
}

inline void handle_analyze_stmt(IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeQualifiedName) {
            handle_qualified_name(child, DataTableName, ContextUse);
        } else if (child->get_ir_type() == IRTypeOptNameList) {
            handle_opt_name_list(child, DataColumnName, ContextUse);
        }
    }
}

inline void handle_attach_stmt(IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeSconst) {
            IR* new_name = new IR(IRTypeIDENT, "database_0", DataDatabaseName, ContextUse, child->get_mapped_token_node());
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_detach_stmt(IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeColLabel) {
            IR* new_name = new IR(IRTypeIDENT, "database_0", DataDatabaseName, ContextUse, child->get_mapped_token_node());
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_opt_database_alias(IR*& cur_ir)
{
    BEGIN;

    if (!children.empty()) {
        IR* new_name = new IR(IRTypeIDENT, "database_alias_0", DataDatabaseAliasName, ContextDefine, children.back()->get_mapped_token_node());
        cur_ir->swap_one_child(children.back(), new_name);
    }
}

inline void handle_generic_reset(IR*& cur_ir)
{
    BEGIN;

    if (children.front()->get_ir_type() == IRTypeVarName) {
        IR* new_name = new IR(SymbolTerm, IRTypeALL, string("ALL"), nullptr, nullptr, nullptr);
        cur_ir->swap_one_child(children.front(), new_name);
    }
}

inline void handle_table_id(IR*& cur_ir)
{
    BEGIN;
    IR* new_name = new IR(IRTypeIDENT, "v00", DataTableName, ContextUse, children.back()->get_mapped_token_node());
    cur_ir->free_children();
    cur_ir->add_one_child(new_name, 0);
}

inline void handle_view_stmt(IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeQualifiedName) {
            handle_qualified_name(child, DataViewName, ContextDefine);
        } else if (child->get_ir_type() == IRTypeOptColumnList) {
            handle_opt_column_list(child, ContextDefine);
        }
    }
}

void duckdb_ir_context_setup(RSG* rsg, IR*& cur_ir)
{
    IRTYPE cur_ir_type = cur_ir->get_ir_type();

    switch (cur_ir_type) {
    case (IRTypeRelationExpr):
        handle_relation_expr(cur_ir);
        break;
    case (IRTypeRelationExprOptAlias):
        handle_relation_expr_opt_alias(cur_ir);
        break;
    case (IRTypeAlterTableStmt):
        handle_alter_table_stmt(cur_ir);
        break;

    case (IRTypeColumnDef):
        handle_column_def(cur_ir);
        break;

    case (IRTypeTableConstraint):
        handle_table_constraint(cur_ir);
        break;

    case (IRTypeAlterTableCmd):
        handle_alter_table_cmd(cur_ir);
        break;

    case (IRTypeGenericOptionElem):
        handle_generic_option_elem(cur_ir);
        break;

    case (IRTypeDeallocateStmt):
        handle_deallocate_stmt(cur_ir);
        break;

    case (IRTypeRenameStmt):
        handle_rename_stmt(cur_ir);
        break;

    case (IRTypeInsertStmt):
        handle_insert_stmt(cur_ir);
        break;

    case (IRTypeQualifiedName):
        handle_qualified_name(cur_ir, DataTableName, ContextUse);
        break;

    case (IRTypeIndexElem):
        handle_index_elem(cur_ir);
        break;

    case (IRTypeOptConfExpr):
        handle_opt_conf_expr(cur_ir);
        break;

    case (IRTypeInsertColumnItem):
        handle_insert_column_item(cur_ir);
        break;

    case (IRTypeSetTarget):
        handle_set_target(cur_ir);
        break;

    case (IRTypeTargetEl):
        handle_target_el(cur_ir);
        break;

    case (IRTypeOptCollate):
        handle_opt_collate(cur_ir);
        break;

    case (IRTypeOptClass):
        handle_opt_class(cur_ir);
        break;

    case (IRTypeCreateTypeStmt):
        handle_create_type_stmt(cur_ir);
        break;

    case (IRTypePragmaStmt):
        handle_pragma_stmt(cur_ir);
        break;

    case (IRTypeCreateSeqStmt):
        handle_create_seq_stmt(cur_ir);
        break;

    case (IRTypeCreateSecretStmt):
        handle_create_secret_stmt(cur_ir);
        break;

    case (IRTypeOptColumnList):
        handle_opt_column_list(cur_ir);
        break;

    case (IRTypeColumnList):
        handle_column_list(cur_ir);
        break;

    case (IRTypeColumnElem):
        handle_column_elem(cur_ir);
        break;

    case (IRTypeUpdateExtensionsStmt):
        handle_update_extensions_stmt(cur_ir);
        break;

    case (IRTypeExecuteStmt):
        handle_execute_stmt(cur_ir);
        break;

    case (IRTypeCreateAsTarget):
        handle_create_as_target(cur_ir);
        break;

    case (IRTypeExecuteParamExpr):
        handle_execute_param_expr(cur_ir);
        break;

    case (IRTypeAlterSeqStmt):
        handle_alter_seq_stmt(cur_ir);
        break;

    case (IRTypeDropSecretStmt):
        handle_drop_secret_stmt(cur_ir);
        break;

    case (IRTypeTransactionStmt):
        handle_transaction_stmt(cur_ir);
        break;

    case (IRTypeUseStmt):
        handle_use_stmt(cur_ir);
        break;

    case (IRTypeCreateStmt):
        handle_create_stmt(cur_ir);
        break;

    case (IRTypeColConstraint):
        handle_col_constraint(cur_ir);
        break;

    case (IRTypeColConstraintElem):
        handle_col_constraint_elem(cur_ir);
        break;

    case (IRTypeExistingIndex):
        handle_existing_index(cur_ir);
        break;

    case (IRTypeDefElem):
        handle_def_elem(cur_ir);
        break;

    case (IRTypeFuncType):
        handle_func_type(cur_ir);
        break;

    case (IRTypeTableLikeClause):
        handle_table_like_clause(cur_ir);
        break;

    case (IRTypeDropStmt):
        handle_drop_stmt(cur_ir);
        break;

    case (IRTypeCreateFunctionStmt):
        handle_create_function_stmt(cur_ir);
        break;

    case (IRTypeCommonTableExpr):
        handle_common_table_expr(cur_ir);
        break;

    case (IRTypeUpdateStmt):
        handle_update_stmt(cur_ir);
        break;

    case (IRTypeTableRef):
        handle_table_ref(cur_ir);
        break;

    case (IRTypeReturningClause):
        handle_returning_clause(cur_ir);
        break;

    case (IRTypeCopyStmt):
        handle_copy_stmt(cur_ir);
        break;

    case (IRTypeCopyFileName):
        handle_copy_file_name(cur_ir);
        break;

    case (IRTypeCopyGenericOptElem):
        handle_copy_generic_opt_elem(cur_ir);
        break;

    case (IRTypeCopyOptItem):
        handle_copy_opt_item(cur_ir);
        break;

    case (IRTypeOptSelect):
        handle_opt_select(cur_ir);
        break;

    case (IRTypeSimpleSelect):
        handle_simple_select(cur_ir);
        break;

    case (IRTypeOptTempTableName):
        handle_opt_temp_table_name(cur_ir);
        break;

    case (IRTypeNameListOptCommaOptBracket):
        handle_name_list_opt_comma_opt_bracket(cur_ir);
        break;

    case (IRTypeSinglePivotValue):
        handle_single_pivot_value(cur_ir);
        break;

    case (IRTypeOptSampleFunc):
        handle_opt_sample_func(cur_ir);
        break;

    case (IRTypeLockedRelsList):
        handle_locked_rels_list(cur_ir);
        break;

    case (IRTypeAliasClause):
        handle_alias_clause(cur_ir);
        break;

    case (IRTypeFuncApplication):
        handle_func_application(cur_ir);
        break;

    case (IRTypeFuncArgExpr):
        handle_func_arg_expr(cur_ir);
        break;

    case (IRTypeJoinQual):
        handle_join_qual(cur_ir);
        break;

    case (IRTypeTableFuncElement):
        handle_table_func_element(cur_ir);
        break;

    case (IRTypeOptCollateClause):
        handle_opt_collate_clause(cur_ir);
        break;

    case (IRTypeFuncAliasClause):
        handle_func_alias_clause(cur_ir);
        break;

    case (IRTypePivotValue):
        handle_pivot_value(cur_ir);
        break;

    case (IRTypeUnpivotHeader):
        handle_unpivot_header(cur_ir);
        break;

    case (IRTypeUnpivotValue):
        handle_unpivot_value(cur_ir);
        break;

    case (IRTypeJoinedTable):
        handle_joined_table(cur_ir);
        break;

    case (IRTypeOptColDefList):
        handle_opt_col_def_list(cur_ir);
        break;

    case (IRTypeColidTypeList):
        handle_colid_type_list(cur_ir);
        break;

    case (IRTypeQualifiedTypename):
        handle_qualified_type_name(cur_ir);
        break;

    case (IRTypeGenericType):
        handle_generic_type(cur_ir);
        break;

    case (IRTypeAExpr):
        handle_a_expr(cur_ir);
        break;

    case (IRTypeColumnrefOptIndirection):
        handle_columnref_opt_indirection(cur_ir);
        break;

    case (IRTypeColumnref):
        handle_columnref(cur_ir);
        break;

    case (IRTypeIndirectionExpr):
        handle_indirection_expr(cur_ir);
        break;

    case (IRTypeExtractArg):
        handle_extract_arg(cur_ir);
        break;

    case (IRTypeOverClause):
        handle_over_clause(cur_ir);
        break;

    case (IRTypeOptExistingWindowName):
        handle_opt_existing_window_name(cur_ir);
        break;

    case (IRTypeDictArg):
        handle_dict_arg(cur_ir);
        break;

    case (IRTypeAnyOperator):
        handle_any_operator(cur_ir);
        break;

    case (IRTypeExtendedIndirectionEl):
        handle_extended_indirection_el(cur_ir);
        break;

    case (IRTypeExceptList):
        handle_except_list(cur_ir);
        break;

    case (IRTypeFuncName):
        handle_func_name(cur_ir);
        break;

    case (IRTypeTypeFunctionName):
        handle_type_function_name(cur_ir);
        break;

    case (IRTypeFunctionNameToken):
        handle_function_name_token(cur_ir);
        break;

    case (IRTypeTypeNameToken):
        handle_type_name_token(cur_ir);
        break;

    case (IRTypeAnyName):
        handle_any_name(cur_ir);
        break;

    case (IRTypeAttrs):
        handle_attrs(cur_ir);
        break;

    case (IRTypePrepareStmt):
        handle_prepare_stmt(cur_ir);
        break;

    case (IRTypeCreateSchemaStmt):
        handle_create_schema_stmt(cur_ir);
        break;

    case (IRTypeIndexStmt):
        handle_index_stmt(cur_ir);
        break;

    case (IRTypeAccessMethod):
        handle_access_method(cur_ir);
        break;

    case (IRTypeAlterObjectSchemaStmt):
        handle_alter_object_schema_stmt(cur_ir);
        break;

    case (IRTypeOptColId):
        handle_opt_col_id(cur_ir);
        break;

    case (IRTypeCommentOnStmt):
        handle_comment_on_stmt(cur_ir);
        break;

    case (IRTypeExportStmt):
        handle_export_stmt(cur_ir);
        break;

    case (IRTypeImportStmt):
        handle_import_stmt(cur_ir);
        break;

    case (IRTypeSetRest):
        handle_set_rest(cur_ir);
        break;

    case (IRTypeGenericSet):
        handle_generic_set(cur_ir);
        break;

    case (IRTypeZoneValue):
        handle_zone_value(cur_ir);
        break;

    case (IRTypeVarValue):
        handle_var_value(cur_ir);
        break;

    case (IRTypeOptExtVersion):
        handle_opt_ext_version(cur_ir);
        break;

    case (IRTypeVacuumStmt):
        handle_vacuum_stmt(cur_ir);
        break;

    case (IRTypeAnalyzeStmt):
        handle_analyze_stmt(cur_ir);
        break;

    case (IRTypeAttachStmt):
        handle_attach_stmt(cur_ir);
        break;

    case (IRTypeDetachStmt):
        handle_detach_stmt(cur_ir);
        break;

    case (IRTypeOptDatabaseAlias):
        handle_opt_database_alias(cur_ir);
        break;

    case (IRTypeIdentList):
        // handle_ident_list(cur_ir);
        break;
    case (IRTypeIdentName):
        // handle_ident_Name(cur_ir);
        break;

    case (IRTypeGenericReset):
        handle_generic_reset(cur_ir);
        break;

    /* Changed to qualified name in the latest update. */
    // case (IRTypeTableId):
    //   handle_table_id(cur_ir);
    //   break;

    case (IRTypeViewStmt):
        handle_view_stmt(cur_ir);
        break;

    default:
        break;
    }

    // IMPORTANT!!!
    for (auto& interesting_type : DuckDBFuzzerConfigurations::duckdb_interesting_ir_types) {
        if (interesting_type == cur_ir_type) {
            cur_ir->set_is_favor(IsFavor::favor);
            break;
        }
    }
}