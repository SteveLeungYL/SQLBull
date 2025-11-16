//
// Created by XXX on 4/11/24.
//

#ifndef RSG_CPP_FEEDBACK_MAPPER_H
#define RSG_CPP_FEEDBACK_MAPPER_H

#include "dbms_connector.h"
#include "ir_wrapper.h"
#include "query_instantiator.h"
#include "query_plan_handl.h"
#include "results_handler.h"
#include "rsg.h"

#include <string>
#include <vector>

class FeedbackMapper {
public:
    virtual void feedback_mapping(vector<IR*>, QueryPlanHandler*);

    explicit FeedbackMapper(
        RSG* rsg_in,
        IRWrapper* p_in,
        DBMSConnector* p_conn_in,
        QueryInstantiator* p_instan_in)
        : rsg(rsg_in)
        , p_ir_wrapper(p_in)
        , p_dbms_conn(p_conn_in)
        , p_query_instan(p_instan_in)
    {
    }

    // avoid C++ delete warnings.
    virtual ~FeedbackMapper() = default;

protected:
    // helper functions.
    virtual bool query_ir_minimizer(IR*);

private:
    RSG* rsg;
    IRWrapper* p_ir_wrapper;
    DBMSConnector* p_dbms_conn;
    QueryInstantiator* p_query_instan;
};

#endif // RSG_CPP_FEEDBACK_MAPPER_H
