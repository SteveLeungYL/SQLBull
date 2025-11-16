//
// Created by XXX on 3/25/24.
//

#ifndef RSG_CPP_COCKROACHDB_CONNECTOR_H
#define RSG_CPP_COCKROACHDB_CONNECTOR_H

#include "../../headers/dbms_connector.h"

class CockroachDBConnector : public DBMSConnector {
public:
    CockroachDBConnector(ResultHandler* p_res_in)
        : DBMSConnector(p_res_in)
    {
    }

    virtual unsigned char run_target(char** argv, string cmd_str,
        int is_reset_server = 1) override;

    virtual void write_to_testcase(string&) override;

    virtual void init_forkserver(char** argv) override;

    virtual void record_code_coverage(char** argv) override;

    virtual void restart_dbms(char** argv) override;
};

#endif // RSG_CPP_COCKROACHDB_CONNECTOR_H
