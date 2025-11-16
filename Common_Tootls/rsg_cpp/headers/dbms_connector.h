//
// Created by XXX on 3/25/24.
//

#ifndef RSG_CPP_DBMS_CONNECTOR_H
#define RSG_CPP_DBMS_CONNECTOR_H

#include <atomic>
#include <cassert>
#include <cstring>
#include <fcntl.h>
#include <filesystem>
#include <iostream>
#include <string>
#include <sys/mman.h>
#include <sys/resource.h> // Don't remove
#include <sys/shm.h>
#include <sys/wait.h> // Don't remove

#include "alloc-inl.h"
#include "config.h"
#include "fstream"
#include "results_handler.h"
#include "signal.h"
#include "types.h"
#include "unistd.h"

using namespace std;

enum {
    /* 00 */ FAULT_NONE,
    /* 01 */ FAULT_TMOUT,
    /* 02 */ FAULT_CRASH,
    /* 03 */ FAULT_ERROR,
    /* 04 */ FAULT_NOINST,
    /* 05 */ FAULT_NOBITS,
    /* 06 */ FAULT_SQLERROR
};

class DBMSConnector {
public:
    virtual void set_stop_soon(u8 in)
    {
        this->stop_soon = in;
    }

    virtual void set_hang_tmout(u32 in)
    {
        this->hang_tmout = in;
    }

    virtual void set_mem_limit(u64 in)
    {
        this->mem_limit = in;
    }

    virtual void set_out_dir_fd(s32 in)
    {
        this->out_dir_fd = in;
    }

    virtual void set_child_timed_out(u8 in)
    {
        this->child_timed_out = in;
    }

    virtual void set_bind_to_core_id(int in)
    {
        this->bind_to_core_id = in;
    }

    virtual void set_p_plot_file(FILE* in)
    {
        this->plot_file = in;
    }

    virtual void set_uses_asan(u8 in)
    {
        this->uses_asan = in;
    }

    virtual void set_doc_path(char* in)
    {
        this->doc_path = in;
    }

    virtual void set_exec_tmout(u64 in)
    {
        this->exec_tmout = in;
    }

    virtual void set_trace_bits(u8* in)
    {
        this->trace_bits = in;
    }

    virtual void set_virgin_bits(u8* in)
    {
        this->virgin_bits = in;
    }

    virtual void set_bitmap_changed(u8 in)
    {
        this->bitmap_changed = in;
    }

    virtual void set_dump_library(u8 in)
    {
        this->dump_library = in;
    }

    virtual void set_forksrv_pid(int in)
    {
        this->forksrv_pid.store(in);
    }

    virtual void set_child_pid(int in)
    {
        this->child_pid = in;
    }

    virtual void set_is_timeout(u8 in)
    {
        this->is_timeout = in;
    }

    virtual void set_shm_size(unsigned long in)
    {
        this->shm_size = in;
    }

    virtual void set_socket_path(string in)
    {
        this->socket_path = in;
    }

    virtual void set_bind_to_port(int in)
    {
        this->bind_to_port = in;
    }

    [[nodiscard]] u64 get_exec_tmout()
    {
        return this->exec_tmout;
    }

    [[nodiscard]] u64 get_slowest_exec_ms()
    {
        return this->slowest_exec_ms;
    }

    [[nodiscard]] u32 get_hang_tmout()
    {
        return this->hang_tmout;
    }

    [[nodiscard]] u64 get_mem_limit()
    {
        return this->mem_limit;
    }

    [[nodiscard]] s32 get_out_dir_fd()
    {
        return this->out_dir_fd;
    }

    [[nodiscard]] s32 get_dev_urandom_fd()
    {
        return this->dev_urandom_fd;
    }

    [[nodiscard]] FILE* get_p_plot_file()
    {
        return this->plot_file;
    }

    [[nodiscard]] u8 get_uses_asan()
    {
        return this->uses_asan;
    }

    [[nodiscard]] char* get_doc_path()
    {
        return this->doc_path;
    }

    [[nodiscard]] u8* get_trace_bits()
    {
        return this->trace_bits;
    }

    [[nodiscard]] u8* get_virgin_bits()
    {
        return this->virgin_bits;
    }

    [[nodiscard]] u8 get_bitmap_changed()
    {
        return this->bitmap_changed;
    }

