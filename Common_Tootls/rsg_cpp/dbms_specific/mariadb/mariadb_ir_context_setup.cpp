//
// Created by XXX on 12/11/24.
//

#include "mariadb_ir_context_setup.h"
#include "mariadb_fuzzer_configurations.h"
#include <climits>
#include <initializer_list>

#include "../../headers/fuzzer_configurations.h"

#include <vector>

using namespace std;

#define BEGIN vector<IR*> children = cur_ir->get_children();

inline void handle_IDENT(RSG* rsg, IR*& cur_ir, DATATYPE data_type = DataTableName, DATAFLAG context_type = ContextUse)
{
    // Just a placeholder.
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIDENT) {
            auto* new_name = new IR(IRTypeIDENT, string("v00"), data_type, context_type, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_table_list(RSG* rsg, IR*& cur_ir, DATATYPE data_type = DataTableName, DATAFLAG context_type = ContextUse) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIDENT) {
            handle_IDENT(rsg, child, data_type, context_type);
        }
    }
}

inline void handle_table_name(RSG* rsg, IR*& cur_ir, DATATYPE data_type = DataTableName, DATAFLAG context_type = ContextUse) {
    cur_ir->free_children();
    auto* new_name = new IR(IRTypeIDENT, string("v00"), data_type, context_type, nullptr);
    cur_ir->add_one_child(new_name, 0);
}

inline void handle_opt_ident(RSG* rsg, IR*& cur_ir, DATATYPE data_type, DATAFLAG context_type) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeTableName) {
            handle_table_name(rsg, child, data_type, context_type);
        }
    }
}   

inline void handle_field_ident(RSG* rsg, IR*& cur_ir, DATATYPE data_type = DataColumnName, DATAFLAG context_type = ContextUse) {
    cur_ir->free_children();
    auto* new_name = new IR(IRTypeIDENT, string("v00"), data_type, context_type, nullptr);
    cur_ir->add_one_child(new_name, 0);
}

inline void handle_table_ident(RSG* rsg, IR*& cur_ir, DATAFLAG context_type = ContextUseTop)
{
    // Fail-safe approach. 
    cur_ir->free_children();
    auto* new_name = new IR(IRTypeIDENT, string("v00"), DataTableName, context_type, nullptr);
    cur_ir->add_one_child(new_name, 0);
    return;
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

inline void handle_optional_connection_name(RSG* rsg, IR*& cur_ir)
{
    BEGIN;
    if (children.size() != 0) {
        auto* child = children[0];
        auto* new_name = new IR(IRTypeIDENT, string("connection_name_0"), DataUnknownType, ContextUnknown, nullptr);
        cur_ir->swap_one_child(child, new_name);
    }
    return;
}

inline void handle_alt_part_name_item(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIDENT) {
            auto* new_name = new IR(IRTypeIDENT, string("partition_name_0"), DataPartitionName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
            break;
        }
    }
}

inline void handle_create(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    DATATYPE data_type = DataDatabaseName;
    string str = "";

    if (children.size() > 3 && children[2]->get_ir_type() == IRTypeTABLESYM) {
        data_type = DataTableName;
        str = "v0";
    } else if (children.size() > 3 && children[2]->get_ir_type() == IRTypeSEQUENCESYM) {
        data_type = DataSequenceName;
        str = "sequence_0";
    } else if (children.size() > 3 && (children[1]->get_ir_type() == IRTypeINDEXSYM || children[2]->get_ir_type() == IRTypeINDEXSYM)) {
        data_type = DataIndexName;
        str = "index_0";
    } else if (children.size() > 2 && children[1]->get_ir_type() == IRTypeDATABASE) {
        data_type = DataDatabaseName;
        str = "database_0";
    } else if (children.size() > 6 && (children[3]->get_ir_type() == IRTypeVIEWSYM || children[4]->get_ir_type() == IRTypeVIEWSYM || 
    children[5]->get_ir_type() == IRTypeVIEWSYM) ) {
        data_type = DataViewName;
        str = "view_0";
    } else if (children.size() > 3 && children[2]->get_ir_type() == IRTypeTRIGGERSYM) {
        data_type = DataTriggerName;
        str = "trigger_0";
    } else if (children.size() > 3 && children[2]->get_ir_type() == IRTypeEVENTSYM) {
        data_type = DataEventName;
        str = "event_0";
    } else if (children.size() > 3 && (children[1]->get_ir_type() == IRTypeROLESYM || children[1]->get_ir_type() == IRTypeUSERSYM ) ) {
        data_type = DataRoleName;
        str = "role_0";
    }

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIDENT || child->get_ir_type() == IRTypeIdentOrText) {
            auto* new_name = new IR(IRTypeIDENT, str, data_type, ContextDefine, nullptr);
            cur_ir->swap_one_child(child, new_name);
            break;
        }
    }
}

