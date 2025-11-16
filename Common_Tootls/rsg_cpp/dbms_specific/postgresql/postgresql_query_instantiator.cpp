//
// Created by XXX on 10/21/24.
//

#include "postgresql_query_instantiator.h"
#include "../../headers/fuzzer_configurations.h"
#include "../../headers/rsg.h"
#include "../../headers/utils.h"
#include <algorithm>
#include <cfloat>
#include <climits>
#include <deque>
#include <fstream>
#include <list>
#include <sstream>
#include <utility>

using namespace std;

#define find_vector(x, y) (find(x.begin(), x.end(), y) != x.end())
#define remove_vector(x, y) \
    (x.erase(std::remove(x.begin(), x.end(), y), x.end()));
#define find_map(x, y) (x.count(y) > 0)
#define remove_map(x, y) (x.erase(y))

void PostgreSQLQueryInstantiator::init_data_library()
{

    // string func_file_name = FuzzerConfigurations::FUNCTION_TYPE_PATH;

    // ifstream input_file(func_file_name);

    // cout << "[*] begin init function_types library: " << func_file_name << endl;

    // string function_types_path = FuzzerConfigurations::FUNCTION_TYPE_PATH;
    // std::stringstream buffer_func_types;
    // buffer_func_types << input_file.rdbuf();
    // string func_types_str = buffer_func_types.str();

    // constr_sql_func_lib(func_types_str, all_saved_func_name, func_type_lib,
    //     func_str_to_type_map);
    // cout << "[*] Getting all_saved_func_name.size(): "
    //      << this->all_saved_func_name.size() << endl;

    // input_file.close();
    // cout << "[*] end init function_types library: " << func_file_name << endl;

    // init value_libary
    vector<string> value_lib_init = { std::to_string(0),
        std::to_string(1),
        std::to_string(2),
        std::to_string((unsigned long)LONG_MAX),
        std::to_string((unsigned long)LONG_MIN),
        std::to_string((unsigned long)ULONG_MAX),
        std::to_string((unsigned long)CHAR_BIT),
        std::to_string((unsigned long)SCHAR_MIN),
        std::to_string((unsigned long)SCHAR_MAX),
        std::to_string((unsigned long)UCHAR_MAX),
        std::to_string((unsigned long)CHAR_MIN),
        std::to_string((unsigned long)CHAR_MAX),
        std::to_string((unsigned long)MB_LEN_MAX),
        std::to_string((unsigned long)SHRT_MIN),
        std::to_string((unsigned long)INT_MIN),
        std::to_string((unsigned long)INT_MAX),
        std::to_string((unsigned long)SCHAR_MIN),
        std::to_string((unsigned long)SCHAR_MIN),
        std::to_string((unsigned long)UINT_MAX),
        std::to_string((unsigned long)FLT_MAX),
        std::to_string((unsigned long)DBL_MAX),
        std::to_string((unsigned long)LDBL_MAX),
        std::to_string((unsigned long)FLT_MIN),
        std::to_string((unsigned long)DBL_MIN),
        std::to_string((unsigned long)LDBL_MIN),
        "10",
        "100" };

    value_library_.insert(value_library_.begin(), value_lib_init.begin(),
        value_lib_init.end());

    string_library_.push_back("x");
    string_library_.push_back("xxx");
    string_library_.push_back("yyy");
    string_library_.push_back("test");
    string_library_.push_back("integrity-check");



    // ifstream input_pragma("./pragma");
    // string s;
    // cout << "start init pragma" << endl;
    // while (getline(input_pragma, s)) {
    //     if (s.empty())
    //         continue;
    //     auto pos = s.find('=');
    //     if (pos == string::npos)
    //         continue;

    //     string k = s.substr(0, pos - 1);
    //     string v = s.substr(pos + 2);
    //     if (find(v_cmds_.begin(), v_cmds_.end(), k) == v_cmds_.end()) {
    //         v_cmds_.push_back(k);
    //     }
    //     m_cmd_value_lib_[k].push_back(v);
    // }
}

void PostgreSQLQueryInstantiator::constr_sql_func_lib_helper(json& json_obj, vector<string>& v_all_func_str,
    map<DATAAFFINITYTYPE, vector<string>>& func_type_lib,
    map<string, vector<FunctionSignature>>& func_str_to_type_map)
{

    for (json::iterator it = json_obj.begin(); it != json_obj.end(); it++) {
        auto cur_set_node = it.value();
        // if (!cur_set_node.at("enabled")) {
        //     // Ignored the not enabled.
        //     continue;
        // }

        FUNCTIONTYPE func_type = get_functype_by_string(cur_set_node.at("func_catalog"));
        if (
            func_type == FUNCCRYPTO || func_type == FUNCSYSTEMINFO) {
            continue;
        }

        string func_name = string(cur_set_node.at("func_name"));

        //        cerr << "\n\n\nHandling function name: " << func_name << "\n\n\n";

        FunctionSignature single_signature;

        auto params_node = cur_set_node.at("arg_types");

        int arg_idx = 0;
        for (auto& arg_type : params_node) {
            DataAffinity cur_arg_type;
            cur_arg_type.set_data_affinity(get_dataaffi_by_string(arg_type));
            single_signature.param_data_affi.push_back(cur_arg_type);
            arg_idx++;
        }

        single_signature.return_data_affi = DataAffinity(get_dataaffi_by_string(cur_set_node.at("ret_type")));

        if (func_str_to_type_map.find(func_name) == func_str_to_type_map.end()) {
            func_str_to_type_map[func_name] = vector<FunctionSignature>();
            v_all_func_str.push_back(func_name);
        }
        func_str_to_type_map[func_name].push_back(single_signature);
    }
}

void PostgreSQLQueryInstantiator::constr_sql_func_lib(string func_types_str, vector<string>& v_all_func_str,
    map<DATAAFFINITYTYPE, vector<string>>& func_type_lib,
    map<string, vector<FunctionSignature>>& func_str_to_type_map)
{

    try {
        auto json_obj = json::parse(func_types_str);
        constr_sql_func_lib_helper(json_obj, v_all_func_str, func_type_lib, func_str_to_type_map);
    } catch (json::parse_error& ex) {
        cerr << "\n\n\nJSON PARSING ERROR!!!\n\n\n";
    }
}

void PostgreSQLQueryInstantiator::constr_key_pair_datatype_lib_helper(json& key_pair_json, vector<string>& v_all_key_str, map<string, DataAffinity>& mapped_key_pair)
{

    for (json::iterator it = key_pair_json.begin(); it != key_pair_json.end(); it++) {
        auto cur_set_node = it.value();
        if (!cur_set_node["enabled"]) {
            // Ignore the not enabled.
            continue;
        }

        DataAffinity cur_affi;
        string var_name = string(cur_set_node["var_name"]);

        auto params_node = cur_set_node["params"];

        string affi_type_str = string(params_node.at("type"));

        DATAAFFINITYTYPE affi_type = get_dataaffi_by_string(affi_type_str);

        cur_affi.set_data_affinity(affi_type);

        if (affi_type_str == "AFFIENUM") {
            // Save all the ENUM types into the Data Affinity structure.
            auto enum_values_node = params_node.at("enum_values");
            vector<string> v_enum_values_str;
            for (json::iterator enum_it = enum_values_node.begin(); enum_it != enum_values_node.end(); enum_it++) {
                v_enum_values_str.push_back(enum_it.value());
            }
            cur_affi.set_v_enum_str(v_enum_values_str);

        } else if (affi_type_str == "AFFIINT") {
            // If the integer value has range or enum, only save them.
            if (params_node.at("is_enum")) {
                cur_affi.set_is_enum(true);
                cur_affi.set_is_range(false);
                auto enum_values_node = params_node.at("enum_values");
                vector<string> v_enum_values_str;
                for (json::iterator enum_it = enum_values_node.begin(); enum_it != enum_values_node.end(); enum_it++) {
                    v_enum_values_str.push_back(to_string(enum_it.value()));
                }
                cur_affi.set_v_enum_str(v_enum_values_str);

            } else if (params_node.at("is_range")) {
                cur_affi.set_is_enum(false);
                cur_affi.set_is_range(true);

                auto min_value = params_node.at("range").at("min");
                auto max_value = params_node.at("range").at("max");

                cur_affi.set_int_range(min_value, max_value);
            } else {
                // Simple setup of the int type, no restrictions.
                cur_affi.set_is_enum(false);
                cur_affi.set_is_range(false);
            }

        } else if (affi_type_str == "AFFIONOFF") {
            // Save all the ENUM types into the Data Affinity structure.
            // For AFFIONOFF. The enum only has two string "on" and "off".

            // Pass. No need to do anything.
        } else if (affi_type_str == "AFFIONOFFAUTO") {
            // Save all the ENUM types into the Data Affinity structure.
            // For AFFIONOFFAUTO. The enum only has three string "on", "off" and "auto".

            // Pass. No need to do anything.
        } else if (affi_type_str == "AFFIBOOL") {
            // Save all the ENUM types into the Data Affinity structure.
            // For AFFIBOOL. The enum only has two string "true" and "false".

            // Pass. No need to do anything.
        }
        mapped_key_pair[var_name] = cur_affi;
        v_all_key_str.push_back(var_name);
    }

    // Finished the set session handling.
    return;
}

void PostgreSQLQueryInstantiator::constr_key_pair_datatype_lib(string key_pair_str, vector<string>& v_all_key_str, map<string, DataAffinity>& mapped_key_pair)
{
    if (key_pair_str.size() == 0 || key_pair_str[0] != '[') {
        // Return a default Data Affinity. With AFFIUNKNOWN.
        cerr << "\n\n\nInside the construct_set_session_library, not "
                "getting a correct json file. \n\n\n";
        cerr << key_pair_str << "\n\n\n";
        return;
    }

    try {
        auto json_obj = json::parse(key_pair_str);
        constr_key_pair_datatype_lib_helper(json_obj, v_all_key_str, mapped_key_pair);
    } catch (json::parse_error& ex) {
        cerr << "\n\n\nJSON PARSING ERROR!!!\n\n\n";
    }

    return;
}

bool PostgreSQLQueryInstantiator::fill_one_stmt_helper(IR* cur_stmt)
{

    /*
     * Split the statement into different subquery first, and gather the IR nodes from each
     * subquery/rootquery. Connect the IRs back after IR gathering.
     * The subquery split can be reordered, e.g., the select_stmt in CREATE TABLE AS select_stmt
     * should be instantiated before the root crate_table_stmt.
     */
    bool res = true;

#ifdef DEBUG
    cerr << "Getting full stmt: " << cur_stmt->to_string() << "\n\n\n";
#endif

    /* m_substmt_save, used for reconstruct the tree. */
    map<IR*, pair<int, IR*>> m_substmt_save;
    auto substmts = split_to_substmt(cur_stmt, m_substmt_save);

    int substmt_num = substmts.size();

    if (substmt_num > 10) {
        connect_back(m_substmt_save);
#ifdef DEBUG
        cerr << "Dependency Error: the query is too complicated to fix. Has more "
                "than 10 nested subquery. \n\n\n"; // Ad-hoc number, just based on
        // intuition.
#endif
        return false;
    }

    // double nested. First wrap different subquery level.
    // the second vector holds all the interesting IRs inside one
    // stmt/substmt.
    vector<vector<IR*>> all_interesting_ir_to_fix;

    for (auto& substmt : substmts) {
        substmt->set_parent_node(nullptr);

        vector<IR*> all_ir_of_interest;
        this->instan_preprocessing(substmt, all_ir_of_interest);

        all_interesting_ir_to_fix.push_back(all_ir_of_interest);
    }

    res &= connect_back(m_substmt_save) && res;

    res &= instan_dependency(cur_stmt, all_interesting_ir_to_fix);

    return res;
}

/*
** From the outer most parent-statements to the inner most sub-statements.
*/
vector<IR*> PostgreSQLQueryInstantiator::split_to_substmt(IR* cur_stmt,
    map<IR*, pair<int, IR*>>& m_save)
{
    /* This function is responsible to detect
     * and detach all the subqueries from the statement.
     * Additionally, it needs to decide the order of the
     * subquery instantiation.
     * For normal subquery, it can use variables defined in the
     * parent query, which means they should be fixed later than the
     * root query.
     * However, for WITH clause SELECT, CREATE VIEW and CREATE TABLE AS,
     * the subquery that defines the main semantic should be fixed earlier,
     * so that the root stmt can correctly map the dependencies to the subquery
     * tables/columns.
     */
    vector<IR*> res;

    // include the stmt root first.
    deque<IR*> bfs { cur_stmt };

    /* The root cur_stmt should always be saved. */
    bfs.push_back(cur_stmt);

    vector<IRTYPE> v_all_split_substmt_types { IRTypeSelectStmt };
    vector<IR*> v_all_split_ir = this->ir_wrapper.get_ir_node_in_stmt_with_type(cur_stmt, v_all_split_substmt_types, false, true);
    vector<IR*> v_all_sub_select_ir = this->ir_wrapper.get_ir_node_in_stmt_with_type(cur_stmt, IRTypeSelectStmt, false, true);

    for (IR* cur_split_node : v_all_split_ir) {
        IR* select_sub_node = nullptr;
        int idx = 0;
        for (IR*& cur_child : cur_split_node->get_children()) {
            if (cur_child->get_ir_type() == IRTypeSelectStmt) {
                select_sub_node = cur_child;
                break;
            }
            idx++;
        }

        if (select_sub_node != nullptr) {
            cur_split_node->detach_one_child(select_sub_node);
            // These sub-statement should be instantiated first.
            bfs.push_front(select_sub_node);
            m_save[cur_split_node] = make_pair(idx, select_sub_node);
        }
    }

    for (IR* select_sub_node : v_all_split_ir) {
        if (select_sub_node == cur_stmt) {
            continue;
        }
        if (select_sub_node->get_parent_node() == nullptr) {
            continue;
        }

        IR* parent_node = select_sub_node->get_parent_node();

        int idx = 0;
        for (IR*& tmp_child : parent_node->get_children()) {
            if (tmp_child == select_sub_node) {
                break;
            }
            idx++;
        }

        parent_node->detach_one_child(select_sub_node);
        // These sub-statement should be instantiated AFTER the parent stmt.
        bfs.push_back(select_sub_node);
        m_save[parent_node] = make_pair(idx, select_sub_node);
    }

    for (auto ptr = bfs.begin(); ptr != bfs.end(); ptr++) {
        res.push_back(*ptr);
    }

    return res;
}

bool PostgreSQLQueryInstantiator::connect_back(map<IR*, pair<int, IR*>>& m_save)
{
    for (auto& iter : m_save) {
        iter.first->add_one_child(iter.second.second, iter.second.first);
    }
    return true;
}

/* Helper functions for the instantiation. */

bool PostgreSQLQueryInstantiator::instan_preprocessing(IR* stmt_root,
    vector<IR*>& ordered_all_subquery_ir)
{
    ordered_all_subquery_ir = this->ir_wrapper.get_ir_node_in_stmt_with_type(stmt_root, FuzzerConfigurations::DataType2Instantiate, false, true);

    // Because of the partial instantiation, the stmt_root might not be the statement root.
    // So, also consider it as potential instantiation target.
    IRTYPE stmt_root_type = stmt_root->get_ir_type();
    if (find(FuzzerConfigurations::DataType2Instantiate.begin(), FuzzerConfigurations::DataType2Instantiate.end(), stmt_root_type) != FuzzerConfigurations::DataType2Instantiate.end()) {
        ordered_all_subquery_ir.push_back(stmt_root);
    }

#ifdef DEBUG
    cerr << "Gathering substmt interesting irs for instantiation: " << stmt_root->to_string();
    cerr << "\n";
    cerr << "For sub_stmt: ";
    stmt_root->debug(cerr, 0);
    cerr << "\n, gathering ir: \n";
    for (auto tmp_ir : ordered_all_subquery_ir) {
        tmp_ir->debug(cerr, 0);
        cerr << "\n";
    }
    cerr << "\n\n\n";
#endif
    return true;
}

bool PostgreSQLQueryInstantiator::remove_type_annotation(IR* cur_stmt_root,
    vector<IR*>& ir_to_deep_drop)
{
    // TODO:: FIXME::
    return true;
}

void PostgreSQLQueryInstantiator::instan_database_schema_name(IR* ir_to_fix)
{
    if (ir_to_fix->get_data_type() == DataDatabaseName) {
        ir_to_fix->set_str_val("memory");
    } else if (ir_to_fix->get_data_type() == DataSchemaName) {
        ir_to_fix->set_str_val("main");
    }
}