    [[nodiscard]] u8 get_dump_library()
    {
        return this->dump_library;
    }

    [[nodiscard]] int get_forksrv_pid()
    {
        return this->forksrv_pid.load();
    }

    [[nodiscard]] int get_child_pid()
    {
        return this->child_pid;
    }

    [[nodiscard]] int get_is_timeout()
    {
        return this->is_timeout;
    }

    virtual unsigned char run_target(char** argv, string cmd_str,
        int is_reset_server = 1)
        = 0;

    /* Spin up fork server (instrumented mode only). The idea is explained here:

       http://lcamtuf.blogspot.com/2014/10/fuzzing-binaries-without-execve.html

       In essence, the instrumentation allows us to skip execve(), and just keep
       cloning a stopped child. So, we just execute once, and then send commands
       through a pipe. The other part of this logic is in afl-as.h. */
    virtual void init_forkserver(char** argv) = 0;

    virtual void reset_database_without_restart(char** argv, const unsigned int& exec_tmout)
    {
        run_target(argv, "", 1);
    }

    virtual void restart_dbms(char** argv) = 0;

    virtual void write_to_testcase(string&) = 0;

    /* Tell the DBMS to output the code coverage information. */
    virtual void record_code_coverage(char** argv) = 0;

    DBMSConnector(ResultHandler* p_res_in)
        : forksrv_pid(-1)
        , child_pid(-1)
        , is_timeout(false)
        , kill_signal(0)
        , out_fd(-1)
        , program_output_fd(-1)
        , child_timed_out(0)
        , fsrv_ctl_fd(0)
        , fsrv_st_fd(0)
        , p_res_handl(p_res_in)
        , stop_soon(0)
        , exec_tmout(EXEC_TIMEOUT)
        , hang_tmout(EXEC_TIMEOUT)
        , slowest_exec_ms(0)
        , bitmap_changed(0)
        , map_file_id(0)
        , bind_to_core_id(-1)
        , mem_limit(MEM_LIMIT)
        , dev_null_fd(-1)
        , dev_urandom_fd(-1)
        , out_dir_fd(-1)
        , plot_file(nullptr)
        , uses_asan(0)
    {
        // Setup shm.
        trace_bits = (u8*)malloc(MAP_SIZE);
        virgin_bits = (u8*)malloc(MAP_SIZE);
        virgin_tmout = (u8*)malloc(MAP_SIZE);
        virgin_crash = (u8*)malloc(MAP_SIZE);
        count_class_lookup16 = (u16*)malloc(2 * 65536);
        count_class_lookup8 = (u8*)malloc(256);
        simplify_lookup = (u8*)malloc(256);

        memset(virgin_bits, 255, MAP_SIZE);
        memset(virgin_tmout, 255, MAP_SIZE);
        memset(virgin_crash, 255, MAP_SIZE);
        memset(trace_bits, 0, MAP_SIZE);
        memset_array();
        init_count_class16();

        map_id_out_f.open("./map_id_triggered_" + std::to_string(bind_to_core_id) + ".txt",
            std::ofstream::out | std::ofstream::trunc);

        dev_null_fd = open("/dev/null", O_RDWR);
        if (dev_null_fd < 0)
            PFATAL("Unable to open /dev/null");

        dev_urandom_fd = open("/dev/urandom", O_RDONLY);
        if (dev_urandom_fd < 0)
            PFATAL("Unable to open /dev/urandom");
    };

    virtual ~DBMSConnector()
    {
        if (shm_id != -1) {
            // Remove the shared memory, if allocated.
            shmctl(shm_id, IPC_RMID, NULL);
        } else {
            // Just simple malloc, just free.
            free(trace_bits);
        }
        free(virgin_bits);
        free(virgin_tmout);
        free(virgin_crash);
        free(count_class_lookup16);
        free(count_class_lookup8);
        free(simplify_lookup);
        if (out_fd != -1) {
            close(out_fd);
        }
        if (program_output_fd != -1) {
            close(program_output_fd);
        }
    };

    /* Code coverage related helper functions.  */

    void memset_array()
    {
        simplify_lookup[0] = 1;
        memset(simplify_lookup + 1, 128, 255);

        count_class_lookup8[0] = 0;
        count_class_lookup8[1] = 1;
        count_class_lookup8[2] = 2;
        count_class_lookup8[3] = 4;
        memset(count_class_lookup8 + 4, 8, 7 - 4 + 1);
        memset(count_class_lookup8 + 8, 16, 15 - 8 + 1);
        memset(count_class_lookup8 + 16, 32, 32 - 16);
        memset(count_class_lookup8 + 32, 64, 128 - 32);
        memset(count_class_lookup8 + 128, 128, 128);
    }