inline void handle_server_def(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIdentOrText) {
            auto* new_name = new IR(IRTypeIDENT, string("server_0"), DataServerName, ContextDefine, nullptr);
            cur_ir->swap_one_child(child, new_name);
            break;
        }
    }
}

inline void handle_event_tail(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeSpName) {
            auto* new_name = new IR(IRTypeIDENT, string("event_0"), DataEventName, ContextDefine, nullptr);
            cur_ir->swap_one_child(child, new_name);
            break;
        }
    }
        
}

inline void handle_drop_routine(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (int i = 0; i < children.size(); i++) {
        auto* child = children[i];
        if (child->get_ir_type() == IRTypeIDENT) {
            auto* new_name = new IR(IRTypeIDENT, string("routine_0"), DataEventName, ContextUndefine, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }

        // Remove the "." ident. 
        if (i != children.size() - 1) {
            for (size_t j = children.size() - 1; j >= i + 1; j--) {
                cur_ir->detach_one_child(children[j]);
                children[j]->deep_drop();
            }
            return;
        }
    }

}

inline void handle_call(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    bool is_second_ident = false;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIDENT) {
            if (!is_second_ident) {
                auto* new_name = new IR(IRTypeIDENT, string("routine_0"), DataEventName, ContextUse, nullptr);
                cur_ir->swap_one_child(child, new_name);
            } else if (child->get_ir_type() != IRTypeOptSpCparamList) { // Remove the ". ident . ident"
                auto* new_name = new IR(SymbolTerm, IRTypeUnknownType, string(""), nullptr, nullptr, nullptr);
                cur_ir->swap_one_child(child, new_name);
            }
            is_second_ident = true;
        }
    }
}


inline void handle_sp_param_name(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    // TODO:: FIXME. 
    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIDENT) {
            auto* new_name = new IR(IRTypeIDENT, string("param_0"), DataUnknownType, ContextDefine  , nullptr);
            cur_ir->swap_one_child(child, new_name);
            break;
        }
    }
        
}   

inline void handle_sp_name(RSG* rsg, IR*& cur_ir) {
    cur_ir->free_children();
    auto* new_name = new IR(IRTypeIDENT, string("sp_name_0"), DataProcedureName, ContextUse, nullptr);
    cur_ir->add_one_child(new_name, 0);
}


inline void handle_label_ident(RSG* rsg, IR*& cur_ir)
{
    cur_ir->free_children();

    auto* new_name = new IR(IRTypeIDENT, string("label_name_0"), DataUnknownType, ContextUse, nullptr);
    cur_ir->add_one_child(new_name, 0);
}

inline void handle_ident_or_empty(RSG* rsg, IR*& cur_ir) {
    cur_ir->free_children();
}

inline void handle_sp_proc_stmt_close(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIDENT) {
            auto* new_name = new IR(IRTypeIDENT, string("cursor_name_0"), DataCursorName, ContextUndefine, nullptr);
            cur_ir->swap_one_child(child, new_name);
            break;
        }
    }
}

inline void handle_sp_fetch_list(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIDENT) {
            auto* new_name = new IR(IRTypeIDENT, string("cursor_name_0"), DataCursorName, ContextUndefine, nullptr);
            cur_ir->swap_one_child(child, new_name);
            break;
        }
    }
}

inline void handle_create_like(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeTableIdent) {
            handle_table_ident(rsg, child, ContextUse);
        }
    }
}

inline void handle_part_field_item(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIDENT) {
            auto* new_name = new IR(IRTypeIDENT, string("c1"), DataColumnName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
            break;
        }
    }
}

inline void handle_sub_part_field_item(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIDENT) {
            auto* new_name = new IR(IRTypeIDENT, string("c1"), DataColumnName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
            break;
        }
    }
}

inline void handle_part_definition(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypePartName) {
            auto* new_name = new IR(IRTypeIDENT, string("p1"), DataPartitionName, ContextDefine, nullptr);
            cur_ir->swap_one_child(child, new_name);
            break;
        }
    }
}

inline void handle_part_name(RSG* rsg, IR*& cur_ir)
{
    // Just a placeholder.
    cur_ir->free_children();
    auto* new_name = new IR(IRTypeIDENT, string("p1"), DataPartitionName, ContextUse, nullptr);
    cur_ir->add_one_child(new_name, 0);
}

inline void handle_sub_part_definition(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeSubName) {
            auto* new_name = new IR(IRTypeIDENT, string("s1"), DataPartitionName, ContextDefine, nullptr);
            cur_ir->swap_one_child(child, new_name);
            break;
        }
    }
}

inline void handle_engine_defined_option(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIDENT) {
            auto* new_name = new IR(SymbolTerm, IRTypeUnknownType, string(" DEFAULT "), nullptr, nullptr, nullptr);
            cur_ir->swap_one_child(child, new_name);
            break;
        }
    }
}

inline void handle_charset_name(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIdentOrText) {
            auto* new_name = new IR(SymbolTerm, IRTypeUnknownType, string("BINARY"), nullptr, nullptr, nullptr);
            cur_ir->swap_one_child(child, new_name);
            break;
        }
    }
}