void PostgreSQLQueryInstantiator::instan_window_name(IR* ir_to_fix)
{
    if (ir_to_fix->get_data_type() != DataWindowName) {
        return;
    }

    if (ir_to_fix->get_data_flag() == ContextDefine) {
        string new_window_name = gen_window_name();
        ir_to_fix->set_str_val(new_window_name);
        ir_to_fix->set_is_instantiated(true);
        this->query_instan_data->v_window_name_single.push_back(new_window_name);
    }

    else if (ir_to_fix->get_data_flag() == ContextUse) {
        if (!this->query_instan_data->v_window_name_single.empty()) {
            string window_name = vector_rand_ele(this->query_instan_data->v_window_name_single);
            ir_to_fix->set_str_val(window_name);
            ir_to_fix->set_is_instantiated(true);
        } else {
            ir_to_fix->set_str_val("no_window_name");
            ir_to_fix->set_is_instantiated(true);
        }
    }
}

void PostgreSQLQueryInstantiator::instan_table_name(IR* ir_to_fix, bool& is_replace_table)
{
    // if (this->ir_wrapper.is_ir_in<IRTYPE>(ir_to_fix, IRTypeSetClause)
    // || this->ir_wrapper.is_ir_in<IRTYPE>(ir_to_fix, IRTypeStorageParameter)
    // || this->ir_wrapper.is_ir_in<IRTYPE>(ir_to_fix, IRTypeIndexFlagsParam)
    // ) {
    //     return;
    // }

    if ((ir_to_fix->get_data_type() == DataTableName) && (ir_to_fix->get_data_flag() == ContextDefine || ir_to_fix->get_data_flag() == ContextReplaceDefine)) {
        string new_name = this->gen_table_name();
        ir_to_fix->set_str_val(new_name);
        ir_to_fix->set_is_instantiated(true);

        // Save the table name that just defined inside this single statement.
        // Will permanently save this table name at the end of the function.
        this->query_instan_data->v_create_table_names_single.push_back(new_name);
#ifdef DEBUG
        cerr << "Dependency: Added to v_table_names: " << new_name
             << ", in kDataTableName with kDefine or kReplace. \n\n\n";
        cerr << "Dependency: All current statement defined name: ";
        for (string& all_defined_name : query_instan_data->v_create_table_names_single) {
            cerr << all_defined_name << " ";
        }
        cerr << "Dependency: All previously saved table names: ";
        for (string& all_used_name : query_instan_data->v_table_names) {
            cerr << "previously saved table used names: " << all_used_name
                 << "\n\n\n";
        }
#endif

        this->query_instan_data->v_table_names_single.push_back(new_name);

        if (ir_to_fix->get_data_flag() == ContextReplaceDefine) {
            // If the newly defined table is marked as ContextReplaceDefine, which
            // means the statement is related to ALTER TABLE v0 RENAME TO v1; Mark
            // the replacing table mark.
            is_replace_table = true;
        }
    }

    else if ((ir_to_fix->get_data_type() == DataTableName) && (ir_to_fix->get_data_flag() == ContextUndefine || ir_to_fix->get_data_flag() == ContextReplaceUndefine)) {
        if (this->query_instan_data->v_table_names.size() > 0) {
            // Choose random table name that defined before to drop.
            string removed_table_name = this->query_instan_data->v_table_names[get_rand_int(this->query_instan_data->v_table_names.size())];
            this->query_instan_data->v_table_names.erase(std::remove(this->query_instan_data->v_table_names.begin(),
                                                             this->query_instan_data->v_table_names.end(), removed_table_name),
                this->query_instan_data->v_table_names.end());
            // Also remove the v_table_with_partition, if matched.
            this->query_instan_data->v_table_with_partition.erase(std::remove(this->query_instan_data->v_table_with_partition.begin(),
                                                                      this->query_instan_data->v_table_with_partition.end(),
                                                                      removed_table_name),
                this->query_instan_data->v_table_with_partition.end());

            // FIXME:: Should we also remove the table name string inside the
            // v_create_table_names_single?

            ir_to_fix->set_str_val(removed_table_name);
            ir_to_fix->set_is_instantiated(true);
#ifdef DEBUG
            cerr << "Dependency: Removed from v_table_names: " << removed_table_name
                 << ", in TypeDataTableName with ContextUndefine \n\n\n";
#endif

            this->query_instan_data->v_table_names_single.push_back(removed_table_name);

            if (is_replace_table && this->query_instan_data->v_create_table_names_single.size() != 0) {
                // In most of the case, the replacement would only have one pair of
                // table names.
                string new_table_name = this->query_instan_data->v_create_table_names_single.back();
                this->query_instan_data->m_table2columns[new_table_name] = this->query_instan_data->m_table2columns[removed_table_name];
            }
        } else {
#ifdef DEBUG
            cerr << "Dependency Error: Failed to find info in v_table_names, "
                    "in DataTableName with ContextUndefine. \n\n\n";
#endif
            // Randomly delete a not existed table.
            ir_to_fix->set_str_val("x");
            ir_to_fix->set_is_instantiated(true);
        }
    }

    else if (ir_to_fix->get_data_type() == DataTableName && ir_to_fix->get_data_flag() == ContextUseTop) {
        // This is a special case for the FROM clause.
        // We need to find the closest table name in the FROM clause.
        string chosen_table_name = "v00";
        if (this->query_instan_data->v_table_names.size() != 0) {
            chosen_table_name = this->query_instan_data->v_table_names[get_rand_int(this->query_instan_data->v_table_names.size())];
        }

        // if (chosen_table_name == "v00" && this->query_instan_data->v_table_names_single.size() != 0 && get_pct_hit(50)) {
        //     chosen_table_name = this->query_instan_data->v_table_names_single[get_rand_int(this->query_instan_data->v_table_names_single.size())];
        // }

        string table_alias_name = gen_table_alias_name();
        bool is_aliased = false;
        string new_table_name = chosen_table_name;
        if (this->ir_wrapper.is_ir_in(ir_to_fix, IRTypeJoinedTable)) {
            is_aliased = true;
            new_table_name = chosen_table_name + " AS " + table_alias_name;
        }

        ir_to_fix->set_str_val(new_table_name);
        ir_to_fix->set_is_instantiated(true);

        // Save the table name used in this statement.
        this->query_instan_data->v_table_names_single.push_back(chosen_table_name);
        if (is_aliased) {
            this->query_instan_data->v_table_names_single.push_back(table_alias_name);
            this->query_instan_data->v_table_alias_names_single.push_back(table_alias_name);
            this->query_instan_data->m_alias2table_single[table_alias_name] = chosen_table_name;
            for (auto& col : this->query_instan_data->m_table2columns[chosen_table_name]) {
                this->query_instan_data->m_alias_table2column_single[table_alias_name].push_back(col);
            }
        }
    }

    else if (ir_to_fix->get_data_type() == DataTableName && ir_to_fix->get_data_flag() == ContextUse) {

        /* INFO:: CockroachDB does not have the syntax of PARTITION OF table.
         * Therefore, we don't need to consider the PARTITION OF
         * partitioned_table grammar.
         * */

        if (this->query_instan_data->v_table_names.size() == 0 && this->query_instan_data->v_table_names_single.size() == 0 && this->query_instan_data->v_create_table_names_single.size() == 0) {
#ifdef DEBUG
            cerr << "Dependency Error: Failed to find info in v_table_names "
                    "and v_create_table_names_single, in kDataTableName with "
                    "ContextUse. \n\n\n";
#endif
            ir_to_fix->set_is_instantiated(true);
            ir_to_fix->set_str_val("x");
            return;
        }
        string used_name = "";

        if (this->query_instan_data->v_table_alias_names_single.size() != 0 && get_pct_hit(50)) {
            used_name = this->query_instan_data->v_table_alias_names_single[get_rand_int(
                this->query_instan_data->v_table_alias_names_single.size())];
            // Save it to the v_table_names_single, so that the ContextUsedFollow can
            // use this name.
            this->query_instan_data->v_table_names_single.push_back(used_name);
        } else if (this->query_instan_data->v_table_names_single.size() != 0) {
            // If the statement use some table names before,
            // we can refer to the table name here.
            // We can imagine v_table_names_single could contain
            // alias name defined in WITH clause or other places.
            used_name = this->query_instan_data->v_table_names_single[get_rand_int(this->query_instan_data->v_table_names_single.size())];

            // int trial = 10;
            //             while (this->ir_wrapper.is_ir_in(ir_to_fix, IRTypeInsertCmd) && trial-- != 0 && find(this->query_instan_data->v_view_name.begin(), this->query_instan_data->v_view_name.end(), used_name) != this->query_instan_data->v_view_name.end()) {
            //                 // Getting view_name in the Insert Stmt. Retry.
            // #ifdef DEBUG
            //                 cerr << "\n\n\nRetry table name fixing in the INSERT statement. "
            //                         "\n\n\n";
            // #endif
            //                 used_name = this->query_instan_data->v_table_names_single[get_rand_int(this->query_instan_data->v_table_names_single.size())];
            //             }

        } else if (this->query_instan_data->v_create_table_names_single.size() != 0) {
            // If cannot find any table names defined or used before,
            // consider the table name that just defined in this statement.
            used_name = this->query_instan_data->v_create_table_names_single[get_rand_int(
                this->query_instan_data->v_create_table_names_single.size())];
        } else if (this->query_instan_data->v_table_names.size() != 0) {
            used_name = this->query_instan_data->v_table_names[get_rand_int(this->query_instan_data->v_table_names.size())];

            // int trial = 10;
            //             while (this->ir_wrapper.is_ir_in(ir_to_fix, IRTypeInsertStmt) && trial-- != 0 && find(this->query_instan_data->v_view_name.begin(), this->query_instan_data->v_view_name.end(), used_name) != this->query_instan_data->v_view_name.end()) {
            //                 // Getting view_name in the Insert Stmt. Retry.
            // #ifdef DEBUG
            //                 cerr << "\n\n\nRetry table name fixing in the INSERT statement. "
            //                         "\n\n\n";
            // #endif
            //                 used_name = this->query_instan_data->v_table_names[get_rand_int(this->query_instan_data->v_table_names.size())];
            //             }

        } else {
#ifdef DEBUG
            cerr << "Cannot find any used or defined table names. Use simple x "
                    "as name. \n\n\n";
#endif
            used_name = "x";
        }
        ir_to_fix->set_str_val(used_name);
        ir_to_fix->set_is_instantiated(true);
        // Save the table name used in this statement.
        // The saved table name can be referred later by
        //   contextUseFollow.
        this->query_instan_data->v_table_names_single.push_back(used_name);
#ifdef DEBUG
        cerr << "Dependency: In the context of ContextUsed table, we got "
                "table_name: "
             << used_name << ". \n\n\n";
        for (string& all_used_name : query_instan_data->v_table_names) {
            cerr << "Dependency: All saved table used names: " << all_used_name
                 << "\n\n\n";
        }
        for (string& all_used_name : query_instan_data->v_create_table_names_single) {
            cerr << "Dependency: All saved table used names: " << all_used_name
                 << "\n\n\n";
        }
#endif
    }

    // May not be used.
    else if (ir_to_fix->get_data_type() == DataTableName && ir_to_fix->get_data_flag() == ContextUseFollow) {

        if (this->query_instan_data->v_table_alias_names_single.size() == 0 && this->query_instan_data->v_table_names.size() == 0 && this->query_instan_data->v_table_names_single.size() == 0 && this->query_instan_data->v_create_table_names_single.size() == 0) {
#ifdef DEBUG
            cerr << "Dependency Error: Failed to find info in v_table_names "
                    "and v_create_table_names_single, in kDataTableName with "
                    "ContextUse. \n\n\n";
#endif
            ir_to_fix->set_is_instantiated(true);
            ir_to_fix->set_str_val("x");
            return;
        }
        string used_name = "";

#ifdef DEBUG
        cerr << "\n\n\nDEBUG: In Table ContextUseFollow: getting "
                "v_table_alias_names_single.size(): "
             << query_instan_data->v_table_alias_names_single.size()
             << ", v_table_names_single: " << query_instan_data->v_table_names_single.size()
             << ", v_create_table_names_single"
             << query_instan_data->v_create_table_names_single.size() << "\n\n\n";
#endif
        // For the ContextUseFollow, we should use table name that already
        // mentioned in the current statement.
        // For example, for `v0.v1`, where v0 is imported from `FROM v0;`
        // Therefore, we should not directly use the Table Alias name.
        // If the table alias is defined in the FROM clause,
        // then the alias name should also be in the v_table_names_single.
        if (this->query_instan_data->v_table_names_single.size() != 0) {
            used_name = this->query_instan_data->v_table_names_single[get_rand_int(this->query_instan_data->v_table_names_single.size())];
        } else if (this->query_instan_data->v_create_table_names_single.size() != 0) {
            // If cannot find any table names defined or used before,
            // consider the table name that defined from previous statements.
            // Not sure whether this situation is possible or not.
#ifdef DEBUG
            cerr << "\n\n\nIn the scenario of table name ContextUseFollow, "
                    "cannot find table name inside "
                    "v_table_names_single. Use previous defined table names "
                    "instead. \n\n\n";
#endif
            used_name = this->query_instan_data->v_create_table_names_single[get_rand_int(
                this->query_instan_data->v_create_table_names_single.size())];
        } else if (this->query_instan_data->v_table_names.size() != 0) {
            // If the statement use some table names before,
            // we can refer to the table name here.
            // We can imagine v_table_names_single could contain
            // alias name defined in WITH clause or other places.
#ifdef DEBUG
            cerr << "\n\n\nIn the scenario of table name ContextUseFollow, "
                    "cannot find table name inside "
                    "v_table_names_single. Use previous defined table names "
                    "instead. \n\n\n";
#endif
            used_name = this->query_instan_data->v_table_names[get_rand_int(this->query_instan_data->v_table_names.size())];
        } else {
#ifdef DEBUG
            cerr << "Cannot find any used or defined table names. Use simple x "
                    "as name. \n\n\n";
#endif
            used_name = "x";
        }

        // Check whether the chosen alias name is inside the enforced table alias
        // mapping.
        if (this->query_instan_data->m_enforced_table2alias_single.count(used_name) != 0 && this->query_instan_data->m_enforced_table2alias_single[used_name].size() != 0) {
#ifdef DEBUG
            cerr << "\n\n\nDependency: Inside the table name use follow "
                    "instantiation, forced map the table name "
                 << used_name << " to ";
#endif
            used_name = vector_rand_ele(this->query_instan_data->m_enforced_table2alias_single[used_name]);
#ifdef DEBUG
            cerr << used_name << "\n\n\n";
#endif
        }

        ir_to_fix->set_str_val(used_name);
        ir_to_fix->set_is_instantiated(true);

#ifdef DEBUG
        cerr << "Dependency: In the context of ContextUsed table, we got "
                "table_name: "
             << used_name << ". \n\n\n";
        for (string& all_used_name : query_instan_data->v_table_names) {
            cerr << "Dependency: All saved table used names: " << all_used_name
                 << "\n\n\n";
        }
        for (string& all_used_name : query_instan_data->v_create_table_names_single) {
            cerr << "Dependency: All saved table used names: " << all_used_name
                 << "\n\n\n";
        }
#endif
    }
    // end of table_name instantiation function.
}

