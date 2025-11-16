//
// Created by XXX on 3/18/24.
//

#include "cockroachdb_ir_context_setup.h"
#include "cockroachdb_fuzzer_configurations.h"

inline static void handle_table_index_name(IR*& cur_ir, DATAFLAG table_flag, DATAFLAG index_flag)
{

    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() == 3) {
        const int table_idx = 0;
        IR* new_name = new IR(IRTypeTableName, children[table_idx]->to_string(), DataTableName, ContextUse, children[table_idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[table_idx], new_name, true);

        const int index_idx = 2;
        IR* new_idx_name = new IR(IRTypeIndexName, children[table_idx]->to_string(), DataIndexName, index_flag, children[index_idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[index_idx], new_idx_name, true);
    }

    else {
        // children_size() == 1;
        const int index_idx = 0;
        IR* new_name = new IR(IRTypeIndexName, children[index_idx]->to_string(), DataIndexName, index_flag, children[index_idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[index_idx], new_name, true);
    }
}

inline static void handle_name_list(IR*& cur_ir, DATATYPE data_type, DATAFLAG data_flag)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() == 3) {
        handle_name_list(children[0], data_type, data_flag);
        const int idx = 2;
        IR* new_name = new IR(IRTypeName, children[idx]->to_string(), data_type, data_flag,
            children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    } else {
        const int idx = 0;
        IR* new_name = new IR(IRTypeName, children[idx]->to_string(), data_type, data_flag,
            children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }
}

inline static void handle_opt_index_name(IR*& cur_ir, DATATYPE data_type, DATAFLAG data_flag)
{
    IR* opt_name_ir = cur_ir->get_children().front();
    vector<IR*> children = opt_name_ir->get_children();

    if (!children.empty()) {
        IR* new_name = new IR(IRTypeIndexName, children.front()->to_string(), data_type, data_flag, cur_ir->get_mapped_token_node());
        cur_ir->free_children();
        cur_ir->set_children_nodes({ new_name });
    } else {
        // cut empty subtree. switch to terminating empty IR.
        cur_ir->free_children();
        cur_ir->set_symbol_type(SymbolTerm);
        cur_ir->set_str_val("");
    }
}

inline static void handle_opt_family_name(IR*& cur_ir, DATATYPE data_type, DATAFLAG data_flag)
{
    IR* opt_name_ir = cur_ir->get_children().front();
    vector<IR*> children = opt_name_ir->get_children();

    if (!children.empty()) {
        IR* new_name = new IR(IRTypeFamilyName, children.front()->to_string(), data_type, data_flag, cur_ir->get_mapped_token_node());
        cur_ir->free_children();
        cur_ir->set_children_nodes({ new_name });
    } else {
        // cut empty subtree. switch to terminating empty IR.
        cur_ir->free_children();
        cur_ir->set_symbol_type(SymbolTerm);
        cur_ir->set_str_val("");
    }
}

inline static void handle_relation_expr(IR*& cur_ir, DATATYPE data_type, DATAFLAG data_flag)
{
    IR* opt_name_ir = cur_ir->get_children().front();
    vector<IR*> children = opt_name_ir->get_children();

    if (!children.empty() && children[0]->get_ir_type() == IRTypeTableName) {
        IR* new_name = new IR(IRTypeTableName, children.front()->to_string(), data_type, data_flag, cur_ir->get_mapped_token_node());
        cur_ir->swap_one_child(children[0], new_name);
    }

    if (children.size() > 1 && children[1]->get_ir_type() == IRTypeTableName) {
        const int idx = 1;
        IR* new_name = new IR(IRTypeTableName, children[idx]->to_string(), data_type, data_flag, children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name);
    }

    if (children.size() > 2 && children[2]->get_ir_type() == IRTypeTableName) {
        const int idx = 2;
        IR* new_name = new IR(IRTypeTableName, children[idx]->to_string(), data_type, data_flag, children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name);
    }
}

inline static void handle_relation_expr_list(IR*& cur_ir, DATATYPE data_type, DATAFLAG data_flag)
{
    IR* opt_name_ir = cur_ir->get_children().front();
    vector<IR*> children = opt_name_ir->get_children();

    if (children.size() == 1 && children[0]->get_ir_type() == IRTypeRelationExpr) {
        handle_relation_expr(children[0], data_type, data_flag);
    }

    if (children.size() == 3 && children[2]->get_ir_type() == IRTypeRelationExpr) {
        handle_relation_expr(children[2], data_type, data_flag);
        handle_relation_expr_list(children[0], data_type, data_flag);
    }
}

inline static void handle_type_name_list(IR*& cur_ir, DATATYPE data_type, DATAFLAG data_flag)
{
    IR* opt_name_ir = cur_ir->get_children().front();
    vector<IR*> children = opt_name_ir->get_children();

    if (children.size() == 1 && children[0]->get_ir_type() == IRTypeTypeName) {
        const int idx = 0;
        IR* new_name = new IR(IRTypeTypeName, children[idx]->to_string(), DataTypeName, ContextUse,
            children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }

    if (children.size() == 3 && children[2]->get_ir_type() == IRTypeTypeName) {
        int idx = 2;
        IR* new_name = new IR(IRTypeTypeName, children[idx]->to_string(), DataTypeName, ContextUse,
            children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);

        idx = 0;
        handle_type_name_list(children[0], data_type, data_flag);
    }
}

inline static void handle_region_name_list(IR*& cur_ir, DATATYPE data_type, DATAFLAG data_flag)
{
    vector<IR*> all_children = cur_ir->get_children_ref();
    vector<pair<IR*, IR*>> v_replace_pair;
    for (IR*& cur_child : all_children) {
        IR* new_name = new IR(IRTypeRegionName, cur_child->to_string(), data_type, data_flag,
            cur_child->get_mapped_token_node());
        //        cur_ir->swap_one_child(cur_child, new_name, true);
        v_replace_pair.push_back(pair<IR*, IR*>(cur_child, new_name));
    }

    for (pair<IR*, IR*> cur_pair : v_replace_pair) {
        cur_ir->swap_one_child(cur_pair.first, cur_pair.second, true);
    }
}

inline static void handle_opt_regions_list(IR*& cur_ir, DATATYPE data_type, DATAFLAG data_flag)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 2 && children[2]->get_ir_type() == IRTypeRegionNameList) {
        handle_region_name_list(children[2], data_type, data_flag);
    }
}
inline static void handle_qualifiable_schema_name(IR*& cur_ir, DATATYPE data_type, DATAFLAG data_flag)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() == 3) {
        children.back()->deep_drop();
        children.pop_back();
        children.back()->deep_drop();
        children.pop_back();
    }

    IR* new_name = new IR(IRTypeSchemaName, children.front()->to_string(), data_type, data_flag,
        children.front()->get_mapped_token_node());
    cur_ir->swap_one_child(children.front(), new_name, true);
}

inline static void handle_schema_name_list(IR*& cur_ir, DATATYPE data_type, DATAFLAG data_flag)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() == 1) {
        handle_qualifiable_schema_name(children.front(), data_type, data_flag);
    } else if (children.size() == 3) {
        handle_schema_name_list(children.front(), data_type, data_flag);
        handle_qualifiable_schema_name(children.back(), data_type, data_flag);
    }
}

inline static void handle_opt_schema_name(IR*& cur_ir, DATATYPE data_type, DATAFLAG data_flag)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() == 1) {
        handle_qualifiable_schema_name(children.front(), data_type, data_flag);
    } else {
        return;
    }
}

inline static void handle_col_qualification(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();
    if (children.size() == 2 && children[1]->get_ir_type() == IRTypeCollationName) {
        const int idx = 1;
        IR* new_col_name = new IR(IRTypeCollationName, children[idx]->to_string(), DataCollationName, ContextUse, children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_col_name, true);
    }

    if (children.size() == 3 && children[1]->get_ir_type() == IRTypeConstraintName) {
        const int idx = 1;
        IR* new_col_name = new IR(IRTypeConstraintName, children[idx]->to_string(), DataConstraintName, ContextUse, children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_col_name, true);
    }

    if (children.size() != 0 && children.back()->get_ir_type() == IRTypeFamilyName) {
        if (children.front()->get_ir_type() == IRTypeFAMILY) {
            IR* new_col_name = new IR(IRTypeFamilyName, children.back()->to_string(), DataFamilyName,
                ContextUse, children.back()->get_mapped_token_node());
            cur_ir->swap_one_child(children.back(), new_col_name, true);
        } else {
            IR* new_col_name = new IR(IRTypeFamilyName, children.back()->to_string(), DataFamilyName,
                ContextDefine, children.back()->get_mapped_token_node());
            cur_ir->swap_one_child(children.back(), new_col_name, true);
        }
    }
}

inline static void handle_opt_collate(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();
    if (children.size() == 2 && children[1]->get_ir_type() == IRTypeCollationName) {
        IR* new_col_name = new IR(IRTypeCollationName, children[1]->to_string(), DataCollationName, ContextUse, children[1]->get_mapped_token_node());
        children[1]->deep_drop();
        children[1] = new_col_name;
        new_col_name->set_parent_node(cur_ir);
    }
}

inline static void handle_a_expr(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();
    if (children.size() == 3 && children[2]->get_ir_type() == IRTypeCollationName) {
        IR* new_col_name = new IR(IRTypeCollationName, children[2]->to_string(), DataCollationName, ContextUse, children[2]->get_mapped_token_node());
        children[2]->deep_drop();
        children[2] = new_col_name;
        new_col_name->set_parent_node(cur_ir);
    }
}

inline static void handle_alter_zone_partition_stmt(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();
    if (children.size() > 2 && children[2]->get_ir_type() == IRTypePartitionName) {
        IR* new_par_name = new IR(IRTypePartitionName, children[2]->to_string(), DataPartitionName, ContextUse, children[2]->get_mapped_token_node());
        children[2]->deep_drop();
        children[2] = new_par_name;
        new_par_name->set_parent_node(cur_ir);
    }

    if (children.size() > 5 && children[5]->get_ir_type() == IRTypeTableName) {
        const int idx = 5;
        IR* new_name = new IR(IRTypeTableName, children[idx]->to_string(), DataTableName, ContextUse,
            children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }
}

inline static void handle_logical_replication_resources(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();
    if (children.size() == 2 && children[1]->get_ir_type() == IRTypeDbObjectName) {
        IR* new_par_name = new IR(IRTypeTableName, children[1]->to_string(), DataTableName, ContextUse, children[1]->get_mapped_token_node());
        children[1]->deep_drop();
        children[1] = new_par_name;
        new_par_name->set_parent_node(cur_ir);
    }

    if (children.size() == 2 && children[1]->get_ir_type() == IRTypeDatabaseName) {
        IR* new_par_name = new IR(IRTypeDatabaseName, children[1]->to_string(), DataDatabaseName, ContextUse, children[1]->get_mapped_token_node());
        children[1]->deep_drop();
        children[1] = new_par_name;
        new_par_name->set_parent_node(cur_ir);
    }

    if (children.size() == 3 && children[2]->get_ir_type() == IRTypeDbObjectName) {
        IR* new_par_name = new IR(IRTypeTableName, children[1]->to_string(), DataTableName, ContextUse, children[2]->get_mapped_token_node());
        children[2]->deep_drop();
        children[2] = new_par_name;
        new_par_name->set_parent_node(cur_ir);
    }
}

inline static void handle_show_zone_stmt(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 5 && children[5]->get_ir_type() == IRTypePartitionName) {
        IR* new_par_name = new IR(IRTypePartitionName, children[5]->to_string(), DataPartitionName, ContextUse, children[5]->get_mapped_token_node());
        children[5]->deep_drop();
        children[5] = new_par_name;
        new_par_name->set_parent_node(cur_ir);
    }

    if (children.size() > 1 && children.back()->get_ir_type() == IRTypeZoneName) {
        IR* new_name = new IR(IRTypeZoneName, children.back()->to_string(), DataZoneName, ContextUse, children.back()->get_mapped_token_node());
        children.back()->deep_drop();
        children.back() = new_name;
        new_name->set_parent_node(cur_ir);
    }

    if (children.size() != 0 && children.back()->get_ir_type() == IRTypeDatabaseName) {
        IR* new_name = new IR(IRTypeDatabaseName, children.back()->to_string(), DataDatabaseName, ContextUse, children.back()->get_mapped_token_node());
        cur_ir->swap_one_child(children.back(), new_name, true);
    }

    if (children.size() != 0 && children.back()->get_ir_type() == IRTypeTableName) {
        IR* new_name = new IR(IRTypeTableName, children.back()->to_string(), DataTableName, ContextUse, children.back()->get_mapped_token_node());
        cur_ir->swap_one_child(children.back(), new_name, true);
    }

    if (children.size() > 5 && children[5]->get_ir_type() == IRTypeTableName) {
        const int idx = 5;
        IR* new_name = new IR(IRTypeTableName, children[idx]->to_string(), DataTableName, ContextUse, children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }
}

inline static void handle_partition(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();
    if (children.size() == 2 && children[1]->get_ir_type() == IRTypePartitionName) {
        const int idx = 1;
        IR* new_name = new IR(IRTypePartitionName, children[idx]->to_string(), DataPartitionName, ContextUse, children[1]->get_mapped_token_node());
        children[idx]->deep_drop();
        children[idx] = new_name;
        new_name->set_parent_node(cur_ir);
    }
}

inline static void handle_create_index_stmt(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 8 && children[7]->get_ir_type() == IRTypeIndexName) {
        const int idx = 7;
        IR* new_name = new IR(IRTypeIndexName, children[idx]->to_string(), DataIndexName, ContextDefine, children[idx]->get_mapped_token_node());
        children[idx]->deep_drop();
        children[idx] = new_name;
        new_name->set_parent_node(cur_ir);
    }

    if (children.size() > 9 && children[8]->get_ir_type() == IRTypeIndexName) {
        const int idx = 8;
        IR* new_name = new IR(IRTypeIndexName, children[idx]->to_string(), DataIndexName, ContextDefine, children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }

    if (children.size() > 5 && children[4]->get_ir_type() == IRTypeOptIndexName) {
        const int idx = 4;
        handle_opt_index_name(children[idx], DataIndexName, ContextDefine);
    }

    if (children.size() > 6 && children[5]->get_ir_type() == IRTypeOptIndexName) {
        const int idx = 5;
        handle_opt_index_name(children[idx], DataIndexName, ContextDefine);
    }

    vector<pair<IR*, IR*>> v_replace_pair;
    for (IR*& cur_child : children) {
        if (cur_child->get_ir_type() == IRTypeTableName) {
            IR* new_name = new IR(IRTypeTableName, cur_child->to_string(), DataTableName, ContextUse,
                cur_child->get_mapped_token_node());
            //            cur_ir->swap_one_child(cur_child, new_name, true);
            v_replace_pair.push_back(pair<IR*, IR*>(cur_child, new_name));
        }
    }

    for (pair<IR*, IR*> cur_pair : v_replace_pair) {
        cur_ir->swap_one_child(cur_pair.first, cur_pair.second, true);
    }
}

inline static void handle_alter_rename_index_stmt(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() != 0 && children.back()->get_ir_type() == IRTypeIndexName) {
        IR* new_name = new IR(IRTypeIndexName, children.back()->to_string(), DataIndexName, ContextReplaceDefine, children.back()->get_mapped_token_node());
        children.back()->deep_drop();
        children.pop_back();
        children.push_back(new_name);
        new_name->set_parent_node(cur_ir);
    }

    vector<IR*> v_ir_to_handle;
    for (IR*& child_ir : children) {
        if (child_ir->get_ir_type() == IRTypeTableIndexName) {
            v_ir_to_handle.push_back(child_ir);
            //            handle_table_index_name(child_ir, ContextUse, ContextReplaceUndefine);
        }
    }

    for (IR* cur_ir_to_handel : v_ir_to_handle) {
        handle_table_index_name(cur_ir_to_handel, ContextUse, ContextReplaceUndefine);
    }
}

