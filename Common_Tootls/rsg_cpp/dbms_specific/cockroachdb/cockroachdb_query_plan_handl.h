//
// Created by XXX on 3/22/24.
//

#ifndef RSG_CPP_COCKROACHDB_QUERY_PLAN_HANDL_H
#define RSG_CPP_COCKROACHDB_QUERY_PLAN_HANDL_H

#include "../../headers/query_plan_handl.h"
#include <regex>

class CockroachDBQueryPlanHandl : public QueryPlanHandler {
private:
    string clean_up_operand_names(const string& cur_line);

    vector<string> retrieve_query_plan_opt(const string in);

public:
    virtual vector<string> retrieve_query_plan(const string&, const string&) override;
};

#endif // RSG_CPP_COCKROACHDB_QUERY_PLAN_HANDL_H
