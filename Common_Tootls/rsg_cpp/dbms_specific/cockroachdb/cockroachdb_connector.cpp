//
// Created by XXX on 3/25/24.
//

#include "cockroachdb_connector.h"
#include "../../headers/types.h"
#include "../../headers/utils.h"
#include <fstream>
#include <iostream>
#include <sys/time.h>

unsigned char CockroachDBConnector::run_target(char** argv, string cmd_str,
    int is_reset_server)
{
    struct itimerval it;
    it.it_interval.tv_sec = 0;
    it.it_interval.tv_usec = 0;
    u32 prev_timed_out = 0;
    u64 exec_ms = 0;
    is_timeout = false;
    s32 res;

    int status = 0;
    u32 tb4;

    child_timed_out = 0;

    // Init forkserver if it is not started.
    if (forksrv_pid == -1) {
        init_forkserver(argv);
    }

    /* After this memset, trace_bits[] are effectively volatile, so we
     must prevent any earlier operations from venturing into that
     territory. */

BEGIN:

    write_to_testcase(cmd_str);

    // Send the signal to notify the CockroachDB to start executions.
    // If the is_reset_server_only is 1, then the CockroachDB server
    // will reset its database.
    while ((res = write(fsrv_ctl_fd, &is_reset_server,
                sizeof(is_reset_server)))
        != 4) {
        if (stop_soon) {
            return FAULT_NONE;
        }

        // Make sure the CockroachDB process is restart correctly.
        restart_dbms(argv);

        memset(trace_bits, 0, MAP_SIZE);
        MEM_BARRIER();
    }

    //  if (cmd_str == "") {
    //      return FAULT_NONE;
    //  }

    /* Inside the parent process.
  // Wait for the child process.
  // Check the execution status.
  // Let the signal handler handle the timeout situation.
  */

    // Setup the timeout struct.
    it.it_value.tv_sec = (this->get_exec_tmout() / 1000);
    it.it_value.tv_usec = (this->get_exec_tmout() % 1000) * 1000;

    setitimer(ITIMER_REAL, &it, NULL);

    if ((res = read(fsrv_st_fd, &status, 4)) != 4) {

        /* Get the timeout message before looping the forksrv_pid.  */
        bool cur_is_timeout = is_timeout;

        cerr << "The CockroachDB process is not responding? Could be timeout "
                "killed or crashed. is_timeout: "
             << cur_is_timeout << "\n\n\n";

        // Clean up the fd before calling init_forkserver.
        close(fsrv_ctl_fd);
        close(fsrv_st_fd);

        // Block the execution until handle_timeout has been finished.
        do {
        } while (forksrv_pid != -1);

        // Restart the argv execution.
        init_forkserver(argv);

        // Return the error.
        if (cur_is_timeout) {
            return FAULT_TMOUT;
        } else {
            return FAULT_CRASH;
        }
    }

    if (status > 0) {
        // Reach maximum time execution, CockroachDB auto exit.
        // Relaunch CockroachDB.
        close(fsrv_ctl_fd);
        close(fsrv_st_fd);
        forksrv_pid.store(-1);
        init_forkserver(argv);
    }

    getitimer(ITIMER_REAL, &it);
    exec_ms = (u64)this->get_exec_tmout() - (it.it_value.tv_sec * 1000 + it.it_value.tv_usec / 1000);

    // Cancel the SIGALRM timer.
    it.it_value.tv_sec = 0;
    it.it_value.tv_usec = 0;

    setitimer(ITIMER_REAL, &it, NULL);

    // Log the query execution results.
    string res_str = read_file_to_str("query_res_out.txt");
    this->p_res_handl->set_tmp_cur_res(res_str);

    prev_timed_out = child_timed_out;

    /* It makes sense to account for the slowest units only if the testcase was
  run under the user defined timeout. */
    if (this->get_exec_tmout() <= exec_tmout && (slowest_exec_ms < exec_ms)) {
        slowest_exec_ms = exec_ms;
    }

    return FAULT_NONE;
}

