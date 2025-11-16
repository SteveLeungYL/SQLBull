//
// Created by XXX on 3/22/24.
//

#include "../headers/results_handler.h"
#include "../headers/utils.h"

using std::string;

bool ResultHandler::is_res_error(const string& in_ret)
{
#define ff(x) findStringIn(in_ret, x)
    if (ff("ERROR: ")) {
        return true;
    } else {
        return false;
    }
}

bool ResultHandler::is_dbms_internal_error(const string& in_ret)
{
    return false;
}

ResultType ResultHandler::check_results(string& cur_res)
{
    if (is_dbms_internal_error(cur_res)) {
        return ResultInternalError;
    }

    if (is_res_error(cur_res)) {
        return ResultError;
    }

    return ResultNormal;
}
