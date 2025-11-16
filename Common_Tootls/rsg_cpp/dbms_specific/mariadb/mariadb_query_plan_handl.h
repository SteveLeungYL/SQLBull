//
// Created by XXX on 10/21/24.
//

#ifndef MARIADB_QUERY_PLAN_HANDL_H
#define MARIADB_QUERY_PLAN_HANDL_H

#include "../../headers/query_plan_handl.h"

class MariaDBQueryPlanHandl : public QueryPlanHandler {
private:
    string clean_up_operand_names(const string& cur_line);

    vector<string> retrieve_query_plan_opt(const string in);

public:
    virtual vector<string> retrieve_query_plan(const string&, const string&) override;
};

#endif // MARIADB_QUERY_PLAN_HANDL_H
