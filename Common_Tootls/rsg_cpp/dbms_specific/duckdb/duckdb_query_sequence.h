//
// Created by XXX on 10/21/24.
//

#ifndef DUCKDB_QUERY_SEQUENCE_H
#define DUCKDB_QUERY_SEQUENCE_H

#include "../../headers/fuzzing_sequence_queue.h"
#include "../../headers/query_sequence.h"

class DuckDBQuerySequenceGenerator : public QuerySequenceGenerator {
protected:
    QueryStmt* generate_next_query_helper(RSG* p_rsg, FuzzingSequenceQueue* p_queue) override;

public:
    QuerySequenceGenerator* deep_copy() override
    {
        auto* res = new DuckDBQuerySequenceGenerator();
        return this->deep_copy_helper(res);
    };

    // Custom Constructor.
    DuckDBQuerySequenceGenerator();
    ~DuckDBQuerySequenceGenerator() final = default;
};

#endif // DUCKDB_QUERY_SEQUENCE_H
