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

static unsigned global_instan_idx = 0;

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

#if defined(mysqldb) || defined(mariadb) || defined(postgresql)
const auto first_table_create_stmt = QueryStmt("CREATE TABLE v00 (c01 INT, c02 TEXT);");
const auto first_index_create_stmt = QueryStmt("CREATE INDEX i03 ON v00 (c01);");
#else
const auto first_table_create_stmt = QueryStmt("CREATE TABLE v00 (c01 INT, c02 STRING);");
const auto first_index_create_stmt = QueryStmt("CREATE INDEX i03 ON v00 (c01, c02);");
#endif

const auto first_insert_stmt = QueryStmt("INSERT INTO v00 (c01, c02) VALUES (0, 'abc');");

static RSG* rsg = nullptr;
static QueryInstantiator* p_instantiator = nullptr;
static ResultHandler* p_result_handl = nullptr;
static QueryPlanHandler* p_query_plan_handl = nullptr;
static DBMSConnector* p_dbms_connector = nullptr;
static FeedbackMapper* p_feedback_mapper = nullptr;
static IRWrapper* p_ir_wrapper = nullptr;
static QuerySequenceGenerator* p_init_query_sequence_gen = nullptr;
static QueryImporter* p_query_importer = nullptr;

char *argv[] = { (char*)"sqlite", NULL };

void pre_run_setup(DBMSConnector* p_dbms_connector) {

    auto* query_seq_gen = p_init_query_sequence_gen->deep_copy();

    // Execute the already existing good query statements.
    // Pre-inserted by the query sequence generator.
    for (auto* p_pre_init_stmt: query_seq_gen->p_query_sequence->get_good_query_stmts() ) {
        p_dbms_connector->run_target(argv, p_pre_init_stmt->to_string(), 0);
        p_pre_init_stmt->res_str = p_result_handl->get_tmp_cur_res();
        p_dbms_connector->record_code_coverage(argv);
        p_dbms_connector->has_new_bits(p_dbms_connector->get_virgin_bits(), p_pre_init_stmt->to_string());
    }

    auto* first_table_create_stmt_tmp = first_table_create_stmt.deep_copy();
    query_seq_gen->p_query_sequence->append_good_stmt(first_table_create_stmt_tmp);
    p_dbms_connector->run_target(argv, first_table_create_stmt.to_string(), 0);
    first_table_create_stmt_tmp->res_str = p_result_handl->get_tmp_cur_res();
    p_dbms_connector->record_code_coverage(argv);
    p_dbms_connector->has_new_bits(p_dbms_connector->get_virgin_bits(), first_table_create_stmt.to_string());

    auto* first_index_create_stmt_tmp = first_index_create_stmt.deep_copy();
    query_seq_gen->p_query_sequence->append_good_stmt(first_index_create_stmt_tmp);
    p_dbms_connector->run_target(argv, first_index_create_stmt.to_string(), 0);
    first_index_create_stmt_tmp->res_str = p_result_handl->get_tmp_cur_res();
    p_dbms_connector->record_code_coverage(argv);
    p_dbms_connector->has_new_bits(p_dbms_connector->get_virgin_bits(), first_index_create_stmt.to_string());

    auto* first_insert_stmt_tmp = first_insert_stmt.deep_copy();
    query_seq_gen->p_query_sequence->append_good_stmt(first_insert_stmt_tmp);
    p_dbms_connector->run_target(argv, first_insert_stmt.to_string(), 0);
    first_insert_stmt_tmp->res_str = p_result_handl->get_tmp_cur_res();
    p_dbms_connector->record_code_coverage(argv);
    p_dbms_connector->has_new_bits(p_dbms_connector->get_virgin_bits(), first_insert_stmt.to_string());

}

int main(int argc, char** dump_argv)
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
    p_init_query_sequence_gen = new CockroachDBQuerySequenceGenerator();
    p_feedback_mapper = new FeedbackMapper(
        rsg, p_ir_wrapper, p_dbms_connector, p_instantiator);
    p_dbms_connector->setup_actual_shm(); // Use the shared memory to communicate the code coverage between sqlite
                                    // and this fuzzer. This is the preferred and traditional way.
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
    p_init_query_sequence_gen = new DuckDBQuerySequenceGenerator();
    p_feedback_mapper = new FeedbackMapper(
        rsg, p_ir_wrapper, p_dbms_connector, p_instantiator);
    p_dbms_connector->setup_actual_shm(); // Use the shared memory to communicate the code coverage between sqlite
                                    // and this fuzzer. This is the preferred and traditional way.
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
    p_init_query_sequence_gen = new SQLiteQuerySequenceGenerator();
    p_feedback_mapper = new FeedbackMapper(
        rsg, p_ir_wrapper, p_dbms_connector, p_instantiator);
    p_dbms_connector->setup_actual_shm(); // Use the shared memory to communicate the code coverage between sqlite
                                    // and this fuzzer. This is the preferred and traditional way.
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
    p_init_query_sequence_gen = new MySQLQuerySequenceGenerator();
    p_dbms_connector->setup_actual_shm(); // Use the shared memory to communicate the code coverage between mysql
                                          // and this fuzzer. This is the preferred and traditional way.
    p_dbms_connector->set_socket_path("/tmp/mysql.sock");
    fstream pid_file("./pid_pass_to_fuzzer", ios::out);
    pid_file << "0";
    pid_file.close();
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
    p_init_query_sequence_gen = new MariaDBQuerySequenceGenerator();
    p_dbms_connector->setup_actual_shm(); // Use the shared memory to communicate the code coverage between mysql
                                          // and this fuzzer. This is the preferred and traditional way.
    p_dbms_connector->set_socket_path("/tmp/mysql.sock");
    fstream pid_file("./pid_pass_to_fuzzer", ios::out);
    pid_file << "0";
    pid_file.close();
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
    p_init_query_sequence_gen = new PostgreSQLQuerySequenceGenerator();
    p_dbms_connector->setup_actual_shm(); // Use the shared memory to communicate the code coverage between mysql
                                          // and this fuzzer. This is the preferred and traditional way.
