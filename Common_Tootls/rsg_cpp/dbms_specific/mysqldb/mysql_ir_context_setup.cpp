//
// Created by XXX on 12/11/24.
//

#include "mysql_ir_context_setup.h"
#include "mysql_fuzzer_configurations.h"
#include <initializer_list>

#include "../../headers/fuzzer_configurations.h"

#define BEGIN vector<IR*> children = cur_ir->get_children();

inline void handle_IDENT(RSG* rsg, IR*& cur_ir)
{
    // Just a placeholder.

    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIDENT) {
            auto* new_name = new IR(IRTypeIDENT, string("v00"), DataTableName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_simple_ident(RSG* rsg, IR*& cur_ir, DATATYPE data_type, DATAFLAG context_type);

inline void handle_ident_list(RSG* rsg, IR*& cur_ir, DATATYPE data_type, DATAFLAG context_type)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeSimpleIdent) {
            handle_simple_ident(rsg, child, data_type, context_type);
        } else if (child->get_ir_type() == IRTypeIdentList) {
            handle_ident_list(rsg, child, data_type, context_type);
        }
    }
}

inline void handle_simple_ident(RSG* rsg, IR*& cur_ir, DATATYPE data_type = DataColumnName, DATAFLAG context_type = ContextUse)
{
    cur_ir->free_children();
    auto* new_name = new IR(IRTypeIDENT, string("v00"), data_type, context_type, nullptr);
    cur_ir->add_one_child(new_name, 0);
}

inline void handle_ident_list_arg(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIdentList) {
            handle_ident_list(rsg, child, DataColumnName, ContextUse);
        }
    }
}

inline void handle_simple_ident_q(RSG* rsg, IR*& cur_ir, DATATYPE data_type = DataColumnName, DATAFLAG context_type = ContextUse)
{
    // Not need to handle.
    // Will drop later.
}

inline void handle_simple_ident_nospvar(RSG* rsg, IR*& cur_ir, DATATYPE data_type = DataColumnName, DATAFLAG context_type = ContextUse)
{
    BEGIN;

    cur_ir->free_children();

    auto* new_name = new IR(IRTypeIDENT, string("v00"), data_type, context_type, nullptr);
    cur_ir->add_one_child(new_name, 0);
}

inline void handle_table_wild(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    vector<IR*> new_children;
    if (children.size() == 5) {
        for (int idx = 0; idx < children.size(); idx++) {
            if (idx < 2) {
                children[idx]->deep_drop();
            } else if (children[idx]->get_ir_type() == IRTypeIDENT) {
                children[idx]->deep_drop();
                auto* new_name = new IR(IRTypeIDENT, string("v00"), DataTableName, ContextUse, nullptr);
                new_children.push_back(new_name);
            } else {
                new_children.push_back(children[idx]);
            }
        }
    } else {
        for (auto& child : children) {
            if (child->get_ir_type() == IRTypeIDENT) {
                child->deep_drop();
                auto* new_name = new IR(IRTypeIDENT, string("v00"), DataTableName, ContextUse, nullptr);
                new_children.push_back(new_name);
            } else {
                new_children.push_back(child);
            }
        }
    }

    cur_ir->set_children_nodes(new_children);
}

inline void handle_simple_ident_list(RSG* rsg, IR*& cur_ir, DATATYPE data_type, DATAFLAG context_type)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIDENT) {
            auto* new_name = new IR(IRTypeIDENT, string("v00"), data_type, context_type, nullptr);
            cur_ir->swap_one_child(child, new_name);
        } else if (child->get_ir_type() == IRTypeSimpleIdentList) {
            handle_simple_ident_list(rsg, child, data_type, context_type);
        }
    }
}

inline void handle_opt_derived_column_list(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeSimpleIdentList) {
            handle_simple_ident_list(rsg, child, DataColumnAliasName, ContextDefine);
        }
    }
}

inline void handle_ident_string_list(RSG* rsg, IR*& cur_ir, DATATYPE data_type, DATAFLAG context_type)
{
    BEGIN;
    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIDENT) {
            auto* new_name = new IR(IRTypeIDENT, string("v00"), data_type, context_type, nullptr);
            cur_ir->swap_one_child(child, new_name);
        } else if (child->get_ir_type() == IRTypeIdentStringList) {
            handle_ident_string_list(rsg, child, data_type, context_type);
        }
    }
}

inline void handle_ident_or_empty(RSG* rsg, IR*& cur_ir, DATATYPE data_type, DATAFLAG context_type)
{
    BEGIN;

    string str = "";
    if (data_type == DataDatabaseName) {
        str = "test123";
    } else if (data_type == DataTableName) {
        str = "v00";
    }

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIDENT) {
            auto* new_name = new IR(IRTypeIDENT, str, data_type, context_type, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_using_list(RSG* rsg, IR*& cur_ir, DATATYPE data_type, DATAFLAG context_type)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIdentStringList) {
            handle_ident_string_list(rsg, child, data_type, context_type);
        }
    }
}

inline void handle_table_reference(RSG* rsg, IR*& cur_ir, DATAFLAG context_type);

inline void handle_single_table(RSG* rsg, IR*& cur_ir, DATAFLAG context_type)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeTableIdent) {
            auto* new_name = new IR(IRTypeIDENT, string("v00"), DataTableName, context_type, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_single_table_parens(RSG* rsg, IR*& cur_ir, DATAFLAG context_type)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeSingleTable) {
            handle_single_table(rsg, child, context_type);
        } else if (child->get_ir_type() == IRTypeSingleTableParens) {
            handle_single_table_parens(rsg, child, context_type);
        }
    }
}

inline void handle_derived_table(RSG* rsg, IR*& cur_ir, DATAFLAG context_type)
{
    // No need to handle.
}

inline void handle_jt_column(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIDENT) {
            auto* new_name = new IR(IRTypeIDENT, string("c01"), DataColumnName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_table_function(RSG* rsg, IR*& cur_ir, DATAFLAG context_type)
{
    // No need to handle.
}

inline void handle_table_reference_list(RSG* rsg, IR*& cur_ir, DATAFLAG context_type)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeTableReference) {
            handle_table_reference(rsg, child, context_type);
        } else if (child->get_ir_type() == IRTypeTableReferenceList) {
            handle_table_reference_list(rsg, child, context_type);
        }
    }
}

inline void handle_table_reference_list_parens(RSG* rsg, IR*& cur_ir, DATAFLAG context_type)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeTableReferenceListParens) {
            handle_table_reference_list_parens(rsg, child, context_type);
        } else if (child->get_ir_type() == IRTypeTableReference) {
            handle_table_reference(rsg, child, context_type);
        } else if (child->get_ir_type() == IRTypeTableReferenceList) {
            handle_table_reference_list(rsg, child, context_type);
        }
    }
}

inline void handle_table_factor(RSG* rsg, IR*& cur_ir, DATAFLAG context_type)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeSingleTable) {
            handle_single_table(rsg, child, context_type);
        } else if (child->get_ir_type() == IRTypeSingleTableParens) {
            handle_single_table_parens(rsg, child, context_type);
        } else if (child->get_ir_type() == IRTypeDerivedTable) {
            handle_derived_table(rsg, child, context_type);
        } else if (child->get_ir_type() == IRTypeTableFunction) {
            handle_table_function(rsg, child, context_type);
        } else if (child->get_ir_type() == IRTypeTableReferenceListParens) {
            handle_table_reference_list_parens(rsg, child, context_type);
        // } else if (child->get_ir_type() == IRTypeJoinedTableParens) {
        //     handle_joined_table_parens(rsg, child, context_type);
        }
    }
}

inline void handle_joined_table(RSG* rsg, IR*& cur_ir, DATAFLAG context_type);

inline void handle_joined_table_parens(RSG* rsg, IR*& cur_ir, DATAFLAG context_type)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeJoinedTable) {
            handle_joined_table(rsg, child, context_type);
        } else if (child->get_ir_type() == IRTypeJoinedTableParens) {
            handle_joined_table_parens(rsg, child, context_type);
        }
    }
}

inline void handle_joined_table(RSG* rsg, IR*& cur_ir, DATAFLAG context_type)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeTableFactor) {
            handle_table_factor(rsg, child, context_type);
        } else if (child->get_ir_type() == IRTypeJoinedTableParens) {
            handle_joined_table_parens(rsg, child, context_type);
        } else if (child->get_ir_type() == IRTypeTableReference) {
            handle_table_reference(rsg, child, context_type);
        } else if (child->get_ir_type() == IRTypeUsingList) {
            handle_using_list(rsg, child, DataColumnName, ContextUse);
        }
    }
}

inline void handle_esc_table_reference(RSG* rsg, IR*& cur_ir, DATAFLAG context_type)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeTableFactor) {
            handle_table_factor(rsg, child, context_type);
        } else if (child->get_ir_type() == IRTypeJoinedTable) {
            handle_joined_table(rsg, child, context_type);
        }
    }
}

inline void handle_table_reference(RSG* rsg, IR*& cur_ir, DATAFLAG context_type)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeTableFactor) {
            handle_table_factor(rsg, child, context_type);
        } else if (child->get_ir_type() == IRTypeJoinedTable) {
            handle_joined_table(rsg, child, context_type);
        } else if (child->get_ir_type() == IRTypeEscTableReference) {
            handle_esc_table_reference(rsg, child, context_type);
        }
    }
}

inline void handle_table_element(RSG* rsg, IR*& cur_ir)
{
    // No need to handle.
}

inline void handle_table_element_list(RSG* rsg, IR*& cur_ir)
{
    // No need to handle.
}

inline void handle_deallocate(RSG* rsg, IR*& cur_ir)
{
    BEGIN;
    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIDENT) {
            auto* new_name = new IR(IRTypeIDENT, string("stmt_0"), DataStatementPreparedName, ContextUndefine, child->get_mapped_token_node());

            cur_ir->swap_one_child(child, new_name);
            break;
        }
    }
}

