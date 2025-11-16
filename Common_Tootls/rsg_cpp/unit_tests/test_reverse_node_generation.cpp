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

static RSG* rsg = nullptr;
static QueryInstantiator* p_instantiator = nullptr;
static ResultHandler* p_result_handl = nullptr;
static QueryPlanHandler* p_query_plan_handl = nullptr;
static DBMSConnector* p_dbms_connector = nullptr;
static FeedbackMapper* p_feedback_mapper = nullptr;
static IRWrapper* p_ir_wrapper = nullptr;
static QueryImporter* p_query_importer = nullptr;

int main(int argc, char** argv)
{

    cerr << "Initializing. \n\n\n";

    string grammar_str = read_file_to_str(FuzzerConfigurations::grammar_file_path);
    if (grammar_str.empty()) {
        cerr << "Error: Cannot read grammar_str from file: " << FuzzerConfigurations::grammar_file_path << "\n\n\n";
        abort();
    }

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
#error "Error: Cannot recognize dbms_name: neither cockroachdb nor duckdb. "
#endif

    p_instantiator->init_data_library();
    p_instantiator->p_rsg = rsg;

    cerr << "Caching reverse tree. \n\n\n";
    rsg->cache_reverse_tree();

    cout << "Getting reverse cached tree number: " << rsg->m_cached_reverse_tree.size() << "\n";

    for (const auto& tree_pair: rsg->m_cached_reverse_tree) {
        cout << "For key: " << get_string_by_ir_type(tree_pair.first) << ", getting tree num: " << tree_pair.second.size() << ". \n";
    }

    cout << "End showing cached tree numbers. \n\n\n";

    string ir_type_str;
    cout << "Please input IR type string to invoke the reverse generator: ";
    cin >> ir_type_str;
    IRTYPE ir_type = get_ir_type_by_string(ir_type_str);

    cerr << "Using ir_type: " << get_string_by_ir_type(ir_type) << "\n";

    cerr << "Generating different query from cached reverse tree to given IR type. \n\n";

    if (!(rsg->is_cached_reverse_node_contain_type(ir_type))) {
        cerr << "The reverse generator doesn't cache this IR type. Exit. ";
    } else {
        for (int idx = 0; idx < 10; idx++) {
            cerr << "Path " << idx << ":\n";

            IR* res_ir = rsg->get_random_cached_reverse_generated_ir(ir_type, false, -1);

            if (res_ir == nullptr) {
                cerr << "Error: Getting empty res_ir??? \n\n\n";
                abort();
            }

            cerr << "Getting query parts str: " << res_ir->to_string() << "\n";

            cerr << "Debug view: \n";
            res_ir->debug(cerr);
            cerr << "\n";

            cerr << "END\n\n\n";

            res_ir->deep_drop();
        }
    }

    delete p_instantiator;
    delete p_result_handl;
    delete p_query_plan_handl;
    delete p_dbms_connector;
    delete rsg;
    delete p_feedback_mapper;
    delete p_ir_wrapper;
}