inline void handle_collation_name(RSG* rsg, IR*& cur_ir)
{
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIdentOrText) {
            auto* new_name = new IR(SymbolTerm, IRTypeUnknownType, string("BINARY"), nullptr, nullptr, nullptr);
            cur_ir->swap_one_child(child, new_name);
            break;
        }
    }
        
}

inline void handle_storage_engines(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIdentOrText) {
            auto* new_name = new IR(SymbolTerm, IRTypeUnknownType, string("InnoDB"), nullptr, nullptr, nullptr);
            cur_ir->swap_one_child(child, new_name);
            break;
        }
    }
}

inline void handle_known_storage_engines(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIdentOrText) {
            auto* new_name = new IR(SymbolTerm, IRTypeUnknownType, string("InnoDB"), nullptr, nullptr, nullptr);
            cur_ir->swap_one_child(child, new_name);
            break;
        }
    }
}

inline void handle_key_def(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeOptIdent || child->get_ir_type() == IRTypeIDENT) {
            auto* new_name = new IR(IRTypeIDENT, string("key_name_0"), DataIndexName, ContextDefine, nullptr);
            cur_ir->swap_one_child(child, new_name);
            break;
        }
    }
}

inline void handle_period_for_system_time(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIDENT) {
            auto* new_name = new IR(IRTypeIDENT, string("c1"), DataColumnName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
            break;
        }
    }
}

inline void handle_period_for_application_time(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIDENT) {
            auto* new_name = new IR(IRTypeIDENT, string("c1"), DataColumnName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
            break;
        }
    }
}

inline void handle_constraint(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeOptIdent) {
            handle_opt_ident(rsg, child, DataConstraintName, ContextUse);
        }
    }
}

inline void handle_field_spec(RSG* rsg, IR*& cur_ir, DATAFLAG context_type = ContextDefine) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeFieldIdent) {
            handle_field_ident(rsg, child, DataColumnName, context_type);
        }
    }
}

inline void handle_udt_name(RSG* rsg, IR*& cur_ir) {
    cur_ir->free_children();

    auto* new_name = new IR(IRTypeIDENT, string("func_0"), DataUnknownType, ContextDefine, nullptr);
    cur_ir->add_one_child(new_name, 0);
}

inline void handle_opt_compression_method(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIDENT) {
            auto* new_name = new IR(SymbolTerm, IRTypeUnknownType, string("none"), nullptr, nullptr, nullptr);
            cur_ir->swap_one_child(child, new_name);
            break;
        }
    }
}

inline void handle_old_or_new_charset_name(RSG* rsg, IR*& cur_ir) {
    cur_ir->free_children();

    auto* new_name = new IR(SymbolTerm, IRTypeUnknownType, string("BINARY"), nullptr, nullptr, nullptr);
    cur_ir->add_one_child(new_name, 0);
}

inline void handle_references(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeTableIdent) {
            handle_table_ident(rsg, child, ContextUseTop);
        }
    }

}

inline void handle_ref_list(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIDENT) {
            auto* new_name = new IR(IRTypeIDENT, string("c1"), DataColumnName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
            break;
        }
    }
}


inline void handle_opt_without_overlaps(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIDENT) {
            auto* new_name = new IR(IRTypeIDENT, string("c1"), DataColumnName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
            break;
        }
    }
}   

inline void handle_key_part(RSG* rsg, IR*& cur_ir) {    
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIDENT) {
            auto* new_name = new IR(IRTypeIDENT, string("c1"), DataColumnName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
            break;
        }
    }
}


inline void handle_key_part_simple(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIDENT) {
            auto* new_name = new IR(IRTypeIDENT, string("c1"), DataColumnName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
            break;
        }
    }
}