void CockroachDBConnector::init_forkserver(char** argv)
{

    static struct itimerval it;
    it.it_interval.tv_usec = 0;
    it.it_interval.tv_sec = 0;
    int st_pipe[2], ctl_pipe[2];
    int status;
    s32 rlen;

    child_timed_out = 0;

    ACTF("Spinning up the fork server...");

    if (pipe(st_pipe) || pipe(ctl_pipe))
        PFATAL("pipe() failed");

    forksrv_pid = fork();

    if (forksrv_pid < 0)
        PFATAL("fork() failed");

    if (!forksrv_pid) {
        // Child process.

        struct rlimit r;

        /* Umpf. On OpenBSD, the default fd limit for root users is set to
       soft 128. Let's try to fix that... */

        if (!getrlimit(RLIMIT_NOFILE, &r) && r.rlim_cur < FORKSRV_FD + 2) {

            r.rlim_cur = FORKSRV_FD + 2;
            setrlimit(RLIMIT_NOFILE, &r); /* Ignore errors */
        }

        if (mem_limit) {

            r.rlim_max = r.rlim_cur = ((rlim_t)mem_limit) << 20;

#ifdef RLIMIT_AS

            setrlimit(RLIMIT_AS, &r); /* Ignore errors */

#else

            /* This takes care of OpenBSD, which doesn't have RLIMIT_AS, but
     according to reliable sources, RLIMIT_DATA covers anonymous
     maps - so we should be getting good protection against OOM bugs. */

            setrlimit(RLIMIT_DATA, &r); /* Ignore errors */

#endif /* ^RLIMIT_AS */
        }

        /* Dumping cores is slow and can lead to anomalies if SIGKILL is delivered
       before the dump is complete. */

        r.rlim_max = r.rlim_cur = 0;

        setrlimit(RLIMIT_CORE, &r); /* Ignore errors */

        /* Isolate the process and configure standard descriptors. If out_file is
       specified, stdin is /dev/null; otherwise, out_fd is cloned instead. */

        setsid();

        // Close the stdin, stdout and stderr.
        dup2(dev_null_fd, 0);
        dup2(dev_null_fd, 1);
        dup2(dev_null_fd, 2);

        /* Set up control and status pipes, close the unneeded original fds. */
        // FORKSRV_FD == 198
        if (dup2(ctl_pipe[0], FORKSRV_FD) < 0)
            PFATAL("dup2() failed");
        // FD == 199
        if (dup2(st_pipe[1], FORKSRV_FD + 1) < 0)
            PFATAL("dup2() failed");

        close(ctl_pipe[0]);
        close(ctl_pipe[1]);
        close(st_pipe[0]);
        close(st_pipe[1]);

        close(out_dir_fd);
        close(dev_null_fd);
        close(dev_urandom_fd);
        if (plot_file != nullptr) {
            close(fileno(plot_file));
        }

        /* This should improve performance a bit, since it stops the linker from
       doing extra work post-fork(). */

        if (!getenv("LD_BIND_LAZY"))
            setenv("LD_BIND_NOW", "1", 0);

        /* Set sane defaults for ASAN if nothing else specified. */

        //            setenv("ASAN_OPTIONS",
        //                   "abort_on_error=1:"
        //                   "detect_leaks=0:"
        //                   "symbolize=0:"
        //                   "allocator_may_return_null=1",
        //                   0);

        /* MSAN is tricky, because it doesn't support abort_on_error=1 at this
       point. So, we do this in a very hacky way. */

        //            setenv("MSAN_OPTIONS",
        //                   "exit_code=" STRINGIFY(MSAN_ERROR) ":"
        //                   "symbolize=0:"
        //                   "abort_on_error=1:"
        //                   "allocator_may_return_null=1:"
        //                   "msan_track_origins=0",
        //                   0);

        const char* argv_list[] = { "./covtest.test", nullptr };
        execv("./covtest.test", const_cast<char* const*>(argv_list));
        cerr << "Fatal Error: Should not reach this point. \n\n\n";

        *(u32*)trace_bits = EXEC_FAIL_SIG;
        exit(0);
    }

    /* Close the unneeded endpoints. */

    close(ctl_pipe[0]);
    close(st_pipe[1]);

    fsrv_ctl_fd = ctl_pipe[1];
    fsrv_st_fd = st_pipe[0];

    /* Wait for the fork server to come up, but don't wait too long. */

    it.it_value.tv_sec = ((exec_tmout * FORK_WAIT_MULT) / 1000);
    it.it_value.tv_usec = ((exec_tmout * FORK_WAIT_MULT) % 1000) * 1000;

    setitimer(ITIMER_REAL, &it, NULL);

    rlen = read(fsrv_st_fd, &status, 4);
    //  child_pid = status;
    //  assert(child_pid != -1);

    it.it_value.tv_sec = 0;
    it.it_value.tv_usec = 0;

    setitimer(ITIMER_REAL, &it, NULL);

    /* If we have a four-byte "hello" message from the server, we're all set.
     Otherwise, try to figure out what went wrong. */

    if (rlen == 4) {
        OKF("All right - fork server is up.");
        return;
    }

    if (child_timed_out)
        FATAL("Timeout while initializing fork server (adjusting -t may help)");

    if (waitpid(forksrv_pid, &status, 0) <= 0)
        PFATAL("waitpid() failed");

    if (WIFSIGNALED(status)) {

        if (mem_limit && mem_limit < 500 && uses_asan) {

            SAYF("\n" cLRD "[-] " cRST "Whoops, the target binary crashed suddenly, "
                 "before receiving any input\n"
                 "    from the fuzzer! Since it seems to be built with ASAN and you "
                 "have a\n"
                 "    restrictive memory limit configured, this is expected; please "
                 "read\n"
                 "    %s/notes_for_asan.txt for help.\n",
                doc_path);
        } else if (!mem_limit) {

            SAYF(
                "\n" cLRD "[-] " cRST "Whoops, the target binary crashed suddenly, "
                "before receiving any input\n"
                "    from the fuzzer! There are several probable explanations:\n\n"

                "    - The binary is just buggy and explodes entirely on its own. If "
                "so, you\n"
                "      need to fix the underlying problem or find a better "
                "replacement.\n\n"

#ifdef __APPLE__

                "    - On MacOS X, the semantics of fork() syscalls are non-standard "
                "and may\n"
                "      break afl-fuzz performance optimizations when running "
                "platform-specific\n"
                "      targets. To fix this, set AFL_NO_FORKSRV=1 in the "
                "environment.\n\n"

#endif /* __APPLE__ */

                "    - Less likely, there is a horrible bug in the fuzzer. If other "
                "options\n"
                "      fail, poke <lcamtuf@coredump.cx> for troubleshooting tips.\n");
        } else {

            SAYF("\n" cLRD "[-] " cRST "Whoops, the target binary crashed suddenly, "
                 "before receiving any input\n"
                 "    from the fuzzer! There are several probable explanations:\n\n"

                 "    - The current memory limit (%s) is too restrictive, causing "
                 "the\n"
                 "      target to hit an OOM condition in the dynamic linker. Try "
                 "bumping up\n"
                 "      the limit with the -m setting in the command line. A simple "
                 "way confirm\n"
                 "      this diagnosis would be:\n\n"

#ifdef RLIMIT_AS
                 "      ( ulimit -Sv $[%llu << 10]; /path/to/fuzzed_app )\n\n"
#else
                 "      ( ulimit -Sd $[%llu << 10]; /path/to/fuzzed_app )\n\n"
#endif /* ^RLIMIT_AS */

                 "      Tip: you can use http://jwilk.net/software/recidivm to "
                 "quickly\n"
                 "      estimate the required amount of virtual memory for the "
                 "binary.\n\n"

                 "    - The binary is just buggy and explodes entirely on its own. "
                 "If so, you\n"
                 "      need to fix the underlying problem or find a better "
                 "replacement.\n\n"

#ifdef __APPLE__

                 "    - On MacOS X, the semantics of fork() syscalls are "
                 "non-standard and may\n"
                 "      break afl-fuzz performance optimizations when running "
                 "platform-specific\n"
                 "      targets. To fix this, set AFL_NO_FORKSRV=1 in the "
                 "environment.\n\n"

#endif /* __APPLE__ */

                 "    - Less likely, there is a horrible bug in the fuzzer. If other "
                 "options\n"
                 "      fail, poke <lcamtuf@coredump.cx> for troubleshooting tips.\n",
                DMS(mem_limit << 20), mem_limit - 1);
        }

        FATAL("Fork server crashed with signal %d", WTERMSIG(status));
    }

    if (*(u32*)trace_bits == EXEC_FAIL_SIG)
        FATAL("Unable to execute target application ('%s')", argv[0]);

    if (mem_limit && mem_limit < 500 && uses_asan) {

        SAYF("\n" cLRD "[-] " cRST "Hmm, looks like the target binary terminated "
             "before we could complete a\n"
             "    handshake with the injected code. Since it seems to be built "
             "with ASAN and\n"
             "    you have a restrictive memory limit configured, this is "
             "expected; please\n"
             "    read %s/notes_for_asan.txt for help.\n",
            doc_path);
    } else if (!mem_limit) {

        SAYF("\n" cLRD "[-] " cRST "Hmm, looks like the target binary terminated "
             "before we could complete a\n"
             "    handshake with the injected code. Perhaps there is a horrible "
             "bug in the\n"
             "    fuzzer. Poke <lcamtuf@coredump.cx> for troubleshooting tips.\n");
    } else {

        SAYF(
            "\n" cLRD "[-] " cRST "Hmm, looks like the target binary terminated "
            "before we could complete a\n"
            "    handshake with the injected code. There are %s probable "
            "explanations:\n\n"

            "%s"
            "    - The current memory limit (%s) is too restrictive, causing an "
            "OOM\n"
            "      fault in the dynamic linker. This can be fixed with the -m "
            "option. A\n"
            "      simple way to confirm the diagnosis may be:\n\n"

#ifdef RLIMIT_AS
            "      ( ulimit -Sv $[%llu << 10]; /path/to/fuzzed_app )\n\n"
#else
            "      ( ulimit -Sd $[%llu << 10]; /path/to/fuzzed_app )\n\n"
#endif /* ^RLIMIT_AS */

            "      Tip: you can use http://jwilk.net/software/recidivm to quickly\n"
            "      estimate the required amount of virtual memory for the "
            "binary.\n\n"

            "    - Less likely, there is a horrible bug in the fuzzer. If other "
            "options\n"
            "      fail, poke <lcamtuf@coredump.cx> for troubleshooting tips.\n",
            getenv(DEFER_ENV_VAR) ? "three" : "two",
            getenv(DEFER_ENV_VAR)
                ? "    - You are using deferred forkserver, but __AFL_INIT() is "
                  "never\n"
                  "      reached before the program terminates.\n\n"
                : "",
            DMS(mem_limit << 20), mem_limit - 1);
    }

    //        FATAL("Fork server handshake failed");
    this->forksrv_pid = -1;
}