inline static void handle_sortby(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 3 && children[3]->get_ir_type() == IRTypeIndexName) {
        const int idx = 3;
        IR* new_name = new IR(IRTypeIndexName, children[idx]->to_string(), DataIndexName, ContextUse, children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }

    if (children.size() > 2 && children[2]->get_ir_type() == IRTypeTableName) {
        const int idx = 2;
        IR* new_name = new IR(IRTypeTableName, children[idx]->to_string(), DataTableName, ContextUse, children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }

    if (children.size() > 1 && children[1]->get_ir_type() == IRTypeTableName) {
        const int idx = 1;
        IR* new_name = new IR(IRTypeTableName, children[idx]->to_string(), DataTableName, ContextUse, children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }
}

inline static void handle_index_flags_param(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() == 3 && children[2]->get_ir_type() == IRTypeIndexName) {
        const int idx = 2;
        IR* new_name = new IR(IRTypeIndexName, children[idx]->to_string(), DataIndexName, ContextUse, children[idx]->get_mapped_token_node());
        children[idx]->deep_drop();
        children[idx] = new_name;
        new_name->set_parent_node(cur_ir);
    }
}

inline static void handle_opt_index_flags(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() == 2 && children[1]->get_ir_type() == IRTypeIndexName) {
        const int idx = 1;
        IR* new_name = new IR(IRTypeIndexName, children[idx]->to_string(), DataIndexName, ContextUse, children[idx]->get_mapped_token_node());
        children[idx]->deep_drop();
        children[idx] = new_name;
        new_name->set_parent_node(cur_ir);
    }
}

inline static void handle_table_index_name(IR*& cur_ir)
{

    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() == 3) {
        const int table_idx = 0;
        IR* new_name = new IR(IRTypeTableName, children[table_idx]->to_string(), DataTableName, ContextUse, children[table_idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[table_idx], new_name, true);

        const int index_idx = 2;
        IR* new_idx_name = new IR(IRTypeIndexName, children[table_idx]->to_string(), DataIndexName, ContextUse, children[index_idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[index_idx], new_idx_name, true);
    }

    else {
        // children_size() == 1;
        const int index_idx = 0;
        IR* new_name = new IR(IRTypeIndexName, children[index_idx]->to_string(), DataIndexName, ContextUse, children[index_idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[index_idx], new_name, true);
    }
}

inline static void handle_table_index_name_list(IR*& cur_ir, DATAFLAG index_flag)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() == 3) {
        handle_table_index_name_list(children[0], index_flag);

        const int index_idx = 2;
        IR* new_idx_name = new IR(IRTypeIndexName, children[index_idx]->to_string(), DataIndexName, index_flag, children[index_idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[index_idx], new_idx_name, true);
    }

    else {
        // children_size() == 1;
        const int index_idx = 0;
        IR* new_idx_name = new IR(IRTypeIndexName, children[index_idx]->to_string(), DataIndexName, index_flag, children[index_idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[index_idx], new_idx_name, true);
    }
}

// deprecated.
inline static IR* handle_opt_name(IR*& cur_ir, IRTYPE ir_type, DATATYPE data_type, DATAFLAG data_flag)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (!children.empty()) {
        IR* new_name = new IR(ir_type, children.front()->to_string(), data_type, data_flag, children.front()->get_mapped_token_node());
        cur_ir->swap_one_child(children.front(), new_name, true);
    } else {
        // empty rule.
        // do nothing.
    }
    return cur_ir;
}

inline static void handle_index_def(IR*& cur_ir)
{

    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 3 && children[1]->get_ir_type() == IRTypeOptIndexName) {
        const int idx = 1;
        handle_opt_index_name(children[idx], DataIndexName, ContextDefine);
    }
    if (children.size() > 3 && children[2]->get_ir_type() == IRTypeOptIndexName) {
        const int idx = 2;
        handle_opt_index_name(children[idx], DataIndexName, ContextDefine);
    }
}

inline static void handle_target_elem(IR*& cur_ir)
{

    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() == 3 && children[2]->get_ir_type() == IRTypeTargetName) {
        const int idx = 2;
        IR* new_name = new IR(IRTypeTargetName, children[idx]->to_string(), DataColumnAliasName, ContextDefine, children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }
}

