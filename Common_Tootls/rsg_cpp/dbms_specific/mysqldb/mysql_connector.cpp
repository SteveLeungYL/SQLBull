//
// Created by XXX on 12/10/24.
//

#include "mysql_connector.h"
#include "../../headers/types.h"
#include "../../headers/utils.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sys/time.h>
#include <filesystem>

std::mutex timeout_mutex;
atomic<unsigned long> timeout_id = 0;
atomic<bool> global_is_timeout = false;

unsigned int global_bind_to_port = 0;
string global_socket_path = "";

static int mysql_server_pid = -1;

static bool terminate_query(unsigned long process_id)
{
    MYSQL tmp_m;
    if (mysql_init(&tmp_m) == NULL) {
        mysql_close(&tmp_m);
        return false;
    }

    // cerr << "Using socket: " << socket_path << "\n\n\n";
    if (!mysql_real_connect(&tmp_m, "localhost", "root", "", "test123", global_bind_to_port, global_socket_path.c_str(), 0)) {
        fprintf(stderr, "Connection error5 %u %s\n", mysql_errno(&tmp_m), mysql_error(&tmp_m));
        mysql_close(&tmp_m);
        return false;
    }
    string cmd = "KILL " + to_string(process_id);
    mysql_real_query(&tmp_m, static_cast<const char*>(cmd.c_str()), static_cast<unsigned long>(cmd.size()));
    // cerr << "Terminate_database results: "  << retrieve_query_results(&tmp_m) << "\n\n\n";

    mysql_close(&tmp_m);
    // std::cout << "Timeout!!! Kill query successful. \n\n\n";
    // sleep(1);
    return true;
}

static void timeout_query(unsigned long process_id, unsigned long cur_timeout_id)
{
    // Default timeout is 2 seconds.
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));

    timeout_mutex.lock();

    // The previous execution has already finished. No timeout.
    if (timeout_id.load() != cur_timeout_id) {
        timeout_mutex.unlock();
        return;
    }

    // The prvious execution timeout. Kill it!
    global_is_timeout.store(true);

    timeout_mutex.unlock();

    if (global_is_timeout.load()) {
        terminate_query(process_id);
        kill(mysql_server_pid, SIGKILL);
        cerr << "Timeout!!! Kill query successful. \n\n\n";
        mysql_server_pid = -1;
    }

    // cerr << "\n\n\nQuery terminated!!!!\n\n\n";
}

static void get_mysql_server_pid() {
    if(!filesystem::exists("./pid_pass_to_fuzzer")) {
        return;
    }
    fstream pid_file("./pid_pass_to_fuzzer", ios::in);
    pid_file >> mysql_server_pid;
    pid_file.close();
    filesystem::remove("./pid_pass_to_fuzzer");
    return;
}

bool MySQLConnector::connect()
{
    string dbname;

    while (mysql_server_pid == -1) {
        get_mysql_server_pid();
        if (mysql_server_pid == -1) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    } 
    // Safety check. Update the pid again if existed. 
    get_mysql_server_pid();

    if (m_ != nullptr) {
        mysql_close(m_);
        m_ = nullptr;
    }

    m_ = mysql_init(m_);

    if (m_ == NULL) {
        return false;
    }

    dbname = "test123";
    if (mysql_real_connect(m_, NULL, "root", "", dbname.c_str(), global_bind_to_port, global_socket_path.c_str(), 0) == NULL) {
        fprintf(stderr, "Connection error1 %u %s\n", mysql_errno(m_), mysql_error(m_));
        disconnect();
        counter_++;
        return false;
    }

    return true;
}

void MySQLConnector::disconnect()
{
    mysql_close(m_);
    m_ = NULL;
}

