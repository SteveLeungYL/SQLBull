//
// Created by XXX on 3/22/24.
//

#include "cockroachdb_results_handler.h"
#include "../../headers/utils.h"

bool CockroachDBResultHandler::is_res_error(const string& in_ret)
{
#define ff(x) findStringIn(in_ret, x)
    if (ff("ERROR: ")
        || ff("pq: ")) {
        return true;
    } else {
        return false;
    }
}

bool CockroachDBResultHandler::is_dbms_internal_error(const string& in_ret)
{
    if (ff("Internal Error")
        || ff("unexpected error")) {
        return true;
    } else {
        return false;
    }
}
