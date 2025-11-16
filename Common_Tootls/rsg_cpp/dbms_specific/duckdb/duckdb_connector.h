//
// Created by XXX on 10/14/24.
//

#ifndef DUCKDB_CONNECTOR_H
#define DUCKDB_CONNECTOR_H

#include "../../headers/dbms_connector.h"

class DuckDBConnector : public DBMSConnector {
public:
    DuckDBConnector(ResultHandler* p_res_in)
        : DBMSConnector(p_res_in)
    {
    }

    virtual unsigned char run_target(char** argv, string cmd_str,
        int is_reset_server = 1) override;

    virtual void write_to_testcase(string& in) override;

    virtual void init_forkserver(char** argv) override;

    virtual void record_code_coverage(char** argv) override;

    virtual void restart_dbms(char** argv) override;

private:
    string read_duckdb_output_and_reset_output_file();
};

#endif // DUCKDB_CONNECTOR_H
