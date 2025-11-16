//
// Created by XXX on 12/10/24.
//

#include "postgresql_connector.h"
#include "../../headers/types.h"
#include "../../headers/utils.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sys/time.h>
#include <filesystem>

#include "postgresql/libpq-fe.h"

std::mutex timeout_mutex;
atomic<unsigned long> timeout_id = 0;
atomic<bool> global_is_timeout = false;

unsigned int global_bind_to_port = 0;
string global_socket_path = "";

static void
mutedNoticeProcessor(void *arg, const char *message)
{
  return;
}

bool PostgreSQLConnector::connect()
{
    string conninfo = "postgresql://localhost?port=" + to_string(bind_to_port) + "&dbname=test123";
    this->conn_ = PQconnectdb(conninfo.c_str());
    return this->conn_ != nullptr;
}

void PostgreSQLConnector::disconnect()
{
    if (this->conn_ == nullptr) {
      return;
    }
    PQfinish(this->conn_);
    this->conn_ = nullptr;
}

bool PostgreSQLConnector::check_server_alive()
{
    if (this->conn_ == nullptr) {
        return false;
    }

    auto res = PQstatus(this->conn_);
    if (res == CONNECTION_OK)
      return true;

    return false;
}

void PostgreSQLConnector::restart_dbms(char** argv)
{
    this->reset_database();

    // this->disconnect();
    while (check_server_alive() == false) {
      this->disconnect();
      cerr << "Connection error: Waiting. \n";
      sleep(1);
      this->connect();
    }

    return;
}

void PostgreSQLConnector::reset_database()
{
    while (check_server_alive() == false) {
      disconnect();
      cerr << "Connection error: Waiting. \n";
      sleep(1);
      this->connect();
    }

    PGresult *res = nullptr;
    res = PQexec(this->conn_, "SET client_min_messages TO WARNING;DROP SCHEMA public CASCADE; CREATE SCHEMA public;");
    PQclear(res);
    
    res = PQexec(this->conn_, "DROP DATABASE IF EXISTS test123;");
    PQclear(res);

    res = PQexec(this->conn_, "CREATE DATABASE IF NOT EXISTS test123;");
    PQclear(res);
}