    void init_count_class16(void)
    {

        u32 b1, b2;

        for (b1 = 0; b1 < 256; b1++)
            for (b2 = 0; b2 < 256; b2++)
                count_class_lookup16[(b1 << 8) + b2] = (count_class_lookup8[b1] << 8) | count_class_lookup8[b2];
    }

#if defined(__x86_64__) || defined(__arm64__) || defined(__aarch64__)
    inline void classify_counts(u64* mem)
    {

        u32 i = MAP_SIZE >> 3;

        while (i--) {

            /* Optimize for sparse bitmaps. */

            if (unlikely(*mem)) {

                u16* mem16 = (u16*)mem;

                mem16[0] = count_class_lookup16[mem16[0]];
                mem16[1] = count_class_lookup16[mem16[1]];
                mem16[2] = count_class_lookup16[mem16[2]];
                mem16[3] = count_class_lookup16[mem16[3]];
            }

            mem++;
        }
    }

#else

    inline void classify_counts(u32* mem)
    {

        u32 i = MAP_SIZE >> 2;

        while (i--) {

            /* Optimize for sparse bitmaps. */

            if (unlikely(*mem)) {

                u16* mem16 = (u16*)mem;

                mem16[0] = count_class_lookup16[mem16[0]];
                mem16[1] = count_class_lookup16[mem16[1]];
            }

            mem++;
        }
    }

#endif /* ^__x86_64__ */

#define FF(_b) (0xff << ((_b) << 3))

    /* Count the number of non-255 bytes set in the bitmap. Used strictly for the
       status screen, several calls per second or so. */
    u32 count_non_255_bytes(u8* mem)
    {

        u32* ptr = (u32*)mem;
        u32 i = (MAP_SIZE >> 2);
        u32 ret = 0;

        while (i--) {

            u32 v = *(ptr++);

            /* This is called on the virgin bitmap, so optimize for the most likely
           case. */

            if (v == 0xffffffff)
                continue;
            if ((v & FF(0)) != FF(0))
                ret++;
            if ((v & FF(1)) != FF(1))
                ret++;
            if ((v & FF(2)) != FF(2))
                ret++;
            if ((v & FF(3)) != FF(3))
                ret++;
        }

        return ret;
    }

    /* Count the number of bytes set in the bitmap. Called fairly sporadically,
       mostly to update the status screen or calibrate and examine confirmed
       new paths. */
    u32 count_bytes(u8* mem)
    {

        u32* ptr = (u32*)mem;
        u32 i = (MAP_SIZE >> 2);
        u32 ret = 0;

        while (i--) {

            u32 v = *(ptr++);

            if (!v)
                continue;
            if (v & FF(0))
                ret++;
            if (v & FF(1))
                ret++;
            if (v & FF(2))
                ret++;
            if (v & FF(3))
                ret++;
        }

        return ret;
    }

    /* Destructively simplify trace by eliminating hit count information
       and replacing it with 0x80 or 0x01 depending on whether the tuple
       is hit or not. Called on every new crash or timeout, should be
       reasonably fast. */

#if defined(__x86_64__) || defined(__arm64__) || defined(__aarch64__)

    void simplify_trace(u64* mem)
    {

        u32 i = MAP_SIZE >> 3;

        while (i--) {

            /* Optimize for sparse bitmaps. */

            if (unlikely(*mem)) {

                u8* mem8 = (u8*)mem;

                mem8[0] = simplify_lookup[mem8[0]];
                mem8[1] = simplify_lookup[mem8[1]];
                mem8[2] = simplify_lookup[mem8[2]];
                mem8[3] = simplify_lookup[mem8[3]];
                mem8[4] = simplify_lookup[mem8[4]];
                mem8[5] = simplify_lookup[mem8[5]];
                mem8[6] = simplify_lookup[mem8[6]];
                mem8[7] = simplify_lookup[mem8[7]];
            } else
                *mem = 0x0101010101010101ULL;

            mem++;
        }
    }

#else