inline static void handle_alter_table_cmd(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 6 && children[5]->get_ir_type() == IRTypeConstraintName) {
        if (children.front()->get_ir_type() == IRTypeADD && children[1]->get_ir_type() == IRTypeConstraintName) {
            const int idx = 5;
            IR* new_name = new IR(IRTypeConstraintName, children[idx]->to_string(), DataConstraintName, ContextDefine, children[idx]->get_mapped_token_node());
            cur_ir->swap_one_child(children[idx], new_name, true);
        }
    }

    if (children.size() == 3 && children[2]->get_ir_type() == IRTypeConstraintName) {
        if (children.front()->get_ir_type() == IRTypeVALIDATE) {
            const int idx = 2;
            IR* new_name = new IR(IRTypeConstraintName, children[idx]->to_string(), DataConstraintName, ContextUse, children[idx]->get_mapped_token_node());
            cur_ir->swap_one_child(children[idx], new_name, true);
        }
    }

    if (children.size() > 5 && children[4]->get_ir_type() == IRTypeConstraintName) {
        if (children.front()->get_ir_type() == IRTypeDROP) {
            const int idx = 4;
            IR* new_name = new IR(IRTypeConstraintName, children[idx]->to_string(), DataConstraintName, ContextUndefine, children[idx]->get_mapped_token_node());
            cur_ir->swap_one_child(children[idx], new_name, true);
        }
    }

    if (children.size() > 5 && children[2]->get_ir_type() == IRTypeConstraintName) {
        if (children.front()->get_ir_type() == IRTypeDROP) {
            const int idx = 2;
            IR* new_name = new IR(IRTypeConstraintName, children[idx]->to_string(), DataConstraintName, ContextUndefine, children[idx]->get_mapped_token_node());
            cur_ir->swap_one_child(children[idx], new_name, true);
        }
    }

    if (children.size() > 5 && children[3]->get_ir_type() == IRTypeConstraintName) {
        if (children.front()->get_ir_type() == IRTypeCOMMENT) {
            const int idx = 3;
            IR* new_name = new IR(IRTypeConstraintName, children[idx]->to_string(), DataConstraintName, ContextUse, children[idx]->get_mapped_token_node());
            cur_ir->swap_one_child(children[idx], new_name, true);
        }
    }

    if (children.size() == 5 && children[2]->get_ir_type() == IRTypeColumnName && children[0]->get_ir_type() == IRTypeRENAME) {
        const int idx = 2;
        IR* new_name = new IR(IRTypeColumnName, children[idx]->to_string(), DataColumnName, ContextReplaceUndefine,
            children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }

    if (children.size() == 5 && children[4]->get_ir_type() == IRTypeColumnName && children[0]->get_ir_type() == IRTypeRENAME) {
        const int idx = 4;
        IR* new_name = new IR(IRTypeColumnName, children[idx]->to_string(), DataColumnName, ContextReplaceDefine,
            children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }

    if (children.size() == 5 && children[2]->get_ir_type() == IRTypeColumnName && children[0]->get_ir_type() == IRTypeALTER) {
        const int idx = 2;
        IR* new_name = new IR(IRTypeColumnName, children[idx]->to_string(), DataColumnName, ContextUse,
            children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }

    if (children.size() == 5 && children[2]->get_ir_type() == IRTypeColumnName && children[0]->get_ir_type() == IRTypeDROP) {
        const int idx = 2;
        IR* new_name = new IR(IRTypeColumnName, children[idx]->to_string(), DataColumnName, ContextUndefine,
            children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }

    if (children.size() == 5 && children[4]->get_ir_type() == IRTypeColumnName && children[0]->get_ir_type() == IRTypeDROP) {
        const int idx = 4;
        IR* new_name = new IR(IRTypeColumnName, children[idx]->to_string(), DataColumnName, ContextUndefine,
            children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }
}

inline static void handle_index_elem(IR*& cur_ir, DATATYPE data_type, DATAFLAG data_flag)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    const int idx = 0;
    if (children.size() != 0 && children[idx]->get_ir_type() == IRTypeName) {
        IR* new_name = new IR(IRTypeName, children[idx]->to_string(), data_type, data_flag, children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }
}

inline static void handle_index_params(IR*& cur_ir, DATATYPE data_type, DATAFLAG data_flag)
{
    for (auto*& child : cur_ir->get_children_ref()) {
        if (child->get_ir_type() == IRTypeIndexElem) {
            handle_index_elem(child, data_type, data_flag);
        }
    }
}

inline static void handle_table_constraint(IR*& cur_ir)
{
    // Always ContextDefine.
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() == 3 && children[1]->get_ir_type() == IRTypeConstraintName) {
        const int idx = 1;
        IR* new_name = new IR(IRTypeConstraintName, children[idx]->to_string(), DataConstraintName, ContextDefine, children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }

    if (children.size() > 3 && children[3]->get_ir_type() == IRTypeNameList) {
        const int idx = 3;
        handle_name_list(children[idx], DataForeignKeyName, ContextUse);
    }

    for (auto*& child : children) {
        if (child->get_ir_type() == IRTypeIndexParams) {
            handle_index_params(child, DataColumnName, ContextUse);
        }
    }
}

inline static void handle_on_conflict(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 5 && children[4]->get_ir_type() == IRTypeConstraintName) {
        const int idx = 4;
        IR* new_name = new IR(IRTypeConstraintName, children[idx]->to_string(), DataConstraintName, ContextUse, children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }

    if (children.size() > 3 && children[3]->get_ir_type() == IRTypeNameList) {
        handle_name_list(children[3], DataColumnName, ContextUse);
    }
}

inline static void handle_alter_database_owner(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 3 && children[2]->get_ir_type() == IRTypeDatabaseName) {
        const int idx = 2;
        IR* new_name = new IR(IRTypeDatabaseName, children[idx]->to_string(), DataDatabaseName, ContextUse, children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }
}

inline static void handle_alter_database_set_stmt(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 3 && children[2]->get_ir_type() == IRTypeDatabaseName) {
        const int idx = 2;
        IR* new_name = new IR(IRTypeDatabaseName, children[idx]->to_string(), DataDatabaseName, ContextUse, children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }
}

inline static void handle_alter_database_placement_stmt(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 3 && children[2]->get_ir_type() == IRTypeDatabaseName) {
        const int idx = 2;
        IR* new_name = new IR(IRTypeDatabaseName, children[idx]->to_string(), DataDatabaseName, ContextUse, children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }
}

inline static void handle_alter_database_add_region_stmt(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 2 && children[2]->get_ir_type() == IRTypeDatabaseName) {
        const int idx = 2;
        IR* new_name = new IR(IRTypeDatabaseName, children[idx]->to_string(), DataDatabaseName, ContextUse, children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }

    if (children.size() > 5 && children[5]->get_ir_type() == IRTypeRegionName) {
        const int idx = 5;
        IR* new_name = new IR(IRTypeRegionName, children[idx]->to_string(), DataRegionName, ContextDefine, children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }

    if (children.size() > 8 && children[8]->get_ir_type() == IRTypeRegionName) {
        const int idx = 8;
        IR* new_name = new IR(IRTypeRegionName, children[idx]->to_string(), DataRegionName, ContextDefine, children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }
}

inline static void handle_alter_database_drop_region_stmt(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 3 && children[2]->get_ir_type() == IRTypeDatabaseName) {
        const int idx = 2;
        IR* new_name = new IR(IRTypeDatabaseName, children[idx]->to_string(), DataDatabaseName, ContextUse, children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }

    if (children.size() > 5 && children[5]->get_ir_type() == IRTypeRegionName) {
        const int idx = 5;
        IR* new_name = new IR(IRTypeRegionName, children[idx]->to_string(), DataRegionName, ContextUndefine, children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }

    if (children.size() > 8 && children[8]->get_ir_type() == IRTypeRegionName) {
        const int idx = 8;
        IR* new_name = new IR(IRTypeRegionName, children[idx]->to_string(), DataRegionName, ContextUndefine, children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }
}

inline static void handle_alter_database_survival_goal_stmt(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 3 && children[2]->get_ir_type() == IRTypeDatabaseName) {
        const int idx = 2;
        IR* new_name = new IR(IRTypeDatabaseName, children[idx]->to_string(), DataDatabaseName, ContextUse, children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }
}

inline static void handle_alter_database_primary_region_stmt(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 3 && children[2]->get_ir_type() == IRTypeDatabaseName) {
        const int idx = 2;
        IR* new_name = new IR(IRTypeDatabaseName, children[idx]->to_string(), DataDatabaseName, ContextUse, children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }
}

inline static void handle_alter_database_add_super_region(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 3 && children[2]->get_ir_type() == IRTypeDatabaseName) {
        const int idx = 2;
        IR* new_name = new IR(IRTypeDatabaseName, children[idx]->to_string(), DataDatabaseName, ContextUse, children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }

    if (children.size() > 6 && children[6]->get_ir_type() == IRTypeName) {
        const int idx = 6;
        IR* new_name = new IR(IRTypeRegionName, children[idx]->to_string(), DataRegionName, ContextUse, children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }

    if (children.size() != 0) {
        handle_name_list(children.back(), DataRegionName, ContextUse);
    }
}

inline static void handle_alter_database_drop_super_region(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 3 && children[2]->get_ir_type() == IRTypeDatabaseName) {
        const int idx = 2;
        IR* new_name = new IR(IRTypeDatabaseName, children[idx]->to_string(), DataDatabaseName, ContextUse, children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }

    if (children.size() > 6 && children[6]->get_ir_type() == IRTypeName) {
        const int idx = 6;
        IR* new_name = new IR(IRTypeRegionName, children[idx]->to_string(), DataRegionName, ContextUse, children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }
}

inline static void handle_alter_database_alter_super_region(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 3 && children[2]->get_ir_type() == IRTypeDatabaseName) {
        const int idx = 2;
        IR* new_name = new IR(IRTypeDatabaseName, children[idx]->to_string(), DataDatabaseName, ContextUse, children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }

    if (children.size() > 6 && children[6]->get_ir_type() == IRTypeName) {
        const int idx = 6;
        IR* new_name = new IR(IRTypeRegionName, children[idx]->to_string(), DataRegionName, ContextUse, children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }

    if (children.size() != 0) {
        handle_name_list(children.back(), DataRegionName, ContextUse);
    }
}

inline static void handle_alter_database_set_secondary_region_stmt(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 3 && children[2]->get_ir_type() == IRTypeDatabaseName) {
        const int idx = 2;
        IR* new_name = new IR(IRTypeDatabaseName, children[idx]->to_string(), DataDatabaseName, ContextUse, children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }
}

inline static void handle_alter_database_drop_secondary_region(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 3 && children[2]->get_ir_type() == IRTypeDatabaseName) {
        const int idx = 2;
        IR* new_name = new IR(IRTypeDatabaseName, children[idx]->to_string(), DataDatabaseName, ContextUse, children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }
}

inline static void handle_alter_database_set_zone_config_extension_stmt(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 2 && children[2]->get_ir_type() == IRTypeDatabaseName) {
        const int idx = 2;
        IR* new_name = new IR(IRTypeDatabaseName, children[idx]->to_string(), DataDatabaseName, ContextUse, children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }

    if (children.size() > 7 && children[7]->get_ir_type() == IRTypeRegionName) {
        const int idx = 7;
        IR* new_name = new IR(IRTypeRegionName, children[idx]->to_string(), DataRegionName, ContextUse, children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }
}

inline static void handle_alter_zone_database_stmt(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 3 && children[2]->get_ir_type() == IRTypeDatabaseName) {
        const int idx = 2;
        IR* new_name = new IR(IRTypeDatabaseName, children[idx]->to_string(), DataDatabaseName, ContextUse, children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }
}

inline static void handle_comment_stmt(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 3 && children[3]->get_ir_type() == IRTypeDatabaseName) {
        const int idx = 3;
        IR* new_name = new IR(IRTypeDatabaseName, children[idx]->to_string(), DataDatabaseName, ContextUse, children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }

    if (children.size() > 3 && children[3]->get_ir_type() == IRTypeQualifiableSchemaName) {
        const int idx = 3;
        handle_qualifiable_schema_name(children[idx], DataSchemaName, ContextUse);
    }

    if (children.size() > 3 && children[3]->get_ir_type() == IRTypeTableName) {
        const int idx = 3;
        IR* new_name = new IR(IRTypeTableName, children[idx]->to_string(), DataTableName, ContextUse,
            children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }

    if (children.size() > 3 && children[3]->get_ir_type() == IRTypeTypeName) {
        const int idx = 3;
        IR* new_name = new IR(IRTypeTypeName, children[idx]->to_string(), DataTypeName, ContextUse,
            children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }

    if (children.size() > 6 && children[6]->get_ir_type() == IRTypeTableName) {
        const int idx = 6;
        IR* new_name = new IR(IRTypeTableName, children[idx]->to_string(), DataTableName, ContextUse,
            children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }
}

inline static void handle_drop_database_stmt(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 3 && children[2]->get_ir_type() == IRTypeDatabaseName) {
        const int idx = 2;
        IR* new_name = new IR(IRTypeDatabaseName, children[idx]->to_string(), DataDatabaseName, ContextUndefine, children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }

    if (children.size() > 4 && children[4]->get_ir_type() == IRTypeDatabaseName) {
        const int idx = 4;
        IR* new_name = new IR(IRTypeDatabaseName, children[idx]->to_string(), DataDatabaseName, ContextUndefine, children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }
}

inline static void handle_scrub_database_stmt(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 4 && children[3]->get_ir_type() == IRTypeDatabaseName) {
        const int idx = 3;
        IR* new_name = new IR(IRTypeDatabaseName, children[idx]->to_string(), DataDatabaseName, ContextUse,
            children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }
}

inline static void handle_show_partitions_stmt(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() == 5 && children[4]->get_ir_type() == IRTypeDatabaseName) {
        const int idx = 3;
        IR* new_name = new IR(IRTypeDatabaseName, children[idx]->to_string(), DataDatabaseName, ContextUse,
            children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }

    if (children.size() > 4 && children[4]->get_ir_type() == IRTypeTableName) {
        const int idx = 4;
        IR* new_name = new IR(IRTypeTableName, children[idx]->to_string(), DataTableName, ContextUse,
            children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }
}

inline static void handle_show_indexes_stmt(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() >= 5 && children[4]->get_ir_type() == IRTypeDatabaseName) {
        const int idx = 3;
        IR* new_name = new IR(IRTypeDatabaseName, children[idx]->to_string(), DataDatabaseName, ContextUse,
            children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }

    if (children.size() > 3 && children[3]->get_ir_type() == IRTypeTableName) {
        const int idx = 3;
        IR* new_name = new IR(IRTypeTableName, children[idx]->to_string(), DataTableName, ContextUse,
            children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }
}

inline static void handle_show_ranges_stmt(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.back()->get_ir_type() == IRTypeDatabaseName) {
        IR* new_name = new IR(IRTypeDatabaseName, children.back()->to_string(), DataDatabaseName, ContextUse,
            children.back()->get_mapped_token_node());
        cur_ir->swap_one_child(children.back(), new_name, true);
    }

    if (children.back()->get_ir_type() == IRTypeTableName) {
        IR* new_name = new IR(IRTypeTableName, children.back()->to_string(), DataTableName, ContextUse,
            children.back()->get_mapped_token_node());
        cur_ir->swap_one_child(children.back(), new_name, true);
    }
}

inline static void handle_show_survival_goal_stmt(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.back()->get_ir_type() == IRTypeDatabaseName) {
        IR* new_name = new IR(IRTypeDatabaseName, children.back()->to_string(), DataDatabaseName, ContextUse,
            children.back()->get_mapped_token_node());
        cur_ir->swap_one_child(children.back(), new_name, true);
    }
}

inline static void handle_show_regions_stmt(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.back()->get_ir_type() == IRTypeDatabaseName) {
        IR* new_name = new IR(IRTypeDatabaseName, children.back()->to_string(), DataDatabaseName, ContextUse,
            children.back()->get_mapped_token_node());
        cur_ir->swap_one_child(children.back(), new_name, true);
    }
}

inline static void handle_opt_in_database(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (!children.empty() && children.back()->get_ir_type() == IRTypeDatabaseName) {
        IR* new_name = new IR(IRTypeDatabaseName, children.back()->to_string(), DataDatabaseName, ContextUse,
            children.back()->get_mapped_token_node());
        cur_ir->swap_one_child(children.back(), new_name, true);
    }
}

inline static void handle_alter_database_to_schema_stmt(IR*& cur_ir)
{
    // FIXME:: not sure about this one.
    vector<IR*>& children = cur_ir->get_children_ref();

    if (!children.empty() && children[2]->get_ir_type() == IRTypeDatabaseName) {
        const int idx = 2;
        IR* new_name = new IR(IRTypeDatabaseName, children[idx]->to_string(), DataDatabaseName, ContextUndefine,
            children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }

    if (!children.empty() && children.back()->get_ir_type() == IRTypeDatabaseName) {
        IR* new_name = new IR(IRTypeDatabaseName, children.back()->to_string(), DataDatabaseName, ContextDefine,
            children.back()->get_mapped_token_node());
        cur_ir->swap_one_child(children.back(), new_name, true);
    }
}

inline static void handle_alter_rename_database_stmt(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (!children.empty() && children[2]->get_ir_type() == IRTypeDatabaseName) {
        const int idx = 2;
        IR* new_name = new IR(IRTypeDatabaseName, children[idx]->to_string(), DataDatabaseName, ContextReplaceUndefine,
            children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }

    if (!children.empty() && children.back()->get_ir_type() == IRTypeDatabaseName) {
        IR* new_name = new IR(IRTypeDatabaseName, children.back()->to_string(), DataDatabaseName, ContextReplaceDefine,
            children.back()->get_mapped_token_node());
        cur_ir->swap_one_child(children.back(), new_name, true);
    }
}

inline static void handle_create_database_stmt(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 3 && children[2]->get_ir_type() == IRTypeDatabaseName) {
        const int idx = 2;
        IR* new_name = new IR(IRTypeDatabaseName, children[idx]->to_string(), DataDatabaseName, ContextDefine,
            children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }

    if (children.size() > 5 && children[5]->get_ir_type() == IRTypeDatabaseName) {
        const int idx = 5;
        IR* new_name = new IR(IRTypeDatabaseName, children[idx]->to_string(), DataDatabaseName, ContextDefine,
            children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }

    if (children.size() > 10 && children[10]->get_ir_type() == IRTypeOptRegionsList) {
        const int idx = 10;
        handle_opt_regions_list(children[idx], DataRegionName, ContextUse);
    }

    if (children.size() > 13 && children[13]->get_ir_type() == IRTypeOptRegionsList) {
        const int idx = 13;
        handle_opt_regions_list(children[idx], DataRegionName, ContextUse);
    }
}

inline static void handle_alter_type_stmt(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 6 && children[5]->get_ir_type() == IRTypeColumnName) {
        const int idx = 5;
        IR* new_name = new IR(IRTypeColumnName, children[idx]->to_string(), DataColumnName, ContextUndefine,
            children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }

    if (children.size() > 7 && children[7]->get_ir_type() == IRTypeColumnName) {
        const int idx = 7;
        IR* new_name = new IR(IRTypeColumnName, children[idx]->to_string(), DataColumnName, ContextDefine,
            children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }

    if (children.size() > 2 && children[2]->get_ir_type() == IRTypeTypeName) {
        const int idx = 2;
        IR* new_name = new IR(IRTypeTypeName, children[idx]->to_string(), DataTypeName, ContextUse,
            children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }

    if (children.size() > 5 && children[5]->get_ir_type() == IRTypeSchemaName) {
        const int idx = 5;
        IR* new_name = new IR(IRTypeSchemaName, children[idx]->to_string(), DataSchemaName, ContextUse,
            children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }

    if (children.size() > 5 && children[5]->get_ir_type() == IRTypeName) {
        const int idx = 5;
        IR* new_name = new IR(IRTypeTypename, children[idx]->to_string(), DataTypeName, ContextDefine,
            children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }

    if (children.size() > 5 && children[5]->get_ir_type() == IRTypeName) {
        const int idx = 5;
        IR* new_name = new IR(IRTypeTypeName, children[idx]->to_string(), DataTypeName, ContextDefine, children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }
}

inline static void handle_alter_attribute_action(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 2 && children[2]->get_ir_type() == IRTypeColumnName) {
        const int idx = 2;
        IR* new_name = new IR(IRTypeColumnName, children[idx]->to_string(), DataColumnName, ContextUse,
            children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }

    if (children.size() > 4 && children[4]->get_ir_type() == IRTypeColumnName) {
        const int idx = 4;
        IR* new_name = new IR(IRTypeColumnName, children[idx]->to_string(), DataColumnName, ContextUse,
            children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }

    if (children.size() > 3 && children[3]->get_ir_type() == IRTypeTypeName) {
        const int idx = 3;
        IR* new_name = new IR(IRTypeTypeName, children[idx]->to_string(), DataTypeName, ContextUse,
            children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }

    if (children.size() > 4 && children[4]->get_ir_type() == IRTypeTypeName) {
        const int idx = 4;
        IR* new_name = new IR(IRTypeTypeName, children[idx]->to_string(), DataTypeName, ContextUse,
            children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }

    if (children.size() > 6 && children[6]->get_ir_type() == IRTypeTypeName) {
        const int idx = 6;
        IR* new_name = new IR(IRTypeTypeName, children[idx]->to_string(), DataTypeName, ContextUse,
            children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }
}

inline static void handle_column_table_def(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 0 && children[0]->get_ir_type() == IRTypeColumnName) {
        const int idx = 0;
        IR* new_name = new IR(IRTypeColumnName, children[idx]->to_string(), DataColumnName, ContextDefine,
            children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }
}

inline static void handle_create_as_table_defs(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 0 && children[0]->get_ir_type() == IRTypeColumnName) {
        const int idx = 0;
        IR* new_name = new IR(IRTypeColumnName, children[idx]->to_string(), DataColumnName, ContextDefine,
            children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }

    if (children.size() > 3 && children[2]->get_ir_type() == IRTypeColumnName) {
        const int idx = 2;
        IR* new_name = new IR(IRTypeColumnName, children[idx]->to_string(), DataColumnName, ContextDefine,
            children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }
}

inline static void handle_create_as_param(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 0 && children[0]->get_ir_type() == IRTypeColumnName) {
        const int idx = 0;
        IR* new_name = new IR(IRTypeColumnName, children[idx]->to_string(), DataColumnName, ContextUse,
            children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }
}

inline static void handle_insert_column_item(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 0 && children[0]->get_ir_type() == IRTypeColumnName) {
        const int idx = 0;
        IR* new_name = new IR(IRTypeColumnName, children[idx]->to_string(), DataColumnName, ContextUse,
            children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }
}

inline static void handle_single_set_clause(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 0 && children[0]->get_ir_type() == IRTypeColumnName) {
        const int idx = 0;
        IR* new_name = new IR(IRTypeColumnName, children[idx]->to_string(), DataColumnName, ContextUse,
            children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }
}

inline static void handle_opt_changefeed_family(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 1 && children[1]->get_ir_type() == IRTypeFamilyName) {
        const int idx = 0;
        IR* new_name = new IR(IRTypeFamilyName, children[idx]->to_string(), DataFamilyName, ContextUse,
            children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }
}

inline static void handle_create_as_col_qualification(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 1 && children.back()->get_ir_type() == IRTypeFamilyName) {
        const int idx = 1;
        IR* new_name = new IR(IRTypeFamilyName, children[idx]->to_string(), DataFamilyName, ContextUse,
            children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }
}

inline static void handle_family_def(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 1 && children[1]->get_ir_type() == IRTypeOptFamilyName) {
        const int idx = 1;
        handle_opt_family_name(children[1], DataFamilyName, ContextDefine);
    }

    if (children.size() > 3 && children[3]->get_ir_type() == IRTypeNameList) {
        handle_name_list(children[3], DataFamilyName, ContextUse);
    }
}

inline static void handle_prepare_stmt(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 1 && children[1]->get_ir_type() == IRTypeTableAliasName) {
        const int idx = 1;
        IR* new_name = new IR(IRTypeTableAliasName, children[idx]->to_string(), DataTableAliasName, ContextDefine,
            children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }
}

inline static void handle_execute_stmt(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 1 && children[1]->get_ir_type() == IRTypeTableAliasName) {
        const int idx = 1;
        IR* new_name = new IR(IRTypeTableAliasName, children[idx]->to_string(), DataTableAliasName, ContextUse,
            children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }
}

inline static void handle_insert_target(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 2 && children[2]->get_ir_type() == IRTypeTableAliasName) {
        const int idx = 2;
        IR* new_name = new IR(IRTypeTableAliasName, children[idx]->to_string(), DataTableAliasName, ContextDefine,
            children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }

    if (children.size() > 0 && children[0]->get_ir_type() == IRTypeTableName) {
        const int idx = 0;
        IR* new_name = new IR(IRTypeTableName, children[idx]->to_string(), DataTableName, ContextUse,
            children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }
}

inline static void handle_common_table_expr(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 1 && children[0]->get_ir_type() == IRTypeTableAliasName) {
        const int idx = 0;
        IR* new_name = new IR(IRTypeTableAliasName, children[idx]->to_string(), DataTableAliasName, ContextDefine,
            children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }
}

inline static void handle_alias_clause(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 1 && children[0]->get_ir_type() == IRTypeTableAliasName) {
        const int idx = 0;
        IR* new_name = new IR(IRTypeTableAliasName, children[idx]->to_string(), DataTableAliasName, ContextDefine,
            children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }

    if (children.size() > 2 && children[1]->get_ir_type() == IRTypeTableAliasName) {
        const int idx = 1;
        IR* new_name = new IR(IRTypeTableAliasName, children[idx]->to_string(), DataTableAliasName, ContextDefine,
            children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }
}

inline static void handle_func_alias_clause(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 1 && children[0]->get_ir_type() == IRTypeTableAliasName) {
        const int idx = 0;
        IR* new_name = new IR(IRTypeTableAliasName, children[idx]->to_string(), DataTableAliasName, ContextDefine,
            children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }

    if (children.size() > 2 && children[1]->get_ir_type() == IRTypeTableAliasName) {
        const int idx = 1;
        IR* new_name = new IR(IRTypeTableAliasName, children[idx]->to_string(), DataTableAliasName, ContextDefine,
            children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }
}

inline static void handle_table_expr_opt_alias_idx(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 0 && children.back()->get_ir_type() == IRTypeTableAliasName) {
        IR* new_name = new IR(IRTypeTableAliasName, children.back()->to_string(), DataTableAliasName, ContextDefine,
            children.back()->get_mapped_token_node());
        cur_ir->swap_one_child(children.back(), new_name, true);
    }
}

inline static void handle_create_stats_stmt(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 2 && children[2]->get_ir_type() == IRTypeTableAliasName) {
        const int idx = 2;
        IR* new_name = new IR(IRTypeStatisticsName, children[idx]->to_string(), DataStatisticsName, ContextDefine,
            children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }
}

inline static void handle_window_definition(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 0 && children[0]->get_ir_type() == IRTypeWindowName) {
        const int idx = 0;
        IR* new_name = new IR(IRTypeWindowName, children[idx]->to_string(), DataWindowName, ContextDefine,
            children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }
}

inline static void handle_over_clause(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 1 && children[1]->get_ir_type() == IRTypeWindowName) {
        const int idx = 1;
        IR* new_name = new IR(IRTypeWindowName, children[idx]->to_string(), DataWindowName, ContextUse,
            children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }
}

inline static void handle_refresh_stmt(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 4 && children[4]->get_ir_type() == IRTypeViewName) {
        const int idx = 4;
        IR* new_name = new IR(IRTypeViewName, children[idx]->to_string(), DataViewName, ContextUse,
            children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }
}

inline static void handle_create_view_stmt(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 3 && children[3]->get_ir_type() == IRTypeViewName) {
        const int idx = 3;
        IR* new_name = new IR(IRTypeViewName, children[idx]->to_string(), DataViewName, ContextDefine,
            children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }

    if (children.size() > 4 && children[4]->get_ir_type() == IRTypeViewName) {
        const int idx = 4;
        IR* new_name = new IR(IRTypeViewName, children[idx]->to_string(), DataViewName, ContextDefine,
            children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }

    if (children.size() > 5 && children[5]->get_ir_type() == IRTypeViewName) {
        const int idx = 5;
        IR* new_name = new IR(IRTypeViewName, children[idx]->to_string(), DataViewName, ContextDefine,
            children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }

    if (children.size() > 6 && children[6]->get_ir_type() == IRTypeViewName) {
        const int idx = 6;
        IR* new_name = new IR(IRTypeViewName, children[idx]->to_string(), DataViewName, ContextDefine,
            children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }

    if (children.size() > 7 && children[7]->get_ir_type() == IRTypeViewName) {
        const int idx = 7;
        IR* new_name = new IR(IRTypeViewName, children[idx]->to_string(), DataViewName, ContextDefine,
            children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }
}

inline static void handle_alter_rename_view_stmt(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.back()->get_ir_type() == IRTypeViewName) {
        IR* new_name = new IR(IRTypeViewName, children.back()->to_string(), DataViewName, ContextReplaceDefine,
            children.back()->get_mapped_token_node());
        cur_ir->swap_one_child(children.back(), new_name, true);
    }

    if (children.size() > 2 && children[2]->get_ir_type() == IRTypeRelationExpr) {
        const int idx = 2;
        handle_relation_expr(children[idx], DataViewName, ContextReplaceUndefine);
    }

    if (children.size() > 3 && children[3]->get_ir_type() == IRTypeRelationExpr) {
        const int idx = 3;
        handle_relation_expr(children[idx], DataViewName, ContextReplaceUndefine);
    }

    if (children.size() > 4 && children[4]->get_ir_type() == IRTypeRelationExpr) {
        const int idx = 4;
        handle_relation_expr(children[idx], DataViewName, ContextReplaceUndefine);
    }

    if (children.size() > 5 && children[5]->get_ir_type() == IRTypeRelationExpr) {
        const int idx = 5;
        handle_relation_expr(children[idx], DataViewName, ContextReplaceUndefine);
    }
}

inline static void handle_alter_onetable_stmt(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 2 && children[2]->get_ir_type() == IRTypeRelationExpr) {
        const int idx = 2;
        handle_relation_expr(children[idx], DataTableName, ContextUse);
    }

    if (children.size() > 4 && children[4]->get_ir_type() == IRTypeRelationExpr) {
        const int idx = 4;
        handle_relation_expr(children[idx], DataTableName, ContextUse);
    }
}

inline static void handle_alter_rename_table_stmt(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 2 && children[2]->get_ir_type() == IRTypeRelationExpr) {
        const int idx = 2;
        handle_relation_expr(children[idx], DataTableName, ContextReplaceUndefine);
    }

    if (children.size() > 4 && children[4]->get_ir_type() == IRTypeRelationExpr) {
        const int idx = 4;
        handle_relation_expr(children[idx], DataTableName, ContextReplaceUndefine);
    }

    if (children.size() > 0 && children.back()->get_ir_type() == IRTypeTableName) {
        IR* new_name = new IR(IRTypeTableName, children.back()->to_string(), DataTableName, ContextReplaceDefine, children.back()->get_mapped_token_node());
        cur_ir->swap_one_child(children.back(), new_name, true);
    }
}

inline static void handle_alter_table_set_schema_stmt(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 2 && children[2]->get_ir_type() == IRTypeRelationExpr) {
        const int idx = 2;
        handle_relation_expr(children[idx], DataTableName, ContextUse);
    }

    if (children.size() > 4 && children[4]->get_ir_type() == IRTypeRelationExpr) {
        const int idx = 4;
        handle_relation_expr(children[idx], DataTableName, ContextUse);
    }

    if (children.back()->get_ir_type() == IRTypeSchemaName) {
        IR* new_name = new IR(IRTypeSchemaName, children.back()->to_string(), DataSchemaName, ContextUse,
            children.back()->get_mapped_token_node());
        cur_ir->swap_one_child(children.back(), new_name, true);
    }
}

inline static void handle_alter_table_locality_stmt(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 2 && children[2]->get_ir_type() == IRTypeRelationExpr) {
        const int idx = 2;
        handle_relation_expr(children[idx], DataTableName, ContextUse);
    }

    if (children.size() > 4 && children[4]->get_ir_type() == IRTypeRelationExpr) {
        const int idx = 4;
        handle_relation_expr(children[idx], DataTableName, ContextUse);
    }
}

inline static void handle_alter_table_owner_stmt(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 2 && children[2]->get_ir_type() == IRTypeRelationExpr) {
        const int idx = 2;
        handle_relation_expr(children[idx], DataTableName, ContextUse);
    }

    if (children.size() > 4 && children[4]->get_ir_type() == IRTypeRelationExpr) {
        const int idx = 4;
        handle_relation_expr(children[idx], DataTableName, ContextUse);
    }
}

inline static void handle_alter_view_set_schema_stmt(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 2 && children[2]->get_ir_type() == IRTypeRelationExpr) {
        const int idx = 2;
        handle_relation_expr(children[idx], DataViewName, ContextUse);
    }

    if (children.size() > 3 && children[3]->get_ir_type() == IRTypeRelationExpr) {
        const int idx = 3;
        handle_relation_expr(children[idx], DataViewName, ContextUse);
    }

    if (children.size() > 4 && children[4]->get_ir_type() == IRTypeRelationExpr) {
        const int idx = 4;
        handle_relation_expr(children[idx], DataViewName, ContextUse);
    }

    if (children.size() > 5 && children[5]->get_ir_type() == IRTypeRelationExpr) {
        const int idx = 5;
        handle_relation_expr(children[idx], DataViewName, ContextUse);
    }

    if (children.back()->get_ir_type() == IRTypeSchemaName) {
        IR* new_name = new IR(IRTypeSchemaName, children.back()->to_string(), DataSchemaName, ContextUse,
            children.back()->get_mapped_token_node());
        cur_ir->swap_one_child(children.back(), new_name, true);
    }
}

inline static void handle_alter_view_owner_stmt(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 2 && children[2]->get_ir_type() == IRTypeRelationExpr) {
        const int idx = 2;
        handle_relation_expr(children[idx], DataViewName, ContextUse);
    }

    if (children.size() > 3 && children[3]->get_ir_type() == IRTypeRelationExpr) {
        const int idx = 3;
        handle_relation_expr(children[idx], DataViewName, ContextUse);
    }

    if (children.size() > 4 && children[4]->get_ir_type() == IRTypeRelationExpr) {
        const int idx = 4;
        handle_relation_expr(children[idx], DataViewName, ContextUse);
    }

    if (children.size() > 5 && children[5]->get_ir_type() == IRTypeRelationExpr) {
        const int idx = 5;
        handle_relation_expr(children[idx], DataViewName, ContextUse);
    }
}

inline static void handle_alter_sequence_set_schema_stmt(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 2 && children[2]->get_ir_type() == IRTypeRelationExpr) {
        const int idx = 2;
        handle_relation_expr(children[idx], DataSequenceName, ContextUse);
    }

    if (children.size() > 4 && children[4]->get_ir_type() == IRTypeRelationExpr) {
        const int idx = 4;
        handle_relation_expr(children[idx], DataSequenceName, ContextUse);
    }

    if (children.back()->get_ir_type() == IRTypeSchemaName) {
        IR* new_name = new IR(IRTypeSchemaName, children.back()->to_string(), DataSchemaName, ContextUse,
            children.back()->get_mapped_token_node());
        cur_ir->swap_one_child(children.back(), new_name, true);
    }
}

inline static void handle_alter_sequence_owner_stmt(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 2 && children[2]->get_ir_type() == IRTypeRelationExpr) {
        const int idx = 2;
        handle_relation_expr(children[idx], DataSequenceName, ContextUse);
    }

    if (children.size() > 4 && children[4]->get_ir_type() == IRTypeRelationExpr) {
        const int idx = 4;
        handle_relation_expr(children[idx], DataSequenceName, ContextUse);
    }
}

inline static void handle_alter_rename_sequence_stmt(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 2 && children[2]->get_ir_type() == IRTypeRelationExpr) {
        const int idx = 2;
        handle_relation_expr(children[idx], DataSequenceName, ContextReplaceUndefine);
    }

    if (children.size() > 4 && children[4]->get_ir_type() == IRTypeRelationExpr) {
        const int idx = 4;
        handle_relation_expr(children[idx], DataSequenceName, ContextReplaceUndefine);
    }

    if (children.size() > 5 && children[5]->get_ir_type() == IRTypeSequenceName) {
        const int idx = 5;
        IR* new_name = new IR(IRTypeSequenceName, children[idx]->to_string(), DataSequenceName, ContextReplaceDefine,
            children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }

    if (children.size() > 7 && children[7]->get_ir_type() == IRTypeSequenceName) {
        const int idx = 7;
        IR* new_name = new IR(IRTypeSequenceName, children[idx]->to_string(), DataSequenceName, ContextReplaceDefine,
            children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }
}

inline static void handle_table_ref(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 0 && children[0]->get_ir_type() == IRTypeRelationExpr) {
        const int idx = 0;
        handle_relation_expr(children[idx], DataTableName, ContextUse);
    }
}

inline static void handle_truncate_stmt(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 2 && children[2]->get_ir_type() == IRTypeRelationExpr) {
        const int idx = 2;
        handle_relation_expr_list(children[idx], DataTableName, ContextUse);
    }
}

inline static void handle_create_type_stmt(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 2 && children[2]->get_ir_type() == IRTypeTypeName) {
        const int idx = 2;
        IR* new_name = new IR(IRTypeTypeName, children[idx]->to_string(), DataTypeName, ContextDefine,
            children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }

    if (children.size() > 5 && children[5]->get_ir_type() == IRTypeTypeName) {
        const int idx = 5;
        IR* new_name = new IR(IRTypeTypeName, children[idx]->to_string(), DataTypeName, ContextDefine,
            children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }
}

inline static void handle_unlisten_stmt(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 1 && children[1]->get_ir_type() == IRTypeTypeName) {
        const int idx = 1;
        IR* new_name = new IR(IRTypeTypeName, children[idx]->to_string(), DataTypeName, ContextUse,
            children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }
}

inline static void handle_drop_type_stmt(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 2 && children[2]->get_ir_type() == IRTypeTypeName) {
        const int idx = 2;
        handle_type_name_list(children[idx], DataTypeName, ContextUndefine);
    }

    if (children.size() > 4 && children[4]->get_ir_type() == IRTypeTypeName) {
        const int idx = 4;
        handle_type_name_list(children[idx], DataTypeName, ContextUndefine);
    }
}

inline static void handle_target_types(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 0 && children[0]->get_ir_type() == IRTypeTypeName) {
        const int idx = 0;
        handle_type_name_list(children[idx], DataTypeName, ContextUse);
    }
}

inline static void handle_targets_roles(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 1 && children[1]->get_ir_type() == IRTypeTypeName) {
        const int idx = 1;
        handle_type_name_list(children[idx], DataTypeName, ContextUse);
    }

    if (children.size() > 1 && children[1]->get_ir_type() == IRTypeSchemaNameList) {
        const int idx = 1;
        handle_schema_name_list(children[idx], DataSchemaName, ContextUse);
    }

    // FIXME:: schema_wildcard, wildcard_pattern. Not instantiated. Only appear in the old/latest version?
}

inline static void handle_alter_sequence_options_stmt(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 2 && children[2]->get_ir_type() == IRTypeSequenceName) {
        const int idx = 2;
        IR* new_name = new IR(IRTypeSequenceName, children[idx]->to_string(), DataSequenceName, ContextUse,
            children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }

    if (children.size() > 4 && children[4]->get_ir_type() == IRTypeSequenceName) {
        const int idx = 4;
        IR* new_name = new IR(IRTypeSequenceName, children[idx]->to_string(), DataSequenceName, ContextUse,
            children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }
}

inline static void handle_show_create_stmt(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 3 && children[3]->get_ir_type() == IRTypeSequenceName) {
        const int idx = 3;
        IR* new_name = new IR(IRTypeSequenceName, children[idx]->to_string(), DataSequenceName, ContextDefine,
            children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }

    if (children.size() > 0 && children.back()->get_ir_type() == IRTypeTableName) {
        IR* new_name = new IR(IRTypeTableName, children.back()->to_string(), DataSequenceName, ContextDefine,
            children.back()->get_mapped_token_node());
        cur_ir->swap_one_child(children.back(), new_name, true);
    }
}

inline static void handle_create_sequence_stmt(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 3 && children[3]->get_ir_type() == IRTypeSequenceName) {
        const int idx = 3;
        IR* new_name = new IR(IRTypeSequenceName, children[idx]->to_string(), DataSequenceName, ContextDefine,
            children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }

    if (children.size() > 5 && children[5]->get_ir_type() == IRTypeSequenceName) {
        const int idx = 5;
        IR* new_name = new IR(IRTypeSequenceName, children[idx]->to_string(), DataSequenceName, ContextDefine,
            children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }
}

inline static void handle_locality(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.back()->get_ir_type() == IRTypeRegionName) {
        IR* new_name = new IR(IRTypeRegionName, children.back()->to_string(), DataRegionName, ContextUse,
            children.back()->get_mapped_token_node());
        cur_ir->swap_one_child(children.back(), new_name, true);
    }
}

inline static void handle_primary_region_clause(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.back()->get_ir_type() == IRTypeRegionName) {
        IR* new_name = new IR(IRTypeRegionName, children.back()->to_string(), DataRegionName, ContextUse,
            children.back()->get_mapped_token_node());
        cur_ir->swap_one_child(children.back(), new_name, true);
    }
}

inline static void handle_secondary_region_clause(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.back()->get_ir_type() == IRTypeRegionName) {
        IR* new_name = new IR(IRTypeRegionName, children.back()->to_string(), DataRegionName, ContextUse,
            children.back()->get_mapped_token_node());
        cur_ir->swap_one_child(children.back(), new_name, true);
    }
}

inline static void handle_alter_func_set_schema_stmt(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.back()->get_ir_type() == IRTypeSchemaName) {
        IR* new_name = new IR(IRTypeSchemaName, children.back()->to_string(), DataSchemaName, ContextUse,
            children.back()->get_mapped_token_node());
        cur_ir->swap_one_child(children.back(), new_name, true);
    }
}

inline static void handle_alter_schema_stmt(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.back()->get_ir_type() == IRTypeSchemaName) {
        IR* new_name = new IR(IRTypeSchemaName, children.back()->to_string(), DataSchemaName, ContextDefine,
            children.back()->get_mapped_token_node());
        cur_ir->swap_one_child(children.back(), new_name, true);
    }

    if (children.size() > 2 && children[2]->get_ir_type() == IRTypeQualifiableSchemaName) {
        const int idx = 2;
        handle_qualifiable_schema_name(children[idx], DataSchemaName, ContextUndefine);
    }
}

inline static void handle_opt_in_schema(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (!children.empty() && children.back()->get_ir_type() == IRTypeSchemaName) {
        IR* new_name = new IR(IRTypeSchemaName, children.back()->to_string(), DataSchemaName, ContextUse,
            children.back()->get_mapped_token_node());
        cur_ir->swap_one_child(children.back(), new_name, true);
    }

    if (!children.empty() && children.back()->get_ir_type() == IRTypeSchemaNameList) {
        handle_schema_name_list(children.back(), DataSchemaName, ContextUse);
    }
}

inline static void handle_create_schema_stmt(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (!children.empty() && children.back()->get_ir_type() == IRTypeQualifiableSchemaName) {
        handle_qualifiable_schema_name(children.back(), DataSchemaName, ContextUndefine);
    }

    if (children.size() > 2 && children[2]->get_ir_type() == IRTypeOptSchemaName) {
        const int idx = 2;
        handle_opt_schema_name(children[idx], DataSchemaName, ContextDefine);
    }

    if (children.size() > 5 && children[5]->get_ir_type() == IRTypeOptSchemaName) {
        const int idx = 5;
        handle_opt_schema_name(children[idx], DataSchemaName, ContextDefine);
    }
}

inline static void handle_drop_schema_stmt(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 2 && children[2]->get_ir_type() == IRTypeSchemaNameList) {
        handle_schema_name_list(children[2], DataSchemaName, ContextUndefine);
    }

    if (children.size() > 4 && children[4]->get_ir_type() == IRTypeSchemaNameList) {
        handle_schema_name_list(children[4], DataSchemaName, ContextUndefine);
    }
}

inline static void handle_grant_stmt(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 4 && children[4]->get_ir_type() == IRTypeSchemaNameList) {
        handle_schema_name_list(children[4], DataSchemaName, ContextUse);
    }

    if (children.size() > 7 && children[7]->get_ir_type() == IRTypeSchemaNameList) {
        handle_schema_name_list(children[7], DataSchemaName, ContextUse);
    }
}

inline static void handle_revoke_stmt(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 4 && children[4]->get_ir_type() == IRTypeSchemaNameList) {
        handle_schema_name_list(children[4], DataSchemaName, ContextUse);
    }

    if (children.size() > 7 && children[7]->get_ir_type() == IRTypeSchemaNameList) {
        handle_schema_name_list(children[7], DataSchemaName, ContextUse);
    }

    if (children.size() > 10 && children[10]->get_ir_type() == IRTypeSchemaNameList) {
        handle_schema_name_list(children[10], DataSchemaName, ContextUse);
    }
}

inline static void handle_alter_split_stmt(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 2 && children[2]->get_ir_type() == IRTypeTableName) {
        const int idx = 2;
        IR* new_name = new IR(IRTypeTableName, children[idx]->to_string(), DataTableName, ContextUse,
            children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }
}

inline static void handle_alter_unsplit_stmt(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 2 && children[2]->get_ir_type() == IRTypeTableName) {
        const int idx = 2;
        IR* new_name = new IR(IRTypeTableName, children[idx]->to_string(), DataTableName, ContextUse,
            children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }
}

inline static void handle_alter_relocate_stmt(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 2 && children[2]->get_ir_type() == IRTypeTableName) {
        const int idx = 2;
        IR* new_name = new IR(IRTypeTableName, children[idx]->to_string(), DataTableName, ContextUse,
            children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }
}

inline static void handle_alter_zone_table_stmt(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 2 && children[2]->get_ir_type() == IRTypeTableName) {
        const int idx = 2;
        IR* new_name = new IR(IRTypeTableName, children[idx]->to_string(), DataTableName, ContextUse,
            children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }
}

inline static void handle_alter_scatter_stmt(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 2 && children[2]->get_ir_type() == IRTypeTableName) {
        const int idx = 2;
        IR* new_name = new IR(IRTypeTableName, children[idx]->to_string(), DataTableName, ContextUse,
            children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }
}

inline static void handle_import_stmt(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 2 && children[2]->get_ir_type() == IRTypeTableName) {
        const int idx = 2;
        IR* new_name = new IR(IRTypeTableName, children[idx]->to_string(), DataTableName, ContextUse,
            children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }
}

inline static void handle_copy_stmt(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 1 && children[1]->get_ir_type() == IRTypeTableName) {
        const int idx = 1;
        IR* new_name = new IR(IRTypeTableName, children[idx]->to_string(), DataTableName, ContextUse,
            children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }
}

inline static void handle_create_stats_target(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 0 && children[0]->get_ir_type() == IRTypeTableName) {
        const int idx = 0;
        IR* new_name = new IR(IRTypeTableName, children[idx]->to_string(), DataTableName, ContextUse,
            children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }
}

inline static void handle_changefeed_target(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 1 && children[1]->get_ir_type() == IRTypeTableName) {
        const int idx = 1;
        IR* new_name = new IR(IRTypeTableName, children[idx]->to_string(), DataTableName, ContextUse,
            children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }
}

inline static void handle_analyze_target(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 0 && children[0]->get_ir_type() == IRTypeTableName) {
        const int idx = 0;
        IR* new_name = new IR(IRTypeTableName, children[idx]->to_string(), DataTableName, ContextUse,
            children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }
}

inline static void handle_scrub_table_stmt(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 3 && children[3]->get_ir_type() == IRTypeTableName) {
        const int idx = 3;
        IR* new_name = new IR(IRTypeTableName, children[idx]->to_string(), DataTableName, ContextUse,
            children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }
}

inline static void handle_show_stats_stmt(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 4 && children[4]->get_ir_type() == IRTypeTableName) {
        const int idx = 4;
        IR* new_name = new IR(IRTypeTableName, children[idx]->to_string(), DataTableName, ContextUse,
            children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }

    if (children.size() > 6 && children[6]->get_ir_type() == IRTypeTableName) {
        const int idx = 6;
        IR* new_name = new IR(IRTypeTableName, children[idx]->to_string(), DataTableName, ContextUse,
            children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }
}

inline static void handle_show_columns_stmt(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 3 && children[3]->get_ir_type() == IRTypeTableName) {
        const int idx = 3;
        IR* new_name = new IR(IRTypeTableName, children[idx]->to_string(), DataTableName, ContextUse,
            children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }
}

inline static void handle_show_constraints_stmt(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 3 && children[3]->get_ir_type() == IRTypeTableName) {
        const int idx = 3;
        IR* new_name = new IR(IRTypeTableName, children[idx]->to_string(), DataTableName, ContextUse,
            children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }
}

inline static void handle_show_range_for_row_stmt(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 5 && children[5]->get_ir_type() == IRTypeTableName) {
        const int idx = 5;
        IR* new_name = new IR(IRTypeTableName, children[idx]->to_string(), DataTableName, ContextUse,
            children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }
}

inline static void handle_show_fingerprints_stmt(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 4 && children[4]->get_ir_type() == IRTypeTableName) {
        const int idx = 4;
        IR* new_name = new IR(IRTypeTableName, children[idx]->to_string(), DataTableName, ContextUse,
            children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }
}

inline static void handle_create_table_stmt(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    vector<pair<IR*, IR*>> v_replace_pair;
    for (IR*& child_ir : children) {
        if (child_ir->get_ir_type() == IRTypeTableName) {
            IR* new_name = new IR(IRTypeTableName, child_ir->to_string(), DataTableName, ContextDefine,
                child_ir->get_mapped_token_node());
            v_replace_pair.push_back(pair<IR*, IR*>(child_ir, new_name));
        }
    }
    for (pair<IR*, IR*> cur_pair : v_replace_pair) {
        cur_ir->swap_one_child(cur_pair.first, cur_pair.second, true);
    }
}

inline static void handle_create_table_as_stmt(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    vector<pair<IR*, IR*>> v_replace_pair;
    for (IR*& child_ir : children) {
        if (child_ir->get_ir_type() == IRTypeTableName) {
            IR* new_name = new IR(IRTypeTableName, child_ir->to_string(), DataTableName, ContextDefine,
                child_ir->get_mapped_token_node());
            v_replace_pair.push_back(pair<IR*, IR*>(child_ir, new_name));
        }
    }

    for (pair<IR*, IR*> cur_pair : v_replace_pair) {
        cur_ir->swap_one_child(cur_pair.first, cur_pair.second, true);
    }
}

inline static void handle_table_elem(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 1 && children[1]->get_ir_type() == IRTypeTableName) {
        const int idx = 1;
        IR* new_name = new IR(IRTypeTableName, children[idx]->to_string(), DataTableName, ContextUse,
            children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }
}

inline static void handle_col_qualification_elem(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 1 && children[1]->get_ir_type() == IRTypeTableName) {
        const int idx = 1;
        IR* new_name = new IR(IRTypeTableName, children[idx]->to_string(), DataTableName, ContextUse,
            children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }
}

inline static void handle_constraint_elem(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 5 && children[5]->get_ir_type() == IRTypeTableName) {
        const int idx = 5;
        IR* new_name = new IR(IRTypeTableName, children[idx]->to_string(), DataTableName, ContextUse,
            children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }
}

inline static void handle_table_name_opt_idx(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 1 && children[1]->get_ir_type() == IRTypeTableName) {
        const int idx = 1;
        IR* new_name = new IR(IRTypeTableName, children[idx]->to_string(), DataTableName, ContextUse,
            children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }
}

inline static void handle_db_name(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 3 && children[3]->get_ir_type() == IRTypeDbName) {
        const int idx = 3;
        IR* new_name = new IR(IRTypeDatabaseName, children[idx]->to_string(), DataDatabaseName, ContextUse,
            children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }
}

inline static void handle_explain_option_list(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() == 3) {
        int idx = 0;
        handle_explain_option_list(children[idx]);

        idx = 2;
        IR* new_name = new IR(IRTypeExplainOptionName, children[idx]->to_string(), DataSettingName, ContextUse,
            children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }

    else {
        // size == 1
        int idx = 0;
        IR* new_name = new IR(IRTypeExplainOptionName, children[idx]->to_string(), DataSettingName, ContextUse,
            children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }
}

inline static void handle_close_cursor_stmt(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 1 && children[1]->get_ir_type() == IRTypeCursorName) {
        const int idx = 1;
        IR* new_name = new IR(IRTypeCursorName, children[idx]->to_string(), DataCursorName, ContextUndefine,
            children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }
}

inline static void handle_declare_cursor_stmt(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 1 && children[1]->get_ir_type() == IRTypeCursorName) {
        const int idx = 1;
        IR* new_name = new IR(IRTypeCursorName, children[idx]->to_string(), DataCursorName, ContextDefine,
            children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }
}

inline static void handle_cursor_movement_specifier(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 0 && children.back()->get_ir_type() == IRTypeCursorName) {
        IR* new_name = new IR(IRTypeCursorName, children.back()->to_string(), DataCursorName, ContextDefine,
            children.back()->get_mapped_token_node());
        cur_ir->swap_one_child(children.back(), new_name, true);
    }
}

inline static void handle_column_path(RSG* rsg, IR*& cur_ir)
{
    // all ContextUse
    string ori_str = cur_ir->to_string();
    cur_ir->free_children();
    IR* new_name = new IR(IRTypeColumnName, string(ori_str), DataColumnName, ContextUse, rsg->get_token_from_ir_type(IRTypeColumnName));
    vector<IR*> tmp_v { new_name };
    cur_ir->set_children_nodes(tmp_v);
}

inline static void handle_prefixed_column_path(RSG* rsg, IR*& cur_ir)
{
    // all ContextUse
    string ori_str = cur_ir->to_string();
    cur_ir->free_children();
    IR* new_name = new IR(IRTypeColumnName, string(ori_str), DataColumnName, ContextUse, rsg->get_token_from_ir_type(IRTypeColumnName));
    vector<IR*> tmp_v { new_name };
    cur_ir->set_children_nodes(tmp_v);
}

inline static void handle_column_path_with_star(RSG* rsg, IR*& cur_ir)
{
    // all ContextUse
    cur_ir->free_children();
    IR* new_name = new IR(IRTypeTableName, string("c0"), DataTableName, ContextUse, rsg->get_token_from_ir_type(IRTypeTableName));
    IR* new_dot_term = new IR(SymbolTerm, IRTypeUnknownType, string("."), rsg->get_token_from_ir_type(IRTypeColumnPathWithStar));
    IR* new_star_term = new IR(SymbolTerm, IRTypeUnknownType, string("*"), rsg->get_token_from_ir_type(IRTypeColumnPathWithStar));
    vector<IR*> tmp_v { new_name, new_dot_term, new_star_term };
    cur_ir->set_children_nodes(tmp_v);
}

inline static void handle_func_name(RSG* rsg, IR*& cur_ir)
{
    // all ContextUse
    cur_ir->free_children();
    IR* new_name = new IR(IRTypeFuncName, string("SUM"), DataFunctionName, ContextUse, rsg->get_token_from_ir_type(IRTypeFuncName));
    vector<IR*> tmp_v { new_name };
    cur_ir->set_children_nodes(tmp_v);
}

inline static void handle_type_function_name_no_crdb_extra(RSG* rsg, IR*& cur_ir)
{
    // all ContextUse
    cur_ir->free_children();
    IR* new_name = new IR(IRTypeTypeName, string("INT"), DataTypeName, ContextUse, rsg->get_token_from_ir_type(IRTypeTypeName));
    vector<IR*> tmp_v { new_name };
    cur_ir->set_children_nodes(tmp_v);
}

inline static void handle_param_name(RSG* rsg, IR*& cur_ir)
{
    // all ContextUse
    cur_ir->free_children();
    IR* new_name = new IR(IRTypeFuncName, string("vfunc"), DataFunctionName, ContextUse, rsg->get_token_from_ir_type(IRTypeFuncName));
    vector<IR*> tmp_v { new_name };
    cur_ir->set_children_nodes(tmp_v);
}

inline static void handle_simple_typename(RSG* rsg, IR*& cur_ir)
{
    // all ContextUse
    string ori_str = cur_ir->to_string();
    cur_ir->free_children();
    IR* new_name = new IR(IRTypeTypeName, ori_str, DataTypeName, ContextUse, rsg->get_token_from_ir_type(IRTypeTypeName));
    vector<IR*> tmp_v { new_name };
    cur_ir->set_children_nodes(tmp_v);
}

inline static void handle_complex_db_object_name(RSG* rsg, IR*& cur_ir)
{
    // all ContextUse
    cur_ir->free_children();
    IR* new_name = new IR(IRTypeDatabaseName, string("db0"), DataDatabaseName, ContextUse, rsg->get_token_from_ir_type(IRTypeDatabaseName));
    vector<IR*> tmp_v { new_name };
    cur_ir->set_children_nodes(tmp_v);
}

inline static void handle_attrs(RSG* rsg, IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() == 3 && children[0]->get_ir_type() == IRTypeUnrestrictedName) {
        const int idx = 0;
        handle_attrs(rsg, children[idx]);
    }

    IR* new_name = new IR(IRTypeUnrestrictedName, children.back()->to_string(), DataSettingName, ContextUse,
        children.back()->get_mapped_token_node());
    cur_ir->swap_one_child(children.back(), new_name, true);
}

inline static void handle_d_expr(RSG* rsg, IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() != 0 && children.back()->get_ir_type() == IRTypeUnrestrictedName) {
        // override '(' a_expr ')' '.' unrestricted_name to NULL.
        IR* new_name = new IR(SymbolTerm, IRTypeUnknownType, string("NULL"), nullptr);
        cur_ir->free_children();
        vector<IR*> v_tmp = { new_name };
        cur_ir->set_children_nodes(v_tmp);
    }
}

inline static void handle_complex_table_pattern(RSG* rsg, IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    IR* new_name = new IR(IRTypeTableName, string("v00"), DataTableName, ContextUse, rsg->get_token_from_ir_type(IRTypeTableName));
    cur_ir->free_children();
    vector<IR*> v_tmp = { new_name };
    cur_ir->set_children_nodes(v_tmp);
}

inline static void handle_kv_option(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 0 && children[0]->get_ir_type() == IRTypeName) {
        const int idx = 0;
        IR* new_name = new IR(IRTypeSETTING, children[idx]->to_string(), DataSettingName, ContextUse,
            children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }
}

inline static void handle_cancel_all_jobs_stmt(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 2 && children[2]->get_ir_type() == IRTypeName) {
        const int idx = 2;
        IR* new_name = new IR(IRTypeName, children[idx]->to_string(), DataJobName, ContextUse,
            children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_name, true);
    }
}

inline static void handle_create_extension_stmt(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    vector<pair<IR*, IR*>> v_replace_pair;
    for (IR*& child_ir : children) {
        if (child_ir->get_ir_type() == IRTypeName) {
            IR* new_name = new IR(IRTypeName, child_ir->to_string(), DataExtensionName, ContextDefine,
                child_ir->get_mapped_token_node());
            v_replace_pair.push_back(pair<IR*, IR*>(child_ir, new_name));
        }
    }
    for (pair<IR*, IR*> cur_pair : v_replace_pair) {
        cur_ir->swap_one_child(cur_pair.first, cur_pair.second, true);
    }
}

inline static void handle_drop_index_stmt(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    vector<IR*> v_ir_to_handle;
    for (IR*& child_ir : children) {
        if (child_ir->get_ir_type() == IRTypeDropIndexStmt) {
            v_ir_to_handle.push_back(child_ir);
            //            handle_table_index_name_list(child_ir, ContextUndefine);
        }
    }

    for (IR* cur_ir_to_handel : v_ir_to_handle) {
        handle_table_index_name_list(cur_ir_to_handel, ContextUndefine);
    }
}

inline static void handle_opt_stats_columns(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 1 && children[1]->get_ir_type() == IRTypeNameList) {
        handle_name_list(children[1], DataStatisticsName, ContextUse);
    }
}

inline static void handle_scrub_option(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 2 && children[2]->get_ir_type() == IRTypeNameList) {
        if (children.front()->get_ir_type() == IRTypeINDEX) {
            const int idx = 2;
            handle_name_list(children[idx], DataIndexName, ContextUse);
        } else if (children.front()->get_ir_type() == IRTypeCONSTRAINT) {
            const int idx = 2;
            handle_name_list(children[idx], DataConstraintName, ContextUse);
        }
    }
}

inline static void handle_query_stats_cols(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 1 && children[1]->get_ir_type() == IRTypeNameList) {
        const int idx = 1;
        handle_name_list(children[idx], DataStatisticsName, ContextUse);
    }
}

inline static void handle_grant_targets(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() != 0 && children.back()->get_ir_type() == IRTypeNameList) {
        handle_name_list(children.back(), DataDatabaseName, ContextUse);
    }
}

inline static void handle_backup_targets(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() != 0 && children.back()->get_ir_type() == IRTypeNameList) {
        handle_name_list(children.back(), DataDatabaseName, ContextUse);
    }
}

inline static void handle_partition_by_inner(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 2 && children[2]->get_ir_type() == IRTypeNameList) {
        handle_name_list(children[2], DataColumnName, ContextUse);
    }
}

inline static void handle_opt_storing(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 2 && children[2]->get_ir_type() == IRTypeNameList) {
        handle_name_list(children[2], DataColumnName, ContextUse);
    }
}

inline static void handle_opt_column_list(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 1 && children[1]->get_ir_type() == IRTypeNameList) {
        handle_name_list(children[1], DataColumnName, ContextUse);
    }
}

inline static void handle_join_qual(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 2 && children[2]->get_ir_type() == IRTypeNameList) {
        handle_name_list(children[2], DataColumnName, ContextUse);
    }
}

inline static void handle_labeled_row(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 3 && children[3]->get_ir_type() == IRTypeNameList) {
        handle_name_list(children[3], DataColumnAliasName, ContextDefine);
    }
}

inline static void handle_common_routine_opt_item(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 1 && children[1]->get_ir_type() == IRTypeName) {
        // FIXME
        return;
    }
}

inline static void handle_alter_func_rename_stmt(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 5 && children[5]->get_ir_type() == IRTypeName) {
        const int idx = 5;
        IR* new_idx_name = new IR(IRTypeFuncName, children[idx]->to_string(), DataFunctionName, ContextReplaceDefine, children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_idx_name, true);
    }
}