bool MySQLConnector::check_server_alive()
{
    MYSQL tmp_m;

    if (mysql_init(&tmp_m) == NULL) {
        mysql_close(&tmp_m);
        return false;
    }
    if (mysql_real_connect(&tmp_m, NULL, "root", "", "test_init", global_bind_to_port, global_socket_path.c_str(), CLIENT_MULTI_STATEMENTS) == NULL) {
        fprintf(stderr, "Connection error2 %u %s\n", mysql_errno(&tmp_m), mysql_error(&tmp_m));
        mysql_close(&tmp_m);
        return false;
    }
    mysql_close(&tmp_m);
    return true;
}

void MySQLConnector::restart_dbms(char** argv)
{
    MYSQL tmp_m;

    is_reset_database = 1;

    if (m_ != NULL) {
        disconnect();
    }

    database_id += 1;
    if (mysql_init(&tmp_m) == NULL) {
        mysql_close(&tmp_m);
        return;
    }
    if (mysql_real_connect(&tmp_m, NULL, "root", "", "test_init", global_bind_to_port, global_socket_path.c_str(), 0) == NULL) {
        // fprintf(stderr, "Connection error4 %u %s\n", mysql_errno(&tmp_m), mysql_error(&tmp_m));
        mysql_close(&tmp_m);
        return;
    }

    bool is_error = false;
    vector<string> v_cmd = { "SET GLOBAL TRANSACTION READ WRITE", "SET SESSION TRANSACTION READ WRITE", "RESET PERSIST", "ALTER USER 'root'@'localhost' WITH MAX_USER_CONNECTIONS 0;", "DROP DATABASE IF EXISTS test123", "CREATE DATABASE IF NOT EXISTS test123", "USE test123", "SELECT 'Successful'" };
    for (string cmd : v_cmd) {
        if (mysql_real_query(&tmp_m, cmd.c_str(), cmd.size())) {
            is_error = true;
        }

        timeout_id++;

        retrieve_query_results(&tmp_m, "");
        clean_up_connection(&tmp_m);
    }

    mysql_close(&tmp_m);
    return;
}