inline void handle_alter_list_item(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeFieldSpec) {
            handle_field_spec(rsg, child, ContextUse);
        }
    }

    if (children.size() > 3 && children[1]->get_ir_type() == IRTypeCONSTRAINT) {
        for (auto& child : children) {
            if (child->get_ir_type() == IRTypeFieldIdent) {
                auto* new_name = new IR(IRTypeIDENT, string("index_0"), DataIndexName, ContextUse, nullptr);
                cur_ir->swap_one_child(child, new_name);
                return;
            }
        }
    } else if (children.size() > 3 && children[0]->get_ir_type() == IRTypeCHANGE) {
        for (auto& child : children) {
            if (child->get_ir_type() == IRTypeFieldIdent) {
                auto* new_name = new IR(IRTypeIDENT, string("c0"), DataColumnName, ContextUse, nullptr);
                cur_ir->swap_one_child(child, new_name);
                return;
            }
        }
    } else if (children.size() > 3 && children[0]->get_ir_type() == IRTypeDROP) {
        DATATYPE data_type = DataColumnName;
        if (children[1]->get_ir_type() == IRTypeOptColumn) {
            data_type = DataColumnName;
        } else if (children[1]->get_ir_type() == IRTypeCONSTRAINT) {
            data_type = DataConstraintName;
        } else if (children[1]->get_ir_type() == IRTypeFOREIGN || children[1]->get_ir_type() == IRTypeKeyOrIndex) {
            data_type = DataConstraintName;
        }
        for (auto& child : children) {
            if (child->get_ir_type() == IRTypeFieldIdent) {
                handle_field_ident(rsg, child, data_type, ContextUndefine);
            }
        }
    } else if (children.size() > 3 && children[0]->get_ir_type() == IRTypeAlter) {
        DATATYPE data_type = DataColumnName;
        if (children[1]->get_ir_type() == IRTypeOptColumn) {
            data_type = DataColumnName;
        } else if (children[1]->get_ir_type() == IRTypeKeyOrIndex) {
            data_type = DataIndexName;
        }
        for (auto& child : children) {
            if (child->get_ir_type() == IRTypeFieldIdent || child->get_ir_type() == IRTypeIDENT) {
                auto* new_name = new IR(IRTypeIDENT, string("c0"), data_type, ContextUse, nullptr);
                cur_ir->swap_one_child(child, new_name);
                return;
            }
        }
    } else if (children.size() > 3 && children[0]->get_ir_type() == IRTypeRENAME) {
        if (children[1]->get_ir_type() == IRTypeOptTo) {
            for (auto& child : children) {
                if (child->get_ir_type() == IRTypeTableIdent) {
                    handle_table_ident(rsg, child, ContextDefine);
                }
            }
        } else if (children[1]->get_ir_type() == IRTypeCOLUMNSYM) {
            bool is_second_ident = false;
            for (auto& child : children) {
                if (child->get_ir_type() == IRTypeIDENT) {
                    IR* new_name = nullptr;
                    if (is_second_ident) {
                        new_name = new IR(IRTypeIDENT, string("c1"), DataColumnName, ContextUndefine, nullptr);
                    } else {
                        new_name = new IR(IRTypeIDENT, string("c0"), DataColumnName, ContextDefine, nullptr);
                    }
                    cur_ir->swap_one_child(child, new_name);
                    is_second_ident = true;
                }
            }
        } else if (children[1]->get_ir_type() == IRTypeKeyOrIndex) {
            bool is_second_ident = false;
            for (auto& child : children) {
                if (child->get_ir_type() == IRTypeTableIdent) {
                    IR* new_name = nullptr;
                    if (is_second_ident) {
                        new_name = new IR(IRTypeIDENT, string("c1"), DataIndexName, ContextUndefine, nullptr);
                    } else {
                        new_name = new IR(IRTypeIDENT, string("c0"), DataIndexName, ContextDefine, nullptr);
                    }
                    cur_ir->swap_one_child(child, new_name);
                    is_second_ident = true;
                }
            }
        }
    }

}

inline void handle_alter(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    bool is_view = false;
    bool is_sequence = false;
    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeVIEWSYM) {
            is_view = true;
            continue;
        } else if (child->get_ir_type() == IRTypeSEQUENCESYM) {
            is_sequence = true;
            continue;
        } else if (child->get_ir_type() == IRTypeTableIdent) {
            if (is_view) {
                auto* new_name = new IR(IRTypeIDENT, string("view_name_0"), DataViewName, ContextUse, nullptr);
                cur_ir->swap_one_child(child, new_name);
            } else if (is_sequence) {
                auto* new_name = new IR(IRTypeIDENT, string("sequence_name_0"), DataSequenceName, ContextUse, nullptr);
                cur_ir->swap_one_child(child, new_name);
            } else {
                handle_table_ident(rsg, child, ContextUse);
            }
            return;
        } else if (child->get_ir_type() == IRTypeSpName) {
            auto* new_name = new IR(IRTypeIDENT, string("sp_name_0"), DataUnknownType, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
            return;
        }
    }

    if (children.size() > 3 && children[1]->get_ir_type() == IRTypeDATABASE) {
        for (auto& child : children) {
            if (child->get_ir_type() == IRTypeIDENT || child->get_ir_type() == IRTypeIdentOrEmpty) {
                auto* new_name = new IR(IRTypeIDENT, string("database_name_0"), DataDatabaseName, ContextUse, nullptr);
                cur_ir->swap_one_child(child, new_name);
                break;
            }
        }
    }
}

inline void handle_alter_lock_option(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIDENT) {
            auto* new_name = new IR(SymbolTerm, IRTypeUnknownType, string("DEFAULT"), nullptr, nullptr, nullptr);
            cur_ir->swap_one_child(child, new_name);
            break;
        }
    }
}

inline void handle_repair_table_or_view(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    DATATYPE data_type = DataTableName;
    if (!children.empty() && children[0]->get_ir_type() == IRTypeVIEWSYM) {
        data_type = DataViewName;
    }
    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeTableList) {
            handle_table_list(rsg, child, data_type, ContextUse);
        }
    }
}