    void simplify_trace(u32* mem)
    {

        u32 i = MAP_SIZE >> 2;

        while (i--) {

            /* Optimize for sparse bitmaps. */

            if (unlikely(*mem)) {

                u8* mem8 = (u8*)mem;

                mem8[0] = simplify_lookup[mem8[0]];
                mem8[1] = simplify_lookup[mem8[1]];
                mem8[2] = simplify_lookup[mem8[2]];
                mem8[3] = simplify_lookup[mem8[3]];
            } else
                *mem = 0x01010101;

            mem++;
        }
    }

#endif /* ^__x86_64__ */

    /* Write bitmap to file. The bitmap is useful mostly for the secret
       -B option, to focus a separate fuzzing session on a particular
       interesting input without rediscovering all the others. */

    void write_bitmap(char* out_dir)
    {

        char* fname;
        s32 fd;

        if (!bitmap_changed)
            return;
        bitmap_changed = 0;

        fname = (char*)(alloc_printf("%s/fuzz_bitmap", out_dir));
        fd = open(fname, O_WRONLY | O_CREAT | O_TRUNC, 0640);

        if (fd < 0)
            PFATAL("Unable to open '%s'", fname);

        ck_write(fd, virgin_bits, MAP_SIZE, fname);

        close(fd);
        ck_free(fname);
    }

    /* Read bitmap from file. This is for the -B option again. */
    void read_bitmap(u8* fname)
    {
        if (fname == nullptr) {
            return;
        }

        s32 fd = open((char*)fname, O_RDONLY);

        if (fd < 0)
            PFATAL("Unable to open '%s'", fname);

        ck_read(fd, virgin_bits, MAP_SIZE, fname);

        close(fd);
    }

    vector<u8> get_cur_new_byte(u8* cur, u8* vir)
    {
        vector<u8> new_byte_v;
        for (u8 i = 0; i < 8; i++) {
            if (cur[i] && vir[i] == 0xff)
                new_byte_v.push_back(i);
        }
        return new_byte_v;
    }

    void log_map_id(u32 i, u8 byte, const string& cur_seed_str)
    {
        if (map_id_out_f.fail()) {
            return;
        }
        if (cur_seed_str == "") {
            return;
        }
        i = (MAP_SIZE >> 3) - i - 1;
        u32 actual_idx = i * 8 + byte;

        map_id_out_f << actual_idx << "," << map_file_id << endl;

        fstream map_id_seed_output;
        if (!filesystem::exists("./queue_coverage_id_core/")) {
            filesystem::create_directory("./queue_coverage_id_core/");
        }
        map_id_seed_output.open("./queue_coverage_id_core/" +
                //            to_string(queue_cur->depth) + "_" +
                to_string(map_file_id) + ".txt",
            std::fstream::out | std::fstream::trunc);
        map_id_seed_output << cur_seed_str;
        map_id_seed_output.close();

        if (actual_idx == 0) {
            assert(false);
        }
    }

/* Describe integer as memory size. */
#define CHK_FORMAT(_divisor, _limit_mult, _fmt, _cast)          \
    do {                                                        \
        if (val < (_divisor) * (_limit_mult)) {                 \
            sprintf(tmp[cur], _fmt, ((_cast)val) / (_divisor)); \
            return reinterpret_cast<u8*>(tmp[cur]);             \
        }                                                       \
    } while (0)

    u8* DMS(u64 val)
    {

        static char tmp[12][16];
        static u8 cur;

        cur = (cur + 1) % 12;

        /* 0-9999 */
        CHK_FORMAT(1, 10000, "%llu B", u64);

        /* 10.0k - 99.9k */
        CHK_FORMAT(1024, 99.95, "%0.01f kB", double);

        /* 100k - 999k */
        CHK_FORMAT(1024, 1000, "%llu kB", u64);

        /* 1.00M - 9.99M */
        CHK_FORMAT(1024 * 1024, 9.995, "%0.02f MB", double);

        /* 10.0M - 99.9M */
        CHK_FORMAT(1024 * 1024, 99.95, "%0.01f MB", double);

        /* 100M - 999M */
        CHK_FORMAT(1024 * 1024, 1000, "%llu MB", u64);

        /* 1.00G - 9.99G */
        CHK_FORMAT(1024LL * 1024 * 1024, 9.995, "%0.02f GB", double);

        /* 10.0G - 99.9G */
        CHK_FORMAT(1024LL * 1024 * 1024, 99.95, "%0.01f GB", double);

        /* 100G - 999G */
        CHK_FORMAT(1024LL * 1024 * 1024, 1000, "%llu GB", u64);

        /* 1.00T - 9.99G */
        CHK_FORMAT(1024LL * 1024 * 1024 * 1024, 9.995, "%0.02f TB", double);

        /* 10.0T - 99.9T */
        CHK_FORMAT(1024LL * 1024 * 1024 * 1024, 99.95, "%0.01f TB", double);

#undef CHK_FORMAT

        /* 100T+ */
        strcpy((char*)tmp[cur], "infty");
        return reinterpret_cast<u8*>(tmp[cur]);
    }

