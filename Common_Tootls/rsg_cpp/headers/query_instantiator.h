//
// Created by XXX on 3/19/24.
//

#ifndef RSG_CPP_QUERY_INSTANTIATOR_H
#define RSG_CPP_QUERY_INSTANTIATOR_H

#include "ir.h"
#include "query_sequence.h"
#include <set>
#include <string>
#include <vector>

class RSG;

using namespace std;

class QueryInstantiatorData {
public:
    unsigned long long g_id_counter = 0;

    vector<string> common_string_library_;
    set<IRTYPE> not_mutatable_types_;
    set<IRTYPE> string_types_;
    set<IRTYPE> int_types_;
    set<IRTYPE> float_types_;

    set<IRTYPE> safe_generate_type_;

    vector<string> v_sys_column_name;
    vector<string> v_sys_catalogs_name;

    /* New data library. SQLRight CockroachDB data instantiation. */
    map<string, vector<string>> m_table2columns; // Global Table name to column name mapping.
    map<string, vector<string>> m_table2index; // Global table name to index mapping.
    // We do not save the index to column mapping because it seems unnecessary.
    vector<string> v_table_names; // All saved table names from previous statements.

    // All used table and view names in one query statement. The table names are
    // typically defined in
    //    `FROM` statement.
    vector<string> v_table_names_single;
    // All used column names in one query statement. The column names can be used
    // to identify number of parameters required for the VALUES clause etc.
    vector<string> v_column_names_single;

    // All table names just created in the
    // current stmt but yet to be transmitted into v_table_names.
    vector<string> v_create_table_names_single;
    vector<string> v_create_view_names_single;
    /* Alias names are always local to one statement. */

    // All alias name local to one query statement.
    // Can be used for quick alias name random referencing.
    // Clean up after every single query statement.
    vector<string> v_table_alias_names_single;
    vector<string> v_column_alias_names_single;

    vector<string> v_window_name_single; // All window names in one SELECT statement.

    // Save the relationship between the table/column name to the alias name.
    map<string, string> m_alias2table_single;
    map<string, vector<string>> m_enforced_table2alias_single;
    map<string, vector<string>> m_alias_table2column_single;
    // The column alias is used in limited situations, such as GROUP BY columns AS
    // column_alias, or `SELECT SUM(column) AS c ...` Maybe also from `WITH`
    // clause?
    map<string, string> m_alias2column_single;

    // A mapping from the column name to the datatype class.
    // The datatype class is also responsible to handle literal mutation.
    map<string, DataAffinity*> m_column2datatype;
    map<DATAAFFINITYTYPE, vector<string>> m_datatype2column;

    // A mapping to save all literals that is used inside the
    // whole SQL sequence. It maps the data type to pre-defined
    // literal string.
    map<DATAAFFINITYTYPE, vector<string>> m_datatype2literals;

    // All used table names follow type in one query stmt.
    vector<string> v_statistics_name; // All statistic names defined in the current
    // SQL.
    vector<string> v_sequence_name; // All sequence names defined in the current
    // SQL.
    vector<string> v_constraint_name; // All constraint names defined in the current
    // SQL.
    vector<string> v_family_name; // All family names defined in the current SQL.

    // The purpose to have a vector of view names is because for DROP statement,
    // ALTER stmts etc, mixed with view names and table names are not appropriate.
    vector<string> v_view_name; // All saved view names.
    vector<string> v_foreign_table_name; // All foreign table names defined
    // inthe current SQL.
    vector<string> v_table_with_partition; // All table names that contiains TABLE
    // PARTITIONING.
    map<string, vector<string>> m_table2partition;

    vector<int> v_int_literals;
    vector<double> v_float_literals;
    vector<string> v_string_literals;

    // Helper methods:
    void reset_id_counter() { this->g_id_counter = 0; }

    void reset_data_library()
    {

        for (auto& it : this->m_column2datatype) {
            delete it.second;
        }

        this->reset_id_counter();
        this->m_table2columns.clear();
        this->m_table2partition.clear();
        this->v_table_names.clear();
        this->m_table2index.clear();
        this->m_column2datatype.clear();
        this->m_datatype2column.clear();
        this->m_datatype2literals.clear();
        this->v_statistics_name.clear();
        this->v_sequence_name.clear();
        this->v_view_name.clear();
        this->v_constraint_name.clear();
        this->v_family_name.clear();
        this->v_foreign_table_name.clear();
        this->v_table_with_partition.clear();
        this->v_int_literals.clear();
        this->v_float_literals.clear();
        this->v_string_literals.clear();

        this->reset_data_library_single_stmt();
    }

    void reset_data_library_single_stmt()
    {
        this->v_table_names_single.clear();
        this->v_column_names_single.clear();
        this->v_create_view_names_single.clear();
        this->v_create_table_names_single.clear();
        this->v_table_alias_names_single.clear();
        this->v_column_alias_names_single.clear();
        this->v_window_name_single.clear();
        this->m_alias2table_single.clear();
        this->m_enforced_table2alias_single.clear();
        this->m_alias2column_single.clear();
        this->m_alias_table2column_single.clear();

        this->g_id_counter = this->g_id_counter - (this->g_id_counter % 100) + 100; // For each statement, add 100 gap to separate.
    }