inline void handle_table_column_list(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIDENT) {
            auto* new_name = new IR(IRTypeIDENT, string("c0"), DataColumnName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
            break;
        }
    }
}

inline void handle_table_index_list(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIDENT) {
            auto* new_name = new IR(IRTypeIDENT, string("c0"), DataIndexName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
            break;
        }
    }
}

inline void handle_table_to_table(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    bool is_second_ident = false;
    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeTableIdent) {
            if (!is_second_ident) {
                handle_table_ident(rsg, child, ContextUndefine);
            } else {
                handle_table_ident(rsg, child, ContextDefine);
            }
            is_second_ident = true;
        }
    }
}

inline void handle_for_portion_of_time_clause(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIDENT) {
            auto* new_name = new IR(IRTypeIDENT, string("c0"), DataColumnName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
            break;
        }
    }
}

inline void handle_select_alias(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIDENT || child->get_ir_type() == IRTypeTEXTSTRINGSys) {
            auto* new_name = new IR(IRTypeIDENT, string("alias_0"), DataColumnAliasName, ContextDefine, nullptr);
            cur_ir->swap_one_child(child, new_name);
            break;
        }
    }
}

inline void handle_column_default_non_parenthesized_expr(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIDENT || child->get_ir_type() == IRTypeTableIdent || child->get_ir_type() == IRTypeSimpleIdent 
            || child->get_ir_type() == IRTypeSimpleIdentNospvar
            ) {
            auto* new_name = new IR(IRTypeIDENT, string("c0"), DataColumnName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
            break;
        }
    }
}

inline void handle_ident_func(RSG* rsg, IR*& cur_ir) {
    cur_ir->free_children();
    auto* new_name = new IR(IRTypeIDENT, string("COUNT"), DataFunctionName, ContextUse, nullptr);
    cur_ir->add_one_child(new_name, 0);
    return;
}   

inline void handle_function_call_generic(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    if (children.front()->get_ir_type() == IRTypeIdentCli) {
        children.front()->free_children();
        children[1]->set_str_val(string(""));
        children[1]->set_ir_type(IRTypeUnknownType);
    }
}

inline void handle_window_name(RSG* rsg, IR*& cur_ir) {
    cur_ir->free_children();
    auto* new_name = new IR(IRTypeIDENT, string("window_name_0"), DataWindowName, ContextUse, nullptr);
    cur_ir->add_one_child(new_name, 0);
    return;
}

inline void handle_ident_list_arg(RSG* rsg, IR*& cur_ir) {
    cur_ir->free_children();
    auto* new_name = new IR(IRTypeIDENT, string("c0"), DataColumnName, ContextUse, nullptr);
    cur_ir->add_one_child(new_name, 0);
    return;
}

inline void handle_json_table_column(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIDENT) {
            auto* new_name = new IR(IRTypeIDENT, string("c0"), DataColumnName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
            break;
        }
    }
}

inline void handle_esc_table_ref(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIDENT) {
            auto* new_name = new IR(IRTypeIDENT, string("c0"), DataTableAliasName, ContextDefine, nullptr);
            cur_ir->swap_one_child(child, new_name);
            break;
        }
    }
}

inline void handle_key_usage_element(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIDENT) {
            auto* new_name = new IR(IRTypeIDENT, string("index_0"), DataIndexName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
            break;
        }
    }
}

inline void handle_using_list(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIDENT) {
            auto* new_name = new IR(IRTypeIDENT, string("c0"), DataColumnName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
            break;
        }
    }
}

inline void handle_table_alias_clause(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIdentTableAlias) {
            auto* new_name = new IR(IRTypeIDENT, string("table_alias_0"), DataTableAliasName, ContextDefine, nullptr);
            cur_ir->swap_one_child(child, new_name);
            break;
        }
    }
}

inline void handle_window_def(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIDENT) {
            auto* new_name = new IR(IRTypeIDENT, string("window_name_0"), DataWindowName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
            break;
        }
    }
}

inline void handle_opt_window_ref(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIDENT) {
            auto* new_name = new IR(IRTypeIDENT, string("window_name_0"), DataWindowName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
            break;
        }
    }
}   

inline void handle_alter_order_item(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeSimpleIdentNospvar) {
            auto* new_name = new IR(IRTypeIDENT, string("c0"), DataColumnName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
            break;
        }
    }
}

inline void handle_procedure_clause(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIDENT) {
            auto* new_name = new IR(IRTypeIDENT, string("procedure_0"), DataProcedureName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
            break;
        }
    }
}

inline void handle_select_outvar(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIDENT || child->get_ir_type() == IRTypeIdentOrText) {
            auto* new_name = new IR(IRTypeIDENT, string("var_0"), DataVariableName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
            break;
        }
    }
}

inline void handle_drop(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeTableList) {
            handle_table_list(rsg, child, DataTableName, ContextUndefine);
        }
    }
}