/* Tell the DBMS to output the code coverage information. */
void CockroachDBConnector::record_code_coverage(char** argv)
{
    memset(trace_bits, 0, MAP_SIZE);
    MEM_BARRIER();

    // Send the signal to notify the CockroachDB to start executions.
    // If the is_reset_server_only is 1, then the CockroachDB server
    // will reset its database.
    int log_cov_flag = 3;
    while ((write(fsrv_ctl_fd, &log_cov_flag, sizeof(log_cov_flag))) != 4) {
        if (stop_soon) {
            return;
        }
        // Make sure the CockroachDB process is restart correctly.
        restart_dbms(argv);
    }

    /* Inside the parent process.
// Wait for the child process.
// Check the code coverage.
*/

    int status;
    ssize_t _ = read(static_cast<int>(fsrv_st_fd), &status, 4);

    if (filesystem::exists("./cov_out.bin")) {
        ifstream fin("./cov_out.bin", ios::in | ios::binary);
        fin.read((char*)trace_bits, MAP_SIZE);
        fin.close();
        // Remove the file. Ignore the returned value.
        remove("./cov_out.bin");
    }
    classify_counts((u64*)trace_bits);

    return;
}

void CockroachDBConnector::write_to_testcase(string& input)
{
    ofstream query_input;
    query_input.open("./input_query.sql", ofstream::out);
    query_input << input;
    query_input.close();
    return;
}

void CockroachDBConnector::restart_dbms(char** argv)
{

    // Exit the current CockroachDB server.
    //    int status = 0;
    //    int set_int = 2;
    //    write(fsrv_ctl_fd, &set_int, sizeof(set_int));
    //    read(fsrv_st_fd, &status, 4);
    if (forksrv_pid != -1) {
        kill(forksrv_pid, SIGKILL);
        int status = 0;
        wait(&status);
    }

    close(fsrv_st_fd);
    close(fsrv_ctl_fd);

    forksrv_pid = -1;
    while (forksrv_pid == -1) {
        // For CockroachDB, init_forkserver is just restart the process.
        init_forkserver(argv);
    }
}