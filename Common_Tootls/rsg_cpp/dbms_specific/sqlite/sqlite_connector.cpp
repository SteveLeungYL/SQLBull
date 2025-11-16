//
// Created by XXX on 12/10/24.
//

#include "sqlite_connector.h"
#include "../../headers/types.h"
#include "../../headers/utils.h"
#include <fstream>
#include <iostream>
#include <sys/time.h>

unsigned char SQLiteConnector::run_target(char** argv, string cmd_str,
    int is_reset_server)
{
    this->write_to_testcase(cmd_str);

    static struct itimerval it;
    static u32 prev_timed_out = 0;
    static u64 exec_ms = 0;
    is_timeout = false;

    int status = 0;
    u32 tb4;

    child_timed_out = 0;
    //  child_has_stop = 0;

    while (forksrv_pid == -1) {
        child_timed_out = 0;
        this->init_forkserver(argv);
    }

    /* After this memset, trace_bits[] are effectively volatile, so we
       must prevent any earlier operations from venturing into that
       territory. */

    memset(trace_bits, 0, MAP_SIZE);
    MEM_BARRIER();

    /* If we're running in "dumb" mode, we can't rely on the fork server
       logic compiled into the target program, so we will just keep calling
       execve(). There is a bit of code duplication between here and
       init_forkserver(), but c'est la vie. */

    s32 res;

    /* In non-dumb mode, we have the fork server up and running, so simply
       tell it to have at it, and then read back PID. */

    if ((res = write(fsrv_ctl_fd, &prev_timed_out, 4)) != 4) {

        if (stop_soon)
            return 0;
        RPFATAL(res, "Unable to request new process from fork server (OOM?)");
    }

    if ((res = read(fsrv_st_fd, &child_pid, 4)) != 4) {

        if (stop_soon)
            return 0;
        RPFATAL(res, "Unable to request new process from fork server (OOM?)");
    }

    if (child_pid <= 0)
        FATAL("Fork server is misbehaving (OOM?)");

    /* Configure timeout, as requested by user, then wait for child to terminate.
     */

    it.it_value.tv_sec = (this->get_exec_tmout() / 1000);
    it.it_value.tv_usec = (this->get_exec_tmout() % 1000) * 1000;

    setitimer(ITIMER_REAL, &it, NULL);

    if (is_reset_server) {
        kill(child_pid, SIGKILL);
        int status;
        waitpid(child_pid, &status, 0);
        child_pid = -1;
        prev_timed_out = 1;
        //    child_has_stop = 1;
        child_timed_out = 1;
    }

    /* The SIGALRM handler simply kills the child_pid and sets child_timed_out. */

    if ((res = read(fsrv_st_fd, &status, 4)) != 4) {

        if (stop_soon)
            return 0;
        // return timeout for the error?
        return FAULT_TMOUT;
        // RPFATAL(res, "Unable to communicate with fork server (OOM?)");
    }

    if (!WIFSTOPPED(status)) {
        child_pid = 0;
        //    child_has_stop = 1;
    }

    getitimer(ITIMER_REAL, &it);
    exec_ms = (u64)this->get_exec_tmout() - (it.it_value.tv_sec * 1000 + it.it_value.tv_usec / 1000);

    it.it_value.tv_sec = 0;
    it.it_value.tv_usec = 0;

    setitimer(ITIMER_REAL, &it, NULL);

    /* Any subsequent operations on trace_bits must not be moved by the
       compiler below this point. Past this location, trace_bits[] behave
       very normally and do not have to be treated as volatile. */

    MEM_BARRIER();

    tb4 = *(u32*)trace_bits;

    classify_counts((u64*)trace_bits);

    prev_timed_out = child_timed_out;

    /* Report outcome to caller. */

    if (WIFSIGNALED(status) && !stop_soon) {
        kill_signal = WTERMSIG(status);

        p_res_handl->set_tmp_cur_res(this->read_sqlite_output_and_reset_output_file());

        if (!is_reset_server) {
            if (child_timed_out && kill_signal == SIGKILL)
                return FAULT_TMOUT;
            return FAULT_CRASH;
        }
    }

    // /* A somewhat nasty hack for MSAN, which doesn't support abort_on_error and
    //    must use a special exit code. */

    if (uses_asan && WEXITSTATUS(status) == MSAN_ERROR) {
        kill_signal = 0;
        p_res_handl->set_tmp_cur_res(this->read_sqlite_output_and_reset_output_file());
        return FAULT_CRASH;
    }

    /* It makes sense to account for the slowest units only if the testcase was
    run under the user defined this->get_exec_tmout(). */
    if (!(this->get_exec_tmout() > exec_tmout) && (slowest_exec_ms < exec_ms)) {
        slowest_exec_ms = exec_ms;
    }

    p_res_handl->set_tmp_cur_res(this->read_sqlite_output_and_reset_output_file());
    return FAULT_NONE;
}