void PostgreSQLQueryInstantiator::instan_table_alias_name(IR* ir_to_fix, IR* cur_stmt_root,
    bool is_alias_optional)
{

    /* There is no need to consider the Context in this loop.
     * Because TableAliasName almost always occur on ContextDefine.
     * The Alias name will be saved into the
     */

    // if (this->ir_wrapper.is_ir_in(ir_to_fix, IRTypeIndexFlagsParam)) {
    //     // Do not instantiate the table alias in the Index Flags Param.
    //     return;
    // }

    if (ir_to_fix->get_data_type() == DataTableAliasName) {

        ir_to_fix->set_is_instantiated(true);

        string closest_table_name = "";

        IR* closest_table_ir = this->ir_wrapper.find_closest_nearby_IR_with_type<DATATYPE>(
            ir_to_fix, DataTableName);

        if (closest_table_ir != NULL) {
            closest_table_name = closest_table_ir->get_str_val();
        } else if (this->query_instan_data->v_table_names_single.size() != 0) {
#ifdef DEBUG
            cerr << "\n\n\nError: Dependency: When handling the "
                    "DataTableAliasName, "
                    "cannot find the table name nearby the ir_to_fix(). \n\n\n";
            cerr << "\n\n\n More debugging information: cur node: "
                 << ir_to_fix->to_string()
                 << "; whole statement: " << cur_stmt_root->to_string() << "\n\n\n";
#endif
            closest_table_name = this->query_instan_data->v_table_names_single[get_rand_int(this->query_instan_data->v_table_names_single.size())];
        } else if (this->query_instan_data->v_create_table_names_single.size() != 0) {
#ifdef DEBUG
            cerr << "\n\n\nError: Dependency: When handling the "
                    "DataTableAliasName, "
                    "cannot find the table name nearby the ir_to_fix(). \n\n\n";
            cerr << "\n\n\n More debugging information: cur node: "
                 << ir_to_fix->to_string()
                 << "; whole statement: " << cur_stmt_root->to_string() << "\n\n\n";
#endif
            closest_table_name = this->query_instan_data->v_create_table_names_single[0];
#ifdef DEBUG
            cerr << "Dependency: In kAlias defined, find newly declared table "
                    "name: "
                 << closest_table_name << ". \n\n\n"
                 << endl;
#endif
        } else if (this->query_instan_data->v_table_names.size() != 0) {
#ifdef DEBUG
            cerr << "Error: Dependency: When handling the DataTableAliasName, "
                    "cannot find the table name nearby the ir_to_fix(). ";
            cerr << "\n More debugging information: cur node: "
                 << ir_to_fix->to_string()
                 << "; whole statement: " << cur_stmt_root->to_string();
#endif
            closest_table_name = this->query_instan_data->v_table_names[get_rand_int(this->query_instan_data->v_table_names.size())];
#ifdef DEBUG
            cerr << "Dependency Error: In defined of kDataAliasName, cannot "
                    "find v_table_names_single. Thus find from v_table_name "
                    "instead. Use table name: "
                 << closest_table_name << ". \n\n\n"
                 << endl;
#endif
        } else {
#ifdef DEBUG
            cerr << "Error: Dependency: When handling the DataTableAliasName, "
                    "cannot find the any way to refer to a table name nearby "
                    "the ir_to_fix(). ";
            cerr << "\n More debugging information: cur node: "
                 << ir_to_fix->to_string()
                 << "; whole statement: " << cur_stmt_root->to_string();
#endif
            ir_to_fix->set_str_val("x");
            // Break the current ir instantiation handling.
            return;
        }

#ifdef DEBUG
        cerr << "Dependency: In DataTableAliasName ContextDefined, find "
                "table name: "
             << closest_table_name << ". \n\n\n"
             << endl;
#endif

        if (closest_table_name == "" || closest_table_name == "x" || closest_table_name == "y") {
#ifdef DEBUG
            cerr << "Dependency Error: Cannot find the closest_table_name from "
                    "the query. Error cloest_table_name is: "
                 << closest_table_name << ". In kAliasName Define. \n\n\n";
#endif
            /* Randomly set an alias name to the defined table.
             * And ignore the mapping for the moment
             * */
            string alias_name = gen_table_alias_name();
            ir_to_fix->set_str_val(alias_name);
            this->query_instan_data->v_table_alias_names_single.push_back(alias_name);
            return;
        }

        /* Found the table name that matched to the alias, now generate the
         * alias and save it.  */
        string alias_name = gen_table_alias_name();
        ir_to_fix->set_str_val(alias_name);
        this->query_instan_data->m_alias2table_single[alias_name] = closest_table_name;
        if (!is_alias_optional) {
            this->query_instan_data->m_enforced_table2alias_single[closest_table_name].push_back(alias_name);
        }
        this->query_instan_data->m_alias_table2column_single[alias_name] = this->query_instan_data->m_table2columns[closest_table_name];
        this->query_instan_data->v_table_alias_names_single.push_back(alias_name);
        if (this->ir_wrapper.is_ir_in(ir_to_fix, IRTypeFromClause)) {
#ifdef DEBUG
            cerr << "\n\n\n The table alias: " << alias_name
                 << " is defined "
                    "inside the FROM clause, so we can safely move the alias into "
                    "the "
                    "v_table_name_single. \n\n\n";
#endif
            this->query_instan_data->v_table_names_single.push_back(alias_name);
        }

#ifdef DEBUG
        cerr << "Dependency: In TypeTableAliasName defined, generates: "
             << alias_name << " mapping to table name: " << closest_table_name
             << ". \n\n\n"
             << endl;
#endif
    }
}

void PostgreSQLQueryInstantiator::instan_view_name(IR* ir_to_fix)
{

    // if (this->ir_wrapper.is_ir_in(ir_to_fix, IRTypeSetClause)
    // || this->ir_wrapper.is_ir_in(ir_to_fix, IRTypeStorageParameter)
    // || this->ir_wrapper.is_ir_in(ir_to_fix, IRTypeIndexFlagsParam)
    // ) {
    //     return;
    // }

    /* Context Define. */
    if (ir_to_fix->get_data_type() == DataViewName && ir_to_fix->get_data_flag() == ContextDefine) {

        string new_view_name_str = gen_view_name();
        ir_to_fix->set_str_val(new_view_name_str);
        ir_to_fix->set_is_instantiated(true);

        this->query_instan_data->v_create_view_names_single.push_back(new_view_name_str);

#ifdef DEBUG
        cerr << "Dependency: In kDefine of kDataViewName, generating view "
                "name: "
             << new_view_name_str << "\n\n\n";
#endif
    }

    /* Context Undefine */
    if (ir_to_fix->get_data_type() == DataViewName && ir_to_fix->get_data_flag() == ContextUndefine) {
        if (this->query_instan_data->v_view_name.size() == 0) {
#ifdef DEBUG
            cerr << "Dependency Error: In kUndefine of kDataViewname, cannot "
                    "find view name defined before. \n\n\n";
#endif
            ir_to_fix->set_is_instantiated(true);
            return;
        }
        string view_to_rov_str = vector_rand_ele(this->query_instan_data->v_view_name);
        ir_to_fix->set_str_val(view_to_rov_str);
        ir_to_fix->set_is_instantiated(true);

        auto tmp_end = std::remove(this->query_instan_data->v_view_name.begin(), this->query_instan_data->v_view_name.end(), view_to_rov_str);
        tmp_end = std::remove(this->query_instan_data->v_create_view_names_single.begin(), this->query_instan_data->v_create_view_names_single.end(),
            view_to_rov_str);
        tmp_end = std::remove(this->query_instan_data->v_table_names.begin(), this->query_instan_data->v_table_names.end(), view_to_rov_str);

#ifdef DEBUG
        cerr << "Dependency: In ContextUndefine of kDataViewName, removing "
                "view "
                "name: "
             << view_to_rov_str << "\n\n\n";
#endif
    }

    /* kUse of kDataViewName */
    else if (ir_to_fix->get_data_type() == DataViewName && ir_to_fix->get_data_flag() == ContextUse) {
        if (!this->query_instan_data->v_view_name.size()) {
#ifdef DEBUG
            cerr << "Dependency Error: In ContextUndefine of kDataViewname, "
                    "cannot "
                    "find view name defined before. \n\n\n";
#endif
            return;
        }
        string view_str = vector_rand_ele(this->query_instan_data->v_view_name);
        ir_to_fix->set_str_val(view_str);
        ir_to_fix->set_is_instantiated(true);
        this->query_instan_data->v_table_names_single.push_back(view_str);

#ifdef DEBUG
        cerr << "Dependency: In kUse of kDataViewName, using view name: "
             << view_str << "\n\n\n";
#endif
    }
}

void PostgreSQLQueryInstantiator::instan_partition_name(IR* ir_to_fix)
{

    /* Context Define, Context Use and ContextUndefine of partition name. */
    if (ir_to_fix->get_data_type() == DataPartitionName && ir_to_fix->get_data_flag() == ContextDefine) {

        string new_partition_name_str = gen_partition_name();
        ir_to_fix->set_str_val(new_partition_name_str);
        ir_to_fix->set_is_instantiated(true);

        /* Get the table name that is mentioned by this statement. */
        string cur_table_name = "";
        if (this->query_instan_data->v_create_table_names_single.size() != 0) {
            cur_table_name = this->query_instan_data->v_create_table_names_single.back();
        } else if (this->query_instan_data->v_table_names_single.size() != 0) {
            cur_table_name = vector_rand_ele(this->query_instan_data->v_table_names_single);
        } else if (this->query_instan_data->v_table_with_partition.size() != 0) {
#ifdef DEBUG
            cerr << "Error: When trying to fetch data partition name in: "
                    "partition name define. Cannot find table name defined "
                    "in the statement. Use previous v_table_with_partition "
                    "instead. ";
#endif
            cur_table_name = vector_rand_ele(this->query_instan_data->v_table_with_partition);
        } else if (this->query_instan_data->v_table_names.size() != 0) {
#ifdef DEBUG
            cerr << "Error: When trying to fetch data partition name in: "
                    "partition name define. Cannot find table name defined "
                    "in the statement. Use previous v_table_with_partition "
                    "instead. ";
#endif
            cur_table_name = vector_rand_ele(this->query_instan_data->v_table_names);
        } else {
#ifdef DEBUG
            cerr << "Error: When trying to fetch data partition name in: "
                    "partition name define. Cannot find table name defined "
                    "in the statement. Cannot find anything matched. Not able "
                    " to connect to any table names. ";
#endif
            return;
        }

        this->query_instan_data->v_table_with_partition.push_back(cur_table_name);
        this->query_instan_data->m_table2partition[cur_table_name].push_back(cur_table_name);

#ifdef DEBUG
        cerr << "Dependency: In ContextDefine of DataPartitionName, "
                "generating data partition "
                "name: "
             << new_partition_name_str
             << ", attached to table name: " << cur_table_name << " \n\n\n";
#endif
    }

    if (ir_to_fix->get_data_type() == DataPartitionName && ir_to_fix->get_data_flag() == ContextUse) {

        ir_to_fix->set_is_instantiated(true);

        string cur_table_name = "";
        if (this->query_instan_data->v_table_with_partition.size() > 0) {
            cur_table_name = vector_rand_ele(cur_table_name);
        } else {
#ifdef DEBUG
            cerr << "Error: Inside Context Use of DataPartitionName, cannot "
                    "find pre-defined table that contains partitions. "
                    "Therefore, use dummy x for the partition name. \n";
#endif
            ir_to_fix->set_str_val("x");
            // Skip the subsequent handling. Use the dummy `x`.
            return;
        }

        const vector<string>& all_partitions = this->query_instan_data->m_table2partition[cur_table_name];
        if (all_partitions.size() == 0) {
#ifdef DEBUG
            cerr << "Error: Inside Context Use of DataPartitionName, cannot "
                    "find m_table2partition partitions. Table name is: "
                 << cur_table_name
                 << "Therefore, use dummy x for the partition name. \n";
#endif
            ir_to_fix->set_str_val("x");
            // Skip the subsequent handling. Use the dummy `x`.
            return;
        }

        string used_partition_name = vector_rand_ele(all_partitions);
        ir_to_fix->set_str_val(used_partition_name);

#ifdef DEBUG
        cerr << "Dependency: In kDefine of kDataViewName, using partition "
                "name: "
             << used_partition_name << ", matching from table: " << cur_table_name
             << "\n\n\n";
#endif
        // Succeed. Continue to the next IR.
    }

    else if (ir_to_fix->get_data_type() == DataPartitionName && ir_to_fix->get_data_flag() == ContextUndefine) {
        ir_to_fix->set_is_instantiated(true);

        string cur_table_name = "";
        if (this->query_instan_data->v_table_with_partition.size() > 0) {
            cur_table_name = vector_rand_ele(cur_table_name);
        } else {
#ifdef DEBUG
            cerr << "Error: Inside Context Use of DataPartitionName, cannot "
                    "find pre-defined table that contains partitions. "
                    "Therefore, use dummy x for the partition name. \n";
#endif
            ir_to_fix->set_str_val("x");
            // Skip the subsequent handling. Use the dummy `x`.
            return;
        }

        vector<string>& all_partitions = this->query_instan_data->m_table2partition[cur_table_name];
        if (all_partitions.size() == 0) {
#ifdef DEBUG
            cerr << "Error: Inside Context Use of DataPartitionName, cannot "
                    "find m_table2partition partitions. Table name is: "
                 << cur_table_name
                 << "Therefore, use dummy x for the partition name. \n";
#endif
            ir_to_fix->set_str_val("x");
            // Skip the subsequent handling. Use the dummy `x`.
            return;
        }

        string used_partition_name = vector_rand_ele(all_partitions);
        ir_to_fix->set_str_val(used_partition_name);

        all_partitions.erase(std::remove(all_partitions.begin(),
                                 all_partitions.end(), used_partition_name),
            all_partitions.end());
        if (all_partitions.size() == 0) {
            this->query_instan_data->v_table_with_partition.erase(std::remove(this->query_instan_data->v_table_with_partition.begin(),
                                                                      this->query_instan_data->v_table_with_partition.end(),
                                                                      cur_table_name),
                this->query_instan_data->v_table_with_partition.end());
        }

#ifdef DEBUG
        cerr << "Dependency: In ContextUndefine of kDataPartitionName, "
                "removed partition name: "
             << used_partition_name << ", matching from table: " << cur_table_name
             << "\n\n\n";
#endif
        // Succeed. Continue to the next IR.
    }
}

void PostgreSQLQueryInstantiator::instan_index_name(IR* ir_to_fix, vector<IR*>& ir_to_drop)
{

    if (ir_to_fix->get_data_type() == DataIndexName) {
#ifdef DEBUG
        cerr << "\n\n\nDEBUG: Inside the instan_index_name function \n\n\n";
#endif
        if (ir_to_fix->get_data_flag() == ContextDefine) {
            string tmp_index_name = gen_index_name();
            ir_to_fix->set_str_val(tmp_index_name);
            ir_to_fix->set_is_instantiated(true);

            /* Find the table used in this stmt. */
            if (this->query_instan_data->v_table_names_single.size() != 0) {
                string tmp_table_name = this->query_instan_data->v_table_names_single[0];
                this->query_instan_data->m_table2index[tmp_table_name].push_back(tmp_index_name);
            }
        } else if (ir_to_fix->get_data_flag() == ContextUndefine) {

            string tmp_index_name = "y";

            /* Find the table used in this stmt. */
            if (this->query_instan_data->v_table_names_single.size() != 0) {
                string tmp_table_name = this->query_instan_data->v_table_names_single[0];
                vector<string>& v_index_name = this->query_instan_data->m_table2index[tmp_table_name];
                if (!v_index_name.size())
                    return;
                tmp_index_name = vector_rand_ele(v_index_name);

                vector<string> tmp_v_index_name;
                for (string s : v_index_name) {
                    if (s != tmp_index_name) {
                        tmp_v_index_name.push_back(s);
                    }
                }
                v_index_name = tmp_v_index_name;
            } else {
                for (auto it = this->query_instan_data->m_table2index.begin(); it != this->query_instan_data->m_table2index.end(); it++) {
                    vector<string>& v_index_name = it->second;
                    if (!v_index_name.size())
                        continue;
                    tmp_index_name = vector_rand_ele(v_index_name);

                    vector<string> tmp_v_index_name;
                    for (string s : v_index_name) {
                        if (s != tmp_index_name) {
                            tmp_v_index_name.push_back(s);
                        }
                    }
                    v_index_name = tmp_v_index_name;
                }
            }
            if (tmp_index_name != "y") {
                ir_to_fix->set_str_val(tmp_index_name);
                ir_to_fix->set_is_instantiated(true);
            }
        }

        else if (ir_to_fix->get_data_flag() == ContextUse) {

            string tmp_index_name = "y";

            /* Find the table used in this stmt. */
            if (this->query_instan_data->v_table_names_single.size() != 0) {
                string tmp_table_name = this->query_instan_data->v_table_names_single[0];
                vector<string>& v_index_name = this->query_instan_data->m_table2index[tmp_table_name];
                if (!v_index_name.size()) {
                    tmp_index_name = "y";
                } else {
                    tmp_index_name = vector_rand_ele(v_index_name);
                }
            } else {
                for (auto it = this->query_instan_data->m_table2index.begin(); it != this->query_instan_data->m_table2index.end(); it++) {
                    vector<string>& v_index_name = it->second;
                    if (!v_index_name.size())
                        continue;
                    tmp_index_name = vector_rand_ele(v_index_name);
                }
            }
            if (tmp_index_name != "y") {
                ir_to_fix->set_str_val(tmp_index_name);
                ir_to_fix->set_is_instantiated(true);
                // } else {
                // if (ir_to_fix->get_parent_node() != nullptr && ir_to_fix->get_parent_node()->get_ir_type() == IRTypeIndexFlagsParam) {
                //     ir_to_fix->set_is_instantiated(true); // do not duplicate instan.
                //     IR* parent_node = ir_to_fix->get_parent_node();
                //     for (IR*& cur_child_ir : parent_node->get_children()) {
                //         ir_to_drop.push_back(cur_child_ir);
                //     }
                //     parent_node->detach_children();
                //     IR* tmp_ir = new IR(SymbolTerm, IRTypeASC, string("ASC"), nullptr);
                //     vector<IR*> tmp_children { tmp_ir };
                //     parent_node->set_children_nodes(tmp_children);
                // };
            }
        }
    }
}

