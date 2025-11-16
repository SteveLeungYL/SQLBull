//
// Created by XXX on 3/12/24.
//

#ifndef RSG_CPP_FUZZER_CONFIGURATIONS_H
#define RSG_CPP_FUZZER_CONFIGURATIONS_H

#include "ir_types_common.h"
#include <vector>

namespace FuzzerConfigurations {
constexpr static unsigned int GrammarRuleCatScanDepth = 10;
constexpr static char keyword_mapping_file_path[] = "./ir_types_mapping.txt";
#ifdef duckdb
constexpr static int rsg_generator_comp_depth = 0;
#elif defined(sqlite)
constexpr static int rsg_generator_comp_depth = 2;
#elif defined(cockroachdb)
constexpr static int rsg_generator_comp_depth = 1;
#elif defined(mysqldb)
constexpr static int rsg_generator_comp_depth = 1;
#elif defined(mariadb)
constexpr static int rsg_generator_comp_depth = 1;
#elif defined(postgresql)
constexpr static int rsg_generator_comp_depth = 1;
#else
constexpr static int rsg_generator_comp_depth = 1;
#endif
constexpr static double epsilon = 0.3;

// Hyper-parameters.
/*
 * pct_mutate_from_existing_saved_query: When appending ONE query to an existing query sequence, whether reuse and mutate from existing query or gen new from scratch.
 * pct_mutate_from_saved_interesting_irs: In mutation, whether to use random rule traversal or use previously cached IRs (or reverse tree, based on pct_mutate_existing_sample_from_reversed_cached_tree).
 * pct_gen_new_from_reversed_cached_tree: When generating completely new statement, whether to use reverse tree.
 * pct_mutate_existing_sample_from_reversed_cached_tree: When mutating on an existing query, percentage to directly connect to an existing reverse tree.
 * pct_gen_from_expr_using_reversed_cached_tree: When randomly generating queries from tree, the percentage to use a reverse tree if the current generating node matching one reverse tree.
        This number should not be too big, otherwise, the reverse tree will dominate the random generation, leaving no space to random rule traversal.
 * pct_reverse_prioritize_term_stmt (Deprecated): Doesn't seem to be making sense. (Deprecated)
 * num_reverse_tree_mutation: When mutating one reverse tree, how many times (energy) do you allocate to mutate each node of the tree.
 * reverse_maximum_depth: The maximum depth of the reverse tree. If exceeded, will not use or save.
 * pct_random_choose_rule: The percentage when randomly traversing syntax rule, whether to use random mutation or prioritize terminating rules (even if depth is not reach) (or whether to NOT use MAB, if applied.)
 * pct_use_additive_mutation: Percentage to prioritize additive mutation.
 * pct_use_favor_node: Percentage to prioritize mutating syntax that marked as favored, only used in mutation.
 */

#ifdef duckdb
constexpr static int pct_mutate_from_existing_saved_query = 70;
constexpr static int pct_mutate_from_saved_interesting_irs = 50;
constexpr static int pct_gen_new_from_reversed_cached_tree = 60;
constexpr static int pct_mutate_existing_sample_from_reversed_cached_tree = 30;
constexpr static int pct_gen_from_expr_using_reversed_cached_tree = 3;
constexpr static int pct_reverse_prioritize_term_stmt = 90;
constexpr static int num_reverse_tree_mutation = 3;
constexpr static int reverse_maximum_depth = 20;
#elif defined(mysqldb)
constexpr static int pct_mutate_from_existing_saved_query = 70;
constexpr static int pct_mutate_from_saved_interesting_irs = 30;
constexpr static int pct_gen_new_from_reversed_cached_tree = 40;
constexpr static int pct_mutate_existing_sample_from_reversed_cached_tree = 20;
constexpr static int pct_gen_from_expr_using_reversed_cached_tree = 60;
constexpr static int pct_reverse_prioritize_term_stmt = 90;
constexpr static int num_reverse_tree_mutation = 10;
constexpr static int reverse_maximum_depth = 20;
#elif defined(mariadb)
constexpr static int pct_mutate_from_existing_saved_query = 70;
constexpr static int pct_mutate_from_saved_interesting_irs = 30;
constexpr static int pct_gen_new_from_reversed_cached_tree = 40;
constexpr static int pct_mutate_existing_sample_from_reversed_cached_tree = 20;
constexpr static int pct_gen_from_expr_using_reversed_cached_tree = 60;
constexpr static int pct_reverse_prioritize_term_stmt = 90;
constexpr static int num_reverse_tree_mutation = 10;
constexpr static int reverse_maximum_depth = 20;
#elif defined(postgresql)
constexpr static int pct_mutate_from_existing_saved_query = 70;
constexpr static int pct_mutate_from_saved_interesting_irs = 30;
constexpr static int pct_gen_new_from_reversed_cached_tree = 40;
constexpr static int pct_mutate_existing_sample_from_reversed_cached_tree = 20;
constexpr static int pct_gen_from_expr_using_reversed_cached_tree = 60;
constexpr static int pct_reverse_prioritize_term_stmt = 90;
constexpr static int num_reverse_tree_mutation = 10;
constexpr static int reverse_maximum_depth = 20;
#elif defined(sqlite)
constexpr static int pct_mutate_from_existing_saved_query = 70;
constexpr static int pct_mutate_from_saved_interesting_irs = 50;
constexpr static int pct_gen_new_from_reversed_cached_tree = 60;
constexpr static int pct_mutate_existing_sample_from_reversed_cached_tree = 30;
constexpr static int pct_gen_from_expr_using_reversed_cached_tree = 3;
constexpr static int pct_reverse_prioritize_term_stmt = 20; // This value should be low for SQLite.
constexpr static int num_reverse_tree_mutation = 3;
constexpr static int reverse_maximum_depth = 20;
#elif defined(cockroachdb)
constexpr static int pct_mutate_from_existing_saved_query = 50;
constexpr static int pct_mutate_from_saved_interesting_irs = 50;
constexpr static int pct_gen_new_from_reversed_cached_tree = 90;
constexpr static int pct_mutate_existing_sample_from_reversed_cached_tree = 30;
constexpr static int pct_gen_from_expr_using_reversed_cached_tree = 3;
constexpr static int pct_reverse_prioritize_term_stmt = 90;
constexpr static int num_reverse_tree_mutation = 3;
constexpr static int reverse_maximum_depth = 20;
#endif
constexpr static int pct_random_choose_rule = 30;
constexpr static int pct_use_additive_mutation = 70;
constexpr static int pct_use_favor_node = 70;
constexpr static int pct_use_additive_mutation_preferred = 50;

#if defined(cockroachdb)
constexpr static char grammar_file_path[] = "./cockroach_sql_modi.y";
#elif defined(duckdb)
constexpr static char grammar_file_path[] = "./duckdb_grammar_modi.y";
#elif defined(sqlite)
constexpr static char grammar_file_path[] = "./sqlite_parser.y";
#elif defined(mysqldb)
constexpr static char grammar_file_path[] = "./mysql_grammar_modi.y";
#elif defined(mariadb)
constexpr static char grammar_file_path[] = "./mariadb_grammar_modi.y";
#elif defined(postgresql)
constexpr static char grammar_file_path[] = "./postgresql_gram_modi.y";
#endif

constexpr static char FUNCTION_TYPE_PATH[] = "./function_type_lib.json";
constexpr static char SET_SESSION_PATH[] = "./set_session_variables.json";
constexpr static char STORAGE_PARAM_PATH[] = "./storage_parameter.json";

const static std::vector<DATATYPE> DataType2Instantiate {
    DataColumnName, DataTableName, DataIndexName,
    DataTableAliasName, DataColumnAliasName, DataSequenceName,
    DataViewName, DataConstraintName, DataSequenceName,
    DataTypeName, DataLiteral, DataDatabaseName,
    DataSchemaName, DataViewColumnName, DataFamilyName,
    DataStorageParams, DataFunctionExpr, DataWindowName
};
}

#endif // RSG_CPP_FUZZER_CONFIGURATIONS_H