void SQLiteConnector::init_forkserver(char** argv)
{
    static struct itimerval it;
    it.it_value.tv_sec = 0;
    it.it_value.tv_usec = 0;
    setitimer(ITIMER_REAL, &it, NULL);

    this->setup_stdio();
    OKF(cPIN "Persistent mode binary detected.");
    setenv(PERSIST_ENV_VAR, "1", 1);

    int st_pipe[2], ctl_pipe[2];
    int status;
    s32 rlen;

    ACTF("Spinning up the fork server...");

    if (pipe(st_pipe) || pipe(ctl_pipe))
        PFATAL("pipe() failed");

    forksrv_pid = fork();

    if (forksrv_pid < 0)
        PFATAL("fork() failed");

    if (!forksrv_pid) {

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

        dup2(program_output_fd, 1);
        dup2(program_output_fd,
            2); // Redirect stdout and stderr to the output file.
        dup2(out_fd, 0); // Redirect stdin to input file.

        /* Set up control and status pipes, close the unneeded original fds. */

        if (dup2(ctl_pipe[0], FORKSRV_FD) < 0)
            PFATAL("dup2() failed");
        if (dup2(st_pipe[1], FORKSRV_FD + 1) < 0)
            PFATAL("dup2() failed");

        close(ctl_pipe[0]);
        close(ctl_pipe[1]);
        close(st_pipe[0]);
        close(st_pipe[1]);

        close(out_dir_fd);
        close(program_output_fd);
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

        setenv("ASAN_OPTIONS",
            "abort_on_error=1:"
            "detect_leaks=0:"
            "symbolize=0:"
            "allocator_may_return_null=1",
            0);

        // /* MSAN is tricky, because it doesn't support abort_on_error=1 at this
        //    point. So, we do this in a very hacky way. */

        setenv("MSAN_OPTIONS",
            "exit_code=" STRINGIFY(MSAN_ERROR) ":"
                                               "symbolize=0:"
                                               "abort_on_error=1:"
                                               "allocator_may_return_null=1:"
                                               "msan_track_origins=0",
            0);

        execv("./sqlite3", argv); // Used for start up sqlite3 ???

        /* Use a distinctive bitmap signature to tell the parent about execv()
           falling through. */

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
    child_pid = status;
    assert(child_pid != -1);

    it.it_value.tv_sec = 0;
    it.it_value.tv_usec = 0;

    setitimer(ITIMER_REAL, &it, NULL);

    /* If we have a four-byte "hello" message from the server, we're all set.
       Otherwise, try to figure out what went wrong. */

    if (rlen == 4) {

        // Start the SQLite child process.
        OKF("All right - fork server is up.");
        return;
    }

    // Should not reach here. 
    // Retry. 
    this->forksrv_pid = -1;
    return;

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

    FATAL("Fork server handshake failed");
}

void SQLiteConnector::restart_dbms(char** argv)
{
    while (this->forksrv_pid == -1) {
        cerr << "Running init_forkserver() on SQLite. " << endl;
        this->init_forkserver(argv);
    }

    this->run_target(argv, "SELECT 'abc';", true); // Run a dummy statement while resetting the SQLite process.
}

void SQLiteConnector::write_to_testcase(string& in)
{
    /* After this memset, trace_bits[] are effectively volatile, so we
       must prevent any earlier operations from venturing into that
       territory. */
    lseek(out_fd, 0, SEEK_SET);
    ck_write(out_fd, in.c_str(), in.size(), out_file.c_str());
    if (ftruncate(out_fd, in.size())) {
        PFATAL("ftruncate() failed");
    }
}

void SQLiteConnector::record_code_coverage(char** argv)
{
    /* EMPTY since trace_bits is synced with the SQLite process. No need for explicit recording. */
}

string SQLiteConnector::read_sqlite_output_and_reset_output_file()
{
    // Alternative to read_file_to_str in util.h.
    // This is faster when using shared memory, faster in sqlite communication.
    string program_output_str;
    program_output_str.reserve(100);
    char output_buf[1024 + 1];

    if (child_timed_out) {
        program_output_str.clear();
        return program_output_str;
    }

    lseek(program_output_fd, 0, SEEK_SET);

    // Read maximum 2k bytes of data.
    for (int idx = 0; idx < 2; idx++) {

        ssize_t num_bytes = read(program_output_fd, output_buf, 1024);
        if (num_bytes == 0 || output_buf[0] == '\0')
            break;

        output_buf[num_bytes] = '\0';
        program_output_str += output_buf;
    }

    int tmp = ftruncate(program_output_fd, 0);
    lseek(program_output_fd, 0, SEEK_SET);

    return program_output_str;
}