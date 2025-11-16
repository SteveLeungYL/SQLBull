//
// Created by XXX on 10/21/24.
//

#include "duckdb_results_handler.h"
#include "../../headers/utils.h"

bool DuckDBResultHandler::is_res_error(const string& in_ret)
{
#define ff(x) findStringIn(in_ret, x)
    if (ff("Error: ")) {
        return true;
    } else {
        return false;
    }
}

bool DuckDBResultHandler::is_dbms_internal_error(const string& in_ret)
{
    if (ff("FATAL Error:") || ff("INTERNAL Error") || ff("assertion failure") || ff("InternalException")) {
        return true;
    } else {
        return false;
    }
}