#else
#error "Error: Cannot recognize dbms_name. "
#endif

    p_dbms_connector->set_mem_limit(0);
    // p_dbms_connector->set_doc_path(doc_path);
    p_dbms_connector->set_uses_asan(1);
    p_dbms_connector->set_exec_tmout(4000);
    // p_dbms_connector->set_dump_library(dump_library);

    cerr << "Caching reverse tree. \n\n\n";
    rsg->cache_reverse_tree();

    cout << "Getting reverse cached tree number: " << rsg->m_cached_reverse_tree.size() << "\n";

    for (const auto& tree_pair: rsg->m_cached_reverse_tree) {
        cout << "For key: " << get_string_by_ir_type(tree_pair.first) << ", getting tree num: " << tree_pair.second.size() << ". \n";
    }

    vector<QueryStmt*> v_stmt_list;

    p_instantiator->init_data_library();
    p_instantiator->query_instan_data = new QueryInstantiatorData();
    p_instantiator->p_rsg = rsg;

    // We are actually running this query. 
    p_dbms_connector->restart_dbms(argv);

    pre_run_setup(p_dbms_connector);

    QueryStmt* gen_stmt = nullptr;

    int gen_fail_idx = 0;
    while (true) {

        p_instantiator->reset_data_library();
        gen_stmt = rsg->generate_complete_new_stmt_with_stmt_type(in_type);

        cerr << "From the first generation, getting the following statement: before instantiation: " << gen_stmt->to_string() << "\n\n\n\n";

        p_instantiator->fill_one_stmt(gen_stmt, static_cast<int>(global_instan_idx));
        // Rolling the global_instan_idx to the current instantiation index.
        global_instan_idx += 100;

        cerr << "After instantiation, getting the following statement: " << gen_stmt->to_string() << "\n\n\n\n";

        cerr << "Debug view: \n";
        gen_stmt->stmt_ir->debug(cerr, 0);
        cerr << "\n\n\n\n";

        int ret_res = p_dbms_connector->run_target(argv,
                gen_stmt->to_string(), 0);
        gen_stmt->res_str = p_result_handl->get_tmp_cur_res();

        ResultType res_type = p_result_handl->check_results(gen_stmt->res_str);

        if (res_type == ResultError) {
            cerr << "Error: The generated statement is not valid. \n";
            cerr << "Results: \n" << gen_stmt->res_str << "\n\n\n\n";
            delete gen_stmt;
            gen_stmt = nullptr;
            gen_fail_idx++;

            if (gen_fail_idx > 10) {
                cerr << "Error: Failed to generate a valid statement after 10 attempts. \n";
                return 0;
            }
            continue;
        } else {
            cerr << "The generated statement is valid. \n";
            cerr << "Results: \n" << gen_stmt->res_str << "\n\n\n\n";

            gen_stmt->query_instan_data = p_instantiator->get_query_instan_data()->deep_copy();
            break;
        }
    }

    v_stmt_list.push_back(gen_stmt);

    auto* cur_stmt_of_interest = gen_stmt;

    cerr << "Apply mutations on gen statement. \n\n\n\n";

    for (int i = 0; i < 10; i++) {
        p_instantiator->reset_data_library();
        p_dbms_connector->restart_dbms(argv);

        pre_run_setup(p_dbms_connector);

        auto* mutating_stmt = rsg->mutate_on_input_stmt_ir(cur_stmt_of_interest->stmt_ir->deep_copy(), 0);

        if (mutating_stmt->gen_method != GenMut) {
            delete mutating_stmt;
            continue;
        }

        mutating_stmt->query_instan_data = cur_stmt_of_interest->query_instan_data->deep_copy();

        cerr << "Mutation results " << i << ": " << mutating_stmt->to_string() << "\n\n\n\n";

        p_instantiator->fill_partial_stmt(mutating_stmt, static_cast<int>(global_instan_idx));

        // Rolling the global_instan_idx to the current instantiation index.
        global_instan_idx += 100;

        cerr << "After instantiation: " << i << ", getting the following statement: " << mutating_stmt->to_string() << "\n\n\n\n";

        cerr << "Debug view: \n";
        mutating_stmt->stmt_ir->debug(cerr, 0);

        p_dbms_connector->run_target(argv, mutating_stmt->to_string(), 0);

        mutating_stmt->res_str = p_result_handl->get_tmp_cur_res();

        ResultType res_type = p_result_handl->check_results(mutating_stmt->res_str);

        if (res_type == ResultError) {
            cerr << "Error: The mutated statement is not valid. \n";
            cerr << "Results: \n" << mutating_stmt->res_str << "\n\n\n\n";
            delete mutating_stmt;
        } else {
            cerr << "The mutated statement is valid. \n";
            cerr << "Results: \n" << mutating_stmt->res_str << "\n\n\n\n";
            v_stmt_list.push_back(mutating_stmt);
            cur_stmt_of_interest = mutating_stmt;
            if (cur_stmt_of_interest->query_instan_data != nullptr) {
                delete cur_stmt_of_interest->query_instan_data;
                cur_stmt_of_interest->query_instan_data = nullptr;
            }
            cur_stmt_of_interest->query_instan_data = p_instantiator->get_query_instan_data()->deep_copy();
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
