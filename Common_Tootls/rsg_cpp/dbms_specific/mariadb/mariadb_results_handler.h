//
// Created by XXX on 10/21/24.
//

#ifndef MARIADB_RESULTS_HANDLER_H
#define MARIADB_RESULTS_HANDLER_H

#include "../../headers/results_handler.h"

class MariaDBResultHandler : public ResultHandler {
    // use the original check_results function entry.
    // only override the two helper functions.
public:
    virtual bool is_res_error(const string& in_ret) override;
    virtual bool is_dbms_internal_error(const string& in_ret) override;
};

#endif // MARIADB_RESULTS_HANDLER_H