inline static void handle_alter_func_dep_extension_stmt(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 7 && children[7]->get_ir_type() == IRTypeName) {
        const int idx = 7;
        IR* new_idx_name = new IR(IRTypeFuncName, children[idx]->to_string(), DataFunctionName, ContextDefine, children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_idx_name, true);
    }
}

inline static void handle_deallocate_stmt(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 1 && children[1]->get_ir_type() == IRTypeName) {
        const int idx = 1;
        IR* new_idx_name = new IR(IRTypeName, children[idx]->to_string(), DataStatementPreparedName, ContextUndefine, children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_idx_name, true);
    }

    if (children.size() > 2 && children[2]->get_ir_type() == IRTypeName) {
        const int idx = 2;
        IR* new_idx_name = new IR(IRTypeName, children[idx]->to_string(), DataStatementPreparedName, ContextUndefine, children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_idx_name, true);
    }
}

inline static void handle_privilege(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 0 && children[0]->get_ir_type() == IRTypeName) {
        const int idx = 0;
        IR* new_idx_name = new IR(IRTypePrivilege, children[idx]->to_string(), DataPrivilege, ContextUse, children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_idx_name, true);
    }
}

inline static void handle_resume_all_jobs_stmt(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 2 && children[2]->get_ir_type() == IRTypeName) {
        const int idx = 2;
        IR* new_idx_name = new IR(IRTypeJOB, children[idx]->to_string(), DataJobName, ContextUse, children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_idx_name, true);
    }
}

