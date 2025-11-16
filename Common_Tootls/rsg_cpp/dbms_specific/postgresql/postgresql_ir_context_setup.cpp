//
// Created by XXX on 12/11/24.
//


/* This file is currently still WIP. Will come back later.  */

#include "postgresql_ir_context_setup.h"
#include "postgresql_fuzzer_configurations.h"
#include <initializer_list>

#include "../../headers/fuzzer_configurations.h"

#define BEGIN vector<IR*> children = cur_ir->get_children();

// Forward declarations
inline void handle_IDENT(RSG* rsg, IR*& cur_ir)
{
    // Uncommonly used as fallback placeholder.

    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIDENT) {
            auto* new_name = new IR(IRTypeIDENT, string("v00"), DataTableName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_column_elem(RSG* rsg, IR*& cur_ir, DATAFLAG context_flag = ContextUse) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeColId) {
            auto* new_name = new IR(IRTypeIDENT, string("column_0"), DataColumnName, context_flag, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }

    return;
}

inline void handle_name_list(RSG* rsg, IR*& cur_ir, DATATYPE data_type, DATAFLAG context_flag) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeName) {
            auto* new_name = new IR(IRTypeIDENT, string("va00"), data_type, context_flag, nullptr);
            cur_ir->swap_one_child(child, new_name);
        } else if (child->get_ir_type() == IRTypeNameList) {
            handle_name_list(rsg, child, data_type, context_flag);
        }
    }
}

inline void handle_column_list(RSG* rsg, IR*& cur_ir, DATAFLAG context_flag = ContextUse) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeColumnElem) {
            handle_column_elem(rsg, child, context_flag);
        } else if (child->get_ir_type() == IRTypeColumnList) {
            handle_column_list(rsg, child, context_flag);
        }
    }

    return;
}

inline void handle_opt_single_name(RSG* rsg, IR*& cur_ir, DATATYPE data_type = DataTableName, DATAFLAG context_flag = ContextUse) {
    BEGIN;

    if (children.size() != 1) {
        return;
    }

    cur_ir->free_children();

    IR* new_name = new IR(IRTypeIDENT, string("v00"), data_type, context_flag, nullptr);
    cur_ir->add_one_child(new_name, 0);

    return;
}

inline void handle_opt_qualified_name(RSG* rsg, IR*& cur_ir, DATATYPE data_type = DataTableName, DATAFLAG context_flag = ContextUse) {
    BEGIN;

    if (children.size() != 1) {
        return;
    }

    cur_ir->free_children();

    IR* new_name = new IR(IRTypeIDENT, string("v00"), DataTableName, context_flag, nullptr);
    cur_ir->add_one_child(new_name, 0);
    
    return;
}

inline void handle_qualified_name_list(RSG* rsg, IR*& cur_ir, DATAFLAG context_flag = ContextUse) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeQualifiedName) {
            auto* new_name = new IR(IRTypeIDENT, string("v00"), DataTableName, context_flag, nullptr);
            cur_ir->swap_one_child(child, new_name);
        } else if (child->get_ir_type() == IRTypeQualifiedNameList) {
            handle_qualified_name_list(rsg, child, context_flag);
        }
    }

    return;
}

inline void handle_opt_inherit(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeQualifiedNameList) {
            handle_qualified_name_list(rsg, child, ContextUse);
        }
    }

    return;
}

inline void handle_SCONST(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    cur_ir->set_data_type(DataLiteral);
    cur_ir->set_data_affinity_type(AFFISTRING);
    return;
}

inline void handle_alter_opt_role_elem(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeIDENT) {
            auto* new_name = new IR(SymbolTerm, IRTypeUnknownType, string("PASSWORD NULL"), nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
    
    
}

inline void handle_create_schema_stmt(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeOptSingleName) {
            handle_opt_single_name(rsg, child, DataSchemaName, ContextDefine);
        } else if (child->get_ir_type() == IRTypeColId) {
            auto* new_name = new IR(IRTypeIDENT, string("public"), DataSchemaName, ContextDefine, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }

    return;

}

inline void handle_generic_set(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeVarName) {
            auto* new_name = new IR(IRTypeIDENT, string("var_name"), DataVariableName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }

    // no need to handle var_list or var_value. 

    return;
}

inline void handle_access_method_clause(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeName) {
            auto* new_name = new IR(IRTypeIDENT, string("heap"), DataUnknownType, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
    
}

inline void handle_var_name(RSG* rsg, IR*& cur_ir) {
    cur_ir->free_children();

    IR* new_name = new IR(IRTypeIDENT, string("var_name"), DataVariableName, ContextUse, nullptr);
    cur_ir->add_one_child(new_name, 0);

    return;
}

inline void handle_alter_table_stmt(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    bool is_tablespace = false;
    bool is_index = false;
    bool is_sequence = false;
    bool is_view = false;
    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeRelationExpr) {
            auto* new_name = new IR(IRTypeIDENT, string("v00"), DataTableName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        } else if (child->get_ir_type() == IRTypeTABLESPACE) {
            is_tablespace = true;
            break;
        } else if (child->get_ir_type() == IRTypeINDEX) {
            is_index = true;
            break;
        } else if (child->get_ir_type() == IRTypeSEQUENCE) {
            is_sequence = true;
            break;
        } else if (child->get_ir_type() == IRTypeVIEW) {
            is_view = true;
            // break;
        }
    }

    if (is_tablespace) {
        for (auto& child : children) {
            if (child->get_ir_type() == IRTypeName) {
                auto* new_name = new IR(IRTypeIDENT, string("table_space_0"), DataTableSpaceName, ContextUse, nullptr);
                cur_ir->swap_one_child(child, new_name);
            }
        }
    } else if (is_index) {
        for (auto& child : children) {
            if (child->get_ir_type() == IRTypeName || child->get_ir_type() == IRTypeQualifiedName) {
                auto* new_name = new IR(IRTypeIDENT, string("index_0"), DataIndexName, ContextUse, nullptr);
                cur_ir->swap_one_child(child, new_name);
            }
        }
    } else if (is_sequence) {
        for (auto& child : children) {
            if (child->get_ir_type() == IRTypeName || child->get_ir_type() == IRTypeQualifiedName) {
                auto* new_name = new IR(IRTypeIDENT, string("sequence_0"), DataSequenceName, ContextUse, nullptr);
                cur_ir->swap_one_child(child, new_name);
            }
        }
    } else if (is_view) {
        for (auto& child : children) {
            if (child->get_ir_type() == IRTypeName || child->get_ir_type() == IRTypeQualifiedName) {
                auto* new_name = new IR(IRTypeIDENT, string("view_0"), DataViewName, ContextUse, nullptr);
                cur_ir->swap_one_child(child, new_name);
            }
        }
    }   

    return;
}

inline void handle_partition_cmd(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeQualifiedName) {
            auto* new_name = new IR(IRTypeIDENT, string("partition_0"), DataPartitionName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }

    return;
}

inline void handle_index_partition_cmd(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeQualifiedName) {
            auto* new_name = new IR(IRTypeIDENT, string("partition_0"), DataPartitionName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }

    return;
}

inline void handle_column_def(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeColId) {
            auto* new_name = new IR(IRTypeIDENT, string("column_0"), DataColumnName, ContextDefine, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }

    return;
}

inline void handle_table_constraint(RSG* rsg, IR*& cur_ir, DATAFLAG context_flag = ContextUse) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeName) {
            auto* new_name = new IR(IRTypeIDENT, string("constraint_0"), DataConstraintName, context_flag, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }

    return;
}

inline void handle_type_name(RSG* rsg, IR*& cur_ir) {
    cur_ir->free_children();

    IR* new_name = new IR(IRTypeIDENT, string("INT"), DataTypeName, ContextUse, nullptr);
    cur_ir->add_one_child(new_name, 0);

    return;
}

