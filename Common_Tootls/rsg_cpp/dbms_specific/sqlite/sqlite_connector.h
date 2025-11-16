//
// Created by XXX on 12/10/24.
//

#ifndef SQLITE_CONNECTOR_H
#define SQLITE_CONNECTOR_H

#include "../../headers/dbms_connector.h"

class SQLiteConnector : public DBMSConnector {
public:
    SQLiteConnector(ResultHandler* p_res_in)
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
    string read_sqlite_output_and_reset_output_file();
};

#endif // SQLITE_CONNECTOR_H