inline static void handle_pause_all_jobs_stmt(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 2 && children[2]->get_ir_type() == IRTypeName) {
        const int idx = 2;
        IR* new_idx_name = new IR(IRTypeJOB, children[idx]->to_string(), DataJobName, ContextUse, children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_idx_name, true);
    }
}

inline static void handle_index_elem(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 0 && children[0]->get_ir_type() == IRTypeName) {
        const int idx = 0;
        IR* new_idx_name = new IR(IRTypeColumnName, children[idx]->to_string(), DataIndexName, ContextUse, children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_idx_name, true);
    }
}

inline static void handle_opt_class(IR*& cur_ir)
{
    // always keep opt_class as empty.
    cur_ir->set_symbol_type(SymbolTerm);
    cur_ir->set_str_val("");
    cur_ir->free_children();
}

inline static void handle_savepoint_stmt(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 1 && children[1]->get_ir_type() == IRTypeName) {
        const int idx = 1;
        IR* new_idx_name = new IR(IRTypeSavepointName, children[idx]->to_string(), DataSavePointName, ContextDefine, children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_idx_name, true);
    }
}

inline static void handle_savepoint_name(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 0 && children[0]->get_ir_type() == IRTypeName) {
        const int idx = 0;
        IR* new_idx_name = new IR(IRTypeSavepointName, children[idx]->to_string(), DataSavePointName, ContextUse, children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_idx_name, true);
    }

    if (children.size() > 1 && children[1]->get_ir_type() == IRTypeName) {
        const int idx = 1;
        IR* new_idx_name = new IR(IRTypeSavepointName, children[idx]->to_string(), DataSavePointName, ContextUse, children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_idx_name, true);
    }
}