DATAAFFINITYTYPE PostgreSQLQueryInstantiator::get_nearby_data_affinity(IR* ir_to_fix)
{

    // First, search if we can find a nearby literal that already has the
    // affinity fixed.

    DATAAFFINITYTYPE ret_data_affi;

    vector<DATATYPE> v_matched_literal_types { DataLiteral };
    vector<IRTYPE> v_capped_ir_types { IRTypeSelectStmt };
    IR* near_literal_node = this->ir_wrapper
                                .find_closest_nearby_IR_with_type<vector<DATATYPE>, vector<IRTYPE>>(
                                    ir_to_fix, v_matched_literal_types, v_capped_ir_types);

    if (near_literal_node != NULL && near_literal_node->get_data_affinity_type() != AFFIUNKNOWN && near_literal_node->get_data_affinity_type() != AFFIANY && near_literal_node->get_is_instantiated()) {
        //        ir_to_fix->set_data_affinity(near_literal_node->get_data_affinity());
        ret_data_affi = near_literal_node->get_data_affinity_type();
#ifdef DEBUG
        cerr << "\n\n\nDependency: INFO: From Literal handling, getting "
                "nearby literal: "
             << near_literal_node->to_string()
             << ", the literal comes with affinity: "
             << get_string_by_data_affi(
                    near_literal_node->get_data_affinity_type())
             << "\n\n\n";
#endif
    } else {
        // If we end up in this branch, we cannot find a nearby literal or column
        // names that already has fixed affinity. This is expected, such as case:
        // `SELECT
        // * FROM v0 WHERE v1 = 100;` Then, we should look at the nearby
        // column name for more information.

        IR* nearby_column_ir = this->ir_wrapper.find_closest_nearby_IR_with_type(ir_to_fix,
            DataColumnName);
        if (nearby_column_ir != NULL) {
            string nearby_column_str = nearby_column_ir->get_str_val();
            string actual_column_str = nearby_column_str;
            if (this->query_instan_data->m_column2datatype.count(nearby_column_str) || this->query_instan_data->m_alias2column_single.count(nearby_column_str)) {
                if (this->query_instan_data->m_alias2column_single.count(nearby_column_str)) {
                    actual_column_str = this->query_instan_data->m_alias2column_single[nearby_column_str];
#ifdef DEBUG
                    cerr << "\n\n\nDependency: INFO: In literal fixing, mapping the "
                            "column alias: "
                         << nearby_column_str
                         << " to column name: " << actual_column_str << "\n\n\n";
#endif
                }
                if (this->query_instan_data->m_column2datatype.count(actual_column_str)) {
                    ret_data_affi = this->query_instan_data->m_column2datatype[actual_column_str]->get_data_affinity();
                } else {
                    ret_data_affi = AFFISTRING;
                }
#ifdef DEBUG
                cerr << "Dependency: INFO: From Literal handling, getting "
                        "column name: "
                     << nearby_column_str << ", the column comes with affinity: "
                     << get_string_by_data_affi(ret_data_affi)
                     << "\n\n\n";
#endif
            } else {
                //                ir_to_fix->set_data_affinity(AFFISTRING);
                ret_data_affi = AFFISTRING;
#ifdef DEBUG
                cerr << "Dependency: INFO: From Literal handling, getting "
                        "column name: "
                     << nearby_column_str
                     << ". However, the colum name does not come with affinity: "
                        ", dummy fix the literal to AFFISTRING now."
                     << "\n\n\n";
#endif
            }
        } else {
            // Cannot find nearby COLUMN NAME?
#ifdef DEBUG
            cerr << "\n\n\n Error: For fixing literal, cannot find nearby "
                    "column name definition. "
                    "Use dummy AFFISTRING instead for now. "
                 << "\n\n\n";
#endif
            //            ir_to_fix->set_data_affinity(AFFISTRING);
            ret_data_affi = AFFISTRING;
        }
    }

    return ret_data_affi;
}

string PostgreSQLQueryInstantiator::find_cloest_table_name(IR* ir_to_fix)
{
    string closest_table_name = "";
    IR* closest_table_ir = NULL;
    vector<DATATYPE> search_type { DataTableName, DataTableAliasName };
    vector<IRTYPE> cap_type { IRTypeSelectStmt };
    closest_table_ir = this->ir_wrapper
                           .find_closest_nearby_IR_with_type<vector<DATATYPE>, vector<IRTYPE>>(
                               ir_to_fix, search_type, cap_type);
    if (closest_table_ir != NULL) {
        closest_table_name = closest_table_ir->get_str_val();
#ifdef DEBUG
        cerr << "Dependency: In ContextUse of kDataColumnName, find table name: "
             << closest_table_name << " for column name. \n\n\n"
             << endl;
#endif
    } else if (this->query_instan_data->v_table_names_single.size() != 0) {
        closest_table_name = this->query_instan_data->v_table_names_single[get_rand_int(this->query_instan_data->v_table_names_single.size())];
#ifdef DEBUG
        cerr << "Dependency: In ContextUse of kDataColumnName, find table name: "
             << closest_table_name << " for column name origin. \n\n\n"
             << endl;
#endif
    } else if (this->query_instan_data->v_create_table_names_single.size() != 0) {
        closest_table_name = this->query_instan_data->v_create_table_names_single[0];
#ifdef DEBUG
        cerr << "Dependency: In kUse of kDataColumnName, find newly "
                "declared table name: "
             << closest_table_name << " for column name origin. \n\n\n"
             << endl;
#endif
    } else if (this->query_instan_data->v_table_alias_names_single.size() != 0) {
        ir_to_fix->set_str_val(this->query_instan_data->v_table_alias_names_single[get_rand_int(
            this->query_instan_data->v_table_alias_names_single.size())]);
#ifdef DEBUG
        cerr << "Dependency: In kUse of kDataColumnName, use alias name as "
                "the column name. Use alias name: "
             << ir_to_fix->get_str_val() << " for column name. \n\n\n"
             << endl;
#endif
        // Finished assigning column name. continue;
        ir_to_fix->set_is_instantiated(true);
        return "";

    } else {
#ifdef DEBUG
        cerr << "Dependency Error:  In kUse of kDataColumnName, every table names"
                "are empty. Return empty. \n\n\n"
             << endl;
#endif
    }

    return closest_table_name;
}

void PostgreSQLQueryInstantiator::instan_literal(IR* ir_to_fix, IR* cur_stmt_root,
    vector<IR*>& ir_to_deep_drop)
{

    // if (this->ir_wrapper.is_ir_in(ir_to_fix, IRTypeSetClause)
    // || this->ir_wrapper.is_ir_in(ir_to_fix, IRTypeStorageParameter)
    // || this->ir_wrapper.is_ir_in(ir_to_fix, IRTypeIndexFlagsParam)
    // ) {
    //     /*
    //      * Should not change any literals inside the TypeOptStorageParams and
    //      * TypeSetVar clause. These literals are for Storage Parameters (Storage
    //      * Settings) or SET parameters. These values will be fixed by another
    //      * fixing function, later in the second ir_to_fix loop.
    //      * */
    //     return;
    // }

    /* First Loop, handles IN expression and Values clause.  */
    IRTYPE type = ir_to_fix->get_ir_type();

    if (type == IRTypeSTRING) {
        if (get_pct_hit(50)) {
            string random_string = vector_rand_ele(string_library_);
            ir_to_fix->set_str_val(string("'") + random_string + string("'"));
            ir_to_fix->set_data_affinity(AFFISTRING);
            ir_to_fix->set_is_instantiated(true);
            return;
        }
    } else if (type == IRTypeINTEGER) {
        if (get_pct_hit(50)) {
            string random_string = vector_rand_ele(value_library_); // no need for bracket.
            ir_to_fix->set_str_val(random_string);
            ir_to_fix->set_data_affinity(AFFIINT);
            ir_to_fix->set_is_instantiated(true);
            return;
        }
    }

    if (ir_to_fix->get_data_type() == DataLiteral || type == IRTypeINTEGER || type == IRTypeFLOAT || type == IRTypeBOOLEAN || type == IRTypeSTRING) {
        /* Continue from the previous loop, we now search around the ir_to_fix
         * and see if we can find column name or literals that can help deduce
         * Data Affinity.
         * */

        ir_to_fix->set_is_instantiated(true);

#ifdef DEBUG
        cerr << "\n\n\nTrying to fix literal: " << ir_to_fix->to_string()
             << "\n whole stmt: " << cur_stmt_root->to_string() << "\n\n\n";
#endif

        // Do not change the Data Affinity type for IS / IS NOT `TRUE/FALSE`.
        if (ir_to_fix->get_ir_type() == IRTypeBOOLEAN) {
#ifdef DEBUG
            cerr << "\n\n\nDebug: Instantiate Boolean in IS or IS NOT statement. "
                    "\n\n\n";
#endif
            if (get_pct_hit(50)) {
                ir_to_fix->set_str_val("TRUE");
            } else {
                ir_to_fix->set_str_val("FALSE");
            }
            return;
        }

        // If the literal already has fixed data affinity type, skip the
        // mutation.
        if (ir_to_fix->get_data_flag() == ContextNoModi) {
#ifdef DEBUG
            cerr << "\n\n\nSkip fixing literal: " << ir_to_fix->to_string()
                 << " because it has "
                    "flag ContextNoModi. \n\n\n";
#endif
            return;
        }

        ir_to_fix->set_data_affinity(
            this->get_nearby_data_affinity(ir_to_fix));

        /* After knowing the data affinity of the literal,
         * we have three choices to instantiate the value.
         * 1. If the statement contains one column that matches the
         * data type, use the column with probability.
         * 2. If the current data affinity is the same as previous
         * fixed literals, reuse the value.
         * 3. Mutate to get a new value.
         * */
        if (this->query_instan_data->m_datatype2column.count(ir_to_fix->get_data_affinity_type()) && get_pct_hit(10) // 1/10 chance.
        ) {
            if (ir_to_fix->get_data_affinity_type() != AFFIUNKNOWN) {
                // This is a special context that the Data Affinity type of the
                // column name node has been pre-defined.
                // This is used in query dynamic fixing, where the replaced query nodes
                // are saved in a whole, and the column node data affinity are
                // preserved.
                string cur_chosen_col = vector_rand_ele(this->query_instan_data->m_datatype2column[ir_to_fix->get_data_affinity_type()]);

                bool is_col_imported = false;
                for (string cur_used_table : this->query_instan_data->v_table_names_single) {
                    vector<string> v_imported_col = this->query_instan_data->m_table2columns[cur_used_table];
                    if (find_vector(v_imported_col, cur_chosen_col)) {
                        is_col_imported = true;
                        break;
                    }
                }

                // Fix as column name.
                if (is_col_imported) {
                    ir_to_fix->set_is_instantiated(true);
                    ir_to_fix->set_str_val(cur_chosen_col);
                    return;
                }
            }
        }

        if (this->query_instan_data->m_datatype2literals[ir_to_fix->get_data_affinity_type()].size() != 0 && get_pct_hit(50)) {
            // Reuse previous defined literals.
            string tmp_new_literal = vector_rand_ele(this->query_instan_data->m_datatype2literals[ir_to_fix->get_data_affinity_type()]);
            ir_to_fix->set_str_val(tmp_new_literal);
#ifdef DEBUG
            cerr << "\n\n\nDependency: In Fixing literals, getting new literal: "
                 << ir_to_fix->to_string() << "\n\n\n";
#endif
        } else {
            // Now we ensure the ir_to_fix has an affinity.
            // Mutate the literal with the affinity
            ir_to_fix->mutate_literal(); // Handles everything.
            this->query_instan_data->m_datatype2literals[ir_to_fix->get_data_affinity_type()].push_back(
                ir_to_fix->get_str_val());
#ifdef DEBUG
            cerr << "\n\n\nDependency: In Fixing literals, getting new literal: "
                 << ir_to_fix->to_string()
                 << "\n whole stmt: " << cur_stmt_root->to_string() << "\n\n\n";
#endif
        }
    }
}

