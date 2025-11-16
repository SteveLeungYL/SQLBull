//
// Created by XXX on 3/22/24.
//

#ifndef RSG_CPP_COCKROACHDB_RESULTS_HANDLER_H
#define RSG_CPP_COCKROACHDB_RESULTS_HANDLER_H

#include "../../headers/results_handler.h"

class CockroachDBResultHandler : public ResultHandler {
    // use the original check_results function entry.
    // only override the two helper functions.
public:
    virtual bool is_res_error(const string& in_ret) override;
    virtual bool is_dbms_internal_error(const string& in_ret) override;
};

#endif // RSG_CPP_COCKROACHDB_RESULTS_HANDLER_H
