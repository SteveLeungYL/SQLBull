//
// Created by XXX on 12/10/24.
//

#ifndef MYSQL_CONNECTOR_H
#define MYSQL_CONNECTOR_H

#include "../../headers/dbms_connector.h"

#include <mutex>
#include <thread>

#include <mysql/errmsg.h>
#include <mysql/mysql.h>
#include <mysql/mysqld_error.h>

enum SERVER_STATUS {
    kConnectFailed,
    kExecuteError,
    kServerCrash,
    kNormal,
    kTimeout,
    kSyntaxError,
    kSemanticError
};

class MySQLConnector : public DBMSConnector {
public:
    MySQLConnector(ResultHandler* p_res_in)
        : DBMSConnector(p_res_in)
    {
    }

    ~MySQLConnector()
    {
        disconnect();
    }

    virtual unsigned char run_target(char** argv, string cmd_str,
        int is_reset_server = 1) override;

    virtual void write_to_testcase(string& in) override;

    virtual void init_forkserver(char** argv) override;

    virtual void record_code_coverage(char** argv) override;

    virtual void restart_dbms(char** argv) override;

    bool check_server_alive();

    bool connect();

    void disconnect();

    bool fix_database();

    SERVER_STATUS clean_up_connection(MYSQL* mm);

    int retrieve_query_results_count(MYSQL& m_);

    string retrieve_query_results(MYSQL* m_, string cur_cmd_str);

    virtual void set_socket_path(string in) override;

    virtual void set_bind_to_port(int in) override;

private:
    int is_reset_database = 0;
    unsigned int database_id = 1;

    MYSQL* m_ = nullptr;

    unsigned counter_; // odd for "test", even for "test2"
};

#endif // MYSQL_CONNECTOR_H