void PostgreSQLQueryInstantiator::instan_column_name(IR* ir_to_fix, IR*& cur_stmt_root, bool& is_replace_column, vector<IR*>& ir_to_drop)
{

    // if (this->ir_wrapper.is_ir_in(ir_to_fix, IRTypeSetClause)
    // || this->ir_wrapper.is_ir_in(ir_to_fix, IRTypeStorageParameter)
    // || this->ir_wrapper.is_ir_in(ir_to_fix, IRTypeIndexFlagsParam)
    // ) {
    //     return;
    // }

    if (ir_to_fix->get_data_type() == DataColumnName && (ir_to_fix->get_data_flag() == ContextDefine || ir_to_fix->get_data_flag() == ContextReplaceDefine)) {

        if (ir_to_fix->get_data_flag() == ContextReplaceDefine) {
            is_replace_column = true;
        }
        string new_name = gen_column_name();
        ir_to_fix->set_str_val(new_name);
        ir_to_fix->set_is_instantiated(true);
        string closest_table_name = "";

        /* Attach the newly generated column name to the table. */
        if (this->query_instan_data->v_create_table_names_single.size() > 0) {
            /* We have table name that is newly defined. Should be only one
             * newly created table name. */
            closest_table_name = this->query_instan_data->v_create_table_names_single[0];
#ifdef DEBUG
            cerr << "Dependency: For newly defined column name: " << new_name
                 << ", we find v_create_table_names_single: " << closest_table_name
                 << "\n\n\n";
#endif
        } else if (this->query_instan_data->v_table_names_single.size() != 0) {
            /* We cannot find the newly defined table name, see whether there are
             * local table name used, this is typical in ALTER statement.  */
            closest_table_name = this->query_instan_data->v_table_names_single[0];
#ifdef DEBUG
            cerr << "Dependency: For newly defined column name: " << new_name
                 << ", cannot find v_create_table_names_single, is it in a "
                    "ALTER statement? We find v_table_names_single: "
                 << closest_table_name << "\n\n\n";
#endif
        } else if (this->query_instan_data->v_table_names.size() != 0) {
            /* This is an ERROR. Cannot find the TABLE name to attach to.
            ** 80% chance, keep original.
            ** 20% chance, find any declared table and attached to it. */
            if (get_pct_hit(80)) {
                /* Keep original */
                return;
            }
            closest_table_name = this->query_instan_data->v_table_names[get_rand_int(this->query_instan_data->v_table_names.size())];
#ifdef DEBUG
            cerr << "Dependency ERROR: For newly defined column name: " << new_name
                 << ", ERROR finding matched newly created table names. Used "
                    "previous declared table name: "
                 << closest_table_name << "\n\n\n";
#endif
        }
        if (closest_table_name == "" || closest_table_name == "x" || closest_table_name == "y") {
#ifdef DEBUG
            cerr << "Dependency Error: Cannot find the closest_table_name from "
                    "the query. ";
            cerr << "cloest_table_name returns: " << closest_table_name
                 << "In kDataColumnName, kDefine or kReplace. \n\n\n";
#endif
            /* Randomly set a name to the defined column.
             * And ignore the mapping for the moment
             * */

            /* Unrecognized, keep original */
            // ir_to_fix->str_val_ = gen_column_name();
            return;
        }
#ifdef DEBUG
        cerr << "Dependency: For column_name: " << new_name
             << ", found closest_table_name: " << closest_table_name
             << ". \n\n\n";
#endif

        // Avoid adding duplicated columns to the table mapping.
        vector<string>& cur_col_names = this->query_instan_data->m_table2columns[closest_table_name];
        if (find(cur_col_names.begin(), cur_col_names.end(), new_name) == cur_col_names.end()) {
            cur_col_names.push_back(new_name);
        }

        this->query_instan_data->v_column_names_single.push_back(new_name);
    }

    /* ContextUndefine scenario of the DataColumnName */
    else if (ir_to_fix->get_data_type() == DataColumnName && (ir_to_fix->get_data_flag() == ContextUndefine || ir_to_fix->get_data_flag() == ContextReplaceUndefine)) {
        /* Find the table_name in the query first. */
        string closest_table_name = "";
        IR* closest_table_ir = NULL;
        closest_table_ir = this->ir_wrapper.find_closest_nearby_IR_with_type(
            ir_to_fix, DataTableName);
        if (closest_table_ir != NULL) {
            closest_table_name = closest_table_ir->get_str_val();
#ifdef DEBUG
            cerr << "Dependency: For removing DataColumnName, we find "
                    "closest_table_ir: "
                 << closest_table_name << "\n\n\n";
#endif
        } else if (this->query_instan_data->v_table_names_single.size() != 0) {
            closest_table_name = this->query_instan_data->v_table_names_single[0];
#ifdef DEBUG
            cerr << "Dependency: For removing kDataColumnName: we find "
                    "v_table_names_single: "
                 << closest_table_name << "\n\n\n";
#endif
        }
        if (closest_table_name == "" || closest_table_name == "x" || closest_table_name == "y") {
#ifdef DEBUG
            cerr << "Dependency Error: Cannot find the closest_table_name from "
                    "the query. closest_table_name returns: "
                 << closest_table_name << ". In kDataColumnName, kUndefine. \n\n\n";
#endif
            /* Unrecognized, keep original */
            // return false;
            ir_to_fix->set_is_instantiated(true);
            return;
        }

#ifdef DEBUG
        cerr << "Dependency: In kDataColumnName, kUndefine, found "
                "closest_table_name: "
             << closest_table_name << ". \n\n\n";
#endif

        vector<string>& column_vec = this->query_instan_data->m_table2columns[closest_table_name];
        if (column_vec.size() == 0) {
#ifdef DEBUG
            cerr << "Dependency Error: Cannot find the mapped column_vec for "
                    "table_name: "
                 << closest_table_name << " \n\n\n";
#endif
            /* Not reconized column name. Keep original */
            // ir_to_fix->str_val_ = "y";
            // return false;
            ir_to_fix->set_is_instantiated(true);
            return;
        }
        string removed_column_name = column_vec[get_rand_int(column_vec.size())];
        column_vec.erase(
            std::remove(column_vec.begin(), column_vec.end(), removed_column_name),
            column_vec.end());
        ir_to_fix->set_str_val(removed_column_name);
        ir_to_fix->set_is_instantiated(true);

#ifdef DEBUG
        cerr << "Dependency: In kDataColumnName, kUndefine, found "
                "removed_column_name: "
             << removed_column_name
             << ", from closest_table_name: " << closest_table_name << ". \n\n\n";
#endif

        this->query_instan_data->v_column_names_single.push_back(removed_column_name);

        return;
    }

    else if (ir_to_fix->get_data_type() == DataColumnName && (ir_to_fix->get_data_flag() == ContextUse || ir_to_fix->get_data_flag() == ContextUseTop)) {
#ifdef DEBUG
        cerr << "Dependency: ori column name: " << ir_to_fix->get_str_val()
             << "\n\n\n";
        cerr << "In the kDataColumnName with kUse, found "
                "v_table_alias_names_single.size: "
             << query_instan_data->v_table_alias_names_single.size() << "\n\n\n";
#endif

        ir_to_fix->set_is_instantiated(true);

        if (ir_to_fix->get_data_affinity_type() != AFFIUNKNOWN) {
            // This is a special context that the Data Affinity type of the
            // column name node has been pre-defined.
            // This is used in query dynamic fixing, where the replaced query nodes
            // are saved in a whole, and the column node data affinity are preserved.

#ifdef DEBUG
            cerr << "\n\n\nDEBUG: Special handling of the column name, in dynamic "
                    "fixing"
                    " context. \n\n\n";
#endif

            if (this->query_instan_data->m_datatype2column.count(ir_to_fix->get_data_affinity_type()) == 0) {
                // If it cannot find the matching column names, instantiate this node
                // as an literal.
                ir_to_fix->set_ir_type(IRTypeSTRING);
                ir_to_fix->set_data_type(DataLiteral);
                ir_to_fix->mutate_literal_with_type(ir_to_fix->get_data_affinity_type());
                return;
            }
            string cur_chosen_col = vector_rand_ele(this->query_instan_data->m_datatype2column[ir_to_fix->get_data_affinity_type()]);

            bool is_col_imported = false;
            for (string cur_used_table : this->query_instan_data->v_table_names_single) {
                vector<string> v_imported_col = this->query_instan_data->m_table2columns[cur_used_table];
                if (find_vector(v_imported_col, cur_chosen_col)) {
                    is_col_imported = true;
                    break;
                }
            }

            if (is_col_imported) {
                ir_to_fix->set_is_instantiated(true);
                ir_to_fix->set_str_val(cur_chosen_col);
            } else {
                // If it cannot find the matching column names, instantiate this node
                // as an literal.
                ir_to_fix->set_ir_type(IRTypeSTRING);
                ir_to_fix->set_data_type(DataUnknownType);
                ir_to_fix->mutate_literal();
            }

            return;
        }

        // Actual random mutation of the ColumnName. ContextUse.

        bool is_found = false;
        string closest_table_name = "";
        if (this->query_instan_data->v_table_alias_names_single.size() != 0) {
            closest_table_name = vector_rand_ele(this->query_instan_data->v_table_alias_names_single);
#ifdef DEBUG
            cerr << "Dependency: In column fixing, find table alias name from "
                    "v_table_alias_names_single: "
                 << closest_table_name << ". \n\n\n";
#endif
            is_found = true;
        }
        if (!is_found && this->query_instan_data->v_table_names_single.size() != 0) {
            closest_table_name = vector_rand_ele(this->query_instan_data->v_table_names_single);
#ifdef DEBUG
            cerr << "Dependency: In column fixing, find table alias name from "
                    "v_table_names_single: "
                 << closest_table_name << ". \n\n\n";
#endif
            is_found = true;
        }

        if (!is_found) {

            // Last chance, try to directly search for table name in the tree nodes.
            closest_table_name = this->find_cloest_table_name(ir_to_fix);

            if (closest_table_name == "" || closest_table_name == "x" || closest_table_name == "y") {
#ifdef DEBUG
                cerr << "Dependency : Cannot find the closest_table_name from "
                        "the query. closest_table_name is: "
                     << closest_table_name << ". In kDataColumnName, kUse. \n\n\n";

                cerr << "Choose to use the literal in this scenario now. \n\n\n";
#endif

                if (this->query_instan_data->v_column_names_single.size() > 0) {
                    string new_name = this->query_instan_data->v_column_names_single[get_rand_int(this->query_instan_data->v_column_names_single.size())];
                    ir_to_fix->set_str_val(new_name);
                    ir_to_fix->set_is_instantiated(true);
                    return;
                }

                ir_to_fix->set_is_instantiated(false);
                ir_to_fix->set_ir_type(IRTypeSTRING);
                ir_to_fix->set_data_type(DataLiteral);
                ir_to_fix->set_data_flag(ContextUse);

                this->instan_literal(ir_to_fix, cur_stmt_root, ir_to_drop);

                return;
            }
        }

        vector<string> cur_mapped_column_name_vec;
        if (this->query_instan_data->m_alias_table2column_single.count(closest_table_name) > 0) {
            cur_mapped_column_name_vec = this->query_instan_data->m_alias_table2column_single[closest_table_name];
        } else {
            cur_mapped_column_name_vec = this->query_instan_data->m_table2columns[closest_table_name];
        }

#ifdef DEBUG
        cerr << "Dependency: In kUse of kDataColunName, use origin table "
                "name: "
             << closest_table_name
             << ". column size is: " << cur_mapped_column_name_vec.size()
             << ". \n\n\n";
#endif
        if (cur_mapped_column_name_vec.size() > 0) {
            string cur_chosen_column = cur_mapped_column_name_vec[get_rand_int(
                cur_mapped_column_name_vec.size())];

            // Enforce (table_name/alias_name).column_name
            // But be very careful with all the weird MySQL dialects problems. 
            if (this->query_instan_data->v_table_alias_names_single.size() == 0 ||
                this->ir_wrapper.is_ir_in(ir_to_fix, IRTypeLimitClause) ||
                this->ir_wrapper.is_ir_in(ir_to_fix, IRTypeUsingClause)
            ) {
                ir_to_fix->set_str_val(cur_chosen_column);
            } else {
                ir_to_fix->set_str_val(closest_table_name + "." + cur_chosen_column);
            }

            ir_to_fix->set_is_instantiated(true);
            // if (this->query_instan_data->m_column2datatype.count(cur_chosen_column)) {
            //     ir_to_fix->set_data_affinity(DataAffinity(*this->query_instan_data->m_column2datatype[cur_chosen_column]));
            // }

            this->query_instan_data->v_column_names_single.push_back(cur_chosen_column);

        } else {
            /* Unreconized, keep original */
            // ir_to_fix->str_val_ = "y";
            ir_to_fix->set_is_instantiated(true);
#ifdef DEBUG
            cerr << "Dependency Error: In kDataColumnName, kUse, cannot find "
                    "mapping from table_name: "
                 << closest_table_name << ". \n\n\n";
#endif
        }
    }
}

void PostgreSQLQueryInstantiator::instan_column_alias_name(IR* ir_to_fix, IR* cur_stmt_root,
    vector<IR*>& ir_to_deep_drop)
{

    // if (this->ir_wrapper.is_ir_in(ir_to_fix, IRTypeIndexFlagsParam)) {
    //     return;
    // }

    if (ir_to_fix->get_data_type() == DataColumnAliasName) {

#ifdef DEBUG
        cerr << "\n\n\nDebug::Trying to fix the DataColumnAliasName. \n\n\n";
#endif

        ir_to_fix->set_is_instantiated(true);

        string closest_table_alias_name = "";

        /* Three situations:
         * 1. TypeSelectExprs: `SELECT CustomerID AS ID, CustomerName AS
         * Customer FROM Customers;`
         * 2. TypeAliasClause: `SELECT c.x FROM (SELECT COUNT(*) FROM users) AS
         * c(x);`
         * 3. TypeAliasClause: WITH r(c) AS (SELECT * FROM v0 WHERE v1 = 100)
         * SELECT * FROM r WHERE c = 100;
         *
         * The 2 and 3 cases are similar.
         * */

        bool is_alias_clause = false;
        if (this->ir_wrapper.is_ir_in(ir_to_fix, IRTypeAliasClause)) {
            is_alias_clause = true;
        }

        if (is_alias_clause) {
            /* Fix the TypeAliasClause scenario first.
             * This scenario must be handled before the ContextUse of
             * DataColumnName.
             * In this case, the TypeTableAlias is provided, we need to
             * connect the TypeTableAlias to the TypeColumnAlias.
             * Challenge: We need to make sure the number of
             * alise column matched the SELECT clause element in the subquery.
             * Luckily, we can ensure that when running in this scenario,
             * the subquery has already been instantiated, so that all the column
             * mappings are correct.
             */

            // First, check the nearby select subquery.
            IR* select_subquery = this->ir_wrapper.find_closest_nearby_IR_with_type(ir_to_fix,
                IRTypeSelectStmt);
            if (select_subquery != NULL && select_subquery != cur_stmt_root) {
#ifdef DEBUG
                cerr << "\n\n\nDependency: when fixing the select subquery, "
                        "found select subquery: "
                     << select_subquery->to_string() << "\n\n\n";
#endif
            } else {
#ifdef DEBUG
                cerr << "\n\n\nDependency: Cannot find the select subquery from the "
                        "current stmt. "
                        "Remove the current column alias clause. \n\n\n";
#endif

                IR* opt_alias_clause = this->ir_wrapper.get_ancestor_node_matching_type(
                    ir_to_fix, IRTypeAliasClause);
                if (opt_alias_clause == NULL) {
                    cerr << "\n\n\nFATAL ERROR: Cannot find the TypeAliasClause in the "
                            "TypeAliasClause instantiation. \n\n\n";
                    return;
                }

                for (IR*& cur_child : opt_alias_clause->get_children_ref()) {
                    ir_to_deep_drop.push_back(cur_child);
                    cur_child->set_is_instantiated(true);
                }
                opt_alias_clause->detach_children();

                return;
            }

            // Search whether there are columns defined in the `TypeSelectExprs`.
            vector<IR*> all_column_in_subselect = this->ir_wrapper.get_ir_node_in_stmt_with_type(select_subquery,
                IRTypeSelectStmt);
            vector<IR*> all_table_in_subselect = this->ir_wrapper.get_ir_node_in_stmt_with_type(select_subquery,
                DataTableName);
            // vector<IR*> all_stars_in_subselect = this->ir_wrapper.get_ir_node_in_stmt_with_type(
            //     select_subquery, IRTypeColumnPathWithStar);

            // Try to handle the columns defined in the subquery first.
            // Only look at the columns defined in the SELECT clause:
            // e.g. `SELECT v1, v2 FROM v0`

            vector<IR*> ref_column_in_subselect;
            vector<string> new_column_alias_names;
            string ret_str = "";
            for (auto& cur_column_in_subselect : all_column_in_subselect) {
                if (this->ir_wrapper.is_ir_in(cur_column_in_subselect,
                        IRTypeAliasClause)) {
#ifdef DEBUG
                    cerr << "\n\n\nFound column name in TypeSelectExprs: "
                         << cur_column_in_subselect->to_string() << "\n\n\n";
#endif
                    ref_column_in_subselect.push_back(
                        cur_column_in_subselect);
                }
            }

            int ref_col_idx = 0;
            if (ref_column_in_subselect.size() > 0) {
                for (auto& cur_column_in_sub : ref_column_in_subselect) {
                    string cur_col_in_sub_str = cur_column_in_sub->get_str_val();
                    string new_column_alias_name = gen_column_alias_name();
                    this->query_instan_data->m_alias2column_single[new_column_alias_name] = cur_col_in_sub_str;
                    if (this->query_instan_data->m_column2datatype.count(cur_col_in_sub_str) == 0 && cur_column_in_sub->get_data_type() == DataLiteral) {
                        this->query_instan_data->m_column2datatype[cur_col_in_sub_str] = new DataAffinity(*cur_column_in_sub->get_p_data_affinity());
                    }
                    new_column_alias_names.push_back(new_column_alias_name);
                    if (ref_col_idx > 0) {
                        ret_str += ", ";
                    }
                    ref_col_idx++;
                    ret_str += new_column_alias_name;
#ifdef DEBUG
                    cerr << "\n\n\nMapping alias name: " << new_column_alias_name
                         << " to column name " << cur_col_in_sub_str
                         << " in TypeSelectExprs. ";
#endif
                }
            }
            // Inherit the ref_col_idx.
            //             if (all_stars_in_subselect.size() > 0 && all_table_in_subselect.size() > 0) {
            //                 IR* cur_select_table = all_table_in_subselect.front();
            //                 for (string& matched_column :
            //                     this->query_instan_data->m_table2columns[cur_select_table->get_str_val()]) {
            //                     string new_column_alias_name = gen_column_alias_name();
            //                     this->query_instan_data->m_alias2column_single[new_column_alias_name] = matched_column;
            //                     new_column_alias_names.push_back(new_column_alias_name);
            //                     if (ref_col_idx > 0) {
            //                         ret_str += ", ";
            //                     }
            //                     ref_col_idx++;
            //                     ret_str += new_column_alias_name;
            // #ifdef DEBUG
            //                     cerr << "\n\n\nMapping alias name: " << new_column_alias_name
            //                          << " to column name " << matched_column
            //                          << " in TypeSelectExprs. ";
            // #endif
            //                 }
            //             }

            // Next, match the table alias name.
            IR* alias_table_ir = this->ir_wrapper.find_closest_nearby_IR_with_type<DATATYPE>(
                ir_to_fix, DataTableAliasName);
            string alias_table_str;
            if (alias_table_ir != NULL) {
                alias_table_str = alias_table_ir->get_str_val();
            } else {
#ifdef DEBUG
                cerr << "\n\n\nError: Cannot find table alias name inside the "
                        "TypeAliasClause \n\n\n";
#endif
                ir_to_fix->set_str_val("x");
                return;
            }

            for (string& cur_new_column_alias_name : new_column_alias_names) {
                this->query_instan_data->m_alias_table2column_single[alias_table_str].push_back(
                    cur_new_column_alias_name);
            }

            // Actually replace the current node.
            IR* alias_clause_ir = this->ir_wrapper.get_ancestor_node_matching_type(
                ir_to_fix, IRTypeAliasClause);
            if (alias_clause_ir == NULL) {
#ifdef DEBUG
                cerr << "\n\n\nLogical Error: Cannot find the TypeAliasClauseIR "
                        "from Columnaliaslist. \n\n\n";
#endif
                return;
            }

            for (IR* cur_child : alias_table_ir->get_children_ref()) {
                ir_to_deep_drop.push_back(cur_child);
                cur_child->set_is_instantiated(true);
            }
            alias_table_ir->detach_children();

            //            ir_to_deep_drop.push_back(alias_clause_ir->get_right());
            //            p_oracle->ir_wrapper.iter_cur_node_with_handler(
            //                    alias_clause_ir->get_right(), [](IR *cur_node) -> void {
            //                        cur_node->set_is_instantiated(true);
            //                        cur_node->set_data_flag(ContextNoModi);
            //                    });
            //            if (ret_str != "") {
            //                IR *new_column_alias_list = new IR(TypeColumnDefList, ret_str);
            //                alias_clause_ir->update_right(new_column_alias_list);
            //            } else {
            //                // ret_str == ""
            //                // If no column alias observed, remove the empty bracket.
            //                alias_clause_ir->update_right(NULL);
            //                alias_clause_ir->op_->middle_ = "";
            //                alias_clause_ir->op_->suffix_ = "";
            //            }

            return;

        } else {
            /* Fix the TypeSelectExprs scenario now.
             * No need for extra work for this scenario because it is
             * not very interesting.
             * 1. TypeSelectExprs: `SELECT CustomerID AS ID, CustomerName AS
             * Customer FROM Customers;`
             */

            IR* near_table_ir = this->ir_wrapper.find_closest_nearby_IR_with_type<DATATYPE>(
                ir_to_fix, DataTableName);
            string near_table_str;
            if (near_table_ir != NULL) {
                near_table_str = near_table_ir->get_str_val();
            } else {
#ifdef DEBUG
                cerr << "\n\n\nError: Cannot find table alias name inside the "
                        "TypeAliasClause \n\n\n";
                ir_to_fix->set_str_val("x");
                return;
#endif
            }

            string column_alias_name = gen_column_alias_name();
            ir_to_fix->set_str_val(column_alias_name);

            this->query_instan_data->m_alias_table2column_single[near_table_str].push_back(column_alias_name);
            return;
        }
    }
}

