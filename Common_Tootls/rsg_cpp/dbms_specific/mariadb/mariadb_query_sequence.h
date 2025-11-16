//
// Created by XXX on 10/21/24.
//

#ifndef MARIADB_QUERY_SEQUENCE_H
#define MARIADB_QUERY_SEQUENCE_H

#include "../../headers/fuzzing_sequence_queue.h"
#include "../../headers/query_sequence.h"

class MariaDBQuerySequenceGenerator : public QuerySequenceGenerator {
protected:
    QueryStmt* generate_next_query_helper(RSG* p_rsg, FuzzingSequenceQueue* p_queue) override;

public:
    QuerySequenceGenerator* deep_copy() override
    {
        auto* res = new MariaDBQuerySequenceGenerator();
        return this->deep_copy_helper(res);
    };

    // Custom Constructor.
    MariaDBQuerySequenceGenerator();
    ~MariaDBQuerySequenceGenerator() final = default;
};

#endif // MARIADB_QUERY_SEQUENCE_H
