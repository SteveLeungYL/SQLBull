//
// Created by XXX on 10/21/24.
//

#include "sqlite_query_plan_handl.h"
#include "../../headers/utils.h"
#include <regex>

vector<string> SQLiteQueryPlanHandl::retrieve_query_plan(const string& query_in, const string& res_in)
{
    vector<string> res_out;
    // TODO:: FIXME:: Main logic here.
    return res_out;
}

/* This is a deprecated function that used to check Query Plan from CockroachDB using EXPLAIN OPT command.
 * This command is not available in the older version of the code.
 * */
vector<string> SQLiteQueryPlanHandl::retrieve_query_plan_opt(const string in)
{
    vector<string> res_out;
    // TODO:: FIXME:: Main logic here.
    return res_out;
}