inline void handle_prepare(RSG* rsg, IR*& cur_ir)
{
    BEGIN;
    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIDENT) {
            auto* new_name = new IR(IRTypeIDENT, string("stmt_0"), DataStatementPreparedName, ContextDefine, child->get_mapped_token_node());
            cur_ir->swap_one_child(child, new_name);
            break;
        }
    }
}

inline void handle_prepare_src(RSG* rsg, IR*& cur_ir)
{
    BEGIN;
    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeTEXTSTRINGSys) {
            auto* new_name = new IR(SymbolLit, IRTypeSTRING, string("'select 100'"), child->get_mapped_token_node());
            cur_ir->swap_one_child(child, new_name);
        } else if (child->get_ir_type() == IRTypeIdentOrText) {
            auto* new_name = new IR(IRTypeIDENT, string("stmt_0"), DataStatementPreparedName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_execute(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIDENT) {
            auto* new_name = new IR(IRTypeIDENT, string("stmt_0"), DataStatementPreparedName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_execute_var_ident(RSG* rsg, IR*& cur_ir)
{
    BEGIN;
    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIdentOrText) {
            auto* new_name = new IR(IRTypeIDENT, string("var_0"), DataVariableName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_filter_db_ident(RSG* rsg, IR*& cur_ir)
{
    BEGIN;
    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIDENT) {
            auto* new_name = new IR(IRTypeIDENT, string("test123"), DataDatabaseName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_filter_table_ident(RSG* rsg, IR*& cur_ir)
{
    BEGIN;
    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeSchema) {
            auto* new_name = new IR(IRTypeIDENT, string("test123"), DataTableName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
            continue;
        } else if (child->get_ir_type() == IRTypeIDENT) {
            auto* new_name = new IR(IRTypeIDENT, string("v00"), DataTableName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
            continue;
        }
    }
}

inline void handle_schema(RSG* rsg, IR*& cur_ir)
{
    BEGIN;
    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIDENT) {
            auto* new_name = new IR(IRTypeIDENT, string("test123"), DataDatabaseName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_role(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    if (children.size() == 1) {
        return;
    }

    cur_ir->free_children();
    auto* new_name = new IR(IRTypeCURRENTUSER, string("CURRENT_USER"), DataUnknownType, ContextUse, nullptr);
    cur_ir->add_one_child(new_name, 0);
}

inline void handle_role_ident_or_text(RSG* rsg, IR*& cur_ir)
{
    cur_ir->free_children();
    auto* new_name = new IR(IRTypeCURRENTUSER, string("CURRENT_USER"), DataUnknownType, ContextUse, nullptr);
    cur_ir->add_one_child(new_name, 0);
}

inline void handle_user(RSG* rsg, IR*& cur_ir)
{

    BEGIN;

    if (children.size() == 1) {
        return;
    }

    cur_ir->free_children();

    auto* new_name = new IR(IRTypeCURRENTUSER, string("CURRENT_USER"), DataUnknownType, ContextUse, nullptr);
    cur_ir->add_one_child(new_name, 0);

    return;
}

inline void handle_user_ident_or_text(RSG* rsg, IR*& cur_ir)
{
    cur_ir->free_children();

    auto* new_name = new IR(IRTypeCURRENTUSER, string("CURRENT_USER"), DataUnknownType, ContextUse, nullptr);
    cur_ir->add_one_child(new_name, 0);

    return;
}

inline void handle_source_tls_ciphersuites_def(RSG* rsg, IR*& cur_ir)
{
    cur_ir->free_children();

    auto* new_name = new IR(SymbolTerm, IRTypeNULLSYM, string("NULL"), nullptr, nullptr, nullptr);
    cur_ir->add_one_child(new_name, 0);

    return;
}

inline void handle_create_table_stmt(RSG* rsg, IR*& cur_ir)
{
    BEGIN;
    bool is_first_ident = true;
    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeTableIdent) {
            if (is_first_ident) {
                auto* new_name = new IR(IRTypeIDENT, string("v00"), DataTableName, ContextDefine, nullptr);
                cur_ir->swap_one_child(child, new_name);
                is_first_ident = false;
            } else {
                auto* new_name = new IR(IRTypeIDENT, string("v01"), DataTableName, ContextUseTop, nullptr);
                cur_ir->swap_one_child(child, new_name);
            }
        }
    }
}

inline void handle_column_def(RSG* rsg, IR*& cur_ir)
{
    BEGIN;
    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIDENT) {
            auto* new_name = new IR(IRTypeIDENT, string("c01"), DataColumnName, ContextDefine, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_field_def(RSG* rsg, IR*& cur_ir)
{
    // No need to handle.
}

inline void handle_create_resource_group_stmt(RSG* rsg, IR*& cur_ir)
{
    BEGIN;
    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIDENT) {
            auto* new_name = new IR(SymbolTerm, IRTypeUnknownType, string("USR_default"), nullptr, nullptr, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_create_index_stmt(RSG* rsg, IR*& cur_ir)
{
    BEGIN;
    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIDENT) {
            auto* new_name = new IR(IRTypeIDENT, string("idx_03"), DataIndexName, ContextDefine, nullptr);
            cur_ir->swap_one_child(child, new_name);
        } else if (child->get_ir_type() == IRTypeTableIdent) {
            auto* new_name = new IR(IRTypeIDENT, string("v00"), DataTableName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_server_option(RSG* rsg, IR*& cur_ir)
{
    // give up.
    return;
}

inline void handle_create(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    DATATYPE data_type = DataDatabaseName;
    string str = "";

    if (children[1]->get_ir_type() == IRTypeDATABASE) {
        data_type = DataDatabaseName;
        str = "test123";
    } else if (children[1]->get_ir_type() == IRTypeViewOrTriggerOrSpOrEvent) {
        data_type = DataViewName;
        str = "view_0";
    } else if (children[1]->get_ir_type() == IRTypeLOGFILESYM) {
        data_type = DataUnknownType; // give up.
        str = "logfile_0";
    } else if (children[1]->get_ir_type() == IRTypeTABLESPACESYM) {
        data_type = DataTableSpaceName;
        str = "tablespace_0";
    } else if (children[1]->get_ir_type() == IRTypeUNDOSYM) {
        data_type = DataUnknownType;
        str = "tablespace_0";
    } else if (children[1]->get_ir_type() == IRTypeSERVERSYM) {
        data_type = DataServerName;
        str = "localhost";
    }

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIDENT || child->get_ir_type() == IRTypeIdentOrText) {
            auto* new_name = new IR(IRTypeIDENT, str, data_type, ContextDefine, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_event_tail(RSG* rsg, IR*& cur_ir)
{

    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeSpName) {
            auto* new_name = new IR(IRTypeIDENT, string("procedure_0"), DataEventName, ContextDefine, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_sp_chistic(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIDENT) {
            auto* new_name = new IR(SymbolTerm, IRTypeSQLSYM, string("SQL"), nullptr, nullptr, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_call_stmt(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeSpName) {
            auto* new_name = new IR(IRTypeIDENT, string("procedure_0"), DataEventName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_sp_fdparam(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    // Not accurate.
    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIDENT) {
            auto* new_name = new IR(IRTypeIDENT, string("c_alias_01"), DataColumnAliasName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_sp_pdparam(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    // Not accurate.
    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIDENT) {
            auto* new_name = new IR(IRTypeIDENT, string("c_alias_01"), DataColumnAliasName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_sp_decl(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIDENT) {
            auto* new_name = new IR(IRTypeIDENT, string("c_alias_01"), DataColumnAliasName, ContextDefine, nullptr);
            cur_ir->swap_one_child(child, new_name);
        } else if (child->get_ir_type() == IRTypeType) {
            auto* new_name = new IR(IRTypeIDENT, string("INT"), DataTypeName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_sp_hcond(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIDENT) {
            auto* new_name = new IR(IRTypeIDENT, string("condition_name_1"), DataUnknownType, ContextDefine, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_signal_value(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIDENT) {
            auto* new_name = new IR(IRTypeIDENT, string("condition_name_1"), DataUnknownType, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_signal_allowed_expr(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeSimpleIdent) {
            auto* new_name = new IR(IRTypeIDENT, string("condition_name_1"), DataUnknownType, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_simple_target_specification(RSG* rsg, IR*& cur_ir)
{

    cur_ir->free_children();

    auto* new_name = new IR(IRTypeIDENT, string("target_name_1"), DataUnknownType, ContextUse, nullptr);
    cur_ir->add_one_child(new_name, 0);
}

inline void handle_condition_information_item(RSG* rsg, IR*& cur_ir)
{

    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeConditionInformationItemName) {
            auto* new_name = new IR(IRTypeIDENT, string("target_name_1"), DataUnknownType, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_sp_proc_stmt_leave(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIDENT) {
            auto* new_name = new IR(IRTypeIDENT, string("label_name_1"), DataUnknownType, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_sp_proc_stmt_iterate(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIDENT) {
            auto* new_name = new IR(IRTypeIDENT, string("label_name_1"), DataUnknownType, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_sp_proc_stmt_open(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIDENT) {
            auto* new_name = new IR(IRTypeIDENT, string("label_name_1"), DataUnknownType, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_sp_proc_stmt_fetch(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIDENT) {
            auto* new_name = new IR(IRTypeIDENT, string("cursor_name_0"), DataCursorName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_sp_proc_stmt_close(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIDENT) {
            auto* new_name = new IR(IRTypeIDENT, string("cursor_name_0"), DataCursorName, ContextUndefine, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_sp_fetch_list(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIDENT) {
            auto* new_name = new IR(IRTypeIDENT, string("var_0"), DataVariableName, ContextDefine, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_sp_labeled_control(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeLabelIdent) {
            auto* new_name = new IR(IRTypeIDENT, string("label_name_1"), DataUnknownType, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_sp_opt_label(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeLabelIdent) {
            auto* new_name = new IR(IRTypeIDENT, string("label_name_1"), DataUnknownType, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_sp_labeled_block(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeLabelIdent) {
            auto* new_name = new IR(IRTypeIDENT, string("label_name_1"), DataUnknownType, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_opt_logfile_group_name(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIDENT) {
            auto* new_name = new IR(IRTypeIDENT, string("logfile_group_name_1"), DataUnknownType, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_size_number(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIDENTSys) {
            auto* new_name = new IR(SymbolLit, IRTypeINTEGER, string("1024"), nullptr, nullptr, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_name_list(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIDENT) {
            auto* new_name = new IR(IRTypeIDENT, string("c01"), DataColumnName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_part_definition(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIDENT) {
            auto* new_name = new IR(IRTypeIDENT, string("partition_0"), DataPartitionName, ContextDefine, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_sub_part_definition(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIdentOrText) {
            auto* new_name = new IR(IRTypeIDENT, string("c01"), DataPartitionName, ContextDefine, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_part_option(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIDENT) {
            auto* new_name = new IR(IRTypeIDENT, string("tablespace_0"), DataTableSpaceName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        } else if (child->get_ir_type() == IRTypeIdentOrText) {
            auto* new_name = new IR(IRTypeIDENT, string("InnoDB"), DataUnknownType, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_create_table_option(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    DATATYPE data_type = DataUnknownType;
    string str = "";

    if (children.front()->get_ir_type() == IRTypeENGINESYM || children.front()->get_ir_type() == IRTypeSECONDARYENGINESYM) {
        data_type = DataUnknownType;
        str = "InnoDB";
    } else if (children.front()->get_ir_type() == IRTypeTABLESPACESYM) {
        data_type = DataTableSpaceName;
        str = "tablespace_0";
    }

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIDENT || child->get_ir_type() == IRTypeIdentOrText) {
            auto* new_name = new IR(IRTypeIDENT, str, data_type, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_table_list(RSG* rsg, IR*& cur_ir, DATATYPE data_type, DATAFLAG context_type)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeTableIdent) {
            auto* new_name = new IR(IRTypeIDENT, string("v00"), data_type, context_type, nullptr);
            cur_ir->swap_one_child(child, new_name);
        } else if (child->get_ir_type() == IRTypeTableList) {
            handle_table_list(rsg, child, data_type, context_type);
        }
    }
}

inline void handle_opt_table_list(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeTableList) {
            handle_table_list(rsg, child, DataTableName, ContextUse);
        }
    }
}

inline void handle_default_charset(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeCharsetName) {
            auto* new_name = new IR(IRTypeIDENT, string("latin1"), DataCharsetName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_default_collation(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeCollationName) {
            auto* new_name = new IR(IRTypeIDENT, string("latin1_bin"), DataCollationName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_opt_ident(RSG* rsg, IR*& cur_ir, DATATYPE data_type, DATAFLAG context_type)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIDENT) {
            auto* new_name = new IR(IRTypeIDENT, string("index_name_1"), data_type, context_type, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_opt_index_name_and_type(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIDENT) {
            auto* new_name = new IR(IRTypeIDENT, string("index_name_1"), DataIndexName, ContextDefine, nullptr);
            cur_ir->swap_one_child(child, new_name);
        } else if (child->get_ir_type() == IRTypeOptIdent) {
            handle_opt_ident(rsg, child, DataIndexName, ContextDefine);
        }
    }
}

inline void handle_table_constraint_def(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    DATATYPE data_type = DataConstraintName;
    DATAFLAG context_type = ContextDefine;

    // if (children.front() ->get_ir_type() == IRTypeOptConstraintName) {
    //     data_type = DataTableName;
    //     context_type = ContextUse;
    // }

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeOptIdent) {
            handle_opt_ident(rsg, child, data_type, context_type);
        }
    }
}

inline void handle_opt_constraint_name(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeOptIdent) {
            handle_opt_ident(rsg, child, DataConstraintName, ContextDefine);
        }
    }
}

inline void handle_collation_name(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIdentOrText) {
            auto* new_name = new IR(IRTypeIDENT, string("latin1_bin"), DataCollationName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_character_set(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIdentOrText) {
            auto* new_name = new IR(IRTypeIDENT, string("latin1"), DataCharsetName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_old_or_new_charset_name(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIdentOrText) {
            auto* new_name = new IR(IRTypeIDENT, string("latin1"), DataCharsetName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_references(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeTableIdent) {
            auto* new_name = new IR(IRTypeIDENT, string("v00"), DataTableName, ContextUseTop, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_reference_list(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIDENT) {
            auto* new_name = new IR(IRTypeIDENT, string("c01"), DataColumnName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        } else if (child->get_ir_type() == IRTypeReferenceList) {
            handle_reference_list(rsg, child);
        }
    }
}

inline void handle_key_part(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIDENT) {
            auto* new_name = new IR(IRTypeIDENT, string("c01"), DataColumnName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_alter_table_stmt(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeTableIdent) {
            auto* new_name = new IR(IRTypeIDENT, string("v00"), DataTableName, ContextUseTop, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_alter_procedure_stmt(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeSpName) {
            auto* new_name = new IR(IRTypeIDENT, string("procedure_0"), DataEventName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_alter_function_stmt(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeSpName) {
            auto* new_name = new IR(IRTypeIDENT, string("function_0"), DataFunctionName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_opt_derived_column_list(RSG* rsg, IR*& cur_ir, DATATYPE data_type, DATAFLAG context_type);

inline void handle_view_tail(RSG* rsg, IR*& cur_ir, DATAFLAG context_type)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeTableIdent) {
            auto* new_name = new IR(IRTypeIDENT, string("view_0"), DataViewName, context_type, nullptr);
            cur_ir->swap_one_child(child, new_name);
        } else if (child->get_ir_type() == IRTypeOptDerivedColumnList) {
            handle_opt_derived_column_list(rsg, child, DataViewColumnName, context_type);
        }
    }
}

inline void handle_opt_derived_column_list(RSG* rsg, IR*& cur_ir, DATATYPE data_type, DATAFLAG context_type)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeSimpleIdentList) {
            handle_simple_ident_list(rsg, child, data_type, context_type);
        }
    }
}

inline void handle_alter_view_stmt(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeViewTail) {
            handle_view_tail(rsg, child, ContextUse);
        }
    }
}

inline void handle_alter_event_stmt(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeSpName) {
            auto* new_name = new IR(IRTypeIDENT, string("event_0"), DataEventName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_opt_ev_rename_to(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeSpName) {
            auto* new_name = new IR(IRTypeIDENT, string("event_0"), DataEventName, ContextDefine, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_alter_logfile_stmt(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIDENT) {
            auto* new_name = new IR(IRTypeIDENT, string("logfile_0"), DataUnknownType, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_alter_tablespace_stmt(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    // Handle even the rename. Just rename to the same name then.
    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIDENT) {
            auto* new_name = new IR(IRTypeIDENT, string("tablespace_0"), DataTableSpaceName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_alter_undo_tablespace_stmt(RSG* rsg, IR*& cur_ir)
{
    handle_alter_tablespace_stmt(rsg, cur_ir);
}

inline void handle_alter_server_stmt(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIdentOrText) {
            auto* new_name = new IR(IRTypeIDENT, string("localhost"), DataServerName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_alter_resource_group_stmt(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIDENT) {
            auto* new_name = new IR(IRTypeIDENT, string("resource_group_0"), DataUnknownType, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_all_or_alt_part_name_list(RSG* rsg, IR*& cur_ir)
{
    cur_ir->free_children();

    auto* new_name = new IR(SymbolTerm, IRTypeALL, string("all"), nullptr, nullptr, nullptr);
    cur_ir->add_one_child(new_name, 0);

    return;
}

inline void handle_standalone_alter_commands(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    if (children.front()->get_ir_type() == IRTypeDROP && children.back()->get_ir_type() == IRTypeIdentStringList) {
        handle_ident_string_list(rsg, children.back(), DataPartitionName, ContextUndefine);
    } else if (children.front()->get_ir_type() == IRTypeEXCHANGESYM) {
        for (auto& child : children) {
            if (child->get_ir_type() == IRTypeIDENT) {
                auto* new_name = new IR(IRTypeIDENT, string("partition_0"), DataPartitionName, ContextUse, nullptr);
                cur_ir->swap_one_child(child, new_name);
            } else if (child->get_ir_type() == IRTypeTableIdent) {
                auto* new_name = new IR(IRTypeIDENT, string("table_0"), DataTableName, ContextUse, nullptr);
                cur_ir->swap_one_child(child, new_name);
            }
        }
    } else if (children.size() > 4 && children.front()->get_ir_type() == IRTypeREORGANIZESYM && children[3]->get_ir_type() == IRTypeIdentStringList) {
        handle_ident_string_list(rsg, children[3], DataPartitionName, ContextUse);
    }
}

inline void handle_alter_list_item(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    // ADD opt_column ident field_def opt_references opt_place
    if (children.size() > 4 && children.front()->get_ir_type() == IRTypeADD && children[1]->get_ir_type() == IRTypeOptColumn
        && children[2]->get_ir_type() == IRTypeIDENT && children[3]->get_ir_type() == IRTypeFieldDef) {
        auto* new_name = new IR(IRTypeIDENT, string("c01"), DataColumnName, ContextDefine, nullptr);
        cur_ir->swap_one_child(children[2], new_name);
    }

    // CHANGE opt_column ident ident field_def opt_place
    else if (children.size() > 4 && children.front()->get_ir_type() == IRTypeCHANGE && children[1]->get_ir_type() == IRTypeOptColumn && children[2]->get_ir_type() == IRTypeIDENT && children[3]->get_ir_type() == IRTypeIDENT) {
        auto* new_name = new IR(IRTypeIDENT, string("c01"), DataColumnName, ContextReplaceUndefine, nullptr);
        cur_ir->swap_one_child(children[2], new_name);

        new_name = new IR(IRTypeIDENT, string("c01"), DataColumnName, ContextReplaceDefine, nullptr);
        cur_ir->swap_one_child(children[3], new_name);
    }

    // MODIFY_SYM opt_column ident field_def opt_place
    else if (children.size() > 3 && children.front()->get_ir_type() == IRTypeMODIFYSYM) {
        auto* new_name = new IR(IRTypeIDENT, string("c01"), DataColumnName, ContextUse, nullptr);
        cur_ir->swap_one_child(children[2], new_name);
    }

    else if (children.size() > 2 && children.front()->get_ir_type() == IRTypeDROP) {

        // DROP opt_column ident opt_restrict
        if (children[1]->get_ir_type() == IRTypeOptColumn) {
            auto* new_name = new IR(IRTypeIDENT, string("c01"), DataColumnName, ContextUndefine, nullptr);
            cur_ir->swap_one_child(children[2], new_name);
        }

        // DROP FOREIGN KEY_SYM ident
        else if (children[1]->get_ir_type() == IRTypeFOREIGN) {
            auto* new_name = new IR(IRTypeIDENT, string("c01"), DataForeignKeyName, ContextUndefine, nullptr);
            cur_ir->swap_one_child(children[3], new_name);
        }

        // DROP CHECK_SYM ident
        // DROP CONSTRAINT ident
        else if (children[1]->get_ir_type() == IRTypeCHECKSYM || children[1]->get_ir_type() == IRTypeCONSTRAINT) {
            auto* new_name = new IR(IRTypeIDENT, string("c01"), DataConstraintName, ContextUndefine, nullptr);
            cur_ir->swap_one_child(children[3], new_name);
        }

    }

    // ALTER opt_column ident SET_SYM DEFAULT_SYM signed_literal_or_null
    // ALTER opt_column ident SET_SYM DEFAULT_SYM '(' expr ')'
    // ALTER opt_column ident DROP DEFAULT_SYM
    // ALTER opt_column ident SET_SYM visibility
    else if (children.size() > 3 && children.front()->get_ir_type() == IRTypeALTER && children[1]->get_ir_type() == IRTypeOptColumn && children[2]->get_ir_type() == IRTypeIDENT) {
        auto* new_name = new IR(IRTypeIDENT, string("c01"), DataColumnName, ContextUse, nullptr);
        cur_ir->swap_one_child(children[2], new_name);
    }

    // ALTER INDEX_SYM ident visibility
    else if (children.size() > 2 && children.front()->get_ir_type() == IRTypeALTER && children[1]->get_ir_type() == IRTypeINDEXSYM) {
        auto* new_name = new IR(IRTypeIDENT, string("index_0"), DataIndexName, ContextUse, nullptr);
        cur_ir->swap_one_child(children[2], new_name);
    }

    //| ALTER CHECK_SYM ident constraint_enforcement
    //| ALTER CONSTRAINT ident constraint_enforcement
    else if (children.size() > 2 && children.front()->get_ir_type() == IRTypeALTER && (children[1]->get_ir_type() == IRTypeCHECKSYM || children[1]->get_ir_type() == IRTypeCONSTRAINT)) {
        auto* new_name = new IR(IRTypeIDENT, string("c01"), DataConstraintName, ContextUse, nullptr);
        cur_ir->swap_one_child(children[2], new_name);
    }

    // RENAME opt_to table_ident
    else if (children.size() == 3 && children.front()->get_ir_type() == IRTypeRENAME && children[1]->get_ir_type() == IRTypeOptTo) {
        auto* new_name = new IR(IRTypeIDENT, string("table_0"), DataTableName, ContextDefine, nullptr);
        cur_ir->swap_one_child(children[2], new_name);
    }

    //| RENAME key_or_index ident TO_SYM ident
    else if (children.size() > 3 && children.front()->get_ir_type() == IRTypeRENAME && children[1]->get_ir_type() == IRTypeKeyOrIndex) {
        bool is_first_ident = true;
        for (auto& child : children) {
            if (child->get_ir_type() == IRTypeIDENT) {
                if (is_first_ident) {
                    auto* new_name = new IR(IRTypeIDENT, string("index_0"), DataIndexName, ContextUndefine, nullptr);
                    cur_ir->swap_one_child(child, new_name);
                    is_first_ident = false;
                } else {
                    auto* new_name = new IR(IRTypeIDENT, string("index_0"), DataIndexName, ContextDefine, nullptr);
                    cur_ir->swap_one_child(child, new_name);
                }
            }
        }
    }

    //| RENAME COLUMN_SYM ident TO_SYM ident
    else if (children.size() > 3 && children.front()->get_ir_type() == IRTypeRENAME && children[1]->get_ir_type() == IRTypeCOLUMNSYM) {
        bool is_first_ident = true;
        for (auto& child : children) {
            if (child->get_ir_type() == IRTypeIDENT) {
                if (is_first_ident) {
                    auto* new_name = new IR(IRTypeIDENT, string("c01"), DataColumnName, ContextUndefine, nullptr);
                    cur_ir->swap_one_child(child, new_name);
                    is_first_ident = false;
                } else {
                    auto* new_name = new IR(IRTypeIDENT, string("c02"), DataColumnName, ContextDefine, nullptr);
                    cur_ir->swap_one_child(child, new_name);
                }
            }
        }
    }

    else if (children.size() > 4 && children.front()->get_ir_type() == IRTypeCONVERTSYM) {
        for (auto& child : children) {
            if (child->get_ir_type() == IRTypeCharsetName) {
                auto* new_name = new IR(IRTypeIDENT, string("latin_1"), DataCharsetName, ContextUse, nullptr);
                cur_ir->swap_one_child(child, new_name);
            }
        }
    }

    // Finally!!!
}

inline void handle_opt_place(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIDENT) {
            auto* new_name = new IR(IRTypeIDENT, string("c01"), DataColumnName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_alter_algorithm_option_value(RSG* rsg, IR*& cur_ir)
{
    cur_ir->free_children();

    auto* new_name = new IR(SymbolTerm, IRTypeDEFAULTSYM, string("DEFAULT"), nullptr, nullptr, nullptr);
    cur_ir->add_one_child(new_name, 0);

    return;
}

inline void handle_alter_lock_option_value(RSG* rsg, IR*& cur_ir)
{
    cur_ir->free_children();

    auto* new_name = new IR(SymbolTerm, IRTypeDEFAULTSYM, string("DEFAULT"), nullptr, nullptr, nullptr);
    cur_ir->add_one_child(new_name, 0);

    return;
}

inline void handle_group_replication_user(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeTEXTSTRINGSysNonewline) {
            auto* new_name = new IR(IRTypeIDENT, string("mysql"), DataRoleName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_group_replication_plugin_auth(RSG* rsg, IR*& cur_ir)
{
    // give up...
}

inline void handle_opt_user_option(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeTEXTSTRINGSys) {
            auto* new_name = new IR(IRTypeIDENT, string("mysql"), DataRoleName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_opt_password_option(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeTEXTSTRINGSys) {
            auto* new_name = new IR(IRTypeIDENT, string("''"), DataUnknownType, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_opt_plugin_auth_option(RSG* rsg, IR*& cur_ir)
{
    // give up...
}

inline void handle_opt_plugin_dir_option(RSG* rsg, IR*& cur_ir)
{
    // give up...
}

inline void handle_checksum(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeTableList) {
            handle_table_list(rsg, child, DataTableName, ContextUse);
        }
    }
}

inline void handle_repair_table_stmt(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeTableList) {
            handle_table_list(rsg, child, DataTableName, ContextUse);
        }
    }
}

inline void handle_analyze_table_stmt(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeTableList) {
            handle_table_list(rsg, child, DataTableName, ContextUse);
        }
    }
}

inline void handle_opt_histogram(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIdentStringList) {
            handle_ident_string_list(rsg, child, DataColumnName, ContextUse);
        }
    }
}

inline void handle_check_table_stmt(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeTableList) {
            handle_table_list(rsg, child, DataTableName, ContextUse);
        }
    }
}

inline void handle_optimize_table_stmt(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeTableList) {
            handle_table_list(rsg, child, DataTableName, ContextUse);
        }
    }
}

inline void handle_rename(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeUser) {
            auto* new_name = new IR(IRTypeIDENT, string("mysql"), DataRoleName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_table_to_table(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    bool is_first_ident = true;
    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeTableIdent) {
            if (is_first_ident) {
                auto* new_name = new IR(IRTypeIDENT, string("table_0"), DataTableName, ContextReplaceUndefine, nullptr);
                cur_ir->swap_one_child(child, new_name);
                is_first_ident = false;
            } else {
                auto* new_name = new IR(IRTypeIDENT, string("table_0"), DataTableName, ContextReplaceDefine, nullptr);
                cur_ir->swap_one_child(child, new_name);
            }
        }
    }
}

inline void handle_preload_stmt(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeTableIdent) {
            auto* new_name = new IR(IRTypeIDENT, string("table_0"), DataTableName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_preload_keys(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeTableIdent) {
            auto* new_name = new IR(IRTypeIDENT, string("table_0"), DataTableName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_key_usage_element(RSG* rsg, IR*& cur_ir)
{

    cur_ir->free_children();

    auto* new_name = new IR(SymbolTerm, IRTypePRIMARYSYM, string("PRIMARY"), nullptr, nullptr, nullptr);
    cur_ir->add_one_child(new_name, 0);
}

inline void handle_table_ident_opt_wild(RSG* rsg, IR*& cur_ir, DATATYPE data_type = DataTableName, DATAFLAG context_type = ContextUse)
{
    BEGIN;

    // Maybe wrong?
    vector<IR*> new_children;
    if (children.size() == 4) {
        for (int idx = 0; idx < children.size(); ++idx) {
            if (idx <= 1) {
                children[idx]->deep_drop();
            } else {
                if (children[idx]->get_ir_type() == IRTypeIDENT) {
                    children[idx]->deep_drop();
                    // Always table_name?
                    auto* new_name = new IR(IRTypeIDENT, string("c01"), data_type, context_type, nullptr);
                    new_children.push_back(new_name);
                } else {
                    new_children.push_back(children[idx]);
                }
            }
        }
    } else {
        for (auto& child : children) {
            if (child->get_ir_type() == IRTypeIDENT) {
                child->deep_drop();
                auto* new_name = new IR(IRTypeIDENT, string("v00"), data_type, context_type, nullptr);
                new_children.push_back(new_name);
            } else {
                new_children.push_back(child);
            }
        }
    }

    cur_ir->set_children_nodes(new_children);
}

inline void handle_into_clause(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeTEXTSTRINGFilesystem) {
            auto* new_name = new IR(IRTypeIDENT, string("'./abc'"), DataUnknownType, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_select_var_ident(RSG* rsg, IR*& cur_ir)
{
    cur_ir->free_children();

    auto* new_name = new IR(IRTypeIDENT, string("@var"), DataVariableName, ContextDefine, nullptr);
    cur_ir->add_one_child(new_name, 0);

    return;
}

inline void handle_explicit_table(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeTableIdent) {
            auto* new_name = new IR(IRTypeIDENT, string("table_0"), DataTableName, ContextUseTop, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_use_partition(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeUsingList) {
            handle_using_list(rsg, child, DataPartitionName, ContextUse);
        }
    }
}

inline void handle_opt_table_alias(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIDENT) {
            auto* new_name = new IR(IRTypeIDENT, string("v00"), DataTableAliasName, ContextDefine, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

// inline void handle_sampling_percentage(RSG* rsg, IR*& cur_ir)
// {
//     BEGIN;

//     if (!children.empty() && children.front()->get_ir_type() != IRTypeParamMarker && children.front()->get_ir_type() != IRTypeNUMLiteral) {
//         cur_ir->free_children();
//         auto* new_name = new IR(SymbolLit, IRTypeINTEGER, string("100"), nullptr, nullptr, nullptr);
//         cur_ir->add_one_child(new_name, 0);
//         return;
//     }
// }

inline void handle_from_tables(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeTableReferenceList) {
            handle_table_reference_list(rsg, child, ContextUseTop);
        }
    }
}

inline void handle_select_alias(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIDENT || child->get_ir_type() == IRTypeTEXTSTRINGValidated) {
            auto* new_name = new IR(IRTypeIDENT, string("ta_0"), DataColumnAliasName, ContextDefine, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_function_call_generic(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIDENT) {
            auto* new_name = new IR(IRTypeIDENT, string("SUM"), DataFunctionName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }

    cur_ir->set_data_type(DataFunctionExpr);
}

inline void handle_rvalue_system_or_user_variable(RSG* rsg, IR*& cur_ir)
{
    cur_ir->free_children();

    auto* new_name = new IR(IRTypeIDENT, string("@var"), DataVariableName, ContextUse, nullptr);
    cur_ir->add_one_child(new_name, 0);

    return;
}

inline void handle_in_expression_user_variable_assignment(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIdentOrText) {
            auto* new_name = new IR(IRTypeIDENT, string("var_0"), DataVariableName, ContextUse, nullptr); // no need to add @ here.
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_simple_expr(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    // simple_expr COLLATE_SYM ident_or_text %prec NEG
    if (children.size() > 3 && children[1]->get_ir_type() == IRTypeCOLLATESYM) {
        auto* new_name = new IR(IRTypeIDENT, string("latin1_bin"), DataCollationName, ContextUse, nullptr);
        cur_ir->swap_one_child(children[2], new_name);
    }

    else if (children.size() == 4 && children[1]->get_ir_type() == IRTypeIDENT && children[2]->get_ir_type() == IRTypeExpr) {
        auto* new_name = new IR(IRTypeIDENT, string("var_0"), DataVariableName, ContextDefine, nullptr);
        cur_ir->swap_one_child(children[1], new_name);
    }

    else if (children.front()->get_ir_type() == IRTypeMATCH) {
        handle_ident_list_arg(rsg, children[1]);
    }
}

inline void handle_window_name(RSG* rsg, IR*& cur_ir, DATAFLAG data_flag = ContextUse)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIDENT) {
            auto* new_name = new IR(IRTypeIDENT, string("window_name_0"), DataWindowName, data_flag, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_charset_name(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIdentOrText) {
            auto* new_name = new IR(IRTypeIDENT, string("BINARY"), DataCharsetName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_window_definition(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeWindowName) {
            handle_window_name(rsg, child, ContextDefine);
        }
    }
}

inline void handle_limit_option(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIDENT) {
            cur_ir->free_children();
            auto* new_name = new IR(SymbolLit, IRTypeINTEGER, string("100"), nullptr, nullptr, nullptr);
            new_name->set_data_affinity_type(AFFIINT);
            cur_ir->add_one_child(new_name, 0);
            return;
        }
    }
}

inline void handle_common_table_expr(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIDENT) {
            auto* new_name = new IR(IRTypeIDENT, string("ta_01"), DataTableAliasName, ContextDefine, nullptr);
            cur_ir->swap_one_child(child, new_name);
        } else if (child->get_ir_type() == IRTypeOptDerivedColumnList) {
            handle_opt_derived_column_list(rsg, child);
        }
    }
}

inline void handle_table_alias_ref_list(RSG* rsg, IR*& cur_ir, DATATYPE data_type, DATAFLAG context_type)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeTableIdentOptWild) {
            handle_table_ident_opt_wild(rsg, child, DataTableName, ContextUse);
        } else if (child->get_ir_type() == IRTypeTableAliasRefList) {
            handle_table_alias_ref_list(rsg, child, DataTableName, ContextUse);
        }
    }
}

inline void handle_delete_stmt(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeTableIdent) {
            auto* new_name = new IR(IRTypeIDENT, string("v00"), DataTableName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        } else if (child->get_ir_type() == IRTypeTableReferenceList) {
            handle_table_reference_list(rsg, child, ContextUseTop);
        } else if (child->get_ir_type() == IRTypeTableAliasRefList) {
            handle_table_alias_ref_list(rsg, child, DataTableName, ContextUseTop);
        }
    }
}

inline void handle_drop_table_stmt(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeTableList) {
            handle_table_list(rsg, child, DataTableName, ContextUndefine);
        }
    }
}

inline void handle_drop_index_stmt(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIDENT) {
            auto* new_name = new IR(IRTypeIDENT, string("index_0"), DataIndexName, ContextUndefine, nullptr);
            cur_ir->swap_one_child(child, new_name);
        } else if (child->get_ir_type() == IRTypeTableIdent) {
            auto* new_name = new IR(IRTypeIDENT, string("table_0"), DataTableName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_drop_function_stmt(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    if (children.size() == 4) {
        for (auto& child : children) {
            if (child->get_ir_type() == IRTypeIDENT) {
                auto* new_name = new IR(IRTypeIDENT, string("func_0"), DataFunctionName, ContextUndefine, nullptr);
                cur_ir->swap_one_child(child, new_name);
            }
        }
    } else {
        bool is_first_name = true;
        for (auto& child : children) {
            if (child->get_ir_type() == IRTypeIDENT) {
                if (is_first_name) {
                    is_first_name = false;
                    auto* new_name = new IR(IRTypeIDENT, string("test123"), DataDatabaseName, ContextUse, nullptr);
                    cur_ir->swap_one_child(child, new_name);
                } else {
                    auto* new_name = new IR(IRTypeIDENT, string("func_0"), DataFunctionName, ContextUndefine, nullptr);
                    cur_ir->swap_one_child(child, new_name);
                }
            }
        }
    }
}

inline void handle_drop_resource_group_stmt(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIDENT) {
            auto* new_name = new IR(IRTypeIDENT, string("resource_group_0"), DataUnknownType, ContextUndefine, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_drop_procedure_stmt(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIDENT) {
            auto* new_name = new IR(IRTypeIDENT, string("procedure_0"), DataEventName, ContextUndefine, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_drop_user_stmt(RSG* rsg, IR*& cur_ir)
{
    cur_ir->free_children();
    return;
}

inline void handle_drop_view_stmt(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeTableList) {
            handle_table_list(rsg, child, DataViewName, ContextUndefine);
        }
    }
}

inline void handle_drop_event_stmt(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeSpName) {
            auto* new_name = new IR(IRTypeIDENT, string("event_0"), DataEventName, ContextUndefine, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_drop_trigger_stmt(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeSpName) {
            auto* new_name = new IR(IRTypeIDENT, string("trigger_0"), DataTriggerName, ContextUndefine, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_drop_tablespace_stmt(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIDENT) {
            auto* new_name = new IR(IRTypeIDENT, string("tablespace_0123"), DataUnknownType, ContextUndefine, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_drop_undo_tablespace_stmt(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIDENT) {
            auto* new_name = new IR(IRTypeIDENT, string("tablespace_0123"), DataUnknownType, ContextUndefine, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_drop_logfile_stmt(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIDENT) {
            auto* new_name = new IR(IRTypeIDENT, string("logfile_0"), DataUnknownType, ContextUndefine, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_drop_server_stmt(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIdentOrText) {
            auto* new_name = new IR(IRTypeIDENT, string("server_0"), DataUnknownType, ContextUndefine, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_drop_role_stmt(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeRoleList) {
            auto* new_name = new IR(IRTypeIDENT, string("role_0"), DataRoleName, ContextUndefine, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_insert_stmt(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeTableIdent) {
            auto* new_name = new IR(IRTypeIDENT, string("table_0"), DataTableName, ContextUseTop, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_opt_values_reference(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIDENT) {
            auto* new_name = new IR(IRTypeIDENT, string("var_0"), DataColumnAliasName, ContextDefine, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_replace_stmt(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeTableIdent) {
            auto* new_name = new IR(IRTypeIDENT, string("v00"), DataTableName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_update_stmt(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeTableReferenceList) {
            handle_table_reference_list(rsg, child, ContextUseTop);
        }
    }
}

inline void handle_truncate_stmt(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeTableIdent) {
            auto* new_name = new IR(IRTypeIDENT, string("v00"), DataTableName, ContextUseTop, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_opt_wild_or_where(RSG* rsg, IR*& cur_ir, DATATYPE data_type)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeTEXTSTRINGLiteral) {
            auto* new_name = new IR(IRTypeIDENT, string("v00"), data_type, ContextUseTop, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_show_databases_stmt(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeOptWildOrWhere) {
            handle_opt_wild_or_where(rsg, child, DataDatabaseName);
        }
    }
}

inline void handle_show_tables_stmt(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeOptWildOrWhere) {
            handle_opt_wild_or_where(rsg, child, DataTableName);
        }
    }
}

inline void handle_show_triggers_stmt(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeOptWildOrWhere) {
            handle_opt_wild_or_where(rsg, child, DataTriggerName);
        }
    }
}

inline void handle_show_events_stmt(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeOptWildOrWhere) {
            handle_opt_wild_or_where(rsg, child, DataEventName);
        }
    }
}

inline void handle_show_table_status_stmt(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeOptWildOrWhere) {
            handle_opt_wild_or_where(rsg, child, DataTableName);
        }
    }
}

inline void handle_show_open_tables_stmt(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeOptWildOrWhere) {
            handle_opt_wild_or_where(rsg, child, DataTableName);
        }
    }
}

inline void handle_show_columns_stmt(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeTableIdent) {
            auto* new_name = new IR(IRTypeIDENT, string("v00"), DataTableName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        } else if (child->get_ir_type() == IRTypeOptWildOrWhere) {
            handle_opt_wild_or_where(rsg, child, DataColumnName);
        }
    }
}

inline void handle_show_keys_stmt(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeTableIdent) {
            auto* new_name = new IR(IRTypeIDENT, string("v00"), DataTableName, ContextUseTop, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_show_status_stmt(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeOptWildOrWhere) {
            handle_opt_wild_or_where(rsg, child, DataVariableName);
        }
    }
}

inline void handle_show_variables_stmt(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeOptWildOrWhere) {
            handle_opt_wild_or_where(rsg, child, DataVariableName);
        }
    }
}

inline void handle_show_character_set_stmt(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeOptWildOrWhere) {
            auto* new_name = new IR(IRTypeIDENT, string("latin1"), DataCharsetName, ContextUseTop, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_show_collation_stmt(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeOptWildOrWhere) {
            auto* new_name = new IR(IRTypeIDENT, string("latin1_binary"), DataCollationName, ContextUseTop, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_show_create_database_stmt(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIDENT) {
            auto* new_name = new IR(IRTypeIDENT, string("test123"), DataDatabaseName, ContextUseTop, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_show_create_table_stmt(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeTableIdent) {
            auto* new_name = new IR(IRTypeIDENT, string("v00"), DataTableName, ContextUseTop, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_show_create_view_stmt(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeTableIdent) {
            auto* new_name = new IR(IRTypeIDENT, string("view_0"), DataViewName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_show_replica_status_stmt(RSG* rsg, IR*& cur_ir)
{
    // give up.
}

inline void handle_show_create_procedure_stmt(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeSpName) {
            auto* new_name = new IR(IRTypeIDENT, string("procedure_0"), DataEventName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_show_create_function_stmt(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeSpName) {
            auto* new_name = new IR(IRTypeIDENT, string("function_0"), DataFunctionName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_show_create_trigger_stmt(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeSpName) {
            auto* new_name = new IR(IRTypeIDENT, string("trigger_0"), DataTriggerName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_show_procedure_status_stmt(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeOptWildOrWhere) {
            handle_opt_wild_or_where(rsg, child, DataEventName);
        }
    }
}

inline void handle_show_function_status_stmt(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeOptWildOrWhere) {
            handle_opt_wild_or_where(rsg, child, DataFunctionName);
        }
    }
}

inline void handle_show_procedure_code_stmt(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeSpName) {
            auto* new_name = new IR(IRTypeIDENT, string("procedure_0"), DataEventName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_show_function_code_stmt(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeSpName) {
            auto* new_name = new IR(IRTypeIDENT, string("function_0"), DataFunctionName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_show_create_event_stmt(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeSpName) {
            auto* new_name = new IR(IRTypeIDENT, string("event_0"), DataEventName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_show_create_user_stmt(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeUser) {
            auto* new_name = new IR(IRTypeIDENT, string("mysql"), DataRoleName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_show_parse_tree_stmt(RSG* rsg, IR*& cur_ir)
{
    // give up.
}

inline void handle_engine_or_all(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIdentOrText) {
            auto* new_name = new IR(SymbolTerm, IRTypeALL, string("ALL"), nullptr, nullptr, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_opt_db(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIDENT) {
            auto* new_name = new IR(IRTypeIDENT, string("test123"), DataDatabaseName, ContextUseTop, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_describe_stmt(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeTableIdent) {
            auto* new_name = new IR(IRTypeIDENT, string("v00"), DataTableName, ContextUseTop, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_opt_explain_into(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIdentOrText) {
            auto* new_name = new IR(IRTypeIDENT, string("var_0"), DataVariableName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_opt_explain_for_schema(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIdentOrText) {
            auto* new_name = new IR(IRTypeIDENT, string("test123"), DataDatabaseName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_opt_describe_column(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIdentOrText || child->get_ir_type() == IRTypeIDENT) {
            auto* new_name = new IR(IRTypeIDENT, string("c01"), DataColumnName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_flush_options(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeOptTableList) {
            handle_opt_table_list(rsg, child);
        }
    }
}

inline void handle_persisted_variable_ident(RSG* rsg, IR*& cur_ir)
{
    cur_ir->free_children();

    auto* new_name = new IR(IRTypeIDENT, string("var_0"), DataVariableName, ContextUse, nullptr);
    cur_ir->add_one_child(new_name, 0);
}

inline void handle_use(RSG* rsg, IR*& cur_ir)
{
    cur_ir->free_children();
    return;
}

inline void handle_load_stmt(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeTableIdent) {
            auto* new_name = new IR(IRTypeIDENT, string("v00"), DataTableName, ContextUseTop, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_text_literal(RSG* rsg, IR*& cur_ir)
{
    cur_ir->free_children();

    auto* new_name = new IR(SymbolTerm, IRTypeSTRING, string("v00"), nullptr, nullptr, nullptr);
    new_name->set_data_affinity_type(AFFISTRING);
    cur_ir->add_one_child(new_name, 0);
}

inline void handle_text_string(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeTEXTSTRINGLiteral) {
            auto* new_name = new IR(SymbolTerm, IRTypeSTRING, string("'string'"), nullptr, nullptr, nullptr);
            new_name->set_data_affinity_type(AFFISTRING);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_set(RSG* rsg, IR*& cur_ir)
{
    cur_ir->free_children();
    return;
}

inline void handle_set_resource_group_stmt(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIDENT) {
            auto* new_name = new IR(IRTypeIDENT, string("resource_group_0"), DataUnknownType, ContextUseTop, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
    return;
}

inline void handle_handler_stmt(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeTableIdent) {
            auto* new_name = new IR(IRTypeIDENT, string("v00"), DataTableName, ContextUseTop, nullptr);
            cur_ir->swap_one_child(child, new_name);
        } else if (child->get_ir_type() == IRTypeIDENT) {
            auto* new_name = new IR(IRTypeIDENT, string("handler_0"), DataUnknownType, ContextUseTop, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_revoke(RSG* rsg, IR*& cur_ir)
{
    cur_ir->free_children();
    return;
}

inline void handle_grant(RSG* rsg, IR*& cur_ir)
{
    cur_ir->free_children();
    return;
}

inline void handle_create_user(RSG* rsg, IR*& cur_ir)
{
    cur_ir->free_children();
    return;
}

inline void handle_alter_user(RSG* rsg, IR*& cur_ir)
{
    cur_ir->free_children();
    return;
}

inline void handle_trigger_tail(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeSpName) {
            auto* new_name = new IR(IRTypeIDENT, string("trigger_0"), DataTriggerName, ContextDefine, nullptr);
            cur_ir->swap_one_child(child, new_name);
        } else if (child->get_ir_type() == IRTypeTableIdent) {
            auto* new_name = new IR(IRTypeIDENT, string("v00"), DataTableName, ContextUseTop, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_udf_tail(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIDENT) {
            auto* new_name = new IR(IRTypeIDENT, string("function_0"), DataFunctionName, ContextDefine, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_sf_tail(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeSpName) {
            auto* new_name = new IR(IRTypeIDENT, string("function_0"), DataFunctionName, ContextDefine, nullptr);
            cur_ir->swap_one_child(child, new_name);
        } else if (child->get_ir_type() == IRTypeSpFdparamList) {
            // handle_sp_fdparam_list(rsg, child);
        }
    }
}

inline void handle_json_attribute(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeTEXTSTRINGSys) {
            auto* new_name = new IR(SymbolTerm, IRTypeSTRING, string("'string'"), nullptr, nullptr, nullptr);
            new_name->set_data_affinity_type(AFFIJSONB);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_joined_table_parens(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeJoinedTable) {
            handle_joined_table(rsg, child, ContextUseTop);
        // } else if (child->get_ir_type() == IRTypeJoinedTableParens) {
        //     handle_joined_table_parens(rsg, child);
        }
    }
}

inline void handle_table_ident(RSG* rsg, IR*& cur_ir)
{
    // Fail-safe approach. 
    cur_ir->free_children();
    auto* new_name = new IR(IRTypeIDENT, string("v00"), DataTableName, ContextUseTop, nullptr);
    cur_ir->add_one_child(new_name, 0);
    return;
}

inline void handle_shutdown_stmt(RSG* rsg, IR*& cur_ir)
{
    cur_ir->free_children();
    return;
}   

inline void handle_drop_database_stmt(RSG* rsg, IR*& cur_ir)
{
    cur_ir->free_children();
    return;
}

inline void handle_alter_database_stmt(RSG* rsg, IR*& cur_ir)
{
    cur_ir->free_children();
    return;
}

inline void handle_alter_user_stmt(RSG* rsg, IR*& cur_ir)
{
    cur_ir->free_children();
    return;
}

inline void handle_set_role_stmt(RSG* rsg, IR*& cur_ir)
{
    cur_ir->free_children();
    return;
}


void mysql_ir_context_setup(RSG* rsg, IR*& cur_ir)
{
    IRTYPE cur_ir_type = cur_ir->get_ir_type();

    switch (cur_ir_type) {

    case (IRTypeDeallocate):
        handle_deallocate(rsg, cur_ir);
        break;

    case (IRTypePrepare):
        handle_prepare(rsg, cur_ir);
        break;

    case (IRTypePrepareSrc):
        handle_prepare_src(rsg, cur_ir);
        break;

    case (IRTypeExecute):
        handle_execute(rsg, cur_ir);
        break;

    case (IRTypeExecuteVarIdent):
        handle_execute_var_ident(rsg, cur_ir);
        break;

    case (IRTypeFilterDbIdent):
        handle_filter_db_ident(rsg, cur_ir);
        break;

    case (IRTypeFilterTableIdent):
        handle_filter_table_ident(rsg, cur_ir);
        break;

    case (IRTypeSchema):
        handle_schema(rsg, cur_ir);
        break;

    case (IRTypeUser):
        handle_user(rsg, cur_ir);
        break;

    case (IRTypeUserIdentOrText):
        handle_user_ident_or_text(rsg, cur_ir);
        break;

    case (IRTypeRole):
        handle_role(rsg, cur_ir);
        break;

    case (IRTypeRoleIdentOrText):
        handle_role_ident_or_text(rsg, cur_ir);
        break;

    case (IRTypeSourceTlsCiphersuitesDef):
        handle_source_tls_ciphersuites_def(rsg, cur_ir);
        break;

    case (IRTypeCreateTableStmt):
        handle_create_table_stmt(rsg, cur_ir);
        break;

    case (IRTypeColumnDef):
        handle_column_def(rsg, cur_ir);
        break;

    case (IRTypeFieldDef):
        handle_field_def(rsg, cur_ir);
        break;

    case (IRTypeCreateResourceGroupStmt):
        handle_create_resource_group_stmt(rsg, cur_ir);
        break;

    case (IRTypeCreate):
        handle_create(rsg, cur_ir);
        break;

    case (IRTypeCreateIndexStmt):
        handle_create_index_stmt(rsg, cur_ir);
        break;

    case (IRTypeServerOption):
        handle_server_option(rsg, cur_ir);
        break;

    case (IRTypeEventTail):
        handle_event_tail(rsg, cur_ir);
        break;

    case (IRTypeSpChistic):
        handle_sp_chistic(rsg, cur_ir);
        break;

    case (IRTypeCallStmt):
        handle_call_stmt(rsg, cur_ir);
        break;

    case (IRTypeSpFdparam):
        handle_sp_fdparam(rsg, cur_ir);
        break;

    case (IRTypeSpPdparam):
        handle_sp_pdparam(rsg, cur_ir);
        break;

    case (IRTypeSpDecl):
        handle_sp_decl(rsg, cur_ir);
        break;

    case (IRTypeSignalValue):
        handle_signal_value(rsg, cur_ir);
        break;

    case (IRTypeSignalAllowedExpr):
        handle_signal_allowed_expr(rsg, cur_ir);
        break;

    case (IRTypeSpProcStmtLeave):
        handle_sp_proc_stmt_leave(rsg, cur_ir);
        break;

    case (IRTypeSpProcStmtIterate):
        handle_sp_proc_stmt_iterate(rsg, cur_ir);
        break;

    case (IRTypeSpProcStmtOpen):
        handle_sp_proc_stmt_open(rsg, cur_ir);
        break;

    case (IRTypeSpProcStmtFetch):
        handle_sp_proc_stmt_fetch(rsg, cur_ir);
        break;

    case (IRTypeSpProcStmtClose):
        handle_sp_proc_stmt_close(rsg, cur_ir);
        break;

    case (IRTypeSpFetchList):
        handle_sp_fetch_list(rsg, cur_ir);
        break;

    case (IRTypeSpLabeledControl):
        handle_sp_labeled_control(rsg, cur_ir);
        break;

    case (IRTypeSpOptLabel):
        handle_sp_opt_label(rsg, cur_ir);
        break;

    case (IRTypeSpLabeledBlock):
        handle_sp_labeled_block(rsg, cur_ir);
        break;

    case (IRTypeOptLogfileGroupName):
        handle_opt_logfile_group_name(rsg, cur_ir);
        break;

    case (IRTypeSizeNumber):
        handle_size_number(rsg, cur_ir);
        break;

    case (IRTypeNameList):
        handle_name_list(rsg, cur_ir);
        break;

    case (IRTypePartDefinition):
        handle_part_definition(rsg, cur_ir);
        break;

    case (IRTypeSubPartDefinition):
        handle_sub_part_definition(rsg, cur_ir);
        break;

    case (IRTypePartOption):
        handle_part_option(rsg, cur_ir);
        break;

    case (IRTypeCreateTableOption):
        handle_create_table_option(rsg, cur_ir);
        break;

    case (IRTypeOptTableList):
        handle_opt_table_list(rsg, cur_ir);
        break;

    case (IRTypeDefaultCharset):
        handle_default_charset(rsg, cur_ir);
        break;

    case (IRTypeDefaultCollation):
        handle_default_collation(rsg, cur_ir);
        break;

    case (IRTypeOptIndexNameAndType):
        handle_opt_index_name_and_type(rsg, cur_ir);
        break;

    case (IRTypeTableConstraintDef):
        handle_table_constraint_def(rsg, cur_ir);
        break;

    case (IRTypeOptConstraintName):
        handle_opt_constraint_name(rsg, cur_ir);
        break;

    case (IRTypeCollationName):
        handle_collation_name(rsg, cur_ir);
        break;

    case (IRTypeCharacterSet):
        handle_character_set(rsg, cur_ir);
        break;

    case (IRTypeCharsetName):
        handle_charset_name(rsg, cur_ir);
        break;

    case (IRTypeOldOrNewCharsetName):
        handle_old_or_new_charset_name(rsg, cur_ir);
        break;

    case (IRTypeReferences):
        handle_references(rsg, cur_ir);
        break;

    case (IRTypeReferenceList):
        handle_reference_list(rsg, cur_ir);
        break;

    case (IRTypeKeyPart):
        handle_key_part(rsg, cur_ir);
        break;

    case (IRTypeAlterTableStmt):
        handle_alter_table_stmt(rsg, cur_ir);
        break;

    case (IRTypeAlterProcedureStmt):
        handle_alter_procedure_stmt(rsg, cur_ir);
        break;

    case (IRTypeAlterFunctionStmt):
        handle_alter_function_stmt(rsg, cur_ir);
        break;

    case (IRTypeAlterViewStmt):
        handle_alter_view_stmt(rsg, cur_ir);
        break;

    case (IRTypeAlterEventStmt):
        handle_alter_event_stmt(rsg, cur_ir);
        break;

    case (IRTypeOptEvRenameTo):
        handle_opt_ev_rename_to(rsg, cur_ir);
        break;

    case (IRTypeAlterLogfileStmt):
        handle_alter_logfile_stmt(rsg, cur_ir);
        break;

    case (IRTypeIDENT):
        handle_IDENT(rsg, cur_ir);
        break;

    case (IRTypeAlterTablespaceStmt):
        handle_alter_tablespace_stmt(rsg, cur_ir);
        break;

    case (IRTypeAlterUndoTablespaceStmt):
        handle_alter_undo_tablespace_stmt(rsg, cur_ir);
        break;

    case (IRTypeAlterServerStmt):
        handle_alter_server_stmt(rsg, cur_ir);
        break;

    case (IRTypeAlterResourceGroupStmt):
        handle_alter_resource_group_stmt(rsg, cur_ir);
        break;

    case (IRTypeStandaloneAlterCommands):
        handle_standalone_alter_commands(rsg, cur_ir);
        break;

    case (IRTypeAlterListItem):
        handle_alter_list_item(rsg, cur_ir);
        break;

    case (IRTypeAlterAlgorithmOptionValue):
        handle_alter_algorithm_option_value(rsg, cur_ir);
        break;

    case (IRTypeAlterLockOptionValue):
        handle_alter_lock_option_value(rsg, cur_ir);
        break;

    case (IRTypeOptPlace):
        handle_opt_place(rsg, cur_ir);
        break;

    case (IRTypeGroupReplicationUser):
        handle_group_replication_user(rsg, cur_ir);
        break;

    case (IRTypeOptUserOption):
        handle_opt_user_option(rsg, cur_ir);
        break;

    case (IRTypeOptPasswordOption):
        handle_opt_password_option(rsg, cur_ir);
        break;

    case (IRTypeChecksum):
        handle_checksum(rsg, cur_ir);
        break;

    case (IRTypeRepairTableStmt):
        handle_repair_table_stmt(rsg, cur_ir);
        break;

    case (IRTypeAnalyzeTableStmt):
        handle_analyze_table_stmt(rsg, cur_ir);
        break;

    case (IRTypeOptHistogram):
        handle_opt_histogram(rsg, cur_ir);
        break;

    case (IRTypeCheckTableStmt):
        handle_check_table_stmt(rsg, cur_ir);
        break;

    case (IRTypeOptimizeTableStmt):
        handle_optimize_table_stmt(rsg, cur_ir);
        break;

    case (IRTypeRename):
        handle_rename(rsg, cur_ir);
        break;

    case (IRTypeTableToTable):
        handle_table_to_table(rsg, cur_ir);
        break;

    case (IRTypePreloadStmt):
        handle_preload_stmt(rsg, cur_ir);
        break;

    case (IRTypePreloadKeys):
        handle_preload_keys(rsg, cur_ir);
        break;

    case (IRTypeKeyUsageElement):
        handle_key_usage_element(rsg, cur_ir);
        break;

    case (IRTypeTableIdentOptWild):
        handle_table_ident_opt_wild(rsg, cur_ir);
        break;

    case (IRTypeIntoClause):
        handle_into_clause(rsg, cur_ir);
        break;

    case (IRTypeSelectVarIdent):
        handle_select_var_ident(rsg, cur_ir);
        break;

    case (IRTypeExplicitTable):
        handle_explicit_table(rsg, cur_ir);
        break;

    case (IRTypeUsePartition):
        handle_use_partition(rsg, cur_ir);
        break;

    case (IRTypeOptTableAlias):
        handle_opt_table_alias(rsg, cur_ir);
        break;

    // case (IRTypeSamplingPercentage):
    //     handle_sampling_percentage(rsg, cur_ir);
    //     break;

    case (IRTypeJtColumn):
        handle_jt_column(rsg, cur_ir);
        break;

    case (IRTypeFromTables):
        handle_from_tables(rsg, cur_ir);
        break;

    case (IRTypeTableWild):
        handle_table_wild(rsg, cur_ir);
        break;

    case (IRTypeSelectAlias):
        handle_select_alias(rsg, cur_ir);
        break;

    case (IRTypeFunctionCallGeneric):
        handle_function_call_generic(rsg, cur_ir);
        break;

    case (IRTypeSimpleExpr):
        handle_simple_expr(rsg, cur_ir);
        break;

    case (IRTypeRvalueSystemOrUserVariable):
        handle_rvalue_system_or_user_variable(rsg, cur_ir);
        break;

    case (IRTypeInExpressionUserVariableAssignment):
        handle_in_expression_user_variable_assignment(rsg, cur_ir);
        break;

    case (IRTypeWindowName):
        handle_window_name(rsg, cur_ir);
        break;

    case (IRTypeSimpleIdentNospvar):
        handle_simple_ident_nospvar(rsg, cur_ir);
        break;

    case (IRTypeCommonTableExpr):
        handle_common_table_expr(rsg, cur_ir);
        break;

    case (IRTypeDropTableStmt):
        handle_drop_table_stmt(rsg, cur_ir);
        break;

    case (IRTypeDropIndexStmt):
        handle_drop_index_stmt(rsg, cur_ir);
        break;

    case (IRTypeDropFunctionStmt):
        handle_drop_function_stmt(rsg, cur_ir);
        break;

    case (IRTypeDropResourceGroupStmt):
        handle_drop_resource_group_stmt(rsg, cur_ir);
        break;

    case (IRTypeDropProcedureStmt):
        handle_drop_procedure_stmt(rsg, cur_ir);
        break;

    case (IRTypeDropUserStmt):
        handle_drop_user_stmt(rsg, cur_ir);
        break;

    case (IRTypeDropViewStmt):
        handle_drop_view_stmt(rsg, cur_ir);
        break;

    case (IRTypeDropEventStmt):
        handle_drop_event_stmt(rsg, cur_ir);
        break;

    case (IRTypeDropTriggerStmt):
        handle_drop_trigger_stmt(rsg, cur_ir);
        break;

    case (IRTypeDropTablespaceStmt):
        handle_drop_tablespace_stmt(rsg, cur_ir);
        break;

    case (IRTypeDropUndoTablespaceStmt):
        handle_drop_undo_tablespace_stmt(rsg, cur_ir);
        break;

    case (IRTypeDropLogfileStmt):
        handle_drop_logfile_stmt(rsg, cur_ir);
        break;

    case (IRTypeDropServerStmt):
        handle_drop_server_stmt(rsg, cur_ir);
        break;

    case (IRTypeDropRoleStmt):
        handle_drop_role_stmt(rsg, cur_ir);
        break;

    case (IRTypeInsertStmt):
        handle_insert_stmt(rsg, cur_ir);
        break;

    case (IRTypeOptValuesReference):
        handle_opt_values_reference(rsg, cur_ir);
        break;

    case (IRTypeReplaceStmt):
        handle_replace_stmt(rsg, cur_ir);
        break;

    case (IRTypeUpdateStmt):
        handle_update_stmt(rsg, cur_ir);
        break;

    case (IRTypeTruncateStmt):
        handle_truncate_stmt(rsg, cur_ir);
        break;

    case (IRTypeShowDatabasesStmt):
        handle_show_databases_stmt(rsg, cur_ir);
        break;

    case (IRTypeShowTablesStmt):
        handle_show_tables_stmt(rsg, cur_ir);
        break;

    case (IRTypeShowTriggersStmt):
        handle_show_triggers_stmt(rsg, cur_ir);
        break;

    case (IRTypeShowEventsStmt):
        handle_show_events_stmt(rsg, cur_ir);
        break;

    case (IRTypeShowTableStatusStmt):
        handle_show_table_status_stmt(rsg, cur_ir);
        break;

    case (IRTypeShowOpenTablesStmt):
        handle_show_open_tables_stmt(rsg, cur_ir);
        break;

    case (IRTypeShowColumnsStmt):
        handle_show_columns_stmt(rsg, cur_ir);
        break;

    case (IRTypeShowKeysStmt):
        handle_show_keys_stmt(rsg, cur_ir);
        break;

    case (IRTypeShowStatusStmt):
        handle_show_status_stmt(rsg, cur_ir);
        break;

    case (IRTypeShowVariablesStmt):
        handle_show_variables_stmt(rsg, cur_ir);
        break;

    case (IRTypeShowCharacterSetStmt):
        handle_show_character_set_stmt(rsg, cur_ir);
        break;

    case (IRTypeShowCollationStmt):
        handle_show_collation_stmt(rsg, cur_ir);
        break;

    case (IRTypeShowCreateDatabaseStmt):
        handle_show_create_database_stmt(rsg, cur_ir);
        break;

    case (IRTypeShowCreateTableStmt):
        handle_show_create_table_stmt(rsg, cur_ir);
        break;

    case (IRTypeShowCreateViewStmt):
        handle_show_create_view_stmt(rsg, cur_ir);
        break;

    case (IRTypeShowCreateProcedureStmt):
        handle_show_create_procedure_stmt(rsg, cur_ir);
        break;

    case (IRTypeShowCreateFunctionStmt):
        handle_show_create_function_stmt(rsg, cur_ir);
        break;

    case (IRTypeShowCreateTriggerStmt):
        handle_show_create_trigger_stmt(rsg, cur_ir);
        break;

    case (IRTypeShowProcedureStatusStmt):
        handle_show_procedure_status_stmt(rsg, cur_ir);
        break;

    case (IRTypeShowFunctionStatusStmt):
        handle_show_function_status_stmt(rsg, cur_ir);
        break;

    case (IRTypeShowProcedureCodeStmt):
        handle_show_procedure_code_stmt(rsg, cur_ir);
        break;

    case (IRTypeShowFunctionCodeStmt):
        handle_show_function_code_stmt(rsg, cur_ir);
        break;

    case (IRTypeShowCreateEventStmt):
        handle_show_create_event_stmt(rsg, cur_ir);
        break;

    case (IRTypeShowCreateUserStmt):
        handle_show_create_user_stmt(rsg, cur_ir);
        break;

    // case (IRTypeShowParseTreeStmt):
    //     handle_show_parse_tree_stmt(rsg, cur_ir);
    //     break;

    case (IRTypeEngineOrAll):
        handle_engine_or_all(rsg, cur_ir);
        break;

    case (IRTypeOptDb):
        handle_opt_db(rsg, cur_ir);
        break;

    case (IRTypeDescribeStmt):
        handle_describe_stmt(rsg, cur_ir);
        break;

    // case (IRTypeOptExplainInto):
    //     handle_opt_explain_into(rsg, cur_ir);
    //     break;

    // case (IRTypeOptExplainForSchema):
    //     handle_opt_explain_for_schema(rsg, cur_ir);
    //     break;

    case (IRTypeOptDescribeColumn):
        handle_opt_describe_column(rsg, cur_ir);
        break;

    case (IRTypeFlushOptions):
        handle_flush_options(rsg, cur_ir);
        break;

    case (IRTypeUse):
        handle_use(rsg, cur_ir);
        break;

    case (IRTypeLoadStmt):
        handle_load_stmt(rsg, cur_ir);
        break;

    case (IRTypeTextLiteral):
        handle_text_literal(rsg, cur_ir);
        break;

    case (IRTypeTEXTSTRING):
        handle_text_string(rsg, cur_ir);
        break;

    case (IRTypeSet):
        handle_set(rsg, cur_ir);
        break;

    case (IRTypeSetResourceGroupStmt):
        handle_set_resource_group_stmt(rsg, cur_ir);
        break;

    case (IRTypeHandlerStmt):
        handle_handler_stmt(rsg, cur_ir);
        break;

    case (IRTypeRevoke):
        handle_revoke(rsg, cur_ir);
        break;

    case (IRTypeGrant):
        handle_grant(rsg, cur_ir);
        break;

    case (IRTypeCreateUser):
        handle_create_user(rsg, cur_ir);
        break;

    case (IRTypeAlterUser):
        handle_alter_user(rsg, cur_ir);
        break;

    case (IRTypeTriggerTail):
        handle_trigger_tail(rsg, cur_ir);
        break;

    case (IRTypeUdfTail):
        handle_udf_tail(rsg, cur_ir);
        break;

    case (IRTypeSfTail):
        handle_sf_tail(rsg, cur_ir);
        break;

    case (IRTypeJsonAttribute):
        handle_json_attribute(rsg, cur_ir);
        break;

    case (IRTypeOptDerivedColumnList):
        handle_opt_derived_column_list(rsg, cur_ir, DataColumnName, ContextDefine);
        break;

    case (IRTypeJoinedTableParens):
        handle_joined_table_parens(rsg, cur_ir);
        break;

    case (IRTypeJoinedTable):
        handle_joined_table(rsg, cur_ir, ContextUseTop);
        break;

    case (IRTypeTableIdent):
        handle_table_ident(rsg, cur_ir);
        break;

    case (IRTypeUsingList):
        handle_using_list(rsg, cur_ir, DataColumnName, ContextUse);
        break;

    case (IRTypeShutdownStmt):
        handle_shutdown_stmt(rsg, cur_ir);
        break;

    case (IRTypeDropDatabaseStmt):
        handle_drop_database_stmt(rsg, cur_ir);
        break;

    case (IRTypeAlterDatabaseStmt):
        handle_alter_database_stmt(rsg, cur_ir);
        break;

    case (IRTypeAlterUserStmt):
        handle_alter_user_stmt(rsg, cur_ir);
        break;

    case (IRTypeSetRoleStmt):
        handle_set_role_stmt(rsg, cur_ir);
        break;

    default:
        break;
    }

    // IMPORTANT!!!
    for (auto& interesting_type : MySQLFuzzerConfigurations::mysql_interesting_ir_types) {
        if (interesting_type == IRTypeSelectStmt) {
            continue;
        }
        if (interesting_type == cur_ir_type) {
            cur_ir->set_is_favor(IsFavor::favor);
            break;
        }
    }
}