unsigned char MySQLConnector::run_target(char** argv, string cmd_str,
    int is_reset_server)
{
    p_res_handl->set_tmp_cur_res("");

    bool conn = true;
    if (m_ == NULL) {
        conn = connect();
    }

    int retry_time = 0;
    bool is_already_killed_in_connect = false;
    while (!conn) {
        if (retry_time > 1000 && mysql_server_pid != -1 && !is_already_killed_in_connect) { // 30 seconds?
            kill(mysql_server_pid, SIGKILL);
            cerr << "Cannot access MySQL server through test123 and test_init!!! Force restarted server. \n\n\n";
            mysql_server_pid = -1;
            is_already_killed_in_connect = true;
        }

        if (retry_time != 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(30));
        }
        conn = connect();
        if (!conn) {
            fix_database();
        }
        retry_time++;
    }

    string res_str = "";

    // if (is_reset_database) { // Return true for no error, false for errors.
    //     disconnect();
    //     restart_dbms(argv);
    //     m_ = NULL;
    //     conn = false;
    //     while (!conn) {
    //         //        std::this_thread::sleep_for(std::chrono::milliseconds(30));
    //         conn = connect();
    //         if (!conn)
    //             fix_database();
    //     }
    // }

    std::replace(cmd_str.begin(), cmd_str.end(), '\n', ' ');
    // std::replace(cmd_str.begin(), cmd_str.end(), ';', ' ');

    vector<string> v_cmd_str = { cmd_str };

    SERVER_STATUS correctness;
    int server_response;

    timeout_mutex.lock();
    this->is_timeout = false;
    global_is_timeout.store(false);

    timeout_mutex.unlock();

    has_new_bits(virgin_bits, "123");

    std::thread(timeout_query, m_->thread_id, timeout_id.load()).detach();

    if (is_reset_database != 0) {
        for (string cur_cmd_str : v_cmd_str) {
            trim_string(cur_cmd_str);
            if (cur_cmd_str.empty()) {
                continue;
            }
            server_response = mysql_real_query(m_, cur_cmd_str.c_str(), cur_cmd_str.length());

            timeout_mutex.lock();
            timeout_id++;
            timeout_mutex.unlock();

            res_str += retrieve_query_results(m_, cur_cmd_str) + "\n";
            correctness = clean_up_connection(m_);

            // if (server_response == 0) {
            //   debug_good++;
            // } else {
            //   debug_error++;
            // }

            if (server_response == CR_SERVER_LOST) {
                cerr << "Server Lost or Server Crashes! \n\n\n";
                break;
            }
        }
    } else {
        // is_reset_database == false
        string cur_cmd_str = cmd_str;

        server_response = mysql_real_query(m_, cur_cmd_str.c_str(), cur_cmd_str.length());

        timeout_mutex.lock();
        timeout_id++;
        timeout_mutex.unlock();

        res_str += retrieve_query_results(m_, cur_cmd_str) + "\n";
        correctness = clean_up_connection(m_);

        //   if (server_response == 0) {
        //     debug_good++;
        //   } else {
        //     debug_error++;
        //   }

        if (server_response == CR_SERVER_LOST) {
            cerr << "Server Lost or Server Crashes! \n\n\n";
        }
    }

    // cerr << "Getting results: \n" << res_str << "\n\n\n";
    p_res_handl->set_tmp_cur_res(res_str);

    if (server_response == CR_SERVER_LOST || server_response == CR_SERVER_GONE_ERROR) {

        timeout_mutex.lock();
        timeout_id++;
        timeout_mutex.unlock();

        disconnect();
        if (global_is_timeout.load()) {
            this->is_timeout = true;
            return FAULT_TMOUT;
        } else {
            return FAULT_CRASH;
        }
    }

    auto res = FAULT_NONE;
    // res = correctness;

    timeout_mutex.lock();
    timeout_id++;
    timeout_mutex.unlock();

    auto check_res = check_server_alive();
    if (!check_res) {
        disconnect();
        sleep(2); // waiting for server to be up again
        is_reset_database = 1;
        return FAULT_CRASH;
    }

    timeout_mutex.lock();
    timeout_id++;
    if (global_is_timeout.load()) {
        this->is_timeout = true;
        is_reset_database = 1;
        timeout_mutex.unlock();
        res = FAULT_TMOUT;
        return res;
    }
    timeout_mutex.unlock();

    counter_++;
    //    disconnect();
    return res;
}

bool MySQLConnector::fix_database()
{
    MYSQL tmp_m;

    is_reset_database = 1;

    database_id += 1;
    if (mysql_init(&tmp_m) == NULL) {
        mysql_close(&tmp_m);
        return false;
    }

    // cerr << "Using socket: " << socket_path << "\n\n\n";
    if (mysql_real_connect(&tmp_m, NULL, "root", "", "test_init", bind_to_port, socket_path.c_str(), 0) == NULL) {
        fprintf(stderr, "Connection error3 %u %s\n", mysql_errno(&tmp_m), mysql_error(&tmp_m));
        mysql_close(&tmp_m);
        return false;
    }
    // database_id++;
    bool is_error = false;


    vector<string> v_cmd = { "SET GLOBAL TRANSACTION READ WRITE", "SET SESSION TRANSACTION READ WRITE", "RESET PERSIST",  "DROP DATABASE IF EXISTS test123", "CREATE DATABASE IF NOT EXISTS test123", "USE test123", "SELECT 'Successful'" };
    for (string cmd : v_cmd) {
        std::thread(timeout_query, tmp_m.thread_id, timeout_id.load()).detach();
        if (mysql_real_query(&tmp_m, cmd.c_str(), cmd.size())) {
            is_error = true;
        }
        // cerr << "Fix_database results: "  << retrieve_query_results(&tmp_m, cmd) << "\n\n\n";
        clean_up_connection(&tmp_m);

        timeout_id++;
    }

    mysql_close(&tmp_m);

    return !is_error;
}

