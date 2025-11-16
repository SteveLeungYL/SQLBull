//
// Created by XXX on 10/21/24.
//

#include "postgresql_results_handler.h"
#include "../../headers/utils.h"

bool PostgreSQLResultHandler::is_res_error(const string& in_ret)
{
#define ff(x) findStringIn(in_ret, x)

    if (ff("unexpectedly") || ff("abnormally")) {
        // If lost connection present, the server might has crashed.
        // Do not treat it as an error, as error means SQL error here, and will dump the output. 
        return false;
    } else if (ff("ERROR") || ff("Error:")) {
        return true;
    } else {
        return false;
    }
}

bool PostgreSQLResultHandler::is_dbms_internal_error(const string& in_ret)
{
    if (ff("unexpectedly") || ff("abnormally")
     || ff("Lost connection")
    ) {
        return true;
    } else {
        return false;
    }
}