inline void handle_alter_table_cmd(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    if (children.front()->get_ir_type() == IRTypeINHERIT) {
        IR* new_name = new IR(IRTypeIDENT, string("v00"), DataTableName, ContextUse, nullptr);
        cur_ir->swap_one_child(children.back(), new_name);
        return;
    }

    if (children.size() > 2 && children[1]->get_ir_type() == IRTypeINHERIT) {
        IR* new_name = new IR(IRTypeIDENT, string("v00"), DataTableName, ContextUse, nullptr);
        cur_ir->swap_one_child(children.back(), new_name);
        return;
    }

    bool is_constraint = false;
    bool is_trigger = false;
    bool is_rule = false;
    bool is_tablespace = false;

    DATAFLAG context_flag = ContextUse;
    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeColId) {
            auto* new_name = new IR(IRTypeIDENT, string("column_0"), DataColumnName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        } else if (child->get_ir_type() == IRTypeTableConstraint) {
            handle_table_constraint(rsg, child, ContextDefine);
        } else if (child->get_ir_type() == IRTypeCONSTRAINT) {
            is_constraint = true;
            if (children.front()->get_ir_type() == IRTypeDROP) {
                context_flag = ContextUndefine;
            }
        } else if (child->get_ir_type() == IRTypeTRIGGER) {
            is_trigger = true;
        } else if (child->get_ir_type() == IRTypeRULE) {
            is_rule = true;
        } else if (child->get_ir_type() == IRTypeTABLESPACE) {
            is_tablespace = true;
        } else if (child->get_ir_type() == IRTypeCLUSTER) {
            IR* new_name = new IR(IRTypeIDENT, string("cluster_0"), DataUnknownType, ContextUse, nullptr);
            cur_ir->swap_one_child(children.back(), new_name);
            break;
        }
    }

    if (is_tablespace) {
        IR* new_name = new IR(IRTypeIDENT, string("table_space_0"), DataTableSpaceName, ContextUse, nullptr);
        cur_ir->swap_one_child(children.back(), new_name);
    } else if (is_trigger) {
        IR* new_name = new IR(IRTypeIDENT, string("trigger_0"), DataTriggerName, ContextUse, nullptr);
        cur_ir->swap_one_child(children.back(), new_name);
    } else if (is_rule) {
        IR* new_name = new IR(IRTypeIDENT, string("rule_0"), DataRuleName, ContextUse, nullptr);
        cur_ir->swap_one_child(children.back(), new_name);
    } else if (is_constraint) {
        for (auto& child : children) {
            if (child->get_ir_type() == IRTypeName) {
                auto* new_name = new IR(IRTypeIDENT, string("constraint_0"), DataConstraintName, context_flag, nullptr);
                cur_ir->swap_one_child(child, new_name);
            }
        }
    }

    return;
}

inline void handle_opt_collate_clause(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeAnyName) {
            auto* new_name = new IR(IRTypeIDENT, string("default"), DataCollationName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }

    return;
}

inline void handle_replica_identity(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeName) {
            auto* new_name = new IR(IRTypeIDENT, string("index_0"), DataIndexName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }

    return;
}

inline void handle_reloption_elem(RSG* rsg, IR*& cur_ir) {
    // TODO: handle reloption_elem
}

inline void handle_set_access_method_name(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeColId) {
            auto* new_name = new IR(SymbolTerm, IRTypeDEFAULT, string("DEFAULT"), nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
    return;
}

inline void handle_alter_composite_type_stmt(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeAnyName) {
            auto* new_name = new IR(IRTypeIDENT, string("INT"), DataTypeName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }

    return;
}

inline void handle_alter_type_cmd(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeColId) {
            auto* new_name = new IR(IRTypeIDENT, string("attribute_name_0"), DataUnknownType, ContextUnknown, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }

    return;
}

inline void handle_cursor_name(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    // Just always use cursor_name_0 as the name.
    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeName) {
            auto* new_name = new IR(IRTypeIDENT, string("cursor_name_0"), DataCursorName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }

    return;
}   

inline void handle_copy_stmt(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeQualifiedName) {
            auto* new_name = new IR(IRTypeIDENT, string("v00"), DataTableName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }

    return;
}

inline void handle_opt_column_list(RSG* rsg, IR*& cur_ir, DATAFLAG context_flag = ContextUse) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeColumnList) {
            handle_column_list(rsg, child, context_flag);
        }
    }

    return;
}

inline void handle_col_label(RSG* rsg, IR*& cur_ir, DATATYPE data_type = DataTableName, DATAFLAG context_flag = ContextUse) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeColId) {
            auto* new_name = new IR(IRTypeIDENT, string("v00"), data_type, context_flag, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }

    return;
}   

inline void handle_create_stmt(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    bool is_second_qualified_name = false;
    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeQualifiedName && is_second_qualified_name == false) {
            auto* new_name = new IR(IRTypeIDENT, string("v00"), DataTableName, ContextDefine, nullptr);
            cur_ir->swap_one_child(child, new_name);
        } else if (child->get_ir_type() == IRTypeName) {
            auto* new_name = new IR(IRTypeIDENT, string("v00"), DataTableName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        } else if (child->get_ir_type() == IRTypeQualifiedName && is_second_qualified_name == true) {
            auto* new_name = new IR(IRTypeIDENT, string("partition_name_0"), DataPartitionName, ContextDefine, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }

    return;
}

inline void handle_column_options(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {

        if (child->get_ir_type() == IRTypeColId) {
            auto* new_name = new IR(IRTypeIDENT, string("v00"), DataColumnName, ContextDefine, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }

    return;
}

inline void handle_column_compression(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeColId) {
            auto* new_name = new IR(IRTypeIDENT, string("pglz"), DataCompressionName, ContextUnknown, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }

    return;
}

inline void handle_column_storage(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeColId) {
            auto* new_name = new IR(IRTypeIDENT, string("main"), DataStorageName, ContextUnknown, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }

    return;
}

inline void handle_col_constraint(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeName) {
            auto* new_name = new IR(IRTypeIDENT, string("constraint_name_0"), DataConstraintName, ContextDefine, nullptr);
            cur_ir->swap_one_child(child, new_name);
        } else if (child->get_ir_type() == IRTypeAnyName) {
            auto* new_name = new IR(IRTypeIDENT, string("default"), DataCollationName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }

    return;
}

inline void handle_col_constraint_elem(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeQualifiedName) {
            auto* new_name = new IR(IRTypeIDENT, string("v00"), DataTableName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }

    return;
}

inline void handle_table_like_clause(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeQualifiedName) {
            auto* new_name = new IR(IRTypeIDENT, string("v00"), DataTableName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }

    return;
}

inline void handle_constraint_elem(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeColId) {
            auto* new_name = new IR(IRTypeIDENT, string("constraint_0"), DataConstraintName, ContextDefine, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }

    return;
}

inline void handle_domain_constraint(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeName) {
            auto* new_name = new IR(IRTypeIDENT, string("constraint_0"), DataConstraintName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }

    return;
}   

inline void handle_partition_spec(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeColId) {
            auto* new_name = new IR(IRTypeIDENT, string("partition_name_0"), DataPartitionName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }

    return;
}

inline void handle_part_elem(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeColId) {
            auto* new_name = new IR(IRTypeIDENT, string("c1"), DataColumnName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }

    return;
}

inline void handle_table_access_method_clause(RSG* rsg, IR*& cur_ir) {
    cur_ir->free_children();
    return;
}

inline void handle_opt_table_space(RSG* rsg, IR*& cur_ir) {
    cur_ir->free_children();
    return;
}   

inline void handle_opt_cons_table_space(RSG* rsg, IR*& cur_ir) {
    cur_ir->free_children();
    return;
}

inline void handle_existing_index(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeName) {
            auto* new_name = new IR(IRTypeIDENT, string("index_name_0"), DataIndexName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }

    return;
}

inline void handle_create_stats_stmt(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeOptQualifiedName) {
            handle_opt_qualified_name(rsg, child, DataStatisticsName, ContextUse);
        } else if (child->get_ir_type() == IRTypeOptNameList) {
            // ignore. 
        }
    }

    return;
}


inline void handle_alter_stats_stmt(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeAnyName) {
            auto* new_name = new IR(IRTypeIDENT, string("stats_name_0"), DataStatisticsName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }

    return;
}

inline void handle_create_as_target(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeQualifiedName) {
            auto* new_name = new IR(IRTypeIDENT, string("v00"), DataTableName, ContextDefine, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }

    return;
}

inline void handle_create_mv_target(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeQualifiedName) {
            auto* new_name = new IR(IRTypeIDENT, string("v00"), DataViewName, ContextDefine, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }

    return;
}

inline void handle_refresh_mat_view_stmt(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeQualifiedName) {
            auto* new_name = new IR(IRTypeIDENT, string("v00"), DataViewName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }

    return;
}

inline void handle_create_seq_stmt(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeQualifiedName) {
            auto* new_name = new IR(IRTypeIDENT, string("sequence_name_0"), DataSequenceName, ContextDefine, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }

    return;
}   

inline void handle_alter_seq_stmt(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeQualifiedName) {
            auto* new_name = new IR(IRTypeIDENT, string("sequence_name_0"), DataSequenceName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }

    return;
}

inline void handle_create_plang_stmt(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeName) {
            auto* new_name = new IR(IRTypeIDENT, string("plang_name_0"), DataUnknownType, ContextDefine, nullptr);
            cur_ir->swap_one_child(child, new_name);
        // } else if (child->get_ir_type() == IRTypeHandlerName) {
        //     auto* new_name = new IR(IRTypeIDENT, string("handler_name_0"), DataUnknownType, ContextUse, nullptr);
        //     cur_ir->swap_one_child(child, new_name);
        }
    }

    return;
}

inline void handle_handler_name(RSG* rsg, IR*& cur_ir) {
    cur_ir->free_children();
    auto* new_name = new IR(IRTypeIDENT, string("handler_name_0"), DataUnknownType, ContextUse, nullptr);
    cur_ir->add_one_child(new_name, 0);
    return;
}   

