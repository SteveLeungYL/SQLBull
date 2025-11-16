//
// Created by XXX on 5/20/24.
//

#include "../headers/config.h"
#include "../headers/dbms_connector.h"
#include "../headers/debug.h"
#include "../headers/feedback_mapper.h"
#include "../headers/fuzzer_configurations.h"
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

    /* Finish setup RSG; */
    string grammar_str = read_file_to_str(FuzzerConfigurations::grammar_file_path);
    if (grammar_str.empty()) {
        cerr << "Error: Cannot read grammar_str from file: " << FuzzerConfigurations::grammar_file_path << "\n\n\n";
        abort();
    }

    string in_type_str = "";
    cout << "Input type: \n";
    cin >> in_type_str;
    IRTYPE in_type = get_ir_type_by_string(in_type_str);

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
    p_instantiator->query_instan_data = new QueryInstantiatorData();
    p_instantiator->p_rsg = rsg;

    for (int i = 0; i < 10; i++) {
        QueryStmt* gen_stmt = rsg->generate_complete_new_stmt_with_stmt_type(in_type);

        p_instantiator->reset_data_library();
        p_instantiator->fill_one_stmt(gen_stmt, 0);
        if (gen_stmt->stmt_ir != nullptr) {
            cout << "Getting generated IR function expr: " << gen_stmt->stmt_ir->to_string() << "\n";
        } else {
            cerr << "Error: Getting nullptr. \n";
        }
        delete gen_stmt;
    }

    delete p_instantiator;
    delete p_result_handl;
    delete p_query_plan_handl;
    delete p_dbms_connector;
    delete rsg;
    delete p_feedback_mapper;
    delete p_ir_wrapper;
}