    QueryInstantiatorData* deep_copy()
    {
        auto* res = new QueryInstantiatorData();

        for (auto& it : this->m_column2datatype) {
            // Use the copy constructor.
            res->m_column2datatype[it.first] = new DataAffinity(*it.second);
        }

        res->g_id_counter = this->g_id_counter;

        res->m_table2columns = this->m_table2columns;
        res->m_table2partition = this->m_table2partition;
        res->v_table_names = this->v_table_names;
        res->m_table2index = this->m_table2index;
        res->m_datatype2column = this->m_datatype2column;
        res->m_datatype2literals = this->m_datatype2literals;
        res->v_statistics_name = this->v_statistics_name;
        res->v_sequence_name = this->v_sequence_name;
        res->v_view_name = this->v_view_name;
        res->v_constraint_name = this->v_constraint_name;
        res->v_family_name = this->v_family_name;
        res->v_foreign_table_name = this->v_foreign_table_name;
        res->v_table_with_partition = this->v_table_with_partition;
        res->v_int_literals = this->v_int_literals;
        res->v_float_literals = this->v_float_literals;
        res->v_string_literals = this->v_string_literals;

        res->v_table_names_single = this->v_table_names_single;
        res->v_column_names_single = this->v_column_names_single;
        res->v_create_view_names_single = this->v_create_view_names_single;
        res->v_create_table_names_single = this->v_create_table_names_single;
        res->v_table_alias_names_single = this->v_table_alias_names_single;
        res->v_column_alias_names_single = this->v_column_alias_names_single;
        res->m_alias2table_single = this->m_alias2table_single;
        res->m_enforced_table2alias_single = this->m_enforced_table2alias_single;
        res->m_alias2column_single = this->m_alias2column_single;
        res->m_alias_table2column_single = this->m_alias_table2column_single;

        return res;
    }

    void merge_single_stmt_context(QueryInstantiatorData* merging_context)
    {
        this->v_table_names_single = merging_context->v_table_names_single;
        this->v_column_names_single = merging_context->v_column_names_single;
        this->v_create_view_names_single = merging_context->v_create_view_names_single;
        this->v_create_table_names_single = merging_context->v_create_table_names_single;
        this->v_table_alias_names_single = merging_context->v_table_alias_names_single;
        this->v_column_alias_names_single = merging_context->v_column_alias_names_single;
        this->m_alias2table_single = merging_context->m_alias2table_single;
        this->m_enforced_table2alias_single = merging_context->m_enforced_table2alias_single;
        this->m_alias2column_single = merging_context->m_alias2column_single;
        this->m_alias_table2column_single = merging_context->m_alias_table2column_single;
    }

    // Constructor and Destructor
    QueryInstantiatorData() {};
    ~QueryInstantiatorData()
    {
        for (auto& it : this->m_column2datatype) {
            delete it.second;
        }
        this->m_column2datatype.clear();
    }
};

class QueryInstantiator {
public:
    // helper function
    virtual void reset_data_library()
    {

        if (this->query_instan_data != nullptr) {
            delete this->query_instan_data;
        }
        if (this->query_instan_data_snapshot != nullptr) {
            delete this->query_instan_data_snapshot;
        }
        this->query_instan_data = new QueryInstantiatorData();
        this->setup_init_query_instan_data();
        this->query_instan_data_snapshot = this->query_instan_data->deep_copy();
    };

    virtual void reset_data_library_single_stmt()
    {
        this->query_instan_data->reset_data_library_single_stmt();
        delete this->query_instan_data_snapshot;
        this->query_instan_data_snapshot = query_instan_data->deep_copy();
    };

    virtual void setup_init_query_instan_data()
    {
        if (query_instan_data == nullptr) {
            cerr << "Error, running query instantiator with query_instan_data being NULL. \n\n\n";
            abort();
        }
        query_instan_data->v_table_names.emplace_back("v00");
        query_instan_data->m_table2columns["v00"].emplace_back("c01");
        query_instan_data->m_table2columns["v00"].emplace_back("c02");
        query_instan_data->m_column2datatype["c01"] = new DataAffinity(DATAAFFINITYTYPE::AFFIINT);
        query_instan_data->m_column2datatype["c02"] = new DataAffinity(DATAAFFINITYTYPE::AFFISTRING);
        query_instan_data->m_datatype2column[DATAAFFINITYTYPE::AFFIINT].emplace_back("c01");
        query_instan_data->m_datatype2column[DATAAFFINITYTYPE::AFFISTRING].emplace_back("c02");
        query_instan_data->m_table2index["v00"].emplace_back("i03");

        this->custom_setup_init_query_instan_data();
    }

