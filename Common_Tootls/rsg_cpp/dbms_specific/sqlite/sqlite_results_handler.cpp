//
// Created by XXX on 10/21/24.
//

#include "sqlite_results_handler.h"
#include "../../headers/utils.h"

bool SQLiteResultHandler::is_res_error(const string& in_ret)
{
#define ff(x) findStringIn(in_ret, x)
    if (ff("error")) {
        return true;
    } else {
        return false;
    }
}

bool SQLiteResultHandler::is_dbms_internal_error(const string& in_ret)
{
    if (ff("Fatal Error:") || ff("fatal Error:") || ff("Assertion")) {
        return true;
    } else {
        return false;
    }
}