unsigned char PostgreSQLConnector::run_target(char** argv, string cmd_str,
    int is_reset_server)
{
    PGresult *res;
    int first = 1;
    int nFields;
    int execute_ok = 0;
    string result_str;

    p_res_handl->set_tmp_cur_res("");

    // if (this->check_server_alive() == false) {
    //     this->connect();
    // }

    while (check_server_alive() == false) {
      disconnect();
      cerr << "Connection error: Waiting. \n";
      sleep(1);
      this->connect();
    }

    PQsetNoticeProcessor(this->conn_, mutedNoticeProcessor, nullptr);

    if (is_reset_database) {
      reset_database();
    }

    vector<string> cmd_vec = string_splitter(cmd_str, ";");
    vector<string> timeout_cmd_vec = {"set statement_timeout to 2000; "};
    timeout_cmd_vec.insert(timeout_cmd_vec.end(), cmd_vec.begin(), cmd_vec.end());
    cmd_vec = timeout_cmd_vec;

    auto ret_result = FAULT_NONE;

    time_t begin_timer;
    time_t process_timer;
    bool is_timeout = false;
    time(&begin_timer);
    int backend_pid = 0;

    memset(this->get_trace_bits(), 0, MAP_SIZE);

    // Loop through the cmd_vec.
    for (string& cur_cmd : cmd_vec) {
      is_timeout = false;
      cur_cmd += "; ";
      /* Send our statements off to the server. */
      if (!PQsendQuery(this->conn_, cur_cmd.c_str())) {
        cerr << "Sending statements to server failed: " << PQerrorMessage(this->conn_) << endl;
      }

      backend_pid = PQbackendPID(this->conn_);
      if (kill(backend_pid, 0) != 0) {
        this->disconnect();
        return FAULT_CRASH;
      }

      if (check_status(this->conn_) == false) {
        cerr << "In func execute(), we get kServerCrash. \n";
        this->p_res_handl->set_tmp_cur_res( "In func execute(), we get kServerCrash. \n");
        this->disconnect();
        return FAULT_CRASH;
      }

      /* We want results row-by-row. */
      if (!PQsetSingleRowMode(this->conn_)) {
        cerr << "Setting single row mode failed. \n";
        this->p_res_handl->set_tmp_cur_res( "Setting single row mode failed. \n");
        this->disconnect();
        return FAULT_ERROR;
      }

      /* Loop through the results of our statements. */
      while ( !is_timeout  &&  (res = PQgetResult(this->conn_)) && res != NULL ) {
        switch (PQresultStatus(res)) {
        case PGRES_COMMAND_OK: {
          /* a query command that doesn't return
              * anything was executed properly by the
              * backend */
          break;
        }
        case PGRES_TUPLES_OK:  {/* No more rows from current query. */
          /* We want the next statement's results row-by-row also. */
          if (!PQsetSingleRowMode(this->conn_)) {
            PQclear(res);
            cerr << "Setting single row mode failed. \n";
            this->p_res_handl->set_tmp_cur_res("Setting single row mode failed. \n");
            return FAULT_ERROR;
          }
          first = 1;
          break;
        }
        case PGRES_SINGLE_TUPLE: {
          if (first) {
            /* Produce a "nice" header" */
            // cout << "-----------------------------"
            //         "-----------------------------"
            //         << endl
            //         << "Results of statement number:" << endl;
            /* print out the attribute names */
            nFields = PQnfields(res);
            // for (i = 0; i < nFields; i++) {
            //   cout << "PQfname: " << PQfname(res, i) << endl;
            //   outputStream << PQfname(res, i) << " ";
            // }
            // outputStream << endl;
            first = 0;
          }
          /* print out the row */
          for (int j = 0; j < nFields; j++) {
            // cout << "PQgetvalue: " << PQgetvalue(res, 0, j) << endl;
            // outputStream << PQgetvalue(res, 0, j) << " ";
            const char* res_char = PQgetvalue(res, 0, j);
            if (res_char != NULL && !PQgetisnull(res, 0, j)) {
              result_str += string(res_char) + " ";
            }
            time(&process_timer);
            double run_seconds = difftime(process_timer, begin_timer);
            // cerr << "Getting timeout run_seconds: " << run_seconds << "\n\n\n";
            if (run_seconds > 2.0) {
              result_str += "\nError: Execution is Timeout. \n";
              PGcancel* cancel_conn = PQgetCancel(this->conn_);
              char errbuf[512];
              PQcancel(cancel_conn, errbuf, 512);
              PQfreeCancel(cancel_conn);
              is_timeout = true;
              break;
            }
          }
          result_str += "\n";
          // outputStream << endl;
          // cerr << "result.outputs is: " << result.outputs << "\n\n\n";
          // result.outputs = outputStream.str();
          // execute_ok += 1;
          break;
        }
        case PGRES_FATAL_ERROR: {
          string error_msg = PQerrorMessage(this->conn_);
          result_str += error_msg + "\n";
          break;
        }
        default: {
          /* Always call PQgetResult until it returns null, even on
            * error. */
          // cerr << "Query execution failed: " << PQerrorMessage(conn) << endl;
          // cerr << "PQresultStatus: " << PQresultStatus(res) << endl;
          // postgre_execute_error += 1;
          time(&process_timer);
          double run_seconds = difftime(process_timer, begin_timer);
          // cerr << "Getting timeout run_seconds: " << run_seconds << "\n\n\n";
          if (run_seconds > 2.0) {
            result_str += "\nError: Execution is Timeout. \n";
            PGcancel* cancel_conn = PQgetCancel(this->conn_);
            char errbuf[512];
            PQcancel(cancel_conn, errbuf, 512);
            PQfreeCancel(cancel_conn);
            is_timeout = true;
            break;
          }
        }
        } // switch
        PQclear(res);
        if (is_timeout || ret_result == FAULT_CRASH) {
          break;
        }
      }  // while ( (res = PQgetResult(conn))  && !is_timeout)

      if (is_timeout) {  // Timeout, ignore the following SQL commands.
        // PGcancel* cancelConn = PQgetCancel(this->conn_);
        // PQcancelBlocking(cancelConn);
        // PQcancelFinish(cancelConn);
        PQrequestCancel(this->conn_);
        disconnect();
        ret_result = FAULT_TMOUT;
        break;
      }

      if (ret_result == FAULT_CRASH) {
        disconnect();
        break;
      }

    } // for (string& cur_cmd : cmd_vec)

    if (ret_result != FAULT_TMOUT && ret_result != FAULT_CRASH && kill(backend_pid, 0) != 0) {
      ret_result = FAULT_CRASH;
    }

    this->p_res_handl->set_tmp_cur_res(result_str);

    return ret_result;
}

void PostgreSQLConnector::init_forkserver(char** argv)
{
    return;
}

void PostgreSQLConnector::write_to_testcase(string& in)
{
    return;
}

void PostgreSQLConnector::record_code_coverage(char** argv)
{
    return;
}

void PostgreSQLConnector::set_socket_path(string in)
{
    this->socket_path = in;
    global_socket_path = in;
}

void PostgreSQLConnector::set_bind_to_port(int in)
{
    this->bind_to_port = in;
    global_bind_to_port = in;
}