void PostgreSQLQueryInstantiator::instan_sql_type_name(IR* ir_to_fix)
{

    // if (this->ir_wrapper.is_ir_in(ir_to_fix, IRTypeIndexFlagsParam)) {
    //     return;
    // }

    IRTYPE type = ir_to_fix->get_ir_type();
    DATATYPE data_type = ir_to_fix->get_data_type();
    DATAFLAG data_flag = ir_to_fix->get_data_flag();

    if (data_type == DataTypeName && data_flag == ContextDefine) {
        // Handling of the Column Data Type definition.
        // Use basic types.
        auto tmp_affi_type = get_random_affinity_type();
        string tmp_affi_type_str = get_affinity_type_str_formal(tmp_affi_type);

        ir_to_fix->set_str_val(tmp_affi_type_str);
#ifdef DEBUG
        cerr << "\nFor data type definition, getting new data type: "
             << tmp_affi_type_str << "\n\n\n";
#endif

        // if (this->ir_wrapper.is_ir_in(ir_to_fix, IRTypeColDef)) {
        // IR* column_node = this->ir_wrapper.find_closest_nearby_IR_with_type(ir_to_fix, DataColumnName);
        //
        // string column_str = column_node->get_str_val();
        // DataAffinity* cur_data_affi = new DataAffinity(*column_node->get_p_data_affinity());
        // this->query_instan_data->m_column2datatype[column_str] = cur_data_affi;
        // this->query_instan_data->m_datatype2column[cur_data_affi->get_data_affinity()].push_back(
        //     column_str);
        // #ifdef DEBUG
        //             // cerr << "\nAttach data affinity: "
        //                  << get_string_by_data_affi(cur_data_affi->get_data_affinity())
        //                  << " to column: " << column_str << ". \n\n\n";
        // #endif
        // }
    }
}

void PostgreSQLQueryInstantiator::instan_statistic_name(IR* ir_to_fix)
{

    if (ir_to_fix->get_data_type() == DataStatsName) {
        if (ir_to_fix->get_data_flag() == ContextDefine) {
            string cur_chosen_name = gen_statistic_name();
            ir_to_fix->set_str_val(cur_chosen_name);
            ir_to_fix->set_is_instantiated(true);
            this->query_instan_data->v_statistics_name.push_back(cur_chosen_name);
        }

        else if (ir_to_fix->get_data_flag() == ContextUndefine) {
            if (!this->query_instan_data->v_statistics_name.size())
                return;
            string cur_chosen_name = vector_rand_ele(this->query_instan_data->v_statistics_name);
            ir_to_fix->set_str_val(cur_chosen_name);
            ir_to_fix->set_is_instantiated(true);

            /* remove the statistic name from the vector */
            vector<string> v_tmp;
            for (string& s : this->query_instan_data->v_statistics_name) {
                if (s != cur_chosen_name) {
                    v_tmp.push_back(s);
                }
            }
            this->query_instan_data->v_statistics_name = v_tmp;
        }

        else if (ir_to_fix->get_data_flag() == ContextUse) {
            if (!this->query_instan_data->v_statistics_name.size())
                return;
            string cur_chosen_name = vector_rand_ele(this->query_instan_data->v_statistics_name);
            ir_to_fix->set_str_val(cur_chosen_name);
            ir_to_fix->set_is_instantiated(true);
        }
    }

    return;
}

void PostgreSQLQueryInstantiator::instan_sequence_name(IR* ir_to_fix)
{

    /* Fix for kDataSequenceName */
    if (ir_to_fix->get_data_type() == DataSequenceName) {
        ir_to_fix->set_is_instantiated(true);
        if (ir_to_fix->get_data_flag() == ContextDefine) {
            // string cur_chosen_name = gen_sequence_name();
            // ir_to_fix->set_str_val(cur_chosen_name);

            /* XX: Do not fix for sequence name for now */
            string cur_chosen_name = ir_to_fix->get_str_val();
            this->query_instan_data->v_sequence_name.push_back(cur_chosen_name);
        }

        else if (ir_to_fix->get_data_flag() == ContextUndefine) {
            if (!this->query_instan_data->v_sequence_name.size())
                return;
            string cur_chosen_name = vector_rand_ele(this->query_instan_data->v_sequence_name);
            ir_to_fix->set_str_val(cur_chosen_name);

            /* remove the statistic name from the vector */
            vector<string> v_tmp;
            for (string& s : this->query_instan_data->v_sequence_name) {
                if (s != cur_chosen_name) {
                    v_tmp.push_back(s);
                }
            }
            this->query_instan_data->v_sequence_name = v_tmp;
        }

        else if (ir_to_fix->get_data_flag() == ContextUse) {
            if (!this->query_instan_data->v_sequence_name.size())
                return;
            string cur_chosen_name = vector_rand_ele(this->query_instan_data->v_sequence_name);
            ir_to_fix->set_str_val(cur_chosen_name);
        }
    }

    return;
}

void PostgreSQLQueryInstantiator::instan_constraint_name(IR* ir_to_fix)
{

    /* Fix for kDataConstraintName */
    if (ir_to_fix->get_data_type() == DataConstraintName) {
        ir_to_fix->set_is_instantiated(true);
        if (ir_to_fix->get_data_flag() == ContextDefine) {

            string cur_chosen_name = gen_constraint_name();
            ir_to_fix->set_str_val(cur_chosen_name);
            this->query_instan_data->v_constraint_name.push_back(cur_chosen_name);
        }

        else if (ir_to_fix->get_data_flag() == ContextUndefine) {
            if (!this->query_instan_data->v_constraint_name.size())
                return;
            string cur_chosen_name = vector_rand_ele(this->query_instan_data->v_constraint_name);
            ir_to_fix->set_str_val(cur_chosen_name);

            /* remove the statistic name from the vector */
            vector<string> v_tmp;
            for (string& s : this->query_instan_data->v_constraint_name) {
                if (s != cur_chosen_name) {
                    v_tmp.push_back(s);
                }
            }
            this->query_instan_data->v_constraint_name = v_tmp;
        }

        else if (ir_to_fix->get_data_flag() == ContextUse) {
            if (!this->query_instan_data->v_constraint_name.size())
                return;
            string cur_chosen_name = vector_rand_ele(this->query_instan_data->v_constraint_name);
            ir_to_fix->set_str_val(cur_chosen_name);
        }
    }

    return;
}

void PostgreSQLQueryInstantiator::instan_family_name(IR* ir_to_fix)
{

    /* Fix for DataFamilyName */
    if (ir_to_fix->get_data_type() == DataFamilyName) {
        ir_to_fix->set_is_instantiated(true);
        if (ir_to_fix->get_data_flag() == ContextDefine) {

            string cur_chosen_name = gen_family_name();
            ir_to_fix->set_str_val(cur_chosen_name);
            this->query_instan_data->v_family_name.push_back(cur_chosen_name);
        }

        else if (ir_to_fix->get_data_flag() == ContextUndefine) {
            if (!this->query_instan_data->v_family_name.size())
                return;
            string cur_chosen_name = vector_rand_ele(this->query_instan_data->v_family_name);
            ir_to_fix->set_str_val(cur_chosen_name);

            /* remove the statistic name from the vector */
            vector<string> v_tmp;
            for (string& s : this->query_instan_data->v_family_name) {
                if (s != cur_chosen_name) {
                    v_tmp.push_back(s);
                }
            }
            this->query_instan_data->v_family_name = v_tmp;
        }

        else if (ir_to_fix->get_data_flag() == ContextUse) {
            if (!this->query_instan_data->v_family_name.size())
                return;
            string cur_chosen_name = vector_rand_ele(this->query_instan_data->v_family_name);
            ir_to_fix->set_str_val(cur_chosen_name);
        }
    }

    return;
}

IR* PostgreSQLQueryInstantiator::constr_rand_func_with_affinity(DATAAFFINITYTYPE in_affi)
{

    string cur_func_name = "";
    if (in_affi == AFFIANY || in_affi == AFFIUNKNOWN) {
        cur_func_name = vector_rand_ele(this->all_saved_func_name);
#ifdef DEBUG
        cerr << "\n\n\nDependency: Fixing functions with "
             << get_string_by_data_affi(in_affi)
             << "\nGetting func name: " << cur_func_name << "\n\n\n";
#endif
    } else if (this->func_type_lib.count(in_affi) > 0) {
        cur_func_name = vector_rand_ele(func_type_lib[in_affi]);
#ifdef DEBUG
        cerr << "\n\n\nDependency: Fixing functions with "
             << get_string_by_data_affi(in_affi)
             << "\nGetting func name: " << cur_func_name << "\n\n\n";
#endif
    } else {
        cur_func_name = vector_rand_ele(this->all_saved_func_name);
#ifdef DEBUG
        cerr << "\n\n\nError: Cannot find affinity type in_affi, "
             << get_string_by_data_affi(in_affi)
             << "\nGetting func name: " << cur_func_name << "\n\n\n";
#endif
    }

    // Randomly choose a set of arguments.
    vector<DataAffinity> v_func_affi = vector_rand_ele(func_str_to_type_map[cur_func_name]).param_data_affi;

    int arg_idx = -1;

    ProductionNode* arg_list_prod_node = p_rsg->get_prod_from_ir_type(IRTypeExprList);
    IR* args_list_ir = new IR(SymbolNonTerm, IRTypeExprList, string(), p_rsg->get_token_from_ir_type(IRTypeExprList), &dump_simple_expr_node, arg_list_prod_node);

    for (DataAffinity& cur_arg_affi : v_func_affi) {
        arg_idx++;

        if (arg_idx != 0) {
            IR* tmp_ir = new IR(SymbolTerm, IRTypeUnknownType, string(","), nullptr);
            args_list_ir->get_children_ref().push_back(tmp_ir);
            tmp_ir->set_parent_node(args_list_ir);
        }

        string cur_col_str;
        if (this->query_instan_data->m_datatype2column.count(cur_arg_affi.get_data_affinity())) {
            // Use the data column that match the affinity.
            cur_col_str = vector_rand_ele(
                this->query_instan_data->m_datatype2column[cur_arg_affi.get_data_affinity()]);
        }

        bool is_col_used_ok = false;
        for (string used_table_name : this->query_instan_data->v_table_names_single) {
            vector<string> cur_col_vec = this->query_instan_data->m_table2columns[used_table_name];
            if (find_vector(cur_col_vec, cur_col_str)) {
                is_col_used_ok = true;
                break;
            }
        }

        if (is_col_used_ok && get_pct_hit(66)) {
            IR* new_col_ir = new IR(IRTypeIDENT, cur_col_str, DataColumnName, ContextUse, nullptr);
            new_col_ir->set_mapped_expr_node(&dump_simple_expr_node);

            auto* new_dump_expr = new IR(SymbolNonTerm, IRTypeAExpr, string(), p_rsg->get_token_from_ir_type(IRTypeAExpr), &dump_simple_expr_node, p_rsg->get_prod_from_ir_type(IRTypeAExpr));
            new_dump_expr->set_is_favor(IsFavor::favor);
            new_dump_expr->add_one_child(new_col_ir, 0);

            args_list_ir->get_children_ref().push_back(new_dump_expr);
            new_dump_expr->set_parent_node(args_list_ir);

#ifdef DEBUG
            cerr << "\n\n\nDependency: Getting good to use cur_col_str: "
                 << cur_col_str << "\n\n\n";
#endif
        } else {
            // Use literal that match the affinity type.
            string cur_arg_str = cur_arg_affi.get_mutated_literal();
            IR* new_literal_ir = new IR(SymbolLit, IRTypeSTRING, cur_arg_str, nullptr);
            new_literal_ir->set_data_affinity_type(cur_arg_affi.get_data_affinity());

            auto* new_dump_expr = new IR(SymbolNonTerm, IRTypeAExpr, string(), p_rsg->get_token_from_ir_type(IRTypeAExpr), &dump_simple_expr_node, p_rsg->get_prod_from_ir_type(IRTypeAExpr));
            new_dump_expr->set_is_favor(IsFavor::favor);
            new_dump_expr->add_one_child(new_literal_ir, 0);

            args_list_ir->get_children_ref().push_back(new_dump_expr);
            new_dump_expr->set_parent_node(args_list_ir);
#ifdef DEBUG
            cerr << "\n\n\nDependency: cur_col_str is not referenced before, do "
                    "not use column names, "
                    "use literal instead: "
                 << cur_arg_str << "\n\n\n";
#endif
        }
    }

    IR* ret_ir = new IR(SymbolNonTerm, IRTypeAExpr, string(), p_rsg->get_token_from_ir_type(IRTypeAExpr));

    IR* func_name_ir = new IR(IRTypeIDENT, cur_func_name, DataFunctionName, ContextUse, nullptr);
    func_name_ir->set_is_instantiated(true);
    ret_ir->get_children_ref().push_back(func_name_ir);
    func_name_ir->set_parent_node(ret_ir);

    IR* tmp_0 = new IR(SymbolTerm, IRTypeUnknownType, string("("), nullptr);
    ret_ir->get_children_ref().push_back(tmp_0);
    ret_ir->get_children_ref().push_back(args_list_ir);
    tmp_0->set_parent_node(ret_ir);
    args_list_ir->set_parent_node(ret_ir);
    IR* tmp_1 = new IR(SymbolTerm, IRTypeUnknownType, string(")"), nullptr);
    ret_ir->get_children_ref().push_back(tmp_1);
    tmp_1->set_parent_node(ret_ir);

    return ret_ir;
}

void PostgreSQLQueryInstantiator::instan_func_application(IR* ir_to_fix, IR* cur_stmt_root_ir, vector<IR*>& ir_to_deep_drop,
    bool is_ignore_nested_expr)
{
    IR* parent_node = ir_to_fix->get_parent_node();
    if (parent_node == nullptr) {
#ifdef DEBUG
        cerr << "Error: No parent node for the ir_to_fix: " << ir_to_fix->to_string() << "\n";
#endif
        return;
    }

    // if (this->ir_wrapper.is_ir_in(ir_to_fix, IRTypeSetClause)
    // || this->ir_wrapper.is_ir_in(ir_to_fix, IRTypeStorage)
    //     ) {
    // #ifdef DEBUG
    //         cerr << "\n\n\nInside instan_func_expr, the statment is inside "
    //                 "TypeSetVar or"
    //                 " inside TypeStorageParams, skippped. \n\n\n";
    // #endif
    //         return;
    //     }

    DATAAFFINITYTYPE chosen_affi = get_nearby_data_affinity(ir_to_fix);
    IR* new_func_node = constr_rand_func_with_affinity(chosen_affi);

    if (parent_node->swap_one_child(ir_to_fix, new_func_node, false)) {
        ir_to_deep_drop.push_back(ir_to_fix);
        ir_to_fix->set_is_instantiated(true);
        ir_to_fix = new_func_node;
    }

#ifdef DEBUG
    cerr << "\n\n\nDependency: Inside instan_func_expr, generating new "
            "function: "
         << new_func_node->to_string() << "\n\n\n";
#endif

    ir_to_fix->set_is_instantiated(true);

    return;
}