inline void handle_insert_ident(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeInsertIdent) {
            auto* new_name = new IR(IRTypeIDENT, string("c0"), DataColumnName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
            break;
        }
    }
}

inline void handle_simple_ident_nospvar(RSG* rsg, IR*& cur_ir) {
    cur_ir->free_children();
    auto* new_name = new IR(IRTypeIDENT, string("c0"), DataColumnName, ContextUse, nullptr);
    cur_ir->add_one_child(new_name, 0);
    return;
}

inline void handle_opt_format_json(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIdentOrText) {
            auto* new_name = new IR(SymbolTerm, IRTypeUnknownType, string("JSON"), nullptr, nullptr, nullptr);
            cur_ir->swap_one_child(child, new_name);
            break;
        }
    }
}

inline void handle_use(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIDENT) {
            auto* new_name = new IR(IRTypeIDENT, string("test_123"), DataDatabaseName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
            break;
        }
    }
}

inline void handle_field_or_var(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeSimpleIdentNospvar) {
            auto* new_name = new IR(IRTypeIDENT, string("c0"), DataColumnName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
            break;
        }
    }
}   

inline void handle_opt_column_name_list(RSG* rsg, IR*& cur_ir, DATATYPE data_type = DataColumnName, DATAFLAG context_type = ContextUse) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeCommaSeparatedIdentList) {
            auto* new_name = new IR(IRTypeIDENT, string("c_0"), data_type, context_type, nullptr);
            cur_ir->swap_one_child(child, new_name);
            break;
        }
    }
}

inline void handle_with_column_list(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeOptColumnNameList) {
            auto* new_name = new IR(IRTypeIDENT, string("ca_0"), DataColumnAliasName, ContextDefine, nullptr);
            cur_ir->swap_one_child(child, new_name);
            break;
        }
    }
}   

inline void handle_with_element_head(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIDENT) {
            auto* new_name = new IR(IRTypeIDENT, string("ta_0"), DataTableAliasName, ContextDefine, nullptr);
            cur_ir->swap_one_child(child, new_name);
            break;
        }
    }
}

inline void handle_table_wild(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    if (children.size() == 5) {
        // ident '.' ident '.' '*' // remove the first two children
        cur_ir->detach_one_child(children.front());
        cur_ir->detach_one_child(children[1]);
        children.front()->deep_drop();
        children[1]->deep_drop();
    }

    // Must re-get the children after dropping the first two children.
    children = children.back()->get_children();

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIDENT) {
            auto* new_name = new IR(IRTypeIDENT, string("c0"), DataColumnName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
            break;
        }
    }
}

inline void handle_table_ident_opt_wild(RSG* rsg, IR*& cur_ir) {
    BEGIN;

        if (children.size() == 4) {
        // ident '.' // remove the first two children
        cur_ir->detach_one_child(children.front());
        cur_ir->detach_one_child(children[1]);
        children.front()->deep_drop();
        children[1]->deep_drop();
    }

    // Must re-get the children after dropping the first two children.
    children = children.back()->get_children();

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIDENT) {
            auto* new_name = new IR(IRTypeIDENT, string("v0"), DataTableName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
            break;
        }
    }
}

inline void handle_table_ident_nodb(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIDENT) {
            auto* new_name = new IR(IRTypeIDENT, string("v0"), DataTableName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
            break;
        }
    }
}

inline void handle_grant_ident(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIDENT) {
            auto* new_name = new IR(IRTypeIDENT, string("v0"), DataTableName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
            break;
        }
    }
}

inline void handle_role_name(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIdentOrText) {
            auto* new_name = new IR(IRTypeUnknownType, string("CURRENT_ROLE"), DataUnknownType, ContextUnknown, nullptr);
            cur_ir->swap_one_child(child, new_name);
            break;
        }
    }
}

inline void handle_rollback(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIDENT) {
            auto* new_name = new IR(IRTypeIDENT, string("savepoint_0"), DataSavePointName, ContextUndefine, nullptr);
            cur_ir->swap_one_child(child, new_name);
            break;
        }
    }
}

inline void handle_savepoint(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIDENT) {
            auto* new_name = new IR(IRTypeIDENT, string("savepoint_0"), DataSavePointName, ContextDefine, nullptr);
            cur_ir->swap_one_child(child, new_name);
            break;
        }
    }
}

inline void handle_release(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIDENT) {
            auto* new_name = new IR(IRTypeIDENT, string("savepoint_0"), DataSavePointName, ContextUndefine, nullptr);
            cur_ir->swap_one_child(child, new_name);
            break;
        }
    }
}

inline void handle_view_list(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIDENT) {
            auto* new_name = new IR(IRTypeIDENT, string("view_col_0"), DataViewColumnName, ContextDefine, nullptr);
            cur_ir->swap_one_child(child, new_name);
            break;
        }
    }
}

inline void handle_on_update_cols(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIDENT) {
            auto* new_name = new IR(IRTypeIDENT, string("c0"), DataColumnName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
            break;
        }
    }
}

