//
// Created by XXX on 09/03/24.
//

// This file hold the representation of query sequence. Can be represented as string or IRs.

#ifndef QUERY_SEQ_H
#define QUERY_SEQ_H

#include "ir.h"
#include <cassert>
#include <string>
#include <utility>
#include <vector>

enum QueryStmtType {
    TYPEIR = 0,
    TYPESTRING = 1,
};

enum GenerationMethod {
    GenUnknown = -1, // Not initialized.
    GenAllFromNew = 0, // Generate the whole query from scratch.
    GenMut = 1, // Mutate from pass-in from previous sequence. Has mutating irs and instan data.
    GenMutFromOtherSequence = 2, // Mutate on queries saved from other sequence, have mutating ir subtree but no
                                 // instan data.
    GenFallBackUseOriginalSequence = 3, // Does not generate, when mutating facing error, fall back to use the query
                                        // from the original saved query sequence.
};

class RSG;
class FuzzingSequenceQueue;
class QueryInstantiatorData;

struct QueryStmt {
public:
    QueryStmtType query_stmt_type;
    GenerationMethod gen_method = GenAllFromNew; // just default.
    IR* stmt_ir = nullptr;
    string stmt_str;
    QueryInstantiatorData* query_instan_data = nullptr;
    string res_str;
    vector<IR*> mutating_irs;
    QueryStmt* mutating_original_stmt = nullptr; // the statement ir from saved queue. do not own the ir, do not deep_drop.

    QueryStmt(string in)
    {
        query_stmt_type = TYPESTRING;
        stmt_str = in;
    }
    QueryStmt(IR* in)
    {
        query_stmt_type = TYPEIR;
        stmt_ir = in; // not deepcopy
    }
    ~QueryStmt();

    [[nodiscard]] QueryStmt* deep_copy() const;
    void strip_unnecessary_data();
    [[nodiscard]] string to_string() const;
    void set_query_instan_data(QueryInstantiatorData* in);
};

class QuerySequence {
public:
    vector<QueryStmt*> v_good_query_stmts; // Warning: Doesn't contain the v_mutating_irs vector!
    vector<QueryStmt*> v_all_query_stmts;

    QuerySequence* previous_sequence = nullptr;

    void append_good_stmt(QueryStmt* in);
    void append_error_stmt(QueryStmt* in);

    [[nodiscard]] QuerySequence* deep_copy();
    void strip_unnecessary_data();

    [[nodiscard]] auto get_good_query_stmts() const { return this->v_good_query_stmts; };
    [[nodiscard]] auto get_all_query_stmts() const { return this->v_all_query_stmts; };

    string get_all_query_stmts_str() const;

    string get_query_sequence_str_with_results_stmt_by_stmt() const;
    string get_query_sequence_str_with_results() const;
    string get_query_sequence_str_short() const;

    QuerySequence() = default;
    ~QuerySequence();
};

class QuerySequenceGenerator {
protected:
    [[nodiscard]] QuerySequenceGenerator* deep_copy_helper(QuerySequenceGenerator* res) const;

public:
    bool is_finished_gen = false;
    bool is_total_new_coverage = false;
    unsigned long long instan_idx = 0; // saved the g_counter_id from the instan.
    int gen_idx = 0; // log the current generated query index number.
    GenerationMethod gen_mode = GenUnknown;

    QuerySequence *p_query_sequence = nullptr, // new generated one
        *p_saved_query_sequence = nullptr; // if using mut, grab from saved query sequence.

    void set_instan_gen_id(unsigned long long in);

    [[nodiscard]] QueryStmt* generate_next_query(RSG* p_rsg, FuzzingSequenceQueue* p_queue);

    [[nodiscard]] virtual QuerySequenceGenerator* deep_copy() = 0;

    QuerySequenceGenerator();
    virtual ~QuerySequenceGenerator();

protected:
    virtual QueryStmt* generate_next_query_helper(RSG*, FuzzingSequenceQueue*) = 0;
};

#endif // QUERY_SEQ_H