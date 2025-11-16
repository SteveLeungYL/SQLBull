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

    string ir_type_str;
    cout << "Please input IR type string: ";
    cin >> ir_type_str;
    IRTYPE ir_type = get_ir_type_by_string(ir_type_str);

    cerr << "Getting ir_type: " << get_string_by_ir_type(ir_type) << "\n";

    ProductionNode* prod_node_for_type = rsg->m_ir_types_2_prods[ir_type].front();
    cerr << "From production node: " << prod_node_for_type->get_name() << "\n\n\n";

    for (auto cur_expr : prod_node_for_type->get_parent_exprs()) {
        cerr << "Getting parent expr: " << cur_expr.first->to_string() << ", with idx:" << cur_expr.second << ", from parent_node: " << cur_expr.first->get_parent_production_node()->get_name() << "\n";
    }

    cerr << "Traversing different path from prod node to root. \n\n";

    int succeed_num = 0;

    unordered_map<IRTYPE, int> v_saved_reverse_types;

    for (int idx = 0; idx < 100; idx++) {
        cerr << "Path " << idx << ":\n";
        auto v_path = rsg->reverse_traverse_prod_node_to_stmt(prod_node_for_type);
        for (int j = 1; j < v_path.size(); j++) {
            auto* p_prod = v_path.get_prod_node_on_idx(j);
            auto* p_expr = v_path.get_expr_node_on_idx(j);
            cerr << p_prod->get_name() << ": " << p_expr->to_string() << "\n";
        }

        if (v_path.is_empty()) {
            cerr << "Path is empty, removed by deduplication? \n";
            continue;
        }

        for (int j = 0; j < v_path.size(); j++) {
            auto cur_sub_node = v_path.get_reverse_node_on_idx(j);
            IRTYPE cur_sub_node_type = v_path.get_ir_type_on_idx(j);
#ifdef DEBUG
            cerr << "In RSG::cache_reverse_generated_ir_nodes(), saving IRTYPE: " << get_string_by_ir_type(cur_ir_type)
                 << " with idx:" << j << " in " << v_p_sub_trees->at(j).first->get_name() << "\n";
#endif
            if (v_saved_reverse_types.count(cur_sub_node_type)) {
                v_saved_reverse_types.at(cur_sub_node_type)++;
            } else {
                v_saved_reverse_types[cur_sub_node_type] = 1;
            }
        }

        cerr << "And then, try to mutate the reverse tree: \n";
        vector<ReverseTree*> res_mutated_trees = rsg->mutate_reverse_tree(&v_path);
        cerr << "Getting mutated tree size: " << res_mutated_trees.size() << "\n\n";

        for (auto* cur_mutated_tree : res_mutated_trees) {
            cerr << "Getting mutated tree: \n" << cur_mutated_tree->debug_print() << "\n\n";
            for (int j = 0; j < cur_mutated_tree->size(); j++) {
                auto cur_sub_node = cur_mutated_tree->get_reverse_node_on_idx(j);
                IRTYPE cur_sub_node_type = cur_mutated_tree->get_ir_type_on_idx(j);
#ifdef DEBUG
                cerr << "In RSG::cache_reverse_generated_ir_nodes(), saving IRTYPE: " << get_string_by_ir_type(cur_ir_type)
                     << " with idx:" << j << " in " << v_p_sub_trees->at(j).first->get_name() << "\n";
#endif
                if (v_saved_reverse_types.count(cur_sub_node_type)) {
                    v_saved_reverse_types.at(cur_sub_node_type)++;
                } else {
                    v_saved_reverse_types[cur_sub_node_type] = 1;
                }
            }
        }

        cerr << "End reverse tree mutation. \n\n\n";

        cerr << "\nGenerating IR based on the subtree; \n";

        IR* ir_root = rsg->reverse_generate_ir_from_prod(prod_node_for_type, -1, &v_path, static_cast<int>(v_path.size()));
        auto* cur_stmt = new QueryStmt(ir_root);
        p_instantiator->reset_data_library();
        p_instantiator->fill_one_stmt(cur_stmt, 0);

        cerr << ir_root->to_string() << ";\n";

        cerr << "END\n\n\n";

        succeed_num++;
        delete cur_stmt;
        for (auto* cur_mutated_tree: res_mutated_trees) {
            delete cur_mutated_tree;
        }
    }

    cerr << "\n\nAt last, generating " << succeed_num << "/100 reverse tree matching ir type: " << get_string_by_ir_type(ir_type) << "\n\n";
    #if defined(mariadb)
    if (v_saved_reverse_types.count(IRTypeSelect)) {
        cerr << "Getting number of select_stmt: " << v_saved_reverse_types[IRTypeSelect] << "\n\n";
    #else
    if (v_saved_reverse_types.count(IRTypeSelectStmt)) {
        cerr << "Getting number of select_stmt: " << v_saved_reverse_types[IRTypeSelectStmt] << "\n\n";
    #endif
    } else {
        cerr << "No reverse tree touching SELECT stmt. \n\n\n";
    }

    delete p_instantiator;
    delete p_result_handl;
    delete p_query_plan_handl;
    delete p_dbms_connector;
    delete rsg;
    delete p_feedback_mapper;
    delete p_ir_wrapper;
}