void mariadb_ir_context_setup(RSG* rsg, IR*& cur_ir)
{
    IRTYPE cur_ir_type = cur_ir->get_ir_type();

    switch (cur_ir_type) {

    case (IRTypeDeallocate):
        handle_deallocate(rsg, cur_ir);
        break;

    case (IRTypePrepare):
        handle_prepare(rsg, cur_ir);
        break;

    // case (IRTypePrepareSrc):
    //     handle_prepare_src(rsg, cur_ir);
    //     break;

    case (IRTypeExecute):
        handle_execute(rsg, cur_ir);
        break;

    case (IRTypeOptionalConnectionName):
        handle_optional_connection_name(rsg, cur_ir);
        break;

    case (IRTypeCreate):
        /* FALLTHROUGH */
    case (IRTypeCreateTable):
        /* FALLTHROUGH */
    case (IRTypeCreateIndex):
        /* FALLTHROUGH */
    case (IRTypeCreateView):
        handle_create(rsg, cur_ir);
        break;

    case (IRTypeServerDef):
        handle_server_def(rsg, cur_ir);
        break;

    case (IRTypeEventTail):
        handle_event_tail(rsg, cur_ir);
        break;

    case (IRTypeDropRoutine):
        handle_drop_routine(rsg, cur_ir);
        break;

    case (IRTypeCall):
        handle_call(rsg, cur_ir);   
        break;

    // case (IRTypeSpHcond):
    //     handle_sp_hcond(rsg, cur_ir);
    //     break;

    case (IRTypeLabelIdent):
        handle_label_ident(rsg, cur_ir);
        break;

    case (IRTypeSpProcStmtClose):
        handle_sp_proc_stmt_close(rsg, cur_ir);
        break;

    case (IRTypeSpFetchList):
        handle_sp_fetch_list(rsg, cur_ir);
        break;

    case (IRTypePartFieldItem):
        handle_part_field_item(rsg, cur_ir);
        break;

    case (IRTypeSubPartFieldItem):
        handle_sub_part_field_item(rsg, cur_ir);
        break;

    case (IRTypePartDefinition):
        handle_part_definition(rsg, cur_ir);
        break;

    case (IRTypePartName):
        handle_part_name(rsg, cur_ir);
        break;

    case (IRTypeSubPartDefinition):
        handle_sub_part_definition(rsg, cur_ir);
        break;

    case (IRTypeEngineDefinedOption):
        handle_engine_defined_option(rsg, cur_ir);
        break;

    case (IRTypeCharsetName):
        handle_charset_name(rsg, cur_ir);
        break;

    case (IRTypeCollationName):
        handle_collation_name(rsg, cur_ir);
        break;  

    case (IRTypeStorageEngines):
        handle_storage_engines(rsg, cur_ir);
        break;

    case (IRTypeKnownStorageEngines):
        handle_known_storage_engines(rsg, cur_ir);
        break;

    case (IRTypeKeyDef):
        handle_key_def(rsg, cur_ir);
        break;

    case (IRTypePeriodForSystemTime):
        handle_period_for_system_time(rsg, cur_ir);
        break;

    case (IRTypePeriodForApplicationTime):
        handle_period_for_application_time(rsg, cur_ir);
        break;

    case (IRTypeConstraint):
        handle_constraint(rsg, cur_ir);
        break;

    case (IRTypeFieldIdent):
        handle_field_ident(rsg, cur_ir);
        break;

    case (IRTypeFieldSpec):
        handle_field_spec(rsg, cur_ir);
        break;

    case (IRTypeUdtName):
        handle_udt_name(rsg, cur_ir);
        break;

    case (IRTypeOptCompressionMethod):
        handle_opt_compression_method(rsg, cur_ir);
        break;

    case (IRTypeOldOrNewCharsetName):
        handle_old_or_new_charset_name(rsg, cur_ir);
        break;

    case (IRTypeReferences):
        handle_references(rsg, cur_ir);
        break;

    case (IRTypeRefList):
        handle_ref_list(rsg, cur_ir);
        break;

    case (IRTypeOptWithoutOverlaps):
        handle_opt_without_overlaps(rsg, cur_ir);
        break;

    case (IRTypeKeyPart):
        handle_key_part(rsg, cur_ir);
        break;

    case (IRTypeKeyPartSimple):
        handle_key_part_simple(rsg, cur_ir);
        break;

    case (IRTypeAlter):
        handle_alter(rsg, cur_ir);
        break;

    case (IRTypeSpName):
        handle_sp_name(rsg, cur_ir);
        break;

    case (IRTypeIdentOrEmpty):
        handle_ident_or_empty(rsg, cur_ir);
        break;      

    case (IRTypeAltPartNameItem):
        handle_alt_part_name_item(rsg, cur_ir);
        break;

    case (IRTypeAlterListItem):
        handle_alter_list_item(rsg, cur_ir);
        break;

    case (IRTypeAlterLockOption):
        handle_alter_lock_option(rsg, cur_ir);
        break;

    case (IRTypeTableList):
        handle_table_list(rsg, cur_ir);
        break;

    case (IRTypeRepairTableOrView):
        handle_repair_table_or_view(rsg, cur_ir);
        break;

    case (IRTypeTableIdent):
        handle_table_ident(rsg, cur_ir);
        break;

    case (IRTypeTableColumnList):
        handle_table_column_list(rsg, cur_ir);
        break;

    case (IRTypeTableIndexList):
        handle_table_index_list(rsg, cur_ir);
        break;

    case (IRTypeTableToTable):
        handle_table_to_table(rsg, cur_ir);
        break;

    case (IRTypeForPortionOfTimeClause):
        handle_for_portion_of_time_clause(rsg, cur_ir);
        break;

    case (IRTypeSelectAlias):
        handle_select_alias(rsg, cur_ir);
        break;

    case (IRTypeSimpleIdent):
        handle_simple_ident(rsg, cur_ir);
        break;

    case (IRTypeColumnDefaultNonParenthesizedExpr):
        handle_column_default_non_parenthesized_expr(rsg, cur_ir);
        break;

    case (IRTypeIdentFunc):
        handle_ident_func(rsg, cur_ir);
        break;

    case (IRTypeFunctionCallGeneric):
        handle_function_call_generic(rsg, cur_ir);
        break;

    case (IRTypeWindowName):
        handle_window_name(rsg, cur_ir);
        break;

    case (IRTypeIdentListArg):
        handle_ident_list_arg(rsg, cur_ir);
        break;

    case (IRTypeJsonTableColumn):
        handle_json_table_column(rsg, cur_ir);
        break;
        
    case (IRTypeEscTableRef):
        handle_esc_table_ref(rsg, cur_ir);
        break;

    case (IRTypeKeyUsageElement):
        handle_key_usage_element(rsg, cur_ir);
        break;

    case (IRTypeUsingList):
        handle_using_list(rsg, cur_ir);
        break;

    case (IRTypeTableAliasClause):
        handle_table_alias_clause(rsg, cur_ir);
        break;

    case (IRTypeWindowDef):
        handle_window_def(rsg, cur_ir);
        break;

    case (IRTypeOptWindowRef):
        handle_opt_window_ref(rsg, cur_ir);
        break;

    case (IRTypeAlterOrderItem):
        handle_alter_order_item(rsg, cur_ir);
        break;

    case (IRTypeProcedureClause):
        handle_procedure_clause(rsg, cur_ir);
        break;

    case (IRTypeSelectOutvar):
        handle_select_outvar(rsg, cur_ir);
        break;

    case (IRTypeDrop):
        handle_drop(rsg, cur_ir);
        break;

    case (IRTypeInsertIdent):
        handle_insert_ident(rsg, cur_ir);
        break;

    case (IRTypeSimpleIdentNospvar):
        handle_simple_ident_nospvar(rsg, cur_ir);
        break;

    case (IRTypeOptFormatJson):
        handle_opt_format_json(rsg, cur_ir);
        break;

    case (IRTypeUse):
        handle_use(rsg, cur_ir);
        break;

    case (IRTypeFieldOrVar):
        handle_field_or_var(rsg, cur_ir);
        break;

    case (IRTypeOptColumnNameList):
        handle_opt_column_name_list(rsg, cur_ir);
        break;

    case (IRTypeWithColumnList):
        handle_with_column_list(rsg, cur_ir);
        break;

    case (IRTypeTableWild):
        // fall through
    case (IRTypeSelectSublistQualifiedAsterisk):
        handle_table_wild(rsg, cur_ir);
        break;

    case (IRTypeTableIdentOptWild):
        handle_table_ident_opt_wild(rsg, cur_ir);
        break;

    case (IRTypeTableIdentNodb):
        handle_table_ident_nodb(rsg, cur_ir);
        break;

    case (IRTypeGrantIdent):
        handle_grant_ident(rsg, cur_ir);
        break;

    case (IRTypeRoleName):
        handle_role_name(rsg, cur_ir);
        break;

    case (IRTypeRollback):
        handle_rollback(rsg, cur_ir);
        break;

    case (IRTypeSavepoint):
        handle_savepoint(rsg, cur_ir);
        break;

    case (IRTypeRelease):
        handle_release(rsg, cur_ir);
        break;

    case (IRTypeViewList):
        handle_view_list(rsg, cur_ir);
        break;

    // case (IRTypeOnUpdateCols):
    //     handle_on_update_cols(rsg, cur_ir);
    //     break;

    default:
        break;
    }

    // IMPORTANT!!!
    for (auto& interesting_type : MariaDBFuzzerConfigurations::mariadb_interesting_ir_types) {
        if (interesting_type == IRTypeSelect) {
            continue;
        }
        if (interesting_type == cur_ir_type) {
            cur_ir->set_is_favor(IsFavor::favor);
            break;
        }
    }
}