    virtual void setup_stdio(void)
    {
        pid_t pid = getpid();
        out_file = "./.cur_input_" + to_string(pid);

        unlink(out_file.c_str()); /* ignore errors */
        out_fd = open(out_file.c_str(), O_RDWR | O_CREAT | O_EXCL, 0640);

        program_output_file = "./.cur_output_" + to_string(pid);

        unlink(program_output_file.c_str()); /* Ignore errors */
        program_output_fd = open(program_output_file.c_str(), O_RDWR | O_CREAT | O_EXCL, 0640);

        if (out_fd < 0)
            PFATAL("Unable to create '%s'", out_file.c_str());
        if (program_output_fd < 0)
            PFATAL("Unable to create '%s'", program_output_file.c_str());
    }

    virtual void setup_stdio_shm(void)
    {

        if (out_fd != -1) {
            return;
        }

        pid_t pid = getpid();
        out_file = "/." + to_string(bind_to_core_id) + "_cur_input_" + to_string(pid);

        // shm_unlink(out_file.c_str()); /* ignore errors */
        out_fd = shm_open(out_file.c_str(), O_RDWR | O_CREAT | O_EXCL, 0640);

        program_output_file = "/.cur_output_" + to_string(pid);

        // shm_unlink(program_output_file.c_str()); /* Ignore errors */
        program_output_fd = shm_open(program_output_file.c_str(), O_RDWR | O_CREAT | O_EXCL, 0640);

        if (out_fd < 0)
            PFATAL("Unable to create shm: '%s'", out_file.c_str());
        if (program_output_fd < 0)
            PFATAL("Unable to create shm: '%s'", program_output_file.c_str());
    }

    /* Check if the current execution path brings anything new to the table.
       Update virgin bits to reflect the finds. Returns 1 if the only change is
       the hit-count for a particular tuple; 2 if there are new tuples seen.
       Updates the map, so subsequent calls will always return 0.

       This function is called after every exec() on a fairly large buffer, so
       it needs to be fast. We do this in 32-bit and 64-bit flavors. */

    inline u8 has_new_bits(u8* virgin_map, const string cur_seed_str = "")
    {

        if (dump_library) {
            map_file_id++;
        }

#if defined(__x86_64__) || defined(__arm64__) || defined(__aarch64__)

        u64* current = (u64*)this->get_trace_bits();
        u64* virgin = (u64*)virgin_map;

        u32 i = (MAP_SIZE >> 3);

#else

        u32* current = (u32*)p_dbms_connector->get_trace_bits();
        u32* virgin = (u32*)virgin_map;

        u32 i = (MAP_SIZE >> 2);

#endif /* ^__x86_64__ */

        u8 ret = 0;

        while (i--) {

            /* Optimize for (*current & *virgin) == 0 - i.e., no bits in current bitmap
           that have not been already cleared from the virgin map - since this will
           almost always be the case. */

            if (unlikely(*current) && unlikely(*current & *virgin)) {

                if (likely(ret < 2) || unlikely(dump_library && !map_id_out_f.fail())) {

                    u8* cur = (u8*)current;
                    u8* vir = (u8*)virgin;

                    /* Looks like we have not found any new bytes yet; see if any non-zero
               bytes in current[] are pristine in virgin[]. */

#if defined(__x86_64__) || defined(__arm64__) || defined(__aarch64__)

                    if ((cur[0] && vir[0] == 0xff) || (cur[1] && vir[1] == 0xff) || (cur[2] && vir[2] == 0xff) || (cur[3] && vir[3] == 0xff) || (cur[4] && vir[4] == 0xff) || (cur[5] && vir[5] == 0xff) || (cur[6] && vir[6] == 0xff) || (cur[7] && vir[7] == 0xff)) {
                        ret = 2;
                        if (dump_library && !map_id_out_f.fail() && cur_seed_str != "") {
                            vector<u8> byte = get_cur_new_byte(cur, vir);
                            for (const u8& cur_byte : byte) {
                                // vector<u8> cur_bit = get_cur_new_bit(cur[cur_byte]);
                                log_map_id(i, cur_byte, cur_seed_str);
                            }
                        }
                    } else {
                        if (ret != 2) {
                            ret = 1;
                        }
                    }

#else

                    if ((cur[0] && vir[0] == 0xff) || (cur[1] && vir[1] == 0xff) || (cur[2] && vir[2] == 0xff) || (cur[3] && vir[3] == 0xff))
                        ret = 2;
                    else
                        ret = 1;

#endif /* ^__x86_64__ */
                }

                *virgin &= ~*current;
            }

            current++;
            virgin++;
        }

        if (ret && virgin_map == this->get_virgin_bits())
            this->set_bitmap_changed(1);

        return ret;
    }

