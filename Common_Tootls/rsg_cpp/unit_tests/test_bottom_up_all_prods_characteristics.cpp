//
// Created by XXX on 5/20/24.
//

#include "../headers/config.h"
#include "../headers/dbms_connector.h"
#include "../headers/debug.h"
#include "../headers/feedback_mapper.h"
#include "../headers/fuzzer_configurations.h"
#include "../headers/node.h"
#include "../headers/query_importer.h"
#include "../headers/query_instantiator.h"
#include "../headers/query_plan_handl.h"
#include "../headers/results_handler.h"
#include "../headers/rsg.h"
#include "../headers/types.h"
#include "../headers/utils.h"

#include <unordered_map>
#include <algorithm>

// DBMS specific
#if defined(cockroachdb)
#include "../dbms_specific/cockroachdb/cockroachdb_common.h"
#elif defined(duckdb)
#include "../dbms_specific/duckdb/duckdb_common.h"
#elif defined(sqlite)
#include "../dbms_specific/sqlite/sqlite_common.h"
#elif defined(mysqldb)
#include "../dbms_specific/mysqldb/mysql_common.h"
#elif defined(mariadb)
#include "../dbms_specific/mariadb/mariadb_common.h"
#elif defined(postgresql)
#include "../dbms_specific/postgresql/postgresql_common.h"
#endif

using namespace std;

static RSG* rsg = nullptr;
static QueryInstantiator* p_instantiator = nullptr;
static ResultHandler* p_result_handl = nullptr;
static QueryPlanHandler* p_query_plan_handl = nullptr;
static DBMSConnector* p_dbms_connector = nullptr;
static FeedbackMapper* p_feedback_mapper = nullptr;
static IRWrapper* p_ir_wrapper = nullptr;
static QueryImporter* p_query_importer = nullptr;

size_t get_avg_from_vector(const vector<size_t>& v_in)
{
    if (v_in.empty()) {
        return 0;
    }
    size_t sum = 0;
    for (auto& cur_size : v_in) {
        sum += cur_size;
    }
    return sum / v_in.size();
}

size_t get_median_from_vector(vector<size_t>& v_in)
{
    if (v_in.empty()) {
        return 0;
    }
    sort(v_in.begin(), v_in.end());
    return v_in[v_in.size() / 2];
}

size_t get_max_from_vector(const vector<size_t>& v_in)
{
    if (v_in.empty()) {
        return 0;
    }
    return *max_element(v_in.begin(), v_in.end());
}

size_t get_min_from_vector(const vector<size_t>& v_in)
{
    if (v_in.empty()) {
        return 0;
    }
    return *min_element(v_in.begin(), v_in.end());
}