inline static void handle_col_def_list_no_types(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 0 && children[0]->get_ir_type() == IRTypeName) {
        const int idx = 0;
        IR* new_idx_name = new IR(IRTypeColumnName, children[idx]->to_string(), DataColumnName, ContextDefine, children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_idx_name, true);
    }

    if (children.size() > 2 && children[2]->get_ir_type() == IRTypeName) {
        const int idx = 2;
        IR* new_idx_name = new IR(IRTypeColumnName, children[idx]->to_string(), DataColumnName, ContextDefine, children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_idx_name, true);
    }
}

inline static void handle_col_def(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 0 && children[0]->get_ir_type() == IRTypeName) {
        const int idx = 0;
        IR* new_idx_name = new IR(IRTypeColumnName, children[idx]->to_string(), DataColumnName, ContextDefine, children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_idx_name, true);
    }
}

inline static void handle_opt_existing_window_name(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 0 && children[0]->get_ir_type() == IRTypeName) {
        const int idx = 0;
        IR* new_idx_name = new IR(IRTypeWindowName, children[idx]->to_string(), DataWindowName, ContextUse, children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_idx_name, true);
    }
}

inline static void handle_operator_op(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() > 0 && children[0]->get_ir_type() == IRTypeName) {
        // FIXME::
        return;
    }
}