    // main entry
    virtual bool fill_one_stmt(QueryStmt* cur_stmt, unsigned long long gen_counter_id)
    {
        if (this->query_instan_data->g_id_counter < gen_counter_id) {
            this->query_instan_data->g_id_counter = gen_counter_id;
        }
        if (cur_stmt == nullptr || cur_stmt->stmt_ir == nullptr) {
            return false;
        }

        this->all_mutating_irs = cur_stmt->mutating_irs;

        /* All the fixing steps happens here. */
#ifdef DEBUG
        cerr << "Trying to fix stmt: " << cur_stmt->to_string() << " \n";
#endif

        if (!fill_one_stmt_helper(
                cur_stmt->stmt_ir)) { // Pass in kSpecificStatementType.
            return false;
        }
#ifdef DEBUG
        cerr << "After fixing: " << cur_stmt->to_string() << " \n\n\n";
#endif
        return true;
    }

    virtual bool fill_partial_stmt(QueryStmt* cur_stmt, unsigned long long gen_counter_id)
    {
        if (this->query_instan_data->g_id_counter < gen_counter_id) {
            this->query_instan_data->g_id_counter = gen_counter_id;
        }
        if (cur_stmt == nullptr || cur_stmt->stmt_ir == nullptr) {
            return false;
        }
        this->all_mutating_irs = cur_stmt->mutating_irs;

        this->query_instan_data->merge_single_stmt_context(cur_stmt->query_instan_data);
        return this->fill_one_stmt_helper(cur_stmt->mutating_irs.front());
    }
    virtual bool fill_one_stmt_helper(IR* stmt_ir) = 0;

    // data init
    virtual void init_data_library() = 0;

    virtual void rollback_instan_lib_changes()
    {
        delete this->query_instan_data;
        this->query_instan_data = this->query_instan_data_snapshot->deep_copy();
    };

    // some shared data across all different testing cases
    vector<string> all_saved_func_name;
    map<DATAAFFINITYTYPE, vector<string>> func_type_lib;
    map<string, vector<FunctionSignature>> func_str_to_type_map;

    // Common data libraries. Can be used globally for instantiation.
    vector<string> string_library_;
    set<unsigned long> string_library_hash_;
    vector<string> value_library_;

    // For saving key-value pairs for configurations such as `PRAGMA`.
    vector<string> v_cmds_;
    map<string, vector<string>> m_cmd_value_lib_;

    // Save the mapping from the SET variable name to
    //    the mapped Data Affinity for the variable.
    map<string, DataAffinity> set_session_lib;
    vector<string> all_saved_set_session;

    QueryInstantiatorData *query_instan_data = nullptr, *query_instan_data_snapshot = nullptr;

    virtual IR* constr_rand_func_with_affinity(DATAAFFINITYTYPE in_affi) { return nullptr; }

    string gen_table_name() { return "v" + to_string(query_instan_data->g_id_counter++); }
    string gen_column_name() { return "c" + to_string(query_instan_data->g_id_counter++); }
    string gen_index_name() { return "i" + to_string(query_instan_data->g_id_counter++); }
    string gen_window_name() { return "win" + to_string(query_instan_data->g_id_counter++); }
    string gen_table_alias_name() { return "ta" + to_string(query_instan_data->g_id_counter++); }
    string gen_column_alias_name() { return "ca" + to_string(query_instan_data->g_id_counter++); }
    string gen_statistic_name() { return "s" + to_string(query_instan_data->g_id_counter++); }
    string gen_sequence_name() { return "seq" + to_string(query_instan_data->g_id_counter++); }
    string gen_view_name() { return "view" + to_string(query_instan_data->g_id_counter++); }
    string gen_view_column_name() { return "view_c" + to_string(query_instan_data->g_id_counter++); }
    string gen_partition_name() { return "par" + to_string(query_instan_data->g_id_counter++); }
    string gen_constraint_name() { return "cons_" + to_string(query_instan_data->g_id_counter++); }
    string gen_family_name() { return "family_" + to_string(query_instan_data->g_id_counter++); }
    string gen_trigger_name() { return "trigger_" + to_string(query_instan_data->g_id_counter++); }

    QueryInstantiatorData* get_query_instan_data() { return this->query_instan_data; }
    QueryInstantiatorData* get_query_instan_data_snapshot() { return this->query_instan_data_snapshot; }

    RSG* p_rsg = nullptr;
    vector<IR*> all_mutating_irs;

    virtual void custom_setup_init_query_instan_data() { return; } // for each DBMS's fuzzer to init their own custom data.

    virtual vector<QueryStmt*> construct_custom_pre_insert_stmt() { return {}; }

    virtual ~QueryInstantiator()
    {
        if (this->query_instan_data) {
            delete this->query_instan_data;
            this->query_instan_data = nullptr;
        }
        if (this->query_instan_data_snapshot) {
            delete this->query_instan_data_snapshot;
            this->query_instan_data_snapshot = nullptr;
        }
    };
};

#endif // RSG_CPP_QUERY_INSTANTIATOR_H