inline void handle_create_table_space_stmt(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeName) {
            auto* new_name = new IR(IRTypeIDENT, string("tablespace_name_0"), DataTableSpaceName, ContextDefine, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
    
    return;
}

inline void handle_drop_table_space_stmt(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeName) {
            auto* new_name = new IR(IRTypeIDENT, string("tablespace_name_0"), DataTableSpaceName, ContextUndefine, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }

    return;
}

inline void handle_create_extension_stmt(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeName) {
            auto* new_name = new IR(IRTypeIDENT, string("extension_name_0"), DataUnknownType, ContextDefine, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_create_extension_opt_item(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeName) {
            auto* new_name = new IR(IRTypeIDENT, string("public"), DataSchemaName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }

    return;
}

inline void handle_alter_extension_stmt(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeName) {
            auto* new_name = new IR(IRTypeIDENT, string("extension_name_0"), DataUnknownType, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }

    return;
}

inline void handle_alter_extension_contents_stmt(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    bool is_first = true;
    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeName && is_first) {
            auto* new_name = new IR(IRTypeIDENT, string("extension_name_0"), DataUnknownType, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
            is_first = false;
        } // Give up the second one. 
    }

    return;
}

inline void handle_create_fdw_stmt(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeName) {
            auto* new_name = new IR(IRTypeIDENT, string("fdw_name_0"), DataUnknownType, ContextDefine, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }

    return;
}   

inline void handle_alter_fdw_stmt(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeName) {
            auto* new_name = new IR(IRTypeIDENT, string("fdw_name_0"), DataUnknownType, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }

    return;
}   

inline void handle_create_foreign_server_stmt(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeName) {
            auto* new_name = new IR(IRTypeIDENT, string("foreign_server_name_0"), DataServerName, ContextDefine, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }

    return;
}

inline void handle_alter_foreign_server_stmt(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeName) {
            auto* new_name = new IR(IRTypeIDENT, string("foreign_server_name_0"), DataServerName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }

    return;
}

inline void handle_create_foreign_table_stmt(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeQualifiedName) {
            auto* new_name = new IR(IRTypeIDENT, string("v00"), DataTableName, ContextDefine, nullptr);
            cur_ir->swap_one_child(child, new_name);
        } else if (child->get_ir_type() == IRTypeName) {
            auto* new_name = new IR(IRTypeIDENT, string("foreign_server_name_0"), DataServerName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }

    return;
}

inline void handle_import_foreign_schema_stmt(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    bool is_schema = true;
    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeSCHEMA) {
            is_schema = true;
        } else if (child->get_ir_type() == IRTypeSERVER) {
            is_schema = false;
        } else if (child->get_ir_type() == IRTypeName) {
            if (is_schema) {
                auto* new_name = new IR(IRTypeIDENT, string("schema_name_0"), DataSchemaName, ContextUse, nullptr);
                cur_ir->swap_one_child(child, new_name);
            } else {
                auto* new_name = new IR(IRTypeIDENT, string("server_name_0"), DataServerName, ContextUse, nullptr);
                cur_ir->swap_one_child(child, new_name);
            }
        }
    }
    return;
}   

inline void handle_create_user_mapping_stmt(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeName) {
            auto* new_name = new IR(IRTypeIDENT, string("server_name_0"), DataServerName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }

    return;
}   

inline void handle_drop_user_mapping_stmt(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeName) {
            auto* new_name = new IR(IRTypeIDENT, string("server_name_0"), DataServerName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }

    return;
}       

inline void handle_alter_user_mapping_stmt(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeName) {
            auto* new_name = new IR(IRTypeIDENT, string("server_name_0"), DataServerName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }

    return;
}

inline void handle_create_policy_stmt(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeQualifiedName) {
            auto* new_name = new IR(IRTypeIDENT, string("policy_name_0"), DataPolicyName, ContextDefine, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }

    return;
}

inline void handle_alter_policy_stmt(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeQualifiedName) {
            auto* new_name = new IR(IRTypeIDENT, string("policy_name_0"), DataPolicyName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }

    return;
}

inline void handle_role_list(RSG* rsg, IR*& cur_ir) {   
    cur_ir->free_children();
    auto* new_name = new IR(IRTypeIDENT, string("CURRENT_USER"), DataRoleName, ContextUse, nullptr);
    cur_ir->add_one_child(new_name, 0);
    return;
}

inline void handle_role_spec(RSG* rsg, IR*& cur_ir) {
    cur_ir->free_children();
    auto* new_name = new IR(IRTypeIDENT, string("CURRENT_USER"), DataRoleName, ContextUse, nullptr);
    cur_ir->add_one_child(new_name, 0);
    return;
}

inline void handle_row_security_default_permissive(RSG* rsg, IR*& cur_ir) {
    cur_ir->free_children();
    return;
}

inline void handle_create_am_stmt(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeName) {
            auto* new_name = new IR(IRTypeIDENT, string("am_name_0"), DataUnknownType, ContextDefine, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }

    return;
}

inline void handle_create_trig_stmt(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeName) {
            auto* new_name = new IR(IRTypeIDENT, string("trigger_name_0"), DataTriggerName, ContextDefine, nullptr);
            cur_ir->swap_one_child(child, new_name);
        } else if (child->get_ir_type() == IRTypeQualifiedName) {
            auto* new_name = new IR(IRTypeIDENT, string("table_name_0"), DataTableName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);    
        } else if (child->get_ir_type() == IRTypeFuncName) {
            auto* new_name = new IR(IRTypeIDENT, string("COUNT"), DataFunctionName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }

    return;
}