inline static void handle_ICONST(IR*& cur_ir)
{
    cur_ir->set_symbol_type(SymbolLit);
    cur_ir->set_ir_type(IRTypeICONST);
    cur_ir->set_data_type(DataLiteral);
    cur_ir->set_data_affinity_type(AFFIINT);
}

inline static void handle_FCONST(IR*& cur_ir)
{
    cur_ir->set_symbol_type(SymbolLit);
    cur_ir->set_ir_type(IRTypeFCONST);
    cur_ir->set_data_type(DataLiteral);
    cur_ir->set_data_affinity_type(AFFIFLOAT);
}

inline static void handle_SCONST(IR*& cur_ir)
{
    cur_ir->set_symbol_type(SymbolLit);
    cur_ir->set_ir_type(IRTypeSCONST);
    cur_ir->set_data_type(DataLiteral);
    cur_ir->set_data_affinity_type(AFFISTRING);
}

inline static void handle_BCONST(IR*& cur_ir)
{
    cur_ir->set_symbol_type(SymbolLit);
    cur_ir->set_ir_type(IRTypeBCONST);
    cur_ir->set_data_type(DataLiteral);
    cur_ir->set_data_affinity_type(AFFIBYTES);
}

inline static void handle_BITCONST(IR*& cur_ir)
{
    cur_ir->set_symbol_type(SymbolLit);
    cur_ir->set_ir_type(IRTypeBITCONST);
    cur_ir->set_data_type(DataLiteral);
    cur_ir->set_data_affinity_type(AFFIBIT);
}

inline static void handle_true(IR*& cur_ir)
{
    cur_ir->set_symbol_type(SymbolLit);
    cur_ir->set_ir_type(IRTypeTRUE);
    cur_ir->set_data_type(DataLiteral);
    cur_ir->set_data_affinity_type(AFFIBOOL);
}

inline static void handle_false(IR*& cur_ir)
{
    cur_ir->set_symbol_type(SymbolLit);
    cur_ir->set_ir_type(IRTypeFALSE);
    cur_ir->set_data_type(DataLiteral);
    cur_ir->set_data_affinity_type(AFFIBOOL);
}

inline static void handle_func_application(IR*& cur_ir)
{
    // Let the instantiator to handle the function generation.
    IRTYPE ir_type = IRTypeFuncApplication;
    IR* new_ir = new IR(ir_type, string(""), DataFunctionExpr, ContextUse, cur_ir->get_mapped_token_node());
    new_ir->set_symbol_type(SymbolTerm);
    cur_ir->deep_drop();
    cur_ir = new_ir;
}

inline static void handle_group_clause(IR*& cur_ir)
{
    cur_ir->set_is_favor(IsFavor::favor);
}

inline static void handle_having_clause(IR*& cur_ir)
{
    cur_ir->set_is_favor(IsFavor::favor);
}

inline static void handle_sort_clause(IR*& cur_ir)
{
    cur_ir->set_is_favor(IsFavor::favor);
}

inline static void handle_sort_by_index(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() == 4 && children[2]->get_ir_type() == IRTypeTableName) {
        const int idx = 2;
        IR* new_idx_name = new IR(IRTypeTableName, children[idx]->to_string(), DataTableName, ContextUse, children[idx]->get_mapped_token_node());
        cur_ir->swap_one_child(children[idx], new_idx_name, true);
        return;
    }

    for (IR*& cur_child : children) {
        if (cur_child->get_ir_type() == IRTypeTableName) {
            IR* new_idx_name = new IR(IRTypeTableName, cur_child->to_string(), DataTableName, ContextUse, cur_child->get_mapped_token_node());
            cur_ir->swap_one_child(cur_child, new_idx_name, true);
        }
        if (cur_child->get_ir_type() == IRTypeIndexName) {
            IR* new_idx_name = new IR(IRTypeIndexName, cur_child->to_string(), DataIndexName, ContextUse, cur_child->get_mapped_token_node());
            cur_ir->swap_one_child(cur_child, new_idx_name, true);
        }
    }
}

inline static void handle_role_spec(IR*& cur_ir)
{
    vector<IR*>& children = cur_ir->get_children_ref();

    if (children.size() == 1) {
        const int idx = 0;
        IR* new_idx_name = new IR(IRTypeIDENT, children[idx]->to_string(), DataRoleName, ContextUse, children[idx]->get_mapped_token_node());
        new_idx_name->set_str_val(string("public"));
        cur_ir->swap_one_child(children[idx], new_idx_name, true);
    }
}

inline static void handle_transaction_stmt(IR*& cur_ir)
{
    // Ignore all transaction related statements.
    cur_ir->free_children();
}