SERVER_STATUS MySQLConnector::clean_up_connection(MYSQL* mm)
{
    int res = -1;
    do {
        auto q_result = mysql_store_result(mm);
        if (q_result)
            mysql_free_result(q_result);
    } while ((res = mysql_next_result(mm)) == 0);

    if (res != -1) {
        if (mysql_errno(mm) == 1064) {
            return kSyntaxError;
        } else {
            return kSemanticError;
        }
    }
    return kNormal;
}

int MySQLConnector::retrieve_query_results_count(MYSQL& m_)
{
    MYSQL_ROW row;
    int result_count = 0;
    // string result_string = ""
    // stringstream result_string_stream;
    int status = 0;
    MYSQL_RES* result;
    do {
        /* did current statement return data? */
        result = mysql_store_result(&m_);
        if (result) {
            while ((row = mysql_fetch_row(result)) != NULL)
                result_count++;
        }
        /* more results? -1 = no, >0 = error, 0 = yes (keep looping) */
        if ((status = mysql_next_result(&m_)) > 0)
            break;
        // cerr << "Could not execute statement\n";
    } while (status == 0);

    return result_count;
}

string MySQLConnector::retrieve_query_results(MYSQL* m_, string cur_cmd_str)
{
    MYSQL_ROW row;
    // string result_string = ""
    stringstream result_string_stream;
    int status = 0;
    MYSQL_RES* result;
    string ret_str;

    if (mysql_errno(m_)) {
        ret_str = "Error: " + string(mysql_error(m_)) + "\n";
    }

    do {
        /* did current statement return data? */
        result = mysql_store_result(m_);
        // cerr << "is result empty? " << result << "\n\n\n";
        if (result) {
            /* yes; process rows and free the result set */
            while ((row = mysql_fetch_row(result)) != NULL) {
                for (int i = 0; i < mysql_num_fields(result); i++) {
                    // cerr << "Getting row: " << row[i] << "\n\n\n";
                    result_string_stream << row[i];
                }
            }
            // cerr << "Returned all rows " << "\n\n\n";
        } else /* no result set or error */
        {
            // cerr << "No results get!\n";
            if (mysql_field_count(m_) == 0) {
                // printf("%lld rows affected\n", mysql_affected_rows(m_));
            } else if (mysql_field_count(m_) != 0) {
                // cerr << "Could not retrieve result set\n";
                // break;
            }
        }
        mysql_free_result(result);
        /* more results? -1 = no, >0 = error, 0 = yes (keep looping) */
        if ((status = mysql_next_result(m_)) > 0) {
            // cerr << "Could not execute statement. \n\n\n";
            // break;
        }
        if ((status = mysql_next_result(m_)) == -1) {
            // cerr << "No more results. \n\n\n";
            break;
        }
        // cerr << "Could not execute statement\n";
    } while (status == 0);

    // cerr << "Outputing MySQL message: \nQuery: " << cur_cmd_str << "\nRes: " << result_string_stream.str() << "\n";
    // if (mysql_errno(m_)) {
    //   cerr << "Error message: " << mysql_error(m_) << "\n";
    // }
    // cerr << "\n\n";

    ret_str += result_string_stream.str();
    return ret_str;
}

void MySQLConnector::init_forkserver(char** argv)
{
    return;
}

void MySQLConnector::write_to_testcase(string& in)
{
}

void MySQLConnector::record_code_coverage(char** argv)
{
    /* EMPTY since trace_bits is synced with the MySQL process. No need for explicit recording. */
}

void MySQLConnector::set_socket_path(string in)
{
    this->socket_path = in;
    global_socket_path = in;
}

void MySQLConnector::set_bind_to_port(int in)
{
    this->bind_to_port = in;
    global_bind_to_port = in;
}
