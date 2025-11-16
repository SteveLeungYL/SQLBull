//
// Created by XXX on 9/4/24.
//

#ifndef RSG_CPP_COCKROACHDB_QUERY_SEQUENCE_H
#define RSG_CPP_COCKROACHDB_QUERY_SEQUENCE_H

#include "../../headers/fuzzing_sequence_queue.h"
#include "../../headers/query_sequence.h"

class CockroachDBQuerySequenceGenerator : public QuerySequenceGenerator {
protected:
    QueryStmt* generate_next_query_helper(RSG* p_rsg, FuzzingSequenceQueue* p_queue) override;

public:
    QuerySequenceGenerator* deep_copy() override
    {
        auto* res = new CockroachDBQuerySequenceGenerator();
        return this->deep_copy_helper(res);
    };
};

#endif // RSG_CPP_COCKROACHDB_QUERY_SEQUENCE_H
