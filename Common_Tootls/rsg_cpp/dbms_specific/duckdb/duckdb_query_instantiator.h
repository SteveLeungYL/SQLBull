//
// Created by XXX on 10/21/24.
//

#ifndef DUCKDB_QUERY_INSTANTIATOR_H
#define DUCKDB_QUERY_INSTANTIATOR_H

#include "../../headers/json.hpp"
#include "../../headers/query_instantiator.h"
#include "duckdb_ir_wrapper.h"

using json = nlohmann::json;

class DuckDBQueryInstantiator : public QueryInstantiator {
private:
    bool fill_one_stmt_helper(IR* stmt);
    vector<IR*> split_to_substmt(IR* cur_stmt,
        map<IR*, pair<int, IR*>>& m_save);

    bool connect_back(map<IR*, pair<int, IR*>>& m_save);
    bool instan_preprocessing(IR* stmt_root,
        vector<IR*>& ordered_all_subquery_ir);
    bool instan_dependency(IR* cur_stmt_root,
        const vector<vector<IR*>> all_substmt_ir_of_interest);

    bool remove_type_annotation(IR* cur_stmt_root,
        vector<IR*>& ir_to_deep_drop);
    void instan_database_schema_name(IR* ir_to_fix);
    void instan_table_name(IR* ir_to_fix, bool& is_replace_table);
    void instan_table_alias_name(IR* ir_to_fix, IR* cur_stmt_root,
        bool is_alias_optional);
    void instan_view_name(IR* ir_to_fix);
    void instan_partition_name(IR* ir_to_fix);
    void instan_index_name(IR* ir_to_fix, vector<IR*>&);
    void instan_column_name(IR* ir_to_fix, IR*&, bool&, vector<IR*>&);
    string find_cloest_table_name(IR* ir_to_fix);
    void instan_literal(IR* ir_to_fix, IR* cur_stmt_root,
        vector<IR*>& ir_to_deep_drop);
    DATAAFFINITYTYPE get_nearby_data_affinity(IR* ir_to_fix);
    void instan_column_alias_name(IR* ir_to_fix, IR* cur_stmt_root,
        vector<IR*>& ir_to_deep_drop);
    void instan_sql_type_name(IR* ir_to_fix);
    void instan_func_application(IR* ir_to_fix, IR* cur_stmt_ir_root, vector<IR*>& ir_to_deep_drop,
        bool is_ignore_nested_expr);
    void instan_statistic_name(IR* ir_to_fix);
    void instan_sequence_name(IR* ir_to_fix);
    void instan_constraint_name(IR* ir_to_fix);
    void instan_family_name(IR* ir_to_fix);
    void instan_storage_param(IR* ir_to_fix, vector<IR*>& ir_to_deep_drop);
    void instan_window_name(IR* ir_to_fix);
    void map_create_view(IR* ir_to_fix, IR* cur_stmt_root,
        const vector<vector<IR*>> all_substmt_ir_of_interest);
    void map_create_view_column(IR* ir_to_fix,
        vector<IR*>& ir_to_deep_drop);

    DuckDBIRWrapper ir_wrapper;

    /* shared data for CockroachDB specifically */
    // Save the mapping from the Storage Parameter variable name to
    //    the mapped Data Affinity for the variable.
    map<string, DataAffinity> storage_param_lib;
    vector<string> all_storage_param;

public:
    // inherited interface
    virtual void init_data_library() override;

    // helper function
    void constr_sql_func_lib_helper(json& json_obj, vector<string>& v_all_func_str,
        map<DATAAFFINITYTYPE, vector<string>>& func_type_lib,
        map<string, vector<FunctionSignature>>& func_str_to_type_map);
    void constr_sql_func_lib(string func_types_str, vector<string>& v_all_func_str,
        map<DATAAFFINITYTYPE, vector<string>>& func_type_lib,
        map<string, vector<FunctionSignature>>&);

    void constr_key_pair_datatype_lib_helper(json& key_pair_json, vector<string>& v_all_key_str, map<string, DataAffinity>& mapped_key_pair);
    void constr_key_pair_datatype_lib(string key_pair_str, vector<string>& v_all_key_str, map<string, DataAffinity>& mapped_key_pair);

    virtual IR* constr_rand_func_with_affinity(DATAAFFINITYTYPE in_affi) override;

    // Constructor and Destructor
    DuckDBQueryInstantiator() = default;
    ~DuckDBQueryInstantiator() = default;
};

#endif // DUCKDB_QUERY_INSTANTIATOR_H