    /* Traditional way to setup the shm */
    /* Configure shared memory and virgin_bits. This is called at startup. */

    virtual void setup_actual_shm()
    {

        // memset(this->virgin_bits, 255, MAP_SIZE);
        // memset(this->virgin_tmout, 255, MAP_SIZE);
        // memset(virgin_crash, 255, MAP_SIZE);

        // The trace_bits is allocated in the constructor, free it and replace to shared memory version.
        free(trace_bits);

        if (shm_size != MAP_SIZE) {
            cerr << "shm_size is not equal to MAP_SIZE, set to " << shm_size << "\n\n\n";
        }

        shm_id = shmget(IPC_PRIVATE, shm_size, IPC_CREAT | IPC_EXCL | 0640);

        if (shm_id < 0)
            PFATAL("shmget() failed");

        trace_bits = static_cast<u8*>(shmat(shm_id, NULL, 0));

        // pass the shm_id to the child process.
        string shm_str = to_string(shm_id);
        setenv(SHM_ENV_VAR, shm_str.c_str(), 1);

        std::ofstream out("./shm_env.txt");
        out << shm_str;
        out.close();

        if (!trace_bits)
            PFATAL("shmat() failed");
    }

protected:
    std::atomic<int> forksrv_pid, /* PID of the fork server           */
        child_pid; /* PID of the fuzzed program        */
    bool is_timeout; /* Log whether the current process is timeout  */
    uint8_t kill_signal;
    s32 out_fd; /* Persistent fd for out_file       */
    s32 program_output_fd;
    string out_file;
    string program_output_file;
    volatile u8 child_timed_out;
    u32 fsrv_ctl_fd, /* Fork server control pipe (write) */
        fsrv_st_fd; /* Fork server status pipe (read)   */
    ResultHandler* p_res_handl;
    volatile u8 stop_soon;
    u32 exec_tmout = EXEC_TIMEOUT; /* Configurable exec timeout (ms)   */
    u32 hang_tmout = EXEC_TIMEOUT; /* Timeout used for hang det (ms)   */
    u64 slowest_exec_ms; /* Slowest testcase non hang in ms  */

    u8 *trace_bits, /* SHM with instrumentation bitmap  */
        *virgin_bits, /* Regions yet untouched by fuzzing */
        *virgin_tmout, /* Bits we haven't seen in tmouts   */
        *virgin_crash; /* Bits we haven't seen in crashes  */

    // For shm.
    s32 shm_id = -1;

    u16* count_class_lookup16;
    u8* count_class_lookup8;
    u8* simplify_lookup;

    u8 bitmap_changed; /* Time to update bitmap?           */

    int map_file_id;
    fstream map_id_out_f;
    int bind_to_core_id;

    u64 mem_limit;
    s32 dev_null_fd; /* Persistent fd for /dev/null      */
    s32 dev_urandom_fd; /* Persistent fd for /dev/urandom   */

    s32 out_dir_fd;
    FILE* plot_file;

    u8 uses_asan;

    char* doc_path;
    u8 dump_library;

    unsigned long shm_size = MAP_SIZE;

public:

    int bind_to_port = 0;
    string socket_path = "";
};

#endif // RSG_CPP_DBMS_CONNECTOR_H