inline void handle_trigger_transitions(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeTransitionRelName) {
            auto* new_name = new IR(IRTypeIDENT, string("trigger_transition_name_0"), DataUnknownType, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_opt_constr_from_table(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeQualifiedName) {
            auto* new_name = new IR(IRTypeIDENT, string("v00"), DataTableName, ContextUseTop, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }

    return;
}

inline void handle_create_event_trig_stmt(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeName) {
            auto* new_name = new IR(IRTypeIDENT, string("event_trigger_name_0"), DataTriggerName, ContextDefine, nullptr);
            cur_ir->swap_one_child(child, new_name);
        } else if (child->get_ir_type() == IRTypeColLabel) {
            auto* new_name = new IR(IRTypeIDENT, string("v00"), DataTableName, ContextUseTop, nullptr);
            cur_ir->swap_one_child(child, new_name);
        } else if (child->get_ir_type() == IRTypeFuncName) {
            auto* new_name = new IR(IRTypeIDENT, string("COUNT"), DataFunctionName, ContextUseTop, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
    
}   

inline void handle_event_trigger_when_item(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeColId) {   
            auto* new_name = new IR(IRTypeIDENT, string("var_0"), DataUnknownType, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }

    return;
}   

inline void handle_alter_event_trig_stmt(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeName) {
            auto* new_name = new IR(IRTypeIDENT, string("trigger_name_0"), DataTriggerName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }

    return;
}

inline void handle_create_assertion_stmt(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeAnyName) {
            auto* new_name = new IR(IRTypeIDENT, string("assertion_name_0"), DataUnknownType, ContextDefine, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }

    return;
}


inline void handle_alter_enum_stmt(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeAnyName) {
            auto* new_name = new IR(IRTypeIDENT, string("enum_0"), DataUnknownType, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
            break;
        }
    }
}

inline void handle_create_op_class_stmt(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeAnyName) {
            auto* new_name = new IR(IRTypeIDENT, string("op_class_0"), DataUnknownType, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        } else if (child->get_ir_type() == IRTypeName) {
            auto* new_name = new IR(IRTypeIDENT, string("v00"), DataTableName, ContextUseTop, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_opt_op_family(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeAnyName) {
            auto* new_name = new IR(IRTypeIDENT, string("family_name_0"), DataFamilyName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_opclass_purpose(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeAnyName) {
            auto* new_name = new IR(IRTypeIDENT, string("c01"), DataColumnName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
    
}   

inline void handle_create_op_family_stmt(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeAnyName) {    
            auto* new_name = new IR(IRTypeIDENT, string("family_name_0"), DataFamilyName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);    
        } else if (child->get_ir_type() == IRTypeName) {
            auto* new_name = new IR(IRTypeIDENT, string("gist"), DataUnknownType, ContextUnknown, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_alter_op_family_stmt(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeAnyName) {
            auto* new_name = new IR(IRTypeIDENT, string("family_name_0"), DataUnknownType, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        } else if (child->get_ir_type() == IRTypeName) {
            auto* new_name = new IR(IRTypeIDENT, string("gist"), DataUnknownType, ContextUnknown, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}   

inline void handle_drop_op_class_stmt(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeAnyName) {
            auto* new_name = new IR(IRTypeIDENT, string("op_class_0"), DataUnknownType, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        } else if (child->get_ir_type() == IRTypeName) {
            auto* new_name = new IR(IRTypeIDENT, string("gist"), DataUnknownType, ContextUnknown, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_drop_op_family_stmt(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeAnyName) {
            auto* new_name = new IR(IRTypeIDENT, string("family_name_0"), DataUnknownType, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        } else if (child->get_ir_type() == IRTypeName) {
            auto* new_name = new IR(IRTypeIDENT, string("gist"), DataUnknownType, ContextUnknown, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline DATATYPE get_type_from_object_type_any_name(IR*& cur_ir) {
    BEGIN;

    IR* child = cur_ir->get_children_ref().front();

    if (child->get_ir_type() == IRTypeTABLE) {
        return DataTableName;
    } else if (child->get_ir_type() == IRTypeSEQUENCE) {
        return DataSequenceName;
    } else if (child->get_ir_type() == IRTypeVIEW) {
        return DataViewName;
    } else if (child->get_ir_type() == IRTypeMATERIALIZED) {
        return DataViewName;
    } else if (child->get_ir_type() == IRTypeFOREIGN) {
        return DataTableName;
    } else if (child->get_ir_type() == IRTypeCOLLATION) {
        return DataCollationName;
    } else if (child->get_ir_type() == IRTypeINDEX) {
        return DataIndexName;
    } else if (child->get_ir_type() == IRTypeSTATISTICS) {
        return DataStatisticsName;
    } else {
        return DataUnknownType;
    }
}

inline DATATYPE get_type_from_drop_type_name(IR*& cur_ir) {
    BEGIN;

    IR* child = cur_ir->get_children_ref().front();

    if (child->get_ir_type() == IRTypeACCESS) {
        return DataAccessMethodName;
    } else if (child->get_ir_type() == IRTypeEVENT) {
        return DataEventName; 
    } else if (child->get_ir_type() == IRTypeSERVER) {
        return DataServerName;
    } else {
        return DataUnknownType;
    }
}

inline DATATYPE get_type_from_object_type_name_on_any_name(IR*& cur_ir) {
    BEGIN;

    IR* child = cur_ir->get_children_ref().front();
    
    if (child->get_ir_type() == IRTypePOLICY) {
        return DataUnknownType;
    } else if (child->get_ir_type() == IRTypeRULE) {
        return DataUnknownType;
    } else if (child->get_ir_type() == IRTypeTRIGGER) {
        return DataTriggerName;
    } else {
        return DataUnknownType;
    }
}   

inline DATATYPE get_type_from_object_type_name(IR*& cur_ir) {
    BEGIN;

    IR* child = cur_ir->get_children_ref().front();

    if (child->get_ir_type() == IRTypeDATABASE) {
        return DataDatabaseName;
    } else if (child->get_ir_type() == IRTypeROLE) {
        return DataRoleName;
    } else if (child->get_ir_type() == IRTypeTABLESPACE) {
        return DataTableSpaceName;
    } else if (child->get_ir_type() == IRTypeDropTypeName) {
        return get_type_from_drop_type_name(child);
    } else {
        return DataUnknownType;
    }
}
    
inline void handle_drop_stmt(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    DATATYPE data_type = DataTableName;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeObjectTypeAnyName) {
            data_type = get_type_from_object_type_any_name(child);
            break;
        } else if (child->get_ir_type() == IRTypeDropTypeName) {
            data_type = get_type_from_drop_type_name(child);
            break;
        } else if (child->get_ir_type() == IRTypeObjectTypeNameOnAnyName) {
            data_type = get_type_from_object_type_name_on_any_name(child);
            break;
        } else if (child->get_ir_type() == IRTypeTYPEP) {
            data_type = DataTypeName;
            break;
        } else if (child->get_ir_type() == IRTypeINDEX) {
            data_type = DataIndexName;
            break;
        }
    }

    bool is_first = true;
    for (auto& child : children) {
        if (is_first) {
            if (child->get_ir_type() == IRTypeAnyNameList || child->get_ir_type() == IRTypeNameList || child->get_ir_type() == IRTypeName
            || child->get_ir_type() == IRTypeTypeNameList) {
                IR* new_child = new IR(IRTypeIDENT, string("v00"), data_type, ContextUse, nullptr);
                cur_ir->swap_one_child(child, new_child);
            }
            is_first = false;
        } else {
            if (child->get_ir_type() == IRTypeAnyName) {
                auto* new_name = new IR(IRTypeIDENT, string("v00"), DataTableName, ContextUse, nullptr);
                cur_ir->swap_one_child(child, new_name);
            }
        }
    }

}

inline void handle_any_name(RSG* rsg, IR*& cur_ir) {
    // Just placeholder for any name. 
    cur_ir->free_children();

    IR* new_child = new IR(IRTypeIDENT, string("v00"), DataTableName, ContextUse, nullptr);
    cur_ir->add_one_child(new_child, 0);
}

inline void handle_define_stmt(RSG* rsg, IR*& cur_ir) {
    // TODO: Implement this.
    return;
}

inline void handle_sec_label_stmt(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    DATATYPE data_type = DataTableName;
    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeObjectTypeAnyName) {
            data_type = get_type_from_object_type_any_name(child);
            break;
        } else if (child->get_ir_type() == IRTypeTYPEP) {
            data_type = DataTypeName;
            break;
        } else if (child->get_ir_type() == IRTypeCOLUMN) {
            data_type = DataColumnName;
            break;
        } else if (child->get_ir_type() == IRTypeObjectTypeName) {
            data_type = get_type_from_object_type_name(child);
            break;
        } else if (child->get_ir_type() == IRTypePROCEDURE) {
            data_type = DataProcedureName;
            break;
        }
    }

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeTypename) {
            auto* new_name = new IR(IRTypeIDENT, string("INT"), DataTypeName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        } else if (child->get_ir_type() == IRTypeAnyName) {
            auto* new_name = new IR(IRTypeIDENT, string("c01"), data_type, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }

    return;
}   

inline void handle_col_id(RSG* rsg, IR*& cur_ir) {
    cur_ir->free_children();

    IR* new_child = new IR(IRTypeIDENT, string("c01"), DataUnknownType, ContextUse, nullptr);
    cur_ir->add_one_child(new_child, 0);

    return;
}

inline void handle_DefACLOptionList(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    if (children.size() != 3) {
        return;
    }

    DATATYPE data_type = DataSchemaName;

    if (children[1]->get_ir_type() != IRTypeSCHEMA) {
        data_type = DataRoleName;
    }

    IR* new_child = new IR(IRTypeIDENT, string("role_name_0"), data_type, ContextUse, nullptr);
    cur_ir->swap_one_child(children.back(), new_child);

    return;
}

inline void handle_privilege_target(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    DATATYPE data_type = DataTableName;

    if (children.front()->get_ir_type() == IRTypeTABLE) {
        data_type = DataTableName;
    } else if (children.front()->get_ir_type() == IRTypeSEQUENCE) {
        data_type = DataSequenceName;
    } else if (children.front()->get_ir_type() == IRTypeFOREIGN) {
        data_type = DataTableName;
    } else if (children.front()->get_ir_type() == IRTypeFUNCTION) {
        data_type = DataFunctionName;
    } else if (children.front()->get_ir_type() == IRTypePROCEDURE) {
        data_type = DataProcedureName;
    } else if (children.front()->get_ir_type() == IRTypeDATABASE) {
        data_type = DataDatabaseName;
    } else if (children.front()->get_ir_type() == IRTypeSCHEMA || children.front()->get_ir_type() == IRTypeALL) {
        data_type = DataSchemaName;
    } else if (children.front()->get_ir_type() == IRTypeTABLESPACE) {
        data_type = DataTableSpaceName;
    } else {
        data_type = DataUnknownType;
    }

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeQualifiedNameList || child->get_ir_type() == IRTypeNameList || child->get_ir_type() == IRTypeAnyNameList) {
            auto* new_name = new IR(IRTypeIDENT, string("v00"), data_type, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        } 
    }

    return;
}

inline void handle_index_elem_options(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeOptQualifiedName || child->get_ir_type() == IRTypeAnyName) {
            IR* new_child = new IR(IRTypeIDENT, string("unicode"), DataCollationName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_child);
        }
    }

    return;
}

inline void handle_index_stmt(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeOptSingleName || child->get_ir_type() == IRTypeName) {
            auto* new_name = new IR(IRTypeIDENT, string("index_name_0"), DataIndexName, ContextDefine, nullptr);
            cur_ir->swap_one_child(child, new_name);    
        }
    }

    return;
}

inline void handle_opt_collate(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeAnyName) {
            auto* new_name = new IR(IRTypeIDENT, string("unicode"), DataCollationName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }

}

inline void handle_index_elem(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeColId) {
            auto* new_name = new IR(IRTypeIDENT, string("c01"), DataColumnName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }

    return;
}

inline void handle_create_function_stmt(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeFuncName) {
            auto* new_name = new IR(IRTypeIDENT, string("func_name_0"), DataFunctionName, ContextDefine, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }

    return;
}

inline void handle_function_with_argtypes(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeFuncName) {
            auto* new_name = new IR(IRTypeIDENT, string("func_name_0"), DataFunctionName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        } else if (child->get_ir_type() == IRTypeColId) {
            auto* new_name = new IR(IRTypeIDENT, string("func_name_0"), DataFunctionName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        } else if (child->get_ir_type() == IRTypeIndirection) {
            child->free_children();
            child->set_symbol_type(SymbolTerm);
            // child->set_ir_type(IRTypeUnknownType);
            child->set_str_val("");
        }
    }

}

inline void handle_func_arg(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {

        if (child->get_ir_type() == IRTypeParamName) {
            auto* new_name = new IR(IRTypeIDENT, string("c01"), DataParamName, ContextDefine, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
        
    }
}

inline void handle_param_name(RSG* rsg, IR*& cur_ir) {
    // just a placeholder. 
    cur_ir->free_children();

    IR* new_name = new IR(IRTypeIDENT, string("param_name_0"), DataParamName, ContextUse, nullptr);
    cur_ir->add_one_child(new_name, 0);

    return;
}   

inline void handle_create_transform_stmt(RSG* rsg, IR*& cur_ir) {
    // FIXME:: This is wrong. 
    BEGIN;

    for (auto& child : children) {
        
        if (child->get_ir_type() == IRTypeName) {
            auto* new_name = new IR(IRTypeIDENT, string("SQL"), DataUnknownType, ContextDefine, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_reindex_stmt(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeOptSingleName || child->get_ir_type() == IRTypeQualifiedName) {
            auto* new_name = new IR(IRTypeIDENT, string("index_name_0"), DataIndexName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        } else if (child->get_ir_type() == IRTypeName) {
            auto* new_name = new IR(IRTypeIDENT, string("public"), DataSchemaName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }

}

inline void handle_alter_tbl_spc_stmt(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeName) {
            auto* new_name = new IR(IRTypeIDENT, string("pg_default"), DataTableSpaceName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_rename_stmt(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    DATATYPE data_type = DataTableName;    

    if (children.size() <= 3) {
        return;
    }

    IRTYPE tmp_type = children[1]->get_ir_type();

    if (tmp_type == IRTypeAGGREGATE) {
        data_type = DataFunctionName;
    } else if (tmp_type == IRTypeCOLLATION) {
        data_type = DataCollationName;
    } else if (tmp_type == IRTypeDATABASE) {
        data_type = DataDatabaseName;
    } else if (tmp_type == IRTypeFOREIGN) {
        data_type = DataTableName;
    } else if (tmp_type == IRTypeFUNCTION) {
        data_type = DataFunctionName;
    } else if (tmp_type == IRTypeSCHEMA) {
        data_type = DataSchemaName;
    } else if (tmp_type == IRTypeTABLE) {
        data_type = DataTableName;
    } else if (tmp_type == IRTypeSEQUENCE) {
        data_type = DataSequenceName;
    } else if (tmp_type == IRTypeVIEW || tmp_type == IRTypeMATERIALIZED) {
        data_type = DataViewName;
    } else if (IRTypeINDEX) {
        data_type = DataIndexName;
    }

    bool is_rename_column = false;
    tmp_type = children[children.size() - 2]->get_ir_type();

    if (tmp_type == IRTypeName) {
        is_rename_column = true;
    }

    int index = 0;
    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeQualifiedName || child->get_ir_type() == IRTypeName || child->get_ir_type() == IRTypeRoleId || child->get_ir_type() == IRTypeRelationExpr) {
            if (index == 0) {
                if (is_rename_column) {
                    auto* new_name = new IR(IRTypeIDENT, string("c01"), data_type, ContextUse, nullptr);
                    cur_ir->swap_one_child(child, new_name);
                } else {
                    auto* new_name = new IR(IRTypeIDENT, string("c01"), data_type, ContextUndefine, nullptr);
                    cur_ir->swap_one_child(child, new_name);
                }
            } else if (index == 1) {
                if (is_rename_column) {
                    auto* new_name = new IR(IRTypeIDENT, string("c02"), DataColumnName, ContextUndefine, nullptr);
                    cur_ir->swap_one_child(child, new_name);
                } else {
                    auto* new_name = new IR(IRTypeIDENT, string("c02"), data_type, ContextDefine, nullptr);
                    cur_ir->swap_one_child(child, new_name);
                }
            } else { // index == 2
                auto* new_name = new IR(IRTypeIDENT, string("c02"), DataColumnName, ContextDefine, nullptr);
                cur_ir->swap_one_child(child, new_name);
            }
            index++;
        }
    }

    return;
}

inline void handle_alter_object_schema_stmt(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    if (children.size() <= 4) {
        return;
    }

    DATATYPE data_type = DataTableName;    

    IRTYPE tmp_type = children[1]->get_ir_type();

    if (tmp_type == IRTypeTABLE) {
        data_type = DataTableName;
    } else if (tmp_type == IRTypeVIEW || tmp_type == IRTypeMATERIALIZED) {
        data_type = DataViewName;
    } else if (tmp_type == IRTypeFOREIGN) {
        data_type = DataTableName;
    }

    int index = 0;
    for (auto& child : children) {
        index++;

        if (index == children.size()) {
            auto* new_name = new IR(IRTypeIDENT, string("public"), DataSchemaName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        } else if (child->get_ir_type() == IRTypeQualifiedName || child->get_ir_type() == IRTypeAnyName || child->get_ir_type() == IRTypeName) {
            auto* new_name = new IR(IRTypeIDENT, string("c02"), data_type, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }

}

inline void handle_alter_type_stmt(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeAnyName) {
            auto* new_name = new IR(IRTypeIDENT, string("INT"), DataTypeName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
    
}

inline void handle_alter_owner_stmt(RSG* rsg, IR*& cur_ir) {
    // TODO: Implement this.
}

inline void handle_create_publication_stmt(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeName) {
            auto* new_name = new IR(IRTypeIDENT, string("publication_name_0"), DataUnknownType, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_relation_expr(RSG* rsg, IR*& cur_ir) {
    cur_ir->free_children();

    IR* new_name = new IR(IRTypeIDENT, string("table_name_0"), DataTableName, ContextUse, nullptr);
    cur_ir->add_one_child(new_name, 0);
}   

inline void handle_alter_publication_stmt(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeName) {
            auto* new_name = new IR(IRTypeIDENT, string("publication_name_0"), DataUnknownType, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_create_subscription_stmt(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeName) {
            auto* new_name = new IR(IRTypeIDENT, string("subscription_name_0"), DataUnknownType, ContextDefine, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_alter_subscription_stmt(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeName) {
            auto* new_name = new IR(IRTypeIDENT, string("subscription_name_0"), DataUnknownType, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}


inline void handle_drop_subscription_stmt(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeName) {
            auto* new_name = new IR(IRTypeIDENT, string("subscription_name_0"), DataUnknownType, ContextUndefine, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_rule_stmt(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeName) {
            auto* new_name = new IR(IRTypeIDENT, string("rule_name_0"), DataRuleName, ContextDefine, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_notify_stmt(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeName) {
            auto* new_name = new IR(IRTypeIDENT, string("channel_name_0"), DataChannelName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_listen_stmt(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeName) {
            auto* new_name = new IR(IRTypeIDENT, string("channel_name_0"), DataChannelName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_unlisten_stmt(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeName) {
            auto* new_name = new IR(IRTypeIDENT, string("channel_name_0"), DataChannelName, ContextUndefine, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_transaction_stmt(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    // DATAFLAG context_flag = ContextUse;

    // for (auto& child : children) {
    //     if (child->get_ir_type() == IRTypeRELEASE || child->get_ir_type() == IRTypeROLLBACK) {
    //         auto* new_name = new IR(IRTypeIDENT, string("transaction_name_0"), DataUnknownType, context_flag, nullptr);
    //         cur_ir->swap_one_child(child, new_name);
    //     }
    // }

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeColId) {
            auto* new_name = new IR(IRTypeIDENT, string("transaction_name_0"), DataUnknownType, ContextUnknown, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_view_stmt(RSG* rsg, IR*& cur_ir) {
    BEGIN;
    
    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeQualifiedName) {
            auto* new_name = new IR(IRTypeIDENT, string("view_name_0"), DataViewName, ContextDefine, nullptr);
            cur_ir->swap_one_child(child, new_name);
        } else if (child->get_ir_type() == IRTypeOptColumnList) {
            handle_opt_column_list(rsg, cur_ir, ContextDefine);
        } else if (child->get_ir_type() == IRTypeColumnList) {
            handle_column_list(rsg, cur_ir, ContextDefine);
        }
    }
}

inline void handle_create_db_stmt(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeName) {
            auto* new_name = new IR(IRTypeIDENT, string("name_0"), DataDatabaseName, ContextDefine, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_alter_database_stmt(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeName) {
            auto* new_name = new IR(IRTypeIDENT, string("test123"), DataDatabaseName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_alter_database_set_stmt(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeName) {
            auto* new_name = new IR(IRTypeIDENT, string("test123"), DataDatabaseName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_drop_db_stmt(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeName) {
            auto* new_name = new IR(IRTypeIDENT, string("duck"), DataDatabaseName, ContextUndefine, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
    
}

inline void handle_alter_collation_stmt(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeAnyName) {
            auto* new_name = new IR(IRTypeIDENT, string("default"), DataCollationName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_create_domain_stmt(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeAnyName) {
            auto* new_name = new IR(IRTypeIDENT, string("domain_name_0"), DataUnknownType, ContextDefine, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}   

inline void handle_alter_domain_stmt(RSG* rsg, IR*& cur_ir) {   
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeAnyName) {
            auto* new_name = new IR(IRTypeIDENT, string("domain_name_0"), DataUnknownType, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_create_conversion_stmt(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeAnyName) {
            auto* new_name = new IR(IRTypeIDENT, string("conversion_name_0"), DataUnknownType, ContextDefine, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_prepare_stmt(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeAnyName) {
            auto* new_name = new IR(IRTypeIDENT, string("prepare_name_0"), DataStatementPreparedName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_execute_stmt(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeName) {
            auto* new_name = new IR(IRTypeIDENT, string("prepared_name_0"), DataStatementPreparedName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_deallocate_stmt(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeName) {
            auto* new_name = new IR(IRTypeIDENT, string("prepared_name_0"), DataStatementPreparedName, ContextUndefine, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_insert_target(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeQualifiedName) {
            auto* new_name = new IR(IRTypeIDENT, string("v00"), DataTableName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        } else if (child->get_ir_type() == IRTypeColId) {
            auto* new_name = new IR(IRTypeIDENT, string("va_00"), DataTableAliasName, ContextDefine, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_insert_column_item(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeColId) {
            auto* new_name = new IR(IRTypeIDENT, string("c01"), DataColumnName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_opt_indirection(RSG* rsg, IR*& cur_ir) {
    cur_ir->free_children();
}

inline void handle_returning_option(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeColId) {
            auto* new_name = new IR(IRTypeIDENT, string("ca01"), DataColumnAliasName, ContextDefine, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_relation_expr_opt_alias(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeRelationExpr) {
            auto* new_name = new IR(IRTypeIDENT, string("v00"), DataTableName, ContextUseTop, nullptr);
            cur_ir->swap_one_child(child, new_name);
        } else if (child->get_ir_type() == IRTypeAS) {
            cur_ir->set_str_val("");
            // cur_ir->set_ir_type(IRTypeUnknownType);
        } else if (child->get_ir_type() == IRTypeColId) {
            child->free_children();
        }
    }
}

inline void handle_table_ref(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeRelationExpr) {
            IR* new_child = new IR(IRTypeIDENT, string("v00"), DataTableName, ContextUseTop, nullptr);
            cur_ir->swap_one_child(child, new_child);
        }
    }
}

inline void handle_alias_clause(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeColId) {
            IR* new_child = new IR(IRTypeIDENT, string("va00"), DataTableAliasName, ContextDefine, nullptr);
            cur_ir->swap_one_child(child, new_child);
        } else if (child->get_ir_type() == IRTypeNameList) {
            handle_name_list(rsg, child, DataColumnAliasName, ContextDefine);
        }
    }
}

inline void handle_lock_stmt(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeRelationExprList) {
            IR* new_child = new IR(IRTypeIDENT, string("v00"), DataTableName, ContextUseTop, nullptr);
            cur_ir->swap_one_child(child, new_child);
        }
    }
}


inline void handle_opt_name_list(RSG* rsg, IR*& cur_ir, DATATYPE data_type, DATAFLAG context_flag) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeNameList) {
            handle_name_list(rsg, child, data_type, context_flag);
        }
    }
}

inline void handle_common_table_expr(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeName) {
            auto* new_name = new IR(IRTypeIDENT, string("va00"), DataTableAliasName, ContextDefine, nullptr);
            cur_ir->swap_one_child(child, new_name);
        } else if (child->get_ir_type() == IRTypeOptNameList) {
            handle_opt_name_list(rsg, child, DataColumnAliasName,ContextDefine);
        }
    }
}


inline void handle_set_target(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeColId) {
            IR* new_child = new IR(IRTypeIDENT, string("c01"), DataColumnName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_child);
        } else if (child->get_ir_type() == IRTypeOptIndirection) {
            child->free_children();
        }
    }
}

inline void handle_opt_search_clause(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeColumnList) {
            IR* new_child = new IR(IRTypeIDENT, string("c01"), DataColumnName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_child);
        } else if (child->get_ir_type() == IRTypeColId) {
            IR* new_child = new IR(IRTypeIDENT, string("c01"), DataColumnName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_child);
        }
    }
}

inline void handle_opt_temp_table_name(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeQualifiedName) {
            IR* new_child = new IR(IRTypeIDENT, string("v00"), DataTableName, ContextDefine, nullptr);
            cur_ir->swap_one_child(child, new_child);
        }
    }
}

inline void handle_opt_cycle_clause(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeColId) {
            IR* new_child = new IR(IRTypeIDENT, string("c01"), DataColumnName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_child);
        }
    }
    
}

inline void handle_opt_alias_clause_for_join_using(RSG* rsg, IR*& cur_ir) {
    cur_ir->free_children();
    return;
}

inline void handle_func_alias_clause(RSG* rsg, IR*& cur_ir) {
    cur_ir->free_children();
    return;
}

inline void handle_join_qual(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeNameList) {
            handle_name_list(rsg, child, DataColumnName, ContextUse);
        }
    }
}

inline void handle_extended_relation_expr(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeQualifiedName) {
            IR* new_child = new IR(IRTypeIDENT, string("v00"), DataTableName, ContextUseTop, nullptr);
            cur_ir->swap_one_child(child, new_child);
        }
    }
}

inline void handle_table_func_element(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeColId) {
            IR* new_child = new IR(IRTypeIDENT, string("c01"), DataColumnName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_child);
        }
    }
}

inline void handle_xmltable_column_el(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeColId) {
            IR* new_child = new IR(IRTypeIDENT, string("c01"), DataColumnName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_child);
        }
    }
}

inline void handle_json_table_path_name_opt(RSG* rsg, IR*& cur_ir) {
    cur_ir->free_children();
    return;
}

inline void handle_a_expr(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    if (children.size() == 3 && children[1]->get_ir_type() == IRTypeCOLLATE) {
        IR* new_child = new IR(IRTypeIDENT, string("unicode"), DataCollationName, ContextUnknown, nullptr);
        cur_ir->swap_one_child(children.back(), new_child);
    } 
}

inline void handle_c_expr(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeColumnref) {
            IR* new_child = new IR(IRTypeIDENT, string("c01"), DataColumnName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_child);
        }
    }
}

inline void handle_func_application(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeFuncName) {
            IR* new_child = new IR(IRTypeIDENT, string("COUNT"), DataFunctionName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_child);
        }
    }
}

inline void handle_func_arg_expr(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeParamName) {
            IR* new_child = new IR(IRTypeIDENT, string("c01"), DataColumnName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_child);
        }
    }
}

inline void handle_xml_attribute_el(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeColLabel) {
            IR* new_child = new IR(IRTypeIDENT, string("ca01"), DataColumnAliasName, ContextDefine, nullptr);
            cur_ir->swap_one_child(child, new_child);
        }
    }
}

inline void handle_json_table_column_definition(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeColId) {
            IR* new_child = new IR(IRTypeIDENT, string("c01"), DataColumnName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_child);
        }
    }
}

inline void handle_over_clause(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeColId) {
            IR* new_child = new IR(IRTypeIDENT, string("window_name_0"), DataWindowName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_child);
        }
    }
}   

inline void handle_opt_existing_window_name(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeColId) {
            IR* new_child = new IR(IRTypeIDENT, string("window_name_0"), DataWindowName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_child);
        }
    }
}

inline void handle_window_definition(RSG* rsg, IR*& cur_ir) {
    BEGIN;

    for (auto& child : children) {
        if (child->get_ir_type() == IRTypeColId) {
            IR* new_child = new IR(IRTypeIDENT, string("window_name_0"), DataWindowName, ContextDefine, nullptr);
            cur_ir->swap_one_child(child, new_child);
        }
    }
}


void postgresql_ir_context_setup(RSG* rsg, IR*& cur_ir)
{
    IRTYPE cur_ir_type = cur_ir->get_ir_type();

    switch (cur_ir_type) {

    case (IRTypeIDENT):
        handle_IDENT(rsg, cur_ir);
        break;

    case (IRTypeOptSingleName):
        handle_opt_single_name(rsg, cur_ir);
        break;

    case (IRTypeOptQualifiedName):
        handle_opt_qualified_name(rsg, cur_ir);
        break;

    case (IRTypeSCONST):
        handle_SCONST(rsg, cur_ir);
        break;

    case (IRTypeAlterOptRoleElem):
        handle_alter_opt_role_elem(rsg, cur_ir);
        break;

    case (IRTypeCreateSchemaStmt):
        handle_create_schema_stmt(rsg, cur_ir);
        break;

    case (IRTypeGenericSet):
        handle_generic_set(rsg, cur_ir);
        break;

    case (IRTypeVarName):
        handle_var_name(rsg, cur_ir);
        break;

    case (IRTypeAlterTableStmt):
        handle_alter_table_stmt(rsg, cur_ir);
        break;

    case (IRTypePartitionCmd):
        handle_partition_cmd(rsg, cur_ir);
        break;

    case (IRTypeIndexPartitionCmd):
        handle_index_partition_cmd(rsg, cur_ir);
        break;

    case (IRTypeColumnDef):
        handle_column_def(rsg, cur_ir);
        break;

    case (IRTypeAlterTableCmd):
        handle_alter_table_cmd(rsg, cur_ir);
        break;

    case (IRTypeTableConstraint):
        handle_table_constraint(rsg, cur_ir);
        break;

    case (IRTypeOptCollateClause):
        handle_opt_collate_clause(rsg, cur_ir);
        break;

    case (IRTypeReplicaIdentity):
        handle_replica_identity(rsg, cur_ir);
        break;

    // case (IRTypeReloptionElem):
    //     handle_reloption_elem(rsg, cur_ir);
    //     break;

    case (IRTypeSetAccessMethodName):
        handle_set_access_method_name(rsg, cur_ir);
        break;

    case (IRTypeAlterCompositeTypeStmt):
        handle_alter_composite_type_stmt(rsg, cur_ir);
        break;

    case (IRTypeAlterTypeCmd):
        handle_alter_type_cmd(rsg, cur_ir);
        break;

    case (IRTypeCursorName):
        handle_cursor_name(rsg, cur_ir);
        break;

    case (IRTypeCopyStmt):
        handle_copy_stmt(rsg, cur_ir);
        break;

    case (IRTypeOptColumnList):
        handle_opt_column_list(rsg, cur_ir);
        break;

    case (IRTypeColumnElem):
        handle_column_elem(rsg, cur_ir);
        break; 

    case (IRTypeColumnList):
        handle_column_list(rsg, cur_ir);
        break;

    case (IRTypeColLabel):
        // Just a placeholder.
        handle_col_label(rsg, cur_ir);
        break;

    case (IRTypeCreateStmt):
        handle_create_stmt(rsg, cur_ir);
        break;

    case (IRTypeColumnOptions):
        handle_column_options(rsg, cur_ir);
        break;

    case (IRTypeColumnCompression):
        handle_column_compression(rsg, cur_ir);
        break;

    case (IRTypeColumnStorage):
        handle_column_storage(rsg, cur_ir);
        break;

    case (IRTypeColConstraint):
        handle_col_constraint(rsg, cur_ir);
        break;

    case (IRTypeColConstraintElem):
        handle_col_constraint_elem(rsg, cur_ir);
        break;

    case (IRTypeTableLikeClause):
        handle_table_like_clause(rsg, cur_ir);
        break;

    case (IRTypeConstraintElem):
        handle_constraint_elem(rsg, cur_ir);
        break;

    case (IRTypeDomainConstraint):
        handle_domain_constraint(rsg, cur_ir);
        break;

    case (IRTypeOptInherit):
        handle_opt_inherit(rsg, cur_ir);
        break;

    case (IRTypePartitionSpec):
        handle_partition_spec(rsg, cur_ir);
        break;

    case (IRTypeTableAccessMethodClause):
        handle_table_access_method_clause(rsg, cur_ir);
        break;

    case (IRTypeOptTableSpace):
        handle_opt_table_space(rsg, cur_ir);
        break;

    case (IRTypeOptConsTableSpace):
        handle_opt_cons_table_space(rsg, cur_ir);
        break;

    case (IRTypeExistingIndex):
        handle_existing_index(rsg, cur_ir);
        break;

    case (IRTypeCreateStatsStmt):
        handle_create_stats_stmt(rsg, cur_ir);
        break;

    case (IRTypeAlterStatsStmt):
        handle_alter_stats_stmt(rsg, cur_ir);
        break;

    case (IRTypeCreateAsTarget):
        handle_create_as_target(rsg, cur_ir);
        break;

    case (IRTypeCreateMvTarget):
        handle_create_mv_target(rsg, cur_ir);
        break;

    case (IRTypeRefreshMatViewStmt):
        handle_refresh_mat_view_stmt(rsg, cur_ir);
        break;

    case (IRTypeCreateSeqStmt):
        handle_create_seq_stmt(rsg, cur_ir);
        break;

    case (IRTypeAlterSeqStmt):
        handle_alter_seq_stmt(rsg, cur_ir);
        break;

    case (IRTypeCreatePLangStmt):
        handle_create_plang_stmt(rsg, cur_ir);
        break;

    case (IRTypeHandlerName):
        handle_handler_name(rsg, cur_ir);
        break;

    case (IRTypeCreateTableSpaceStmt):
        handle_create_table_space_stmt(rsg, cur_ir);
        break;

    case (IRTypeDropTableSpaceStmt):
        handle_drop_table_space_stmt(rsg, cur_ir);
        break;

    case (IRTypeCreateExtensionStmt):
        handle_create_extension_stmt(rsg, cur_ir);
        break;

    case (IRTypeCreateExtensionOptItem):
        handle_create_extension_opt_item(rsg, cur_ir);
        break;

    case (IRTypeAlterExtensionStmt):
        handle_alter_extension_stmt(rsg, cur_ir);
        break;

    case (IRTypeAlterExtensionContentsStmt):
        handle_alter_extension_contents_stmt(rsg, cur_ir);
        break;

    case (IRTypeCreateFdwStmt):
        handle_create_fdw_stmt(rsg, cur_ir);
        break;

    case (IRTypeAlterFdwStmt):
        handle_alter_fdw_stmt(rsg, cur_ir);
        break;

    case (IRTypeCreateForeignServerStmt):
        handle_create_foreign_server_stmt(rsg, cur_ir);
        break;

    case (IRTypeAlterForeignServerStmt):
        handle_alter_foreign_server_stmt(rsg, cur_ir);
        break;

    case (IRTypeCreateForeignTableStmt):
        handle_create_foreign_table_stmt(rsg, cur_ir);
        break;

    case (IRTypeImportForeignSchemaStmt):
        handle_import_foreign_schema_stmt(rsg, cur_ir);
        break;

    case (IRTypeCreateUserMappingStmt):
        handle_create_user_mapping_stmt(rsg, cur_ir);
        break;

    case (IRTypeDropUserMappingStmt):
        handle_drop_user_mapping_stmt(rsg, cur_ir);
        break;

    case (IRTypeAlterUserMappingStmt):
        handle_alter_user_mapping_stmt(rsg, cur_ir);
        break;

    case (IRTypeCreatePolicyStmt):
        handle_create_policy_stmt(rsg, cur_ir);
        break;

    case (IRTypeAlterPolicyStmt):
        handle_alter_policy_stmt(rsg, cur_ir);
        break;

    case (IRTypeRoleList):
        handle_role_list(rsg, cur_ir);
        break;

    case (IRTypeRoleSpec):
        handle_role_spec(rsg, cur_ir);
        break;

    case (IRTypeRowSecurityDefaultPermissive):
        handle_row_security_default_permissive(rsg, cur_ir);
        break;

    case (IRTypeCreateAmStmt):
        handle_create_am_stmt(rsg, cur_ir);
        break;

    case (IRTypeCreateTrigStmt):
        handle_create_trig_stmt(rsg, cur_ir);
        break;

    case (IRTypeTriggerTransitions):
        handle_trigger_transitions(rsg, cur_ir);
        break;

    case (IRTypeOptConstrFromTable):
        handle_opt_constr_from_table(rsg, cur_ir);
        break;

    case (IRTypeCreateEventTrigStmt):
        handle_create_event_trig_stmt(rsg, cur_ir);
        break;

    case (IRTypeEventTriggerWhenItem):
        handle_event_trigger_when_item(rsg, cur_ir);
        break;

    case (IRTypeAlterEventTrigStmt):
        handle_alter_event_trig_stmt(rsg, cur_ir);
        break;

    case (IRTypeCreateAssertionStmt):
        handle_create_assertion_stmt(rsg, cur_ir);
        break;

    case (IRTypeDefineStmt):
        handle_define_stmt(rsg, cur_ir);
        break;

    case (IRTypeAlterEnumStmt):
        handle_alter_enum_stmt(rsg, cur_ir);
        break;

    case (IRTypeCreateOpClassStmt):
        handle_create_op_class_stmt(rsg, cur_ir);
        break;

    case (IRTypeOptOpfamily):
        handle_opt_op_family(rsg, cur_ir);
        break;

    case (IRTypeOpclassPurpose):
        handle_opclass_purpose(rsg, cur_ir);
        break;

    case (IRTypeCreateOpFamilyStmt):
        handle_create_op_family_stmt(rsg, cur_ir);
        break;

    case (IRTypeAlterOpFamilyStmt):
        handle_alter_op_family_stmt(rsg, cur_ir);
        break;

    case (IRTypeDropOpClassStmt):
        handle_drop_op_class_stmt(rsg, cur_ir);
        break;

    case (IRTypeDropOpFamilyStmt):
        handle_drop_op_family_stmt(rsg, cur_ir);
        break;

    case (IRTypeDropStmt):
        handle_drop_stmt(rsg, cur_ir);
        break;

    case (IRTypeAnyName):
        // Just placeholder for any name. 
        handle_any_name(rsg, cur_ir);
        break;

    case (IRTypeCommentStmt):
        // give up on this
        break;

    case (IRTypeSecLabelStmt):
        handle_sec_label_stmt(rsg, cur_ir);
        break;

    case (IRTypeColId):
        handle_col_id(rsg, cur_ir);
        break;

    case (IRTypeGrantStmt):
        // TODO: Implement this.
        break;

    case (IRTypePrivilegeTarget):
        handle_privilege_target(rsg, cur_ir);
        break;

    case (IRTypeDefACLOptionList):
        handle_DefACLOptionList(rsg, cur_ir);
        break;

    case (IRTypeIndexStmt):
        handle_index_stmt(rsg, cur_ir);
        break;

    case (IRTypeAccessMethodClause):
        handle_access_method_clause(rsg, cur_ir);
        break;

    case (IRTypeIndexElemOptions):
        handle_index_elem_options(rsg, cur_ir);
        break;

    case (IRTypeIndexElem):
        handle_index_elem(rsg, cur_ir);
        break;

    case (IRTypeOptCollate):
        handle_opt_collate(rsg, cur_ir);
        break;

    case (IRTypeCreateFunctionStmt):
        handle_create_function_stmt(rsg, cur_ir);
        break;

    case (IRTypeFunctionWithArgtypes):
        handle_function_with_argtypes(rsg, cur_ir);
        break;

    case (IRTypeFuncArg):
        handle_func_arg(rsg, cur_ir);
        break;

    case (IRTypeTypename):
        handle_type_name(rsg, cur_ir);
        break;

    case (IRTypeParamName):
        handle_param_name(rsg, cur_ir);
        break;

    case (IRTypeCreateTransformStmt):
        handle_create_transform_stmt(rsg, cur_ir    );
        break;

    case (IRTypeReindexStmt):
        handle_reindex_stmt(rsg, cur_ir);
        break;

    case (IRTypeAlterTblSpcStmt):
        handle_alter_tbl_spc_stmt(rsg, cur_ir);
        break;

    case (IRTypeRenameStmt):
        handle_rename_stmt(rsg, cur_ir);
        break;

    case (IRTypeAlterObjectSchemaStmt):
        handle_alter_object_schema_stmt(rsg, cur_ir);
        break;

    case (IRTypeAlterTypeStmt):
        handle_alter_type_stmt(rsg, cur_ir);
        break;

    case (IRTypeAlterOwnerStmt):
        handle_alter_owner_stmt(rsg, cur_ir);
        break;

    case (IRTypeRelationExpr):
        handle_relation_expr(rsg, cur_ir);
        break;

    case (IRTypeAlterPublicationStmt):
        handle_alter_publication_stmt(rsg, cur_ir);
        break;

    case (IRTypeCreateSubscriptionStmt):
        handle_create_subscription_stmt(rsg, cur_ir);
        break;

    case (IRTypeAlterSubscriptionStmt):
        handle_alter_subscription_stmt(rsg, cur_ir);
        break;

    case (IRTypeDropSubscriptionStmt):
        handle_drop_subscription_stmt(rsg, cur_ir);
        break;

    case (IRTypeRuleStmt):
        handle_rule_stmt(rsg, cur_ir);
        break;

    case (IRTypeNotifyStmt):
        handle_notify_stmt(rsg, cur_ir);
        break;

    case (IRTypeListenStmt):
        handle_listen_stmt(rsg, cur_ir);    
        break;

    case (IRTypeUnlistenStmt):
        handle_unlisten_stmt(rsg, cur_ir);
        break;

    case (IRTypeTransactionStmt):
        handle_transaction_stmt(rsg, cur_ir);
        break;

    case (IRTypeViewStmt):
        handle_view_stmt(rsg, cur_ir);
        break;

    case (IRTypeCreatedbStmt):
        handle_create_db_stmt(rsg, cur_ir);
        break;

    case (IRTypeAlterDatabaseStmt):
        handle_alter_database_stmt(rsg, cur_ir);
        break;

    case (IRTypeAlterDatabaseSetStmt):
        handle_alter_database_set_stmt(rsg, cur_ir);
        break;

    case (IRTypeDropdbStmt):
        handle_drop_db_stmt(rsg, cur_ir);
        break;

    case (IRTypeAlterCollationStmt):
        handle_alter_collation_stmt(rsg, cur_ir);
        break;

    case (IRTypeCreateDomainStmt):
        handle_create_domain_stmt(rsg, cur_ir);
        break;

    case (IRTypeAlterDomainStmt):
        handle_alter_domain_stmt(rsg, cur_ir);
        break;

    case (IRTypeCreateConversionStmt):
        handle_create_conversion_stmt(rsg, cur_ir);
        break;

    case (IRTypePrepareStmt):
        handle_prepare_stmt(rsg, cur_ir);
        break;

    case (IRTypeExecuteStmt):
        handle_execute_stmt(rsg, cur_ir);
        break;

    case (IRTypeDeallocateStmt):
        handle_deallocate_stmt(rsg, cur_ir);
        break;

    case (IRTypeInsertTarget):
        handle_insert_target(rsg, cur_ir);
        break;

    case (IRTypeInsertColumnItem):
        handle_insert_column_item(rsg, cur_ir);
        break;

    case (IRTypeOptIndirection):
        handle_opt_indirection(rsg, cur_ir);
        break;

    case (IRTypeReturningOption):
        handle_returning_option(rsg, cur_ir);
        break;

    case (IRTypeRelationExprOptAlias):
        handle_relation_expr_opt_alias(rsg, cur_ir);
        break;

    case (IRTypeTableRef):
        handle_table_ref(rsg, cur_ir);
        break;

    case (IRTypeLockStmt):
        handle_lock_stmt(rsg, cur_ir);
        break;

    case (IRTypeSetTarget):
        handle_set_target(rsg, cur_ir);
        break;

    case (IRTypeCommonTableExpr):
        handle_common_table_expr(rsg, cur_ir);
        break;

    case (IRTypeOptSearchClause):
        handle_opt_search_clause(rsg, cur_ir);
        break;

    case (IRTypeOptCycleClause):
        handle_opt_cycle_clause(rsg, cur_ir);
        break;

    case (IRTypeOptTempTableName):
        handle_opt_temp_table_name(rsg, cur_ir);
        break;

    case (IRTypeAliasClause):
        handle_alias_clause(rsg, cur_ir);
        break;

    case (IRTypeOptAliasClauseForJoinUsing):
        handle_opt_alias_clause_for_join_using(rsg, cur_ir);
        break;

    case (IRTypeFuncAliasClause):
        handle_func_alias_clause(rsg, cur_ir);
        break;

    case (IRTypeJoinQual):
        handle_join_qual(rsg, cur_ir);
        break;

    case (IRTypeExtendedRelationExpr):
        handle_extended_relation_expr(rsg, cur_ir);
        break;

    case (IRTypeTableFuncElement):
        handle_table_func_element(rsg, cur_ir);
        break;

    case (IRTypeXmltableColumnEl):
        handle_xmltable_column_el(rsg, cur_ir);
        break;

    case (IRTypeJsonTablePathNameOpt):
        handle_json_table_path_name_opt(rsg, cur_ir);
        break;

    case (IRTypeJsonTableColumnDefinition):
        handle_json_table_column_definition(rsg, cur_ir);
        break;

    case (IRTypeAExpr):
        handle_a_expr(rsg, cur_ir);
        break;

    case (IRTypeCExpr):
        handle_c_expr(rsg, cur_ir);
        break;

    case (IRTypeFuncApplication):
        handle_func_application(rsg, cur_ir);
        break;

    case (IRTypeFuncArgExpr):
        handle_func_arg_expr(rsg, cur_ir);
        break;

    case (IRTypeXmlAttributeEl):
        handle_xml_attribute_el(rsg, cur_ir);
        break;

    case (IRTypeWindowDefinition):
        handle_window_definition(rsg, cur_ir);
        break;

    case (IRTypeOverClause):
        handle_over_clause(rsg, cur_ir);
        break;

    case (IRTypeOptExistingWindowName):
        handle_opt_existing_window_name(rsg, cur_ir);
        break;

    default:
        break;
    }

    // IMPORTANT!!!
    for (auto& interesting_type : PostgreSQLFuzzerConfigurations::postgresql_interesting_ir_types) {
        if (interesting_type == IRTypeSelectStmt) {
            continue;
        }
        if (interesting_type == cur_ir_type) {
            cur_ir->set_is_favor(IsFavor::favor);
            break;
        }
    }
}