void PostgreSQLQueryInstantiator::map_create_view(IR* ir_to_fix, IR* cur_stmt_root,
    const vector<vector<IR*>> all_substmt_ir_of_interest)
{

    if (ir_to_fix->get_data_type() != DataViewName && ir_to_fix->get_data_flag() != ContextDefine) {
        return;
    }

    /* Add missing mapping for CREATE VIEW stmt.  */
    /* Check whether we are in the CreateViewStatement. If yes, save the
     * column mapping. */
    IR* cur_ir = ir_to_fix;
    bool is_in_create_view = false;
    if (this->ir_wrapper.is_ir_in(cur_stmt_root, IRTypeViewStmt) || this->ir_wrapper.is_ir_in(cur_stmt_root, IRTypeCreateMatViewStmt)) {
        is_in_create_view = true;
    }
    if (is_in_create_view) {
        /* Added column mapping for CREATE TABLE/VIEW... v0 AS SELECT...
         * statement.
         */
#ifdef DEBUG
        cerr << "Dependency: In CREATE VIEW statement, getting "
                "cur_stmt_ir_to_fix_vec.size: "
             << all_substmt_ir_of_interest.size() << ". \n\n\n";
#endif
        // id_column_name should be in the subquery and already been resolved
        // in the previous loop.
        vector<IR*> tmp_column_vec;
        vector<IR*> all_mentioned_column_vec;
        vector<DATATYPE> column_type_vec { DataColumnName };
        all_mentioned_column_vec = this->ir_wrapper.get_ir_node_in_stmt_with_type(cur_stmt_root, column_type_vec, false, true);

        //        for (IR* cur_mentioned_column : all_mentioned_column_vec) {
        //            if (this->ir_wrapper.is_ir_in(cur_mentioned_column,
        //                    IRTypeTargetEl)) {
        //                tmp_column_vec.push_back(cur_mentioned_column);
        //            }
        //        }
        //        all_mentioned_column_vec = tmp_column_vec;
        //        tmp_column_vec.clear();

        /* Fix: also, add column alias name defined here to the table */
        vector<IR*> all_mentioned_column_alias_vec;
        vector<DATATYPE> column_alias_type_vec { DataColumnAliasName };
        all_mentioned_column_alias_vec = this->ir_wrapper.get_ir_node_in_stmt_with_type(cur_stmt_root, column_alias_type_vec, false, true);

        // for (IR* cur_mentioned_alias : all_mentioned_column_alias_vec) {
        //     if (this->ir_wrapper.is_ir_in(cur_mentioned_alias, IRTypeTargetEl)) {
        //         tmp_column_vec.push_back(cur_mentioned_alias);
        //     }
        // }
        // all_mentioned_column_alias_vec = tmp_column_vec;
        // tmp_column_vec.clear();

#ifdef DEBUG
        cerr << "Dependency: When building extra mapping for CREATE VIEW AS, "
                "collected kDataColumnName.size: "
             << all_mentioned_column_vec.size() << ". \n\n\n";
#endif

        if (all_mentioned_column_alias_vec.size() != 0) {
            this->query_instan_data->m_table2columns[ir_to_fix->get_str_val()].clear();
            for (auto& cur_column_alias_ir : all_mentioned_column_alias_vec) {
                string cur_column_alias = cur_column_alias_ir->get_str_val();
                vector<string>& v_view_column_str = this->query_instan_data->m_table2columns[ir_to_fix->get_str_val()];
                if (find(v_view_column_str.begin(), v_view_column_str.end(),
                        cur_column_alias)
                    == v_view_column_str.end()) {
                    v_view_column_str.push_back(cur_column_alias);
                }
#ifdef DEBUG
                cerr << "Dependency: Adding mappings: For table/view: "
                     << ir_to_fix->get_str_val()
                     << ", map from column alias to column str: " << cur_column_alias
                     << ". \n\n\n";
#endif
            }
        } else {
            for (const IR* const cur_men_column_ir : all_mentioned_column_vec) {
                string cur_men_column_str = cur_men_column_ir->get_str_val();
                if (findStringIn(cur_men_column_str, ".")) {
                    vector<string> v_cur_men_column_str = string_splitter(cur_men_column_str, '.');
                    cur_men_column_str = v_cur_men_column_str[v_cur_men_column_str.size() - 1];
                }
                vector<string>& cur_m_table = this->query_instan_data->m_table2columns[ir_to_fix->get_str_val()];
                if (std::find(cur_m_table.begin(), cur_m_table.end(),
                        cur_men_column_str)
                    == cur_m_table.end()) {
                    this->query_instan_data->m_table2columns[ir_to_fix->get_str_val()].push_back(cur_men_column_str);
#ifdef DEBUG
                    cerr << "Dependency: Adding mappings: For table/view: "
                         << ir_to_fix->get_str_val()
                         << ", map with column: " << cur_men_column_str << ". \n\n\n";
#endif
                }
            }

            /* For CREATE VIEW x AS SELECT * FROM v0; */
            if (all_mentioned_column_vec.size() == 0) {
#ifdef DEBUG
                cerr << "Dependency: For mapping CREATE VIEW, cannot find column "
                        "name in the current subqueries. Thus, see if we can find "
                        "table names, and map from there. \n\n\n";
#endif
                vector<IR*> all_mentioned_table_vec, all_mentioned_table_kUsed_vec;
                vector<DATATYPE> table_type_vec { DataTableName };
                all_mentioned_table_vec = this->ir_wrapper.get_ir_node_in_stmt_with_type(cur_stmt_root, table_type_vec, false, true);
                for (IR* mentioned_table_ir : all_mentioned_table_vec) {
                    if (mentioned_table_ir->get_data_flag() == ContextUse) {
                        all_mentioned_table_kUsed_vec.push_back(mentioned_table_ir);
#ifdef DEBUG
                        cerr << "Dependency: For mapping CREATE VIEW, getting "
                                "mentioned table name: "
                             << mentioned_table_ir->get_str_val() << ". \n\n\n";
#endif
                    }
                }
                for (IR* cur_men_tablename_ir : all_mentioned_table_kUsed_vec) {
                    string cur_men_tablename_str = cur_men_tablename_ir->get_str_val();
                    const vector<string>& cur_men_column_vec = this->query_instan_data->m_table2columns[cur_men_tablename_str];
                    for (const string& cur_men_column_str : cur_men_column_vec) {
                        vector<string>& cur_m_table = this->query_instan_data->m_table2columns[ir_to_fix->get_str_val()];
                        if (std::find(cur_m_table.begin(), cur_m_table.end(),
                                cur_men_column_str)
                            == cur_m_table.end()) {
                            this->query_instan_data->m_table2columns[ir_to_fix->get_str_val()].push_back(
                                cur_men_column_str);
#ifdef DEBUG
                            cerr << "Dependency: Adding mappings: For table/view: "
                                 << ir_to_fix->get_str_val()
                                 << ", map with column: " << cur_men_column_str << "\n\n";
#endif
                        }
                    }
                } // for (IR* cur_men_tablename_ir : all_mentioned_table_kUsed_vec)
            } // if (all_mentioned_column_vec.size() == 0)
        }

        /* The extra mapping only need to be done once. Once reach this point,
         * break the loop. */
        return;
    } // if (is_in_create_view)
}

void PostgreSQLQueryInstantiator::map_create_view_column(IR* ir_to_fix,
    vector<IR*>& ir_to_deep_drop)
{

    IR* type_name_list = this->ir_wrapper.get_ancestor_node_matching_type(ir_to_fix, IRTypeCreateMvTarget);
    if (type_name_list == nullptr || type_name_list->get_children_ref().size() == 0) {
        return;
    }
    type_name_list = type_name_list->get_children_ref()[1];
    if (type_name_list == NULL) {
#ifdef DEBUG
        cerr << "\n\n\nError: In DataViewColumnName fixing. Cannot find the "
                "type_name_list from the statement."
                "More debug info, view column is: "
             << ir_to_fix->to_string() << ". \n\n\n";
#endif
        return;
    }

    if (ir_to_fix->get_data_flag() == ContextDefine) {
        string new_view_column_name = gen_view_column_name();
        ir_to_fix->set_str_val(new_view_column_name);
        ir_to_fix->set_is_instantiated(true);

        if (this->query_instan_data->v_create_view_names_single.size() != 0) {
            string near_view_name_str = this->query_instan_data->v_create_view_names_single.back();
            this->query_instan_data->m_table2columns[near_view_name_str].push_back(new_view_column_name);
        }
    }

    //     IR* near_view_name_node = this->ir_wrapper.find_closest_nearby_IR_with_type(ir_to_fix,
    //         DataViewName);
    //     if (near_view_name_node == NULL) {
    // #ifdef DEBUG
    //         cerr << "\n\n\nError: In DataViewColumnName fixing. Cannot find the "
    //                 "near_view_name from the "
    //                 "statement. More debug info, view column is: "
    //              << ir_to_fix->to_string() << ". \n\n\n";
    // #endif
    //     }
    // string near_view_name_str = near_view_name_node->to_string();
    // vector<string> matched_columns = this->query_instan_data->m_table2columns[near_view_name_str];

    // vector<string> v_new_view_col_name_str;
    //     vector<IR*> v_new_view_column_ir;
    //     int view_col_idx = 0;
    //     for (string cur_matched_columns : matched_columns) {
    //         string new_view_column_name = gen_view_column_name();
    //         v_new_view_col_name_str.push_back(new_view_column_name);
    //         this->query_instan_data->m_column2datatype[new_view_column_name] = new DataAffinity(*this->query_instan_data->m_column2datatype[cur_matched_columns]);
    //         if (this->query_instan_data->m_column2datatype.count(cur_matched_columns)) {
    //             this->query_instan_data->m_datatype2column[this->query_instan_data->m_column2datatype[cur_matched_columns]
    //                                                            ->get_data_affinity()]
    //                 .push_back(new_view_column_name);
    //         }

    //         view_col_idx++;

    //         IR* new_view_column = new IR(SymbolTerm, IRTypeIDENT, new_view_column_name, nullptr);
    //         new_view_column->set_data_type(DataViewColumnName);
    //         new_view_column->set_data_flag(ContextDefine);
    //         new_view_column->set_is_instantiated(true);

    //         v_new_view_column_ir.push_back(new_view_column);

    //         if (view_col_idx != matched_columns.size() - 1) {
    //             IR* sep_node = new IR(SymbolTerm, IRTypeUnknownType, string(","), nullptr);
    //             v_new_view_column_ir.push_back(sep_node);
    //         }

    // #ifdef DEBUG
    //         cerr
    //             << "\n\n\nDependency: INFO:: Transporting data affinity from column: "
    //             << cur_matched_columns << " to view column: " << new_view_column_name
    //             << ", with affinity: "
    //             << get_string_by_data_affi(
    //                    query_instan_data->m_column2datatype[new_view_column_name]->get_data_affinity())
    //             << ". \n\n\n";
    // #endif
    //     }

    // this->query_instan_data->m_table2columns[near_view_name_str] = v_new_view_col_name_str;

    // #ifdef DEBUG
    //     for (string& view_col_name : v_new_view_col_name_str) {
    //         cerr << "\n\n\nDependency: INFO:: Appending new view column: "
    //              << view_col_name << " to view: " << near_view_name_str << ". \n\n\n";
    //     }
    // #endif

    // At last, switch the whole TypeNameList node in the Create View column
    // clause.
    //            ret_str = "(" + ret_str + ")";
    // for (IR*& ori_child : type_name_list->get_children_ref()) {
    //     ir_to_deep_drop.push_back(ori_child);
    //     ori_child->set_is_instantiated(true);
    // }

    // type_name_list->detach_children();

    // type_name_list->set_children_nodes(v_new_view_column_ir);

    return;
}

void PostgreSQLQueryInstantiator::instan_storage_param(IR* ir_to_fix, vector<IR*>& ir_to_deep_drop)
{

    IRTYPE type = ir_to_fix->get_ir_type();
    DATATYPE data_type = ir_to_fix->get_data_type();

    if (data_type == DataStorageParams) {

        // TODO::FIXME
        ir_to_fix->set_is_instantiated(true);

        //        // Do not use param_num == 0;
        //        IR *new_storage_param_node =
        //                this->constr_rand_storage_param(get_rand_int(3) + 1);
        //        new_storage_param_node->set_is_instantiated(true);
        //        opt_storage_params->update_left(new_storage_param_node);
        //        opt_storage_params->update_right(NULL);
    }

    return;
}

void PostgreSQLQueryInstantiator::instan_trigger_name(IR* ir_to_fix, vector<IR*>& ir_to_deep_drop)
{
    /* Fix for kDataTriggerName */
    if (ir_to_fix->get_data_type() == DataTriggerName) {
        ir_to_fix->set_is_instantiated(true);
        if (ir_to_fix->get_data_flag() == ContextDefine) {
            string new_trigger_name = gen_trigger_name();
            ir_to_fix->set_str_val(new_trigger_name);
            v_all_trigger_name.push_back(new_trigger_name);
        } else if (ir_to_fix->get_data_flag() == ContextUndefine) {
            string trigger_to_rov = this->v_all_trigger_name.back();
            this->v_all_trigger_name.pop_back();
            ir_to_fix->set_str_val(trigger_to_rov);
        } else { // (ir_to_fix->get_data_flag() == ContextUse)
            if (this->v_all_trigger_name.size() == 0) {
                ir_to_fix->set_str_val("trigger_any");
                return;
            }
            string trigger_to_use = vector_rand_ele(this->v_all_trigger_name);
            ir_to_fix->set_str_val(trigger_to_use);
        }
    }
}

