//
// Created by XXX on 10/21/24.
//

#ifndef SQLITE_RESULTS_HANDLER_H
#define SQLITE_RESULTS_HANDLER_H

#include "../../headers/results_handler.h"

class SQLiteResultHandler : public ResultHandler {
    // use the original check_results function entry.
    // only override the two helper functions.
public:
    virtual bool is_res_error(const string& in_ret) override;
    virtual bool is_dbms_internal_error(const string& in_ret) override;
};

#endif // SQLITE_RESULTS_HANDLER_H
