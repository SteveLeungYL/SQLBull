//
// Created by XXX on 10/21/24.
//

#ifndef SQLITE_QUERY_SEQUENCE_H
#define SQLITE_QUERY_SEQUENCE_H

#include "../../headers/fuzzing_sequence_queue.h"
#include "../../headers/query_sequence.h"

class SQLiteQuerySequenceGenerator : public QuerySequenceGenerator {
protected:
    QueryStmt* generate_next_query_helper(RSG* p_rsg, FuzzingSequenceQueue* p_queue) override;

public:
    QuerySequenceGenerator* deep_copy() override
    {
        auto* res = new SQLiteQuerySequenceGenerator();
        return this->deep_copy_helper(res);
    };

    // Custom Constructor.
    SQLiteQuerySequenceGenerator();
    ~SQLiteQuerySequenceGenerator() final = default;
};

#endif // SQLITE_QUERY_SEQUENCE_H