bool PostgreSQLQueryInstantiator::instan_dependency(IR* cur_stmt_root,
    const vector<vector<IR*>> all_substmt_ir_of_interest)
{

#ifdef DEBUG
    cerr << "Fix_dependency: cur_stmt_root: " << cur_stmt_root->to_string()
         << ", size of cur_stmt_ir_to_fix_vec " << all_substmt_ir_of_interest.size()
         << ". \n\n\n";
    cerr << "Current root: \n";
    cur_stmt_root->debug(cerr, 0);
    cerr << "\n";
    cerr << "Getting all ir_to_fix: \n";
    for (auto v_tmp : all_substmt_ir_of_interest) {
        for (IR* tmp_ir : v_tmp) {
            tmp_ir->debug(cerr, 0);
            cerr << "\n";
        }
        cerr << "-----";
    }
    cerr << "\n\n\n\n\n\n";
#endif

    /* Used to mark the IRs that are needed to be deep_drop(). However, it is not
     * a good idea to deep_drop in the middle of the instan_dependency() function,
     * some ir_to_fix node might have nested IR strcuture. Use this vector to save
     * all IR that needs deep_drop, and drop them at the end of the function.
     * */
    vector<IR*> ir_to_deep_drop;

    this->remove_type_annotation(cur_stmt_root, ir_to_deep_drop);

#ifdef DEBUG
    cerr << "\n\n\nAfter removing the type annotations, getting "
         << cur_stmt_root->to_string() << "\n\n\n";
#endif

    // If set true, meaning we are in an ALTER xxx RENAME xxx statement.
    bool is_replace_table = false, is_replace_column = false;

    for (const vector<IR*>& ir_to_fix_vec : all_substmt_ir_of_interest) {

#define CHECKINIT(X)                \
    if (X->get_is_instantiated()) { \
        continue;                   \
    }

        // First step, DataDatabaseName and DataSchemaName.
        for (IR* ir_to_fix : ir_to_fix_vec) {
            CHECKINIT(ir_to_fix);
            this->instan_database_schema_name(ir_to_fix);
        }

        // Second, handle the table names.
        // table creation
        for (IR* ir_to_fix : ir_to_fix_vec) {
            CHECKINIT(ir_to_fix);
            if (ir_to_fix->get_data_type() == DataTableName && ir_to_fix->get_data_flag() == ContextDefine || ir_to_fix->get_data_flag() == ContextReplaceDefine) {
                this->instan_table_name(ir_to_fix, is_replace_table);
            }
        }
        for (IR* ir_to_fix : ir_to_fix_vec) {
            CHECKINIT(ir_to_fix);
            if (ir_to_fix->get_data_type() == DataTableName && ir_to_fix->get_data_flag() == ContextUndefine || ir_to_fix->get_data_flag() == ContextReplaceUndefine) {
                this->instan_table_name(ir_to_fix, is_replace_table);
            }
        }

        // Third, DataTableAliasName in the WITH clause.
        /* Fix of DataTableAlias name. */
        /* For DataTableAlias name, do not need to
         * handle ContextUse and ContextUndefine situations.
         * i,e. we only need to consider the ContextDefine.
         * After the handling of current SQL statement finished,
         * all info related to this alias should be removed
         * automatically.
         * */
        for (IR* ir_to_fix : ir_to_fix_vec) {
            if (ir_to_fix->get_is_instantiated()) {
                continue;
            }

            if (ir_to_fix->get_data_type() == DataTableAliasName) {
                // If NOT IN WITH clause, do not fix before the Table Name ContextUse.
                if (!this->ir_wrapper.is_ir_in(ir_to_fix, IRTypeWithClause)) {
                    continue;
                }
                // For the WITH clause table alias, the usage is optional.
                this->instan_table_alias_name(ir_to_fix, cur_stmt_root, true);
            }
        }

        // Fourth, Table name ContextUse.
        /* ContextUse of kDataTableName */
        /* The ContextUseFollow will be handled further below. */
        for (IR* ir_to_fix : ir_to_fix_vec) {
            if (ir_to_fix->get_is_instantiated()) {
                continue;
            }

            if (ir_to_fix->get_data_type() == DataTableName && ir_to_fix->get_data_flag() == ContextUse || ir_to_fix->get_data_flag() == ContextUseTop) {
                this->instan_table_name(ir_to_fix, is_replace_table);
            }
        }

        /* Fifth, fix of DataTableAlias name. */
        /* For DataTableAlias name, do not need to
         * handle ContextUse and ContextUndefine situations.
         * i,e. we only need to consider the ContextDefine.
         * After the handling of current SQL statement finished,
         * all info related to this alias should be removed
         * automatically.
         * */
        for (IR* ir_to_fix : ir_to_fix_vec) {
            if (ir_to_fix->get_is_instantiated()) {
                continue;
            }

            // Fix the other aliases outside the WITH clause.
            if (this->ir_wrapper.is_ir_in(ir_to_fix, IRTypeWithClause)) {
                continue;
            }

            if (ir_to_fix->get_data_type() == DataTableAliasName) {
                ir_to_fix->set_is_instantiated(true);
                // For the table alias that is outside the WITH clause, the usage is
                // enforced!
                this->instan_table_alias_name(ir_to_fix, cur_stmt_root, false);
            }
        }

        // FIXME:: May not be used.
        // Sixth, DataTableName again.
        /* ContextUseFollow of DataTableName. */
        /* This scenario searches for table name usage that is in the WHERE clause.
         */
        for (IR* ir_to_fix : ir_to_fix_vec) {
            if (ir_to_fix->get_is_instantiated()) {
                continue;
            }

            if (ir_to_fix->get_data_type() == DataTableName && ir_to_fix->get_data_flag() == ContextUseFollow) {
                this->instan_table_name(ir_to_fix, is_replace_table);
            }
        }

        // Seventh
        /* Fix for kDataViewName. */
        for (IR* ir_to_fix : ir_to_fix_vec) {
            if (ir_to_fix->get_is_instantiated()) {
                continue;
            }
            if (ir_to_fix->get_data_type() == DataViewName) {
                this->instan_view_name(ir_to_fix);
            }
        }

        // eighth
        /* Fix of DataPartitionName. */
        /* ContextDefine, ContextUse and ContextUndefine of DataPartitionName. */
        for (IR* ir_to_fix : ir_to_fix_vec) {
            if (ir_to_fix->get_is_instantiated()) {
                continue;
            }
            if (ir_to_fix->get_data_type() == DataPartitionName) {
                this->instan_partition_name(ir_to_fix);
            }
        }

        // ninth
        /* Fix of kDataIndex name. */
        for (IR* ir_to_fix : ir_to_fix_vec) {
            if (ir_to_fix->get_is_instantiated()) {
                continue;
            }
            this->instan_index_name(ir_to_fix, ir_to_deep_drop);
        }

        // tenth
        /* DataColumn name handling */
        for (IR* ir_to_fix : ir_to_fix_vec) {
            if (ir_to_fix->get_is_instantiated()) {
                continue;
            }

            /* kDefine and kReplace of kDataColumnName */
            if (ir_to_fix->get_data_type() == DataColumnName && (ir_to_fix->get_data_flag() == ContextDefine || ir_to_fix->get_data_flag() == ContextReplaceDefine)) {

                this->instan_column_name(ir_to_fix, cur_stmt_root, is_replace_column,
                    ir_to_deep_drop);

                /* ContextUndefine scenario of the DataColumnName */
            } else if (ir_to_fix->get_data_type() == DataColumnName && (ir_to_fix->get_data_flag() == ContextUndefine || ir_to_fix->get_data_flag() == ContextReplaceUndefine)) {

                this->instan_column_name(ir_to_fix, cur_stmt_root, is_replace_column,
                    ir_to_deep_drop);
            }
        } // for (IR* ir_to_fix : ir_to_fix_vec)

        // eleventh
        /* Fix of DataColumnAlias name.
         * There are two parts of the DataColumnAliasName handling.
         * The first part is inside TypeAliasClause, where the table
         * alias name and column alias name are all provided.
         * The second part is direct column referencing.
         * For the second part, we choose to ignore the mapping,
         * because these cases are not very interesting, and won't
         * reflect on the outputs.
         * */
        for (IR* ir_to_fix : ir_to_fix_vec) {
            if (ir_to_fix->get_is_instantiated()) {
                continue;
            }

            this->instan_column_alias_name(ir_to_fix, cur_stmt_root, ir_to_deep_drop);
        }

        // 12th
        /* Fix the Data Type identifiers. Must be done after ContextDefine of
         * DataColumnName. */
        for (IR* ir_to_fix : ir_to_fix_vec) {
            if (ir_to_fix->get_is_instantiated()) {
                continue;
            }
            IRTYPE type = ir_to_fix->get_ir_type();
            DATATYPE data_type = ir_to_fix->get_data_type();
            DATAFLAG data_flag = ir_to_fix->get_data_flag();

            if (data_type == DataTypeName && data_flag == ContextDefine) {
                // Handling of the Column Data Type definition.
                // Use basic types.

                this->instan_sql_type_name(ir_to_fix);
            }
        }

        // 13
        /* For ContextUse of DataColumnName.
         * Special case, avoid using duplicated column names
         * in the TypeNameList clause.
         * */
        for (IR* ir_to_fix : ir_to_fix_vec) {
            if (ir_to_fix->get_is_instantiated()) {
                continue;
            }
            if (ir_to_fix->get_data_type() == DataColumnName && ir_to_fix->get_data_flag() == ContextUse) {
                this->instan_column_name(ir_to_fix, cur_stmt_root, is_replace_column,
                    ir_to_deep_drop);
            }
        }

        // 14
        /* kUse of kDataColumnName */
        for (IR* ir_to_fix : ir_to_fix_vec) {
            if (ir_to_fix->get_is_instantiated()) {
                continue;
            }

            if (ir_to_fix->get_data_type() == DataColumnName && ir_to_fix->get_data_flag() == ContextUse) {
                this->instan_column_name(ir_to_fix, cur_stmt_root, is_replace_column,
                    ir_to_deep_drop);
            }
        }

        // FIXME::
        //      /* kUse of DataForeignTable */
        //      for (IR *ir_to_fix : ir_to_fix_vec) {
        //          if (ir_to_fix->get_is_instantiated()) {
        //              continue;
        //          }
        //
        //          if (ir_to_fix->data_type_ == kDataForeignTableName &&
        //              ir_to_fix->data_flag_ == ContextDefine) {
        //              this->instan_foreign_table_name(ir_to_fix, is_debug_info);
        //          }
        //
        //      }

        /* Fix function names.  */
        for (IR* ir_to_fix : ir_to_fix_vec) {

            if (ir_to_fix->get_is_instantiated()) {
                continue;
            }

            /* Fixing for functions.  */
            if (ir_to_fix->get_data_type() == DataFunctionExpr) {
                if (ir_to_fix->get_data_flag() == ContextNoModi || ir_to_fix->get_is_instantiated()) {
                    continue;
                }

                /* ATTENTION: This is not used. Ignore function handling for now!!! */
                // instan_func_application(ir_to_fix, cur_stmt_root, ir_to_deep_drop, false);
            }
        }

        for (IR* ir_to_fix : ir_to_fix_vec) {
            if (ir_to_fix->get_is_instantiated()) {
                continue;
            }
            if (ir_to_fix->get_data_type() == DataWindowName) {
                instan_window_name(ir_to_fix);
            }
        }

        /* Fix for statistic and sequence name */
        for (IR* ir_to_fix : ir_to_fix_vec) {
            if (ir_to_fix->get_is_instantiated()) {
                continue;
            }

            if (ir_to_fix->get_data_type() == DataStatsName) {
                this->instan_statistic_name(ir_to_fix);
            }

            /* Fix for kDataSequenceName */
            if (ir_to_fix->get_data_type() == DataSequenceName) {
                this->instan_sequence_name(ir_to_fix);
            }

            /* Fix for kDataConstraintName */
            if (ir_to_fix->get_data_type() == DataConstraintName) {
                this->instan_constraint_name(ir_to_fix);
            }

            /* Fix for DataFamilyName */
            if (ir_to_fix->get_data_type() == DataFamilyName) {
                this->instan_family_name(ir_to_fix);
            }
        }

        /* Fix the Literal inside VALUES clause. */
        for (IR* ir_to_fix : ir_to_fix_vec) {
            if (ir_to_fix->get_is_instantiated()) {
                continue;
            }

            this->instan_literal(ir_to_fix, cur_stmt_root, ir_to_deep_drop);

        } /* for (IR* ir_to_fix : ir_to_fix_vec) */

        /* The next loop to handle all the Literals, after setting all literals to
         * AFFIUNKNOWN. */
        for (IR* ir_to_fix : ir_to_fix_vec) {
            if (ir_to_fix->get_is_instantiated()) {
                continue;
            }

            IRTYPE type = ir_to_fix->get_ir_type();

            if (ir_to_fix->get_data_type() == DataLiteral) {
                /* Continue from the previous loop, we now search around the ir_to_fix
                 * and see if we can find column name or literals that can help deduce
                 * Data Affinity.
                 * */

                this->instan_literal(ir_to_fix, cur_stmt_root, ir_to_deep_drop);
            }
        } /* for (IR* ir_to_fix : ir_to_fix_vec) */

        for (IR* ir_to_fix : ir_to_fix_vec) {
            if (ir_to_fix->get_is_instantiated()) {
                continue;
            }
            if (ir_to_fix->get_data_type() == DataTriggerName) {
                this->instan_trigger_name(ir_to_fix, ir_to_deep_drop);
            }
        }

        // for (IR* ir_to_fix : ir_to_fix_vec) {
        //     if (ir_to_fix->get_is_instantiated()) {
        //         continue;
        //     }
        //
        //     IRTYPE type = ir_to_fix->get_ir_type();
        //     DATATYPE data_type = ir_to_fix->get_data_type();
        //
        //     if (type == IRTypeStorageParameter) {
        //         this->instan_storage_param(ir_to_fix, ir_to_deep_drop);
        //     }
        // }
    } // for (const vector<IR*>& ir_to_fix_vec : all_substmt_ir_of_interest)

    /* For the newly declared v_table_names_single, save all these newly declared
     * statement to the global v_table_names. */

    for (string cur_add_table : this->query_instan_data->v_create_table_names_single) {
        if (!find_vector(this->query_instan_data->v_table_names, cur_add_table)) {
            this->query_instan_data->v_table_names.push_back(cur_add_table);
        }
    }
    for (string cur_add_table : this->query_instan_data->v_create_view_names_single) {
        if (!find_vector(this->query_instan_data->v_table_names, cur_add_table)) {
            this->query_instan_data->v_table_names.push_back(cur_add_table);
        }
    }
    for (string cur_add_table : this->query_instan_data->v_view_name) {
        if (!find_vector(this->query_instan_data->v_view_name, cur_add_table)) {
            this->query_instan_data->v_view_name.push_back(cur_add_table);
        }
    }

    /* Reiterate the substmt.
    ** Added missing dependency information.
    */
    for (const vector<IR*>& ir_to_fix_vec : all_substmt_ir_of_interest) {

        // /* Added mapping for Inheritance.  */
        // for (IR* ir_to_fix : ir_to_fix_vec) {
        //     if (ir_to_fix->get_data_type() == DataTableName && (cur_stmt_root->get_children_ref().size() > 2 && cur_stmt_root->get_children_ref()[0]->get_ir_type() == IRTypeCreatekw) &&
        //         //        p_oracle->ir_wrapper.is_ir_in(ir_to_fix, kOptInherit) &&
        //         ir_to_fix->get_data_flag() == ContextUse) {
        //         if (this->query_instan_data->v_create_table_names_single.size() > 0) {
        //             string cur_new_table_name_str = this->query_instan_data->v_create_table_names_single.front();
        //             string inherit_table_name_str = ir_to_fix->get_str_val();

        //             vector<string>& inherit_m_tables = this->query_instan_data->m_table2columns[inherit_table_name_str];

        //             for (string col_name : inherit_m_tables) {
        //                 vector<string>& cur_col_list = this->query_instan_data->m_table2columns[cur_new_table_name_str];
        //                 if (find(cur_col_list.begin(), cur_col_list.end(), col_name) == cur_col_list.end()) {
        //                     cur_col_list.push_back(col_name);
        //                 }
        //             }
        //         }
        //     }
        // }

        for (IR* ir_to_fix : ir_to_fix_vec) {

            this->map_create_view(ir_to_fix, cur_stmt_root, all_substmt_ir_of_interest);

        } // for (IR* ir_to_fix : ir_to_fix_vec)

        // The second loop that fix the DataViewColumn.
        // Need to rewrite the column mapping.
        for (IR* ir_to_fix : ir_to_fix_vec) {
            if (ir_to_fix->get_data_type() == DataViewColumnName) {
                // if (cur_stmt_root->get_ir_type() != IRTypeViewStmt) {
                //     cerr << "\n\n\nError: Finding DataViewColumnName that is not in the "
                //             "Create View statement. \n\n\n";
                //     continue;
                // }
                this->map_create_view_column(ir_to_fix, ir_to_deep_drop);
            }
        }
    } // for (const vector<IR *> &ir_to_fix_vec : cur_stmt_ir_to_fix_vec)

    // If the instantiation delete the mutating ir, skip the current query.
    bool is_conflict = false;

#ifdef DEBUG
    cerr << "\nBEGIN mysql_query_instantiator debugging. \n\n\n";
#endif

#ifdef DEBUG
    cerr << "\n\nGetting cur_mutating_ir: \n";
    for (auto& cur_mutating_ir : this->all_mutating_irs) {
        cur_mutating_ir->debug(cerr, 0);
        cerr << "\n\n\n";
    }
    cerr << "\n\n\n";

    cerr << "\n\nGetting ir_to_drop: \n";
    for (IR* ir_to_drop : ir_to_deep_drop) {
        if (ir_to_drop) {
            ir_to_drop->debug(cerr, 0);
            cerr << "\n\n\n";
        }
    }
    cerr << "\n\n\n";
#endif

    for (IR* ir_to_drop : ir_to_deep_drop) {
        if (ir_to_drop) {
            for (auto& cur_mutating_ir : this->all_mutating_irs) {
                if (ir_wrapper.is_ir_in(cur_mutating_ir, ir_to_drop) || cur_mutating_ir == ir_to_drop) {
                    is_conflict = true;
                }
            }
        }
    }

    for (IR* ir_to_drop : ir_to_deep_drop) {
        if (ir_to_drop) {
            ir_to_drop->deep_drop();
        }
    }

#ifdef DEBUG
    cerr << "\nEnd mysql_query_instantiator debugging. \n\n\n";
#endif

    return !is_conflict;
}

void PostgreSQLQueryInstantiator::custom_setup_init_query_instan_data()
{
}
