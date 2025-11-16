//
// Created by XXX on 12/10/24.
//

#ifndef POSTGRESQL_CONNECTOR_H
#define POSTGRESQL_CONNECTOR_H

#include "../../headers/dbms_connector.h"

#include <mutex>
#include <thread>

#include "postgresql/libpq-fe.h"

enum SERVER_STATUS {
    kConnectFailed,
    kExecuteError,
    kServerCrash,
    kNormal,
    kTimeout,
    kSyntaxError,
    kSemanticError
};

class PostgreSQLConnector : public DBMSConnector {
public:
    PostgreSQLConnector(ResultHandler* p_res_in)
        : DBMSConnector(p_res_in)
    {
    }

    ~PostgreSQLConnector() {
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

    void reset_database();

    virtual void set_socket_path(string in) override;

    virtual void set_bind_to_port(int in) override;


  bool check_status(PGconn *conn)
  {
    auto res = PQstatus(conn);
    if (res == CONNECTION_OK)
      return true;

    return false;
  }


private:
    int is_reset_database = 0;
    unsigned int database_id = 1;

    PGconn* conn_ = nullptr;
};

#endif // POSTGRESQL_CONNECTOR_H