void cockroachdb_ir_context_setup(RSG* rsg, IR*& cur_ir)
{
    IRTYPE cur_ir_type = cur_ir->get_ir_type();

    switch (cur_ir_type) {
    case (IRTypeColQualification): {
        handle_col_qualification(cur_ir);
        break;
    }

    case (IRTypeOptCollate): {
        handle_opt_collate(cur_ir);
        break;
    }

    case (IRTypeAExpr): {
        handle_a_expr(cur_ir);
        break;
    }

    case (IRTypeAlterZonePartitionStmt): {
        handle_alter_zone_partition_stmt(cur_ir);
        break;
    }

    case (IRTypeShowZoneStmt): {
        handle_show_zone_stmt(cur_ir);
        break;
    }

    case (IRTypePartition): {
        handle_partition(cur_ir);
        break;
    }

    case (IRTypeCreateIndexStmt): {
        handle_create_index_stmt(cur_ir);
        break;
    }

    case (IRTypeAlterRenameIndexStmt): {
        handle_alter_rename_index_stmt(cur_ir);
        break;
    }

    case (IRTypeSortby): {
        handle_sortby(cur_ir);
        break;
    }

    case (IRTypeIndexFlagsParam): {
        handle_index_flags_param(cur_ir);
        break;
    }

    case (IRTypeOptIndexFlags): {
        handle_opt_index_flags(cur_ir);
        break;
    }

    case (IRTypeTableIndexName): {
        handle_table_index_name(cur_ir);
        break;
    }

    case (IRTypeIndexDef): {
        handle_index_def(cur_ir);
        break;
    }

    case (IRTypeTargetElem): {
        handle_target_elem(cur_ir);
        break;
    }

    case (IRTypeAlterTableCmd): {
        handle_alter_table_cmd(cur_ir);
        break;
    }

    case (IRTypeTableConstraint): {
        handle_table_constraint(cur_ir);
        break;
    }

    case (IRTypeOnConflict): {
        handle_on_conflict(cur_ir);
        break;
    }

    case (IRTypeAlterDatabaseOwner): {
        handle_alter_database_owner(cur_ir);
        break;
    }

    case (IRTypeAlterDatabaseSetStmt): {
        handle_alter_database_set_stmt(cur_ir);
        break;
    }

    case (IRTypeAlterDatabasePlacementStmt): {
        handle_alter_database_placement_stmt(cur_ir);
        break;
    }

    case (IRTypeAlterDatabaseAddRegionStmt): {
        handle_alter_database_add_region_stmt(cur_ir);
        break;
    }

    case (IRTypeAlterDatabaseDropRegionStmt): {
        handle_alter_database_drop_region_stmt(cur_ir);
        break;
    }

    case (IRTypeAlterDatabaseSurvivalGoalStmt): {
        handle_alter_database_survival_goal_stmt(cur_ir);
        break;
    }

    case (IRTypeAlterDatabasePrimaryRegionStmt): {
        handle_alter_database_primary_region_stmt(cur_ir);
        break;
    }

    case (IRTypeAlterDatabaseAddSuperRegion): {
        handle_alter_database_add_super_region(cur_ir);
        break;
    }

    case (IRTypeAlterDatabaseDropSuperRegion): {
        handle_alter_database_drop_super_region(cur_ir);
        break;
    }

    case (IRTypeAlterDatabaseAlterSuperRegion): {
        handle_alter_database_alter_super_region(cur_ir);
        break;
    }

    case (IRTypeAlterDatabaseSetSecondaryRegionStmt): {
        handle_alter_database_set_secondary_region_stmt(cur_ir);
        break;
    }

    case (IRTypeAlterDatabaseDropSecondaryRegion): {
        handle_alter_database_drop_secondary_region(cur_ir);
        break;
    }

    case (IRTypeAlterDatabaseSetZoneConfigExtensionStmt): {
        handle_alter_database_set_zone_config_extension_stmt(cur_ir);
        break;
    }

    case (IRTypeAlterZoneDatabaseStmt): {
        handle_alter_zone_database_stmt(cur_ir);
        break;
    }

    case (IRTypeCommentStmt): {
        handle_comment_stmt(cur_ir);
        break;
    }

    case (IRTypeDropDatabaseStmt): {
        handle_drop_database_stmt(cur_ir);
        break;
    }

    case (IRTypeScrubDatabaseStmt): {
        handle_scrub_database_stmt(cur_ir);
        break;
    }

    case (IRTypeShowPartitionsStmt): {
        handle_show_partitions_stmt(cur_ir);
        break;
    }

    case (IRTypeShowIndexesStmt): {
        handle_show_indexes_stmt(cur_ir);
        break;
    }

    case (IRTypeShowRangesStmt): {
        handle_show_ranges_stmt(cur_ir);
        break;
    }

    case (IRTypeShowSurvivalGoalStmt): {
        handle_show_survival_goal_stmt(cur_ir);
        break;
    }

    case (IRTypeShowRegionsStmt): {
        handle_show_regions_stmt(cur_ir);
        break;
    }

    case (IRTypeOptInDatabase): {
        handle_opt_in_database(cur_ir);
        break;
    }

    case (IRTypeAlterDatabaseToSchemaStmt): {
        handle_alter_database_to_schema_stmt(cur_ir);
        break;
    }

    case (IRTypeAlterRenameDatabaseStmt): {
        handle_alter_rename_database_stmt(cur_ir);
        break;
    }

    case (IRTypeCreateDatabaseStmt): {
        handle_create_database_stmt(cur_ir);
        break;
    }

    case (IRTypeAlterTypeStmt): {
        handle_alter_type_stmt(cur_ir);
        break;
    }

    case (IRTypeAlterAttributeAction): {
        handle_alter_attribute_action(cur_ir);
        break;
    }

    case (IRTypeColumnTableDef): {
        handle_column_table_def(cur_ir);
        break;
    }

    case (IRTypeCreateAsTableDefs): {
        handle_create_as_table_defs(cur_ir);
        break;
    }

    case (IRTypeCreateAsParam): {
        handle_create_as_param(cur_ir);
        break;
    }

    case (IRTypeInsertColumnItem): {
        handle_insert_column_item(cur_ir);
        break;
    }

    case (IRTypeSingleSetClause): {
        handle_single_set_clause(cur_ir);
        break;
    }

    case (IRTypeOptChangefeedFamily): {
        handle_opt_changefeed_family(cur_ir);
        break;
    }

    case (IRTypeCreateAsColQualification): {
        handle_create_as_col_qualification(cur_ir);
        break;
    }

    case (IRTypeFamilyDef): {
        handle_family_def(cur_ir);
        break;
    }

    case (IRTypePrepareStmt): {
        handle_prepare_stmt(cur_ir);
        break;
    }

    case (IRTypeExecuteStmt): {
        handle_execute_stmt(cur_ir);
        break;
    }

    case (IRTypeInsertTarget): {
        handle_insert_target(cur_ir);
        break;
    }

    case (IRTypeCommonTableExpr): {
        handle_common_table_expr(cur_ir);
        break;
    }

    case (IRTypeAliasClause): {
        handle_alias_clause(cur_ir);
        break;
    }

    case (IRTypeFuncAliasClause): {
        handle_func_alias_clause(cur_ir);
        break;
    }

    case (IRTypeTableExprOptAliasIdx): {
        handle_table_expr_opt_alias_idx(cur_ir);
        break;
    }

    case (IRTypeCreateStatsStmt): {
        handle_create_stats_stmt(cur_ir);
        break;
    }

    case (IRTypeWindowDefinition): {
        handle_window_definition(cur_ir);
        break;
    }

    case (IRTypeOverClause): {
        handle_over_clause(cur_ir);
        break;
    }

    case (IRTypeRefreshStmt): {
        handle_refresh_stmt(cur_ir);
        break;
    }

    case (IRTypeCreateViewStmt): {
        handle_create_view_stmt(cur_ir);
        break;
    }

    case (IRTypeAlterRenameViewStmt): {
        handle_alter_rename_view_stmt(cur_ir);
        break;
    }

    case (IRTypeAlterOnetableStmt): {
        handle_alter_onetable_stmt(cur_ir);
        break;
    }

    case (IRTypeAlterRenameTableStmt): {
        handle_alter_rename_table_stmt(cur_ir);
        break;
    }

    case (IRTypeAlterTableSetSchemaStmt): {
        handle_alter_table_set_schema_stmt(cur_ir);
        break;
    }

    case (IRTypeAlterTableLocalityStmt): {
        handle_alter_table_locality_stmt(cur_ir);
        break;
    }

    case (IRTypeAlterTableOwnerStmt): {
        handle_alter_table_owner_stmt(cur_ir);
        break;
    }

    case (IRTypeAlterViewSetSchemaStmt): {
        handle_alter_view_set_schema_stmt(cur_ir);
        break;
    }

    case (IRTypeAlterViewOwnerStmt): {
        handle_alter_view_owner_stmt(cur_ir);
        break;
    }

    case (IRTypeAlterSequenceSetSchemaStmt): {
        handle_alter_sequence_set_schema_stmt(cur_ir);
        break;
    }

    case (IRTypeAlterSequenceOwnerStmt): {
        handle_alter_sequence_owner_stmt(cur_ir);
        break;
    }

    case (IRTypeAlterRenameSequenceStmt): {
        handle_alter_rename_sequence_stmt(cur_ir);
        break;
    }

    case (IRTypeTableRef): {
        handle_table_ref(cur_ir);
        cur_ir->set_is_favor(IsFavor::favor);
        break;
    }

    case (IRTypeTruncateStmt): {
        handle_truncate_stmt(cur_ir);
        break;
    }

    case (IRTypeCreateTypeStmt): {
        handle_create_type_stmt(cur_ir);
        break;
    }

    case (IRTypeUnlistenStmt): {
        handle_unlisten_stmt(cur_ir);
        break;
    }

    case (IRTypeDropTypeStmt): {
        handle_drop_type_stmt(cur_ir);
        break;
    }

    case (IRTypeTargetTypes): {
        handle_target_types(cur_ir);
        break;
    }

    case (IRTypeTargetsRoles): {
        handle_targets_roles(cur_ir);
        break;
    }

    case (IRTypeAlterSequenceOptionsStmt): {
        handle_alter_sequence_options_stmt(cur_ir);
        break;
    }

    case (IRTypeShowCreateStmt): {
        handle_show_create_stmt(cur_ir);
        break;
    }

    case (IRTypeCreateSequenceStmt): {
        handle_create_sequence_stmt(cur_ir);
        break;
    }

    case (IRTypeLocality): {
        handle_locality(cur_ir);
        break;
    }

    case (IRTypePrimaryRegionClause): {
        handle_primary_region_clause(cur_ir);
        break;
    }

    case (IRTypeSecondaryRegionClause): {
        handle_secondary_region_clause(cur_ir);
        break;
    }

    case (IRTypeAlterFuncSetSchemaStmt): {
        handle_alter_func_set_schema_stmt(cur_ir);
        break;
    }

    case (IRTypeAlterSchemaStmt): {
        handle_alter_schema_stmt(cur_ir);
        break;
    }

    case (IRTypeOptInSchema): {
        handle_opt_in_schema(cur_ir);
        break;
    }

    case (IRTypeCreateSchemaStmt): {
        handle_create_schema_stmt(cur_ir);
        break;
    }

    case (IRTypeDropSchemaStmt): {
        handle_drop_schema_stmt(cur_ir);
        break;
    }

    case (IRTypeGrantStmt): {
        handle_grant_stmt(cur_ir);
        break;
    }

    case (IRTypeRevokeStmt): {
        handle_revoke_stmt(cur_ir);
        break;
    }

    case (IRTypeAlterSplitStmt): {
        handle_alter_split_stmt(cur_ir);
        break;
    }

    case (IRTypeAlterUnsplitStmt): {
        handle_alter_unsplit_stmt(cur_ir);
        break;
    }

    case (IRTypeAlterRelocateStmt): {
        handle_alter_relocate_stmt(cur_ir);
        break;
    }

    case (IRTypeAlterZoneTableStmt): {
        handle_alter_zone_table_stmt(cur_ir);
        break;
    }

    case (IRTypeAlterScatterStmt): {
        handle_alter_scatter_stmt(cur_ir);
        break;
    }

    case (IRTypeImportStmt): {
        handle_import_stmt(cur_ir);
        break;
    }

    case (IRTypeCopyStmt): {
        handle_copy_stmt(cur_ir);
        break;
    }

    case (IRTypeCreateStatsTarget): {
        handle_create_stats_target(cur_ir);
        break;
    }

    case (IRTypeChangefeedTarget): {
        handle_changefeed_target(cur_ir);
        break;
    }

    case (IRTypeAnalyzeTarget): {
        handle_analyze_target(cur_ir);
        break;
    }

    case (IRTypeScrubTableStmt): {
        handle_scrub_table_stmt(cur_ir);
        break;
    }

    case (IRTypeShowStatsStmt): {
        handle_show_stats_stmt(cur_ir);
        break;
    }

    case (IRTypeShowColumnsStmt): {
        handle_show_columns_stmt(cur_ir);
        break;
    }

    case (IRTypeShowConstraintsStmt): {
        handle_show_constraints_stmt(cur_ir);
        break;
    }

    case (IRTypeShowRangeForRowStmt): {
        handle_show_range_for_row_stmt(cur_ir);
        break;
    }

    case (IRTypeShowFingerprintsStmt): {
        handle_show_fingerprints_stmt(cur_ir);
        break;
    }

    case (IRTypeCreateTableStmt): {
        handle_create_table_stmt(cur_ir);
        break;
    }

    case (IRTypeCreateTableAsStmt): {
        handle_create_table_as_stmt(cur_ir);
        break;
    }

    case (IRTypeTableElem): {
        handle_table_elem(cur_ir);
        break;
    }

    case (IRTypeColQualificationElem): {
        handle_col_qualification_elem(cur_ir);
        break;
    }

    case (IRTypeConstraintElem): {
        handle_constraint_elem(cur_ir);
        break;
    }

    case (IRTypeTableNameOptIdx): {
        handle_table_name_opt_idx(cur_ir);
        break;
    }

    case (IRTypeDbName): {
        handle_db_name(cur_ir);
        break;
    }

    case (IRTypeExplainOptionList): {
        handle_explain_option_list(cur_ir);
        break;
    }

    case (IRTypeCloseCursorStmt): {
        handle_close_cursor_stmt(cur_ir);
        break;
    }

    case (IRTypeDeclareCursorStmt): {
        handle_declare_cursor_stmt(cur_ir);
        break;
    }

    case (IRTypeCursorMovementSpecifier): {
        handle_cursor_movement_specifier(cur_ir);
        break;
    }

    case (IRTypeColumnPath): {
        handle_column_path(rsg, cur_ir);
        break;
    }

    case (IRTypePrefixedColumnPath): {
        handle_prefixed_column_path(rsg, cur_ir);
        break;
    }

    case (IRTypeColumnPathWithStar): {
        handle_column_path_with_star(rsg, cur_ir);
        break;
    }

    case (IRTypeFuncName): {
        handle_func_name(rsg, cur_ir);
        break;
    }

    case (IRTypeTypeFunctionNameNoCrdbExtra): {
        handle_type_function_name_no_crdb_extra(rsg, cur_ir);
        break;
    }

    case (IRTypeParamName): {
        handle_param_name(rsg, cur_ir);
        break;
    }

    case (IRTypeSimpleTypename): {
        handle_simple_typename(rsg, cur_ir);
        break;
    }

    case (IRTypeComplexDbObjectName): {
        handle_complex_db_object_name(rsg, cur_ir);
        break;
    }

    case (IRTypeAttrs): {
        handle_attrs(rsg, cur_ir);
        break;
    }

    case (IRTypeDExpr): {
        handle_d_expr(rsg, cur_ir);
        break;
    }

    case (IRTypeComplexTablePattern): {
        handle_complex_table_pattern(rsg, cur_ir);
        break;
    }

    case (IRTypeKvOption): {
        handle_kv_option(cur_ir);
        break;
    }

    case (IRTypeCancelAllJobsStmt): {
        handle_cancel_all_jobs_stmt(cur_ir);
        break;
    }

    case (IRTypeCreateExtensionStmt): {
        handle_create_extension_stmt(cur_ir);
        break;
    }

    case (IRTypeDropIndexStmt): {
        handle_drop_index_stmt(cur_ir);
        break;
    }

    case (IRTypeOptStatsColumns): {
        handle_opt_stats_columns(cur_ir);
        break;
    }

    case (IRTypeScrubOption): {
        handle_scrub_option(cur_ir);
        break;
    }

    case (IRTypeQueryStatsCols): {
        handle_query_stats_cols(cur_ir);
        break;
    }

    case (IRTypeGrantTargets): {
        handle_grant_targets(cur_ir);
        break;
    }

    case (IRTypeBackupTargets): {
        handle_backup_targets(cur_ir);
        break;
    }

    case (IRTypePartitionByInner): {
        handle_partition_by_inner(cur_ir);
        break;
    }

    case (IRTypeOptStoring): {
        handle_opt_storing(cur_ir);
        break;
    }

    case (IRTypeOptColumnList): {
        handle_opt_column_list(cur_ir);
        break;
    }

    case (IRTypeJoinQual): {
        handle_join_qual(cur_ir);
        break;
    }

    case (IRTypeLabeledRow): {
        handle_labeled_row(cur_ir);
        break;
    }

    case (IRTypeCommonRoutineOptItem): {
        handle_common_routine_opt_item(cur_ir);
        break;
    }

    case (IRTypeAlterFuncRenameStmt): {
        handle_alter_func_rename_stmt(cur_ir);
        break;
    }

    case (IRTypeAlterFuncDepExtensionStmt): {
        handle_alter_func_dep_extension_stmt(cur_ir);
        break;
    }

    case (IRTypeDeallocateStmt): {
        handle_deallocate_stmt(cur_ir);
        break;
    }

    case (IRTypePrivilege): {
        handle_privilege(cur_ir);
        break;
    }

    case (IRTypeResumeAllJobsStmt): {
        handle_resume_all_jobs_stmt(cur_ir);
        break;
    }

    case (IRTypePauseAllJobsStmt): {
        handle_pause_all_jobs_stmt(cur_ir);
        break;
    }

    case (IRTypeIndexElem): {
        handle_index_elem(cur_ir);
        break;
    }

    case (IRTypeOptClass): {
        handle_opt_class(cur_ir);
        break;
    }

    case (IRTypeSavepointStmt): {
        handle_savepoint_stmt(cur_ir);
        break;
    }

    case (IRTypeSavepointName): {
        handle_savepoint_name(cur_ir);
        break;
    }

    case (IRTypeColDefListNoTypes): {
        handle_col_def_list_no_types(cur_ir);
        break;
    }

    case (IRTypeColDef): {
        handle_col_def(cur_ir);
        break;
    }

    case (IRTypeOptExistingWindowName): {
        handle_opt_existing_window_name(cur_ir);
        break;
    }

    case (IRTypeLogicalReplicationResources): {
        handle_logical_replication_resources(cur_ir);
        break;
    }

    case (IRTypeOperatorOp): {
        handle_operator_op(cur_ir);
        break;
    }

    case (IRTypeICONST): {
        handle_ICONST(cur_ir);
        break;
    }

    case (IRTypeFCONST): {
        handle_FCONST(cur_ir);
        break;
    }

    case (IRTypeSCONST): {
        handle_SCONST(cur_ir);
        break;
    }

    case (IRTypeBCONST): {
        handle_BCONST(cur_ir);
        break;
    }

    case (IRTypeBITCONST): {
        handle_BITCONST(cur_ir);
        break;
    }

    case (IRTypeTRUE): {
        handle_true(cur_ir);
        break;
    }
    case (IRTypeFALSE): {
        handle_false(cur_ir);
        break;
    }
    case (IRTypeFuncApplication): {
        handle_func_application(cur_ir);
        break;
    }

    case (IRTypeGroupClause): {
        handle_group_clause(cur_ir);
        break;
    }

    case (IRTypeHavingClause): {
        handle_having_clause(cur_ir);
        break;
    }

    case (IRTypeOptSortClause):
    case (IRTypeOptSortClauseNoIndex): {
        handle_sort_clause(cur_ir);
        break;
    }

    case (IRTypeSortbyIndex): {
        handle_sort_by_index(cur_ir);
        break;
    }

    case (IRTypeRoleSpec): {
        handle_role_spec(cur_ir);
        break;
    }

    case (IRTypeTransactionStmt): {
        handle_transaction_stmt(cur_ir);
        break;
    }

    default:
        break;
    }

    // IMPORTANT!!!
    for (auto& interesting_type: CockroachDBFuzzerConfigurations::cockroachdb_interesting_ir_types) {
        if (interesting_type == cur_ir_type) {
            cur_ir->set_is_favor(IsFavor::favor);
            break;
        }
    }
}