int main(int argc, char** argv)
{

    /* Finish setup RSG; */
    string grammar_str = read_file_to_str(FuzzerConfigurations::grammar_file_path);
    if (grammar_str.empty()) {
        cerr << "Error: Cannot read grammar_str from file: " << FuzzerConfigurations::grammar_file_path << "\n\n\n";
        abort();
    }

    cerr << "Initializing: \n\n";

#ifdef cockroachdb
    p_ir_wrapper = new CockroachDBIRWrapper();
    p_query_importer = new QueryImporter();
    rsg = get_new_rsg(grammar_str, "cockroachdb", "stmt_without_legacy_transaction", FuzzerConfigurations::epsilon, FuzzingMode::FuzzingModeNormal,
        p_query_importer,
        cockroachdb_comp_expr_filter,
        cockroachdb_keyword_handl, remove_unimpl_cockroachdb,
        cockroachdb_comp_rule_terminator,
        cockroachdb_ir_context_setup,
        p_ir_wrapper);
    p_instantiator = new CockroachDBQueryInstantiator();
    p_result_handl = new CockroachDBResultHandler();
    p_query_plan_handl = new CockroachDBQueryPlanHandl();
    p_dbms_connector = new CockroachDBConnector(p_result_handl);
    p_feedback_mapper = new FeedbackMapper(
        rsg, p_ir_wrapper, p_dbms_connector, p_instantiator);
#elif defined(duckdb)
    p_ir_wrapper = new DuckDBIRWrapper();
    p_query_importer = new QueryImporter();
    rsg = get_new_rsg(grammar_str, "duckdb", "stmt", FuzzerConfigurations::epsilon, FuzzingMode::FuzzingModeNormal,
        p_query_importer,
        duckdb_comp_expr_filter,
        duckdb_keyword_handl, remove_unimpl_duckdb,
        duckdb_comp_rule_terminator,
        duckdb_ir_context_setup,
        p_ir_wrapper);
    p_instantiator = new DuckDBQueryInstantiator();
    p_result_handl = new DuckDBResultHandler();
    p_query_plan_handl = new DuckDBQueryPlanHandl();
    p_dbms_connector = new DuckDBConnector(p_result_handl);
    p_feedback_mapper = new FeedbackMapper(
        rsg, p_ir_wrapper, p_dbms_connector, p_instantiator);
#elif defined(sqlite)
    p_ir_wrapper = new SQLiteIRWrapper();
    p_query_importer = new QueryImporter();
    rsg = get_new_rsg(grammar_str, "sqlite", "cmd", FuzzerConfigurations::epsilon, FuzzingMode::FuzzingModeNormal,
        p_query_importer,
        sqlite_comp_expr_filter,
        sqlite_keyword_handl, remove_unimpl_sqlite,
        sqlite_comp_rule_terminator,
        sqlite_ir_context_setup,
        p_ir_wrapper);
    p_instantiator = new SQLiteQueryInstantiator();
    p_result_handl = new SQLiteResultHandler();
    p_query_plan_handl = new SQLiteQueryPlanHandl();
    p_dbms_connector = new SQLiteConnector(p_result_handl);
    p_feedback_mapper = new FeedbackMapper(
        rsg, p_ir_wrapper, p_dbms_connector, p_instantiator);
#elif defined(mysqldb)
    p_ir_wrapper = new MySQLIRWrapper();
    p_query_importer = new QueryImporter();
    rsg = get_new_rsg(grammar_str, "mysql", "simple_statement", FuzzerConfigurations::epsilon, FuzzingMode::FuzzingModeNormal,
        p_query_importer,
        mysql_comp_expr_filter,
        mysql_keyword_handl, remove_unimpl_mysql,
        mysql_comp_rule_terminator,
        mysql_ir_context_setup,
        p_ir_wrapper);
    p_instantiator = new MySQLQueryInstantiator();
    p_result_handl = new MySQLResultHandler();
    p_query_plan_handl = new MySQLQueryPlanHandl();
    p_dbms_connector = new MySQLConnector(p_result_handl);
    p_feedback_mapper = new FeedbackMapper(
        rsg, p_ir_wrapper, p_dbms_connector, p_instantiator); // use the default one.
#elif defined(mariadb)
    p_ir_wrapper = new MariaDBIRWrapper();
    p_query_importer = new QueryImporter();
    rsg = get_new_rsg(grammar_str, "mariadb", "verb_clause", FuzzerConfigurations::epsilon, FuzzingMode::FuzzingModeNormal,
        p_query_importer,
        mariadb_comp_expr_filter,
        mariadb_keyword_handl, remove_unimpl_mariadb,
        mariadb_comp_rule_terminator,
        mariadb_ir_context_setup,
        p_ir_wrapper);
    p_instantiator = new MariaDBQueryInstantiator();
    p_result_handl = new MariaDBResultHandler();
    p_query_plan_handl = new MariaDBQueryPlanHandl();
    p_dbms_connector = new MariaDBConnector(p_result_handl);
    p_feedback_mapper = new FeedbackMapper(
        rsg, p_ir_wrapper, p_dbms_connector, p_instantiator); // use the default one.
#elif defined(postgresql)
    p_ir_wrapper = new PostgreSQLIRWrapper();
    p_query_importer = new QueryImporter();
    rsg = get_new_rsg(grammar_str, "postgresql", "stmt", FuzzerConfigurations::epsilon, FuzzingMode::FuzzingModeNormal,
        p_query_importer,
        postgresql_comp_expr_filter,
        postgresql_keyword_handl, remove_unimpl_postgresql,
        postgresql_comp_rule_terminator,
        postgresql_ir_context_setup,
        p_ir_wrapper);
    p_instantiator = new PostgreSQLQueryInstantiator();
    p_result_handl = new PostgreSQLResultHandler();
    p_query_plan_handl = new PostgreSQLQueryPlanHandl();
    p_dbms_connector = new PostgreSQLConnector(p_result_handl);
    p_feedback_mapper = new FeedbackMapper(
        rsg, p_ir_wrapper, p_dbms_connector, p_instantiator); // use the default one.
#else
#error "Error: Cannot recognize dbms_name: either cockroachdb nor duckdb. "
#endif

    p_instantiator->init_data_library();
    p_instantiator->p_rsg = rsg;

    unordered_map<ProductionNode*, vector<size_t>> v_saved_bottom_up_size;

    for (auto& m_prod : rsg->m_all_prods_str) {
        for (int idx = 0; idx < 100; idx++) {
            auto v_path = rsg->reverse_traverse_prod_node_to_stmt(m_prod.second.front());
            // for (int j = 1; j < v_path.size(); j++) {
            //     auto* p_prod = v_path.get_prod_node_on_idx(j);
            //     auto* p_expr = v_path.get_expr_node_on_idx(j);
            //     cerr << p_prod->get_name() << ": " << p_expr->to_string() << "\n";
            // }

            if (v_path.is_empty()) {
                // cerr << "Path is empty, removed by deduplication? \n";
                continue;
            }

            // Not applying mutation here. 
            v_saved_bottom_up_size[m_prod.second.front()].push_back(v_path.size());
        }
    }

    vector<size_t> v_avg_depths;
    vector<size_t> v_min_depths;
    vector<size_t> v_max_depths;
    vector<size_t> v_median_depths;
    for (auto& m_prod : rsg->m_all_prods_str) {
        cerr << "Production node: " << m_prod.second.front()->get_name() << ", with \nvector size: " << v_saved_bottom_up_size[m_prod.second.front()].size() <<  "\nbottom up avg size: " << get_avg_from_vector(v_saved_bottom_up_size[m_prod.second.front()]) << ", \nmedian size: " << get_median_from_vector(v_saved_bottom_up_size[m_prod.second.front()]) << ", \nmax size: " << get_max_from_vector(v_saved_bottom_up_size[m_prod.second.front()]) << ", \nmin size: " << get_min_from_vector(v_saved_bottom_up_size[m_prod.second.front()]) << "\n\n\n";
        v_avg_depths.push_back(get_avg_from_vector(v_saved_bottom_up_size[m_prod.second.front()]));
        v_min_depths.push_back(get_min_from_vector(v_saved_bottom_up_size[m_prod.second.front()]));
        v_max_depths.push_back(get_max_from_vector(v_saved_bottom_up_size[m_prod.second.front()]));
        v_median_depths.push_back(get_median_from_vector(v_saved_bottom_up_size[m_prod.second.front()]));
    }

    cerr << "\n\n\nFor all: Average avg depth: " << get_avg_from_vector(v_avg_depths) << "\n\n\n";
    cerr << "\n\n\nFor all: Average min depth: " << get_avg_from_vector(v_min_depths) << "\n\n\n";
    cerr << "\n\n\nFor all: Average max depth: " << get_avg_from_vector(v_max_depths) << "\n\n\n";
    cerr << "\n\n\nFor all: Average median depth: " << get_avg_from_vector(v_median_depths) << "\n\n\n";    

    cerr << "These above average of the min depths: \n";
    for (auto& m_prod : rsg->m_all_prods_str) {
        if (get_min_from_vector(v_saved_bottom_up_size[m_prod.second.front()]) > get_avg_from_vector(v_min_depths)) {
            cerr << m_prod.first << "\n";
        }
    }
    cerr << "\n\n\n";

    delete p_instantiator;
    delete p_result_handl;
    delete p_query_plan_handl;
    delete p_dbms_connector;
    delete rsg;
    delete p_feedback_mapper;
    delete p_ir_wrapper;
}