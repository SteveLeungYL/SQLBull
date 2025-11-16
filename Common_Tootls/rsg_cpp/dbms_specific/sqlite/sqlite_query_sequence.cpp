//
// Created by XXX on 10/21/24.
//

#include "sqlite_query_sequence.h"
#include "../../headers/query_instantiator.h"
#include "../../headers/rsg.h"
#include "sqlite_fuzzer_configurations.h"

QueryStmt* generate_stmt_with_idx(int gen_idx, RSG* p_rsg)
{
    if (gen_idx < 4) {
        if (get_pct_hit(50)) {
            return p_rsg->generate_stmt_with_stmt_type(IRTypeCreateTableStmt);
        } else {
            return p_rsg->generate_stmt_with_stmt_type(IRTypeCreateViewStmt);
        }
    } else if (gen_idx < 8) {
        return p_rsg->generate_stmt_with_stmt_type(IRTypeInsertStmt);
    } else if (gen_idx < 14) {
        if (get_pct_hit(80)) {
            return p_rsg->generate_stmt_with_stmt_type(IRTypeAlterStmt);
        } else {
            return p_rsg->generate_stmt_with_stmt_type(IRTypeCreateIndexStmt);
        }
    } else if (gen_idx < 18) {
        if (get_pct_hit(50)) {
            return p_rsg->generate_stmt_with_stmt_type(IRTypeCreateTriggerStmt);
        } else {
            return p_rsg->generate_stmt_with_stmt_type(IRTypeCreateIndexStmt);
        }
    } else if (gen_idx < 22) {
        // Any statement is welcomed. 
        return p_rsg->generate_stmt_with_stmt_type(IRTypeCmd);
    } else {
        return p_rsg->generate_stmt_with_stmt_type(IRTypeSelectStmt);
    }
}

QueryStmt* SQLiteQuerySequenceGenerator::generate_next_query_helper(RSG* p_rsg, FuzzingSequenceQueue* p_queue)
{
    if (this->gen_mode == GenUnknown) {
        // If already saved 100 query sequences, then mutate only.
        if (get_pct_hit(SQLiteFuzzerConfigurations::pct_mutate_ratio) || p_queue->get_saved_query_sequence_size() >= 100) {
            this->gen_mode = GenMut;
        } else {
            this->gen_mode = GenAllFromNew;
        }
    }

    // Do not use GenMut if there is no saved previous query sequence.
    if (p_queue->get_saved_query_sequence_size() == 0) {
        this->gen_mode = GenAllFromNew;
    }

    if (this->gen_mode == GenAllFromNew) {
        // Generate everything from scratch.

#ifdef DEBUG
        cerr << "In DuckDB generate_next_query_helper, calling generate_stmt_with_idx with GenAllFromNew. \n";
#endif

        QueryStmt* res = generate_stmt_with_idx(this->gen_idx++, p_rsg);
        return res;
    }

    else {
        // gen_mode == GenMut
        if (this->p_saved_query_sequence == nullptr) {
            this->p_saved_query_sequence = p_queue->get_next_query_sequence_from_queue();
        }

        QueryStmt* res = nullptr;
        if (this->gen_idx >= this->p_saved_query_sequence->v_good_query_stmts.size()) {
#ifdef DEBUG
            cerr << "In DuckDB generate_next_query_helper, calling generate_stmt_with_idx with GenMut with " << this->gen_idx << "\n";
#endif
            res = generate_stmt_with_idx(this->gen_idx, p_rsg);
            this->gen_idx++;
        } else if (get_pct_hit(SQLiteFuzzerConfigurations::pct_insert_new_in_mut) && this->p_saved_query_sequence->v_good_query_stmts.size() < 40) {
            // Insert new randomly generated statements.
#ifdef DEBUG
            cerr << "In DuckDB generate_next_query_helper, calling generate_stmt_with_idx with GenMut with " << this->gen_idx << "\n";
#endif
            res = generate_stmt_with_idx(this->gen_idx, p_rsg);
        } else {
            // reuse the query from the previous query sequence.
            while (
                gen_idx < this->p_saved_query_sequence->v_good_query_stmts.size() && !(this->p_saved_query_sequence->v_good_query_stmts[this->gen_idx]->stmt_ir)) {
                this->gen_idx++;
            }
            if (gen_idx >= this->p_saved_query_sequence->v_good_query_stmts.size()) {
                // In case the last statement is just string, no stmt_ir presented.
                return nullptr;
            }
            // mutate_on_input_stmt_ir will recreate a new stmt to return. Clear mutating_irs from saved one.
            res = p_rsg->mutate_on_input_stmt_ir(this->p_saved_query_sequence->v_good_query_stmts[this->gen_idx]->stmt_ir->deep_copy());
            res->mutating_original_stmt = this->p_saved_query_sequence->v_good_query_stmts[this->gen_idx]; // do not own, do not deep copy.
            res->query_instan_data = this->p_saved_query_sequence->v_good_query_stmts[this->gen_idx]->query_instan_data->deep_copy();

#ifdef DEBUG
            cerr << "In DuckDB generate_next_query_helper, calling mutate_on_input_stmt_ir with GenMut with " << this->gen_idx << "\n";
#endif

            this->gen_idx++;
        }

        return res;
    }
}

#define append_stmt(x) this->p_query_sequence->append_good_stmt(new QueryStmt(x));
SQLiteQuerySequenceGenerator::SQLiteQuerySequenceGenerator() {
    // Runs after QuerySequenceGenerator's constructor. The p_query_sequence should be initialized already.
    assert(this->p_query_sequence);

    // Other pragmas are handled by the pre_insert_stmt_vec.
    append_stmt("PRAGMA optimization(0x00000000);"); // Enable all optimizations.
    append_stmt("PRAGMA optimize;");
    append_stmt(".expert");

    append_stmt("CREATE VIRTUAL TABLE v03 USING rtree(tree_id, minX, maxX, minY, maxY);"); // Column type fixed.
    append_stmt("INSERT INTO v03 VALUES (0, 0, 0, 0, 0);"); // Column type fixed.
    append_stmt("CREATE VIRTUAL TABLE v04 USING fts5(c05, c06);"); // not column type permitted.
    append_stmt("INSERT INTO v04 VALUES ('abc', 'def');"); // Column type fixed.
    append_stmt("CREATE TABLE v07 (c08 INTEGER PRIMARY KEY) WITHOUT ROWID;"); // Column type fixed.
    append_stmt("INSERT INTO v07 VALUES (2147483647);"); // Column type fixed.
}
#undef append_stmt
