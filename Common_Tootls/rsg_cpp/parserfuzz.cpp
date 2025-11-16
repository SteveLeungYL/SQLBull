//
// Created by XXX on 3/19/24.
//

#include <cstdlib>
#define AFL_MAIN
#define MESSAGES_TO_STDOUT

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#define _FILE_OFFSET_BITS 64

#include "headers/config.h"
#include "headers/dbms_connector.h"
#include "headers/debug.h"
#include "headers/feedback_mapper.h"
#include "headers/fuzzer_configurations.h"
#include "headers/fuzzing_sequence_queue.h"
#include "headers/query_importer.h"
#include "headers/query_instantiator.h"
#include "headers/query_plan_handl.h"
#include "headers/query_sequence.h"
#include "headers/results_handler.h"
#include "headers/rsg.h"
#include "headers/types.h"
#include "headers/utils.h"

// DBMS specific
#if defined(cockroachdb)
#include "dbms_specific/cockroachdb/cockroachdb_common.h"
#elif defined(duckdb)
#include "dbms_specific/duckdb/duckdb_common.h"
#elif defined(sqlite)
#include "dbms_specific/sqlite/sqlite_common.h"
#elif defined(mysqldb)
#include "dbms_specific/mysqldb/mysql_common.h"
#elif defined(mariadb)
#include "dbms_specific/mariadb/mariadb_common.h"
#elif defined(postgresql)
#include "dbms_specific/postgresql/postgresql_common.h"
#else
#error "No DBMS selected"
#endif

#include "headers/alloc-inl.h"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <ctime>
#include <ctype.h>
#include <dirent.h>
#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <regex>
#include <sched.h>
#include <signal.h>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/select.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>

#include <termios.h>
#include <time.h>
#include <unistd.h>
#include <vector>

#include <filesystem>
#include <fstream>

using namespace std;

#if defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__)
#include <sys/sysctl.h>
#endif /* __APPLE__ || __FreeBSD__ || __OpenBSD__ */

/* For systems that have sched_setaffinity; right now just Linux, but one
   can hope... */

#ifdef __linux__
#define HAVE_AFFINITY 1
#endif /* __linux__ */

/* A toggle to export some variables when building as a library. Not very
   useful for the general public. */

#ifdef AFL_LIB
#define EXP_ST
#else
#define EXP_ST static
#endif /* ^AFL_LIB */

/* Lots of globals, but mostly for the status UI and other things where it
   really makes no sense to haul them around as function parameters. */

#define INIT_LIB_PATH "./cockroach_initlib"
char* g_library_path;

static RSG* rsg = nullptr;
static QueryInstantiator* p_instantiator = nullptr;
static ResultHandler* p_result_handl = nullptr;
static QueryPlanHandler* p_query_plan_handl = nullptr;
static DBMSConnector* p_dbms_connector = nullptr;
static FeedbackMapper* p_feedback_mapper = nullptr;
static IRWrapper* p_ir_wrapper = nullptr;
static QueryImporter* p_query_importer = nullptr;
static QuerySequenceGenerator* p_init_query_sequence_gen = nullptr;
static FuzzingSequenceQueue* p_fuzzing_sequence_queue = nullptr;
static string dbms_name = "";

static uint64_t plot_file_row_num = 0;
static bool is_sample_current_execution = false;

static bool is_save_curr_query_seq = false;

u64 total_input_failed = 0;
u64 total_mutate_all_failed = 0;
u64 total_mutate_failed = 0;
u64 total_mutate_num = 0;
u64 total_append_failed = 0;
u64 total_execute = 0;
u64 total_add_to_queue = 0;
u64 debug_error = 0;
u64 debug_good = 0;
u64 num_valid = 0;
u64 num_parse = 0;
u64 num_mutate_all = 0;
u64 num_reparse = 0;
u64 num_append = 0;
u64 num_validate = 0;

static unsigned long num_subquery = 0;
static unsigned long num_ORDER_GROUP = 0;
static unsigned long num_JOIN = 0;
static unsigned long num_CASE = 0;
static unsigned long num_pattern_total_exec = 0;

u64 total_select_error_num = 0;
u64 total_data_type_error_num = 0;
u64 total_instan_succeed_num = 0;
u64 total_instan_num = 0;
u64 total_alias_type_error_num = 0;
u64 total_instan_caused_error_num = 0;

unsigned long shm_size = MAP_SIZE;

static unsigned global_instan_idx = 0;

int bind_to_port = 5432;
int bind_to_core_id = -1;

string socket_path = "";

u64 cockroach_execute_ok = 0;
u64 cockroach_execute_error = 0;
u64 cockroach_execute_total = 0;

const string explain_prefix = "EXPLAIN ANALYZE ";

#if defined(mysqldb) || defined(mariadb) || defined(postgresql)
const auto first_table_create_stmt = QueryStmt("CREATE TABLE v00 (c01 INT, c02 TEXT);");
const auto first_index_create_stmt = QueryStmt("CREATE INDEX i03 ON v00 (c01);");
#else
const auto first_table_create_stmt = QueryStmt("CREATE TABLE v00 (c01 INT, c02 STRING);");
const auto first_index_create_stmt = QueryStmt("CREATE INDEX i03 ON v00 (c01, c02);");
#endif


const auto first_insert_stmt = QueryStmt("INSERT INTO v00 (c01, c02) VALUES (0, 'abc');");

// The testing_optimizer_disable_rule_probability is not exposed to the user.
// string all_opt_str = "SET testing_optimizer_disable_rule_probability = 0.0;\n";

// string all_opt_str = "SET CLUSTER SETTING
// sql.stats.automatic_collection.enabled = true;\n" "SET CLUSTER SETTING
// sql.stats.histogram_collection.enabled = true;\n" "SET CLUSTER SETTING
// sql.query_cache.enabled = true;\n" "SET reorder_joins_limit = 8;\n";

string no_opt_sql_str = "SET testing_optimizer_disable_rule_probability = 1.0; \n";
// string no_opt_sql_str =  "SET CLUSTER SETTING
// sql.stats.automatic_collection.enabled = false; DELETE FROM
// system.table_statistics WHERE true;\n" "SET CLUSTER SETTING
// sql.stats.histogram_collection.enabled = false;\n" "SET CLUSTER SETTING
// sql.query_cache.enabled = false;\n" "SET reorder_joins_limit = 0;\n";

int map_file_id = 0;
fstream map_id_out_f("./map_id_triggered_" + std::to_string(bind_to_core_id) + ".txt",
    std::ofstream::out | std::ofstream::trunc);

extern int errno;

map<DATATYPE, DATATYPE> relationmap;
map<DATATYPE, DATATYPE> crossmap;

char* g_current_input = NULL;
string g_cockroach_output = "";

IR* g_current_ir = NULL;

u32 g_execute_error = 0;

EXP_ST char *in_dir, /* Input directory with test cases  */
    *out_file, /* File to fuzz, if any             */
    *out_dir, /* Working & output directory       */
    *sync_dir, /* Synchronization directory        */
    *sync_id, /* Fuzzer ID                        */
    *use_banner, /* Display banner                   */
    *in_bitmap, /* Input bitmap                     */
    *doc_path, /* Path to documentation dir        */
    *target_path, /* Path to target binary            */
    *orig_cmdline; /* Original command line            */

EXP_ST u64 mem_limit = MEM_LIMIT; /* Memory cap for child (MB)        */

static u32 stats_update_freq = 1; /* Stats update frequency (execs)   */

EXP_ST u8 skip_deterministic, /* Skip deterministic stages?       */
    dump_library = 0, /* Dump squirrel libraries          */
    disable_dyn_instan = 0, /* Disable Dynamic Instantiation based on error messages.          */
    disable_rsg_generator = 0, /* Dump use RSG to generate new SQL statements          */
    disable_rsg_feedback = 0, /* Disable RSG feedback.          */
    force_deterministic, /* Force deterministic stages?      */
    use_splicing, /* Recombine input files?           */
    dumb_mode, /* Run in non-instrumented mode?    */
    kill_signal, /* Signal that killed the child     */
    resuming_fuzz, /* Resuming an older fuzzing job?   */
    timeout_given, /* Specific timeout given?          */
    not_on_tty, /* stdout is not a tty              */
    term_too_small, /* terminal dimensions too small    */
    uses_asan, /* Target uses ASAN?                */
    no_forkserver, /* Disable forkserver?              */
    crash_mode, /* Crash mode! Yeah!                */
    in_place_resume, /* Attempt in-place resume?         */
    auto_changed, /* Auto-generated tokens changed?   */
    no_cpu_meter_red, /* Feng shui on the status screen   */
    no_arith, /* Skip most arithmetic ops         */
    shuffle_queue, /* Shuffle input queue?             */
    qemu_mode, /* Running in QEMU mode?            */
    skip_requested, /* Skip request, via SIGUSR1        */
    run_over10m, /* Run time over 10 minutes?        */
    persistent_mode, /* Running in persistent mode?      */
    deferred_mode, /* Deferred forkserver mode?        */
    fast_cal; /* Try to calibrate faster?         */

enum FEEDBACKMODE {
    FUZZING_NORMAL = 0,
    FUZZING_QUERY_PLAN = 1,
    FUZZING_DROP_ALL = 2,
    FUZZING_RANDOM_SAVE = 3,
    FUZZING_SAVE_ALL = 4
};

static FEEDBACKMODE feedback_mode;

static s32 out_fd, /* Persistent fd for out_file       */
    program_output_fd;

static string program_input_str; /* String: query used to test sqlite   */
static string
    program_output_str; /* String: query results output from sqlite   */

static uint64_t bug_output_id = 0;
static uint64_t sample_output_id = 0;

static u8 var_bytes[MAP_SIZE]; /* Bytes that appear to be variable */

static s32 shm_id; /* ID of the SHM region             */

static volatile u8 stop_soon, /* Ctrl-C pressed?                  */
    clear_screen = 1, /* Window resized?                  */
    child_timed_out; /* Traced process timed out?        */

EXP_ST u32 queued_paths, /* Total number of queued testcases */
    queued_variable, /* Testcases with variable behavior */
    queued_at_start, /* Total number of initial inputs   */
    queued_discovered, /* Items discovered during this run */
    queued_imported, /* Items imported via -S            */
    queued_favored, /* Paths deemed favorable           */
    queued_with_cov, /* Paths with new coverage bytes    */
    pending_not_fuzzed, /* Queued but not done yet          */
    pending_favored, /* Pending favored paths            */
    cur_skipped_paths, /* Abandoned inputs in cur cycle    */
    cur_depth, /* Current path depth               */
    max_depth, /* Max path depth                   */
    useless_at_start, /* Number of useless starting paths */
    var_byte_count, /* Bitmap bytes with var behavior   */
    current_entry, /* Current queue entry ID           */
    havoc_div = 1; /* Cycle count divisor for havoc    */

EXP_ST u64 total_crashes, /* Total number of crashes          */
    unique_crashes, /* Crashes with unique signatures   */
    total_tmouts, /* Total number of timeouts         */
    unique_tmouts, /* Timeouts with unique signatures  */
    unique_hangs, /* Hangs with unique signatures     */
    total_execs, /* Total execve() calls             */
    start_time, /* Unix start time (ms)             */
    last_path_time, /* Time for most recent path (ms)   */
    last_crash_time, /* Time for most recent crash (ms)  */
    last_hang_time, /* Time for most recent hang (ms)   */
    last_crash_execs, /* Exec counter at last crash       */
    queue_cycle, /* Queue round counter              */
    cycles_wo_finds, /* Cycles without any new paths     */
    trim_execs, /* Execs done to trim input files   */
    bytes_trim_in, /* Bytes coming into the trimmer    */
    bytes_trim_out, /* Bytes coming outa the trimmer    */
    blocks_eff_total, /* Blocks subject to effector maps  */
    blocks_eff_select; /* Blocks selected as fuzzable      */

static u32 subseq_tmouts; /* Number of timeouts in a row      */

static char* stage_name = "init"; /* Name of the current fuzz stage   */
static u8
    *stage_short, /* Short stage name                 */
    *syncing_party; /* Currently syncing with...        */

static s32 stage_cur, stage_max; /* Stage progression                */
static s32 splicing_with = -1; /* Splicing with which test case?   */

static u32 master_id, master_max; /* Master instance job splitting    */

static u32 syncing_case; /* Syncing with case #...           */

static s32 stage_cur_byte, /* Byte offset of current stage op  */
    stage_cur_val; /* Value used for stage op          */

static u8 stage_val_type; /* Value type (STAGE_VAL_*)         */

static u64 stage_finds[32], /* Patterns found per fuzz stage    */
    stage_cycles[32]; /* Execs per fuzz stage             */

static u32 rand_cnt; /* Random number counter            */

static u64 total_cal_us, /* Total calibration time (us)      */
    total_cal_cycles = 1; /* Total calibration cycles         */

static u64 total_bitmap_size, /* Total bit count for all bitmaps  */
    total_bitmap_entries; /* Number of bitmaps counted        */

static s32 cpu_core_count; /* CPU core count                   */

#ifdef HAVE_AFFINITY

static s32 cpu_aff = -1; /* Selected CPU core                */

#endif /* HAVE_AFFINITY */

static FILE* plot_file; /* Gnuplot output file              */

struct queue_entry {

    char* fname; /* File name for the test case      */
    u32 len; /* Input length                     */

    u8 cal_failed, /* Calibration failed?              */
        trim_done, /* Trimmed?                         */
        was_fuzzed, /* Had any fuzzing done yet?        */
        passed_det, /* Deterministic stages passed?     */
        has_new_cov, /* Triggers new coverage?           */
        var_behavior, /* Variable behavior?               */
        favored, /* Currently favored?               */
        fs_redundant; /* Marked as redundant in the fs?   */

    u32 bitmap_size, /* Number of bits set in bitmap     */
        exec_cksum; /* Checksum of the execution trace  */

    u64 exec_us, /* Execution time (us)              */
        handicap, /* Number of queue cycles behind    */
        depth; /* Path depth                       */

    u8* trace_mini; /* Trace bytes, if kept             */
    u32 tc_ref; /* Trace bytes ref count            */

    struct queue_entry *next, /* Next element, if any             */
        *next_100; /* 100 elements ahead               */
};

static struct queue_entry *queue, /* Fuzzing queue (linked list)      */
    *queue_cur, /* Current offset within the queue  */
    *queue_top, /* Top of the list                  */
    *q_prev100; /* Previous 100 marker              */

static struct queue_entry* top_rated[MAP_SIZE]; /* Top entries for bitmap bytes     */

struct extra_data {
    u8* data; /* Dictionary token data            */
    u32 len; /* Dictionary token length          */
    u32 hit_cnt; /* Use count in the corpus          */
};

static struct extra_data* extras; /* Extra tokens to fuzz with        */
static u32 extras_cnt; /* Total number of tokens read      */

static struct extra_data* a_extras; /* Automatically selected extras    */
static u32 a_extras_cnt; /* Total number of tokens available */

static u8* (*post_handler)(u8* buf, u32* len);

/* Interesting values, as per config.h */

static s8 interesting_8[] = { INTERESTING_8 };
static s16 interesting_16[] = { INTERESTING_8, INTERESTING_16 };
static s32 interesting_32[] = { INTERESTING_8, INTERESTING_16, INTERESTING_32 };

/* Fuzzing stages */

enum {
    /* 00 */ STAGE_FLIP1,
    /* 01 */ STAGE_FLIP2,
    /* 02 */ STAGE_FLIP4,
    /* 03 */ STAGE_FLIP8,
    /* 04 */ STAGE_FLIP16,
    /* 05 */ STAGE_FLIP32,
    /* 06 */ STAGE_ARITH8,
    /* 07 */ STAGE_ARITH16,
    /* 08 */ STAGE_ARITH32,
    /* 09 */ STAGE_INTEREST8,
    /* 10 */ STAGE_INTEREST16,
    /* 11 */ STAGE_INTEREST32,
    /* 12 */ STAGE_EXTRAS_UO,
    /* 13 */ STAGE_EXTRAS_UI,
    /* 14 */ STAGE_EXTRAS_AO,
    /* 15 */ STAGE_HAVOC,
    /* 16 */ STAGE_SPLICE
};

/* Stage value types */

enum {
    /* 00 */ STAGE_VAL_NONE,
    /* 01 */ STAGE_VAL_LE,
    /* 02 */ STAGE_VAL_BE
};

/* Get unix time in milliseconds */

static u64 get_cur_time(void)
{

    struct timeval tv;
    struct timezone tz;

    gettimeofday(&tv, &tz);

    return (tv.tv_sec * 1000ULL) + (tv.tv_usec / 1000);
}

/* Get unix time in microseconds */

static u64 get_cur_time_us(void)
{

    struct timeval tv;
    struct timezone tz;

    gettimeofday(&tv, &tz);

    return (tv.tv_sec * 1000000ULL) + tv.tv_usec;
}

/* Generate a random number (from 0 to limit - 1). This may
   have slight bias. */

static inline u32 UR(u32 limit)
{

    if (unlikely(!rand_cnt--)) {

        u32 seed[2];

        ck_read(p_dbms_connector->get_dev_urandom_fd(), &seed, sizeof(seed), "/dev/urandom");

        srandom(seed[0]);
        rand_cnt = (RESEED_RNG / 2) + (seed[1] % RESEED_RNG);
    }

    return random() % limit;
}

/* Shuffle an array of pointers. Might be slightly biased. */

static void shuffle_ptrs(void** ptrs, u32 cnt)
{

    u32 i;

    for (i = 0; i < cnt - 2; i++) {

        u32 j = i + UR(cnt - i);
        void* s = ptrs[i];
        ptrs[i] = ptrs[j];
        ptrs[j] = s;
    }
}

#ifdef HAVE_AFFINITY

/* Build a list of processes bound to specific cores. Returns -1 if nothing
   can be found. Assumes an upper bound of 4k CPUs. */

static void bind_to_free_cpu(int bind_to_core_id = -1)
{

    DIR* d;
    struct dirent* de;
    cpu_set_t c;

    u8 cpu_used[4096] = { 0 };
    u32 i;

    if (cpu_core_count < 2)
        return;

    if (getenv("AFL_NO_AFFINITY")) {

        WARNF("Not binding to a CPU core (AFL_NO_AFFINITY set).");
        return;
    }

    d = opendir("/proc");

    if (!d) {

        WARNF("Unable to access /proc - can't scan for free CPU cores.");
        return;
    }

    ACTF("Checking CPU core loadout...");

    /* Introduce some jitter, in case multiple AFL tasks are doing the same
       thing at the same time... */

    usleep(R(1000) * 250);

    /* Scan all /proc/<pid>/status entries, checking for Cpus_allowed_list.
       Flag all processes bound to a specific CPU using cpu_used[]. This will
       fail for some exotic binding setups, but is likely good enough in almost
       all real-world use cases. */

    while ((de = readdir(d))) {

        u8* fn;
        FILE* f;
        u8 tmp[MAX_LINE];
        u8 has_vmsize = 0;

        if (!isdigit(de->d_name[0]))
            continue;

        fn = alloc_printf("/proc/%s/status", de->d_name);

        if (!(f = fopen(fn, "r"))) {
            ck_free(fn);
            continue;
        }

        while (fgets(tmp, MAX_LINE, f)) {

            u32 hval;

            /* Processes without VmSize are probably kernel tasks. */

            if (!strncmp(tmp, "VmSize:\t", 8))
                has_vmsize = 1;

            if (!strncmp(tmp, "Cpus_allowed_list:\t", 19) && !strchr((char*)tmp, int('-')) && !strchr((char*)tmp, int(',')) && sscanf(tmp + 19, "%u", &hval) == 1 && hval < sizeof(cpu_used) && has_vmsize) {

                cpu_used[hval] = 1;
                break;
            }
        }

        ck_free(fn);
        fclose(f);
    }

    closedir(d);

    for (i = 0; i < cpu_core_count; i++)
        if (!cpu_used[i])
            break;

    if (i == cpu_core_count) {

        SAYF("\n" cLRD "[-] " cRST
             "Uh-oh, looks like all %u CPU cores on your system are allocated to\n"
             "    other instances of afl-fuzz (or similar CPU-locked tasks). "
             "Starting\n"
             "    another fuzzer on this machine is probably a bad plan, but if "
             "you are\n"
             "    absolutely sure, you can set AFL_NO_AFFINITY and try again.\n",
            cpu_core_count);

        FATAL("No more free CPU cores");
    }
    if (bind_to_core_id < cpu_core_count && bind_to_core_id >= 0) {
        i = u32(bind_to_core_id);
        OKF("By command flags, binding to #%u.", i);
    } else {
        OKF("Found a free CPU core, binding to #%u.", i);
    }

    cpu_aff = i;

    CPU_ZERO(&c);
    CPU_SET(i, &c);

    if (sched_setaffinity(0, sizeof(c), &c))
        PFATAL("sched_setaffinity failed");
}

#endif /* HAVE_AFFINITY */

#ifndef IGNORE_FINDS

/* Helper function to compare buffers; returns first and last differing offset.
   We use this to find reasonable locations for splicing two files. */

static void locate_diffs(u8* ptr1, u8* ptr2, u32 len, s32* first, s32* last)
{

    s32 f_loc = -1;
    s32 l_loc = -1;
    u32 pos;

    for (pos = 0; pos < len; pos++) {

        if (*(ptr1++) != *(ptr2++)) {

            if (f_loc == -1)
                f_loc = pos;
            l_loc = pos;
        }
    }

    *first = f_loc;
    *last = l_loc;

    return;
}

#endif /* !IGNORE_FINDS */

/* Describe integer. Uses 12 cyclic static buffers for return values. The value
   returned should be five characters or less for all the integers we reasonably
   expect to see. */

static u8* DI(u64 val)
{

    static u8 tmp[12][16];
    static u8 cur;

    cur = (cur + 1) % 12;

#define CHK_FORMAT(_divisor, _limit_mult, _fmt, _cast)          \
    do {                                                        \
        if (val < (_divisor) * (_limit_mult)) {                 \
            sprintf(tmp[cur], _fmt, ((_cast)val) / (_divisor)); \
            return tmp[cur];                                    \
        }                                                       \
    } while (0)

    /* 0-9999 */
    CHK_FORMAT(1, 10000, "%llu", u64);

    /* 10.0k - 99.9k */
    CHK_FORMAT(1000, 99.95, "%0.01fk", double);

    /* 100k - 999k */
    CHK_FORMAT(1000, 1000, "%lluk", u64);

    /* 1.00M - 9.99M */
    CHK_FORMAT(1000 * 1000, 9.995, "%0.02fM", double);

    /* 10.0M - 99.9M */
    CHK_FORMAT(1000 * 1000, 99.95, "%0.01fM", double);

    /* 100M - 999M */
    CHK_FORMAT(1000 * 1000, 1000, "%lluM", u64);

    /* 1.00G - 9.99G */
    CHK_FORMAT(1000LL * 1000 * 1000, 9.995, "%0.02fG", double);

    /* 10.0G - 99.9G */
    CHK_FORMAT(1000LL * 1000 * 1000, 99.95, "%0.01fG", double);

    /* 100G - 999G */
    CHK_FORMAT(1000LL * 1000 * 1000, 1000, "%lluG", u64);

    /* 1.00T - 9.99G */
    CHK_FORMAT(1000LL * 1000 * 1000 * 1000, 9.995, "%0.02fT", double);

    /* 10.0T - 99.9T */
    CHK_FORMAT(1000LL * 1000 * 1000 * 1000, 99.95, "%0.01fT", double);

    /* 100T+ */
    strcpy(tmp[cur], "infty");
    return tmp[cur];
}

/* Describe float. Similar to the above, except with a single
   static buffer. */

static u8* DF(double val)
{

    static u8 tmp[16];

    if (val < 99.995) {
        sprintf(tmp, "%0.02f", val);
        return tmp;
    }

    if (val < 999.95) {
        sprintf(tmp, "%0.01f", val);
        return tmp;
    }

    return DI((u64)val);
}

/* Describe integer as memory size. */

static u8* DMS(u64 val)
{

    static u8 tmp[12][16];
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
    strcpy(tmp[cur], "infty");
    return tmp[cur];
}

/* Describe time delta. Returns one static buffer, 34 chars of less. */

static u8* DTD(u64 cur_ms, u64 event_ms)
{

    static u8 tmp[64];
    u64 delta;
    s32 t_d, t_h, t_m, t_s;

    if (!event_ms)
        return "none seen yet";

    delta = cur_ms - event_ms;

    t_d = delta / 1000 / 60 / 60 / 24;
    t_h = (delta / 1000 / 60 / 60) % 24;
    t_m = (delta / 1000 / 60) % 60;
    t_s = (delta / 1000) % 60;

    sprintf(tmp, "%s days, %u hrs, %u min, %u sec", DI(t_d), t_h, t_m, t_s);
    return tmp;
}

/* Append new test case to the queue. */

static void add_to_queue(u8* fname, u32 len, u8 passed_det)
{

    struct queue_entry* q = reinterpret_cast<struct queue_entry*>(ck_alloc(sizeof(struct queue_entry)));

    q->fname = fname;
    q->len = len;
    q->depth = cur_depth + 1;
    q->passed_det = passed_det;

    if (q->depth > max_depth)
        max_depth = q->depth;

    if (queue_top) {

        queue_top->next = q;
        queue_top = q;
    } else
        q_prev100 = queue = queue_top = q;

    queued_paths++;
    pending_not_fuzzed++;

    cycles_wo_finds = 0;

    if (!(queued_paths % 100)) {

        q_prev100->next_100 = q;
        q_prev100 = q;
    }

    last_path_time = get_cur_time();
}

/* Destroy the entire queue. */

EXP_ST void destroy_queue(void)
{

    struct queue_entry *q = queue, *n;

    while (q) {

        n = q->next;
        ck_free(q->fname);
        ck_free(q->trace_mini);
        ck_free(q);
        q = n;
    }
}

/* Count the number of bits set in the provided bitmap. Used for the status
   screen several times every second, does not have to be fast. */

static u32 count_bits(u8* mem)
{

    u32* ptr = (u32*)mem;
    u32 i = (MAP_SIZE >> 2);
    u32 ret = 0;

    while (i--) {

        u32 v = *(ptr++);

        /* This gets called on the inverse, virgin bitmap; optimize for sparse
       data. */

        if (v == 0xffffffff) {
            ret += 32;
            continue;
        }

        v -= ((v >> 1) & 0x55555555);
        v = (v & 0x33333333) + ((v >> 2) & 0x33333333);
        ret += (((v + (v >> 4)) & 0xF0F0F0F) * 0x01010101) >> 24;
    }

    return ret;
}

/* Compact trace bytes into a smaller bitmap. We effectively just drop the
   count information here. This is called only sporadically, for some
   new paths. */

static void minimize_bits(u8* dst, u8* src)
{

    u32 i = 0;

    while (i < MAP_SIZE) {

        if (*(src++))
            dst[i >> 3] |= 1 << (i & 7);
        i++;
    }
}

/* Load postprocessor, if available. */

static void setup_post(void)
{

    void* dh;
    u8* fn = getenv("AFL_POST_LIBRARY");
    u32 tlen = 6;

    if (!fn)
        return;

    ACTF("Loading postprocessor from '%s'...", fn);

    dh = dlopen(fn, RTLD_NOW);
    if (!dh)
        FATAL("%s", dlerror());

    post_handler = dlsym(dh, "afl_postprocess");
    if (!post_handler)
        FATAL("Symbol 'afl_postprocess' not found.");

    /* Do a quick test. It's better to segfault now than later =) */

    post_handler("hello", &tlen);

    OKF("Postprocessor installed successfully.");
}

/* Read all testcases from the input directory, then queue them for testing.
   Called at startup. */

static void read_testcases(void)
{

    struct dirent** nl;
    s32 nl_cnt;
    u32 i;
    u8* fn;

    /* Auto-detect non-in-place resumption attempts. */

    fn = alloc_printf("%s/queue", in_dir);
    if (!access(fn, F_OK))
        in_dir = fn;
    else
        ck_free(fn);

    ACTF("Scanning '%s'...", in_dir);

    /* We use scandir() + alphasort() rather than readdir() because otherwise,
     the ordering  of test cases would vary somewhat randomly and would be
     difficult to control. */

    nl_cnt = scandir(in_dir, &nl, NULL, alphasort);

    if (nl_cnt < 0) {

        if (errno == ENOENT || errno == ENOTDIR)

            SAYF("\n" cLRD "[-] " cRST "The input directory does not seem to be "
                 "valid - try again. The fuzzer needs\n"
                 "    one or more test case to start with - ideally, a small file "
                 "under 1 kB\n"
                 "    or so. The cases must be stored as regular files directly in "
                 "the input\n"
                 "    directory.\n");

        PFATAL("Unable to open '%s'", in_dir);
    }

    if (shuffle_queue && nl_cnt > 1) {

        ACTF("Shuffling queue...");
        shuffle_ptrs((void**)nl, nl_cnt);
    }

    for (i = 0; i < nl_cnt; i++) {

        struct stat st;

        u8* fn = alloc_printf("%s/%s", in_dir, nl[i]->d_name);
        u8* dfn = alloc_printf("%s/.state/deterministic_done/%s", in_dir, nl[i]->d_name);

        u8 passed_det = 0;

        free(nl[i]); /* not tracked */

        if (lstat(fn, &st) || access(fn, R_OK))
            PFATAL("Unable to access '%s'", fn);

        /* This also takes care of . and .. */

        if (!S_ISREG(st.st_mode) || !st.st_size || strstr((char*)fn, "/README.txt")) {

            ck_free(fn);
            ck_free(dfn);
            continue;
        }

        if (st.st_size > MAX_FILE)
            FATAL("Test case '%s' is too big (%s, limit is %s)", fn, DMS(st.st_size),
                DMS(MAX_FILE));

        /* Check for metadata that indicates that deterministic fuzzing
       is complete for this entry. We don't want to repeat deterministic
       fuzzing when resuming aborted scans, because it would be pointless
       and probably very time-consuming. */

        if (!access(dfn, F_OK))
            passed_det = 1;
        ck_free(dfn);

        add_to_queue(fn, st.st_size, passed_det);
    }

    free(nl); /* not tracked */

    if (!queued_paths) {

        SAYF("\n" cLRD "[-] " cRST "Looks like there are no valid test cases in "
             "the input directory! The fuzzer\n"
             "    needs one or more test case to start with - ideally, a small "
             "file under\n"
             "    1 kB or so. The cases must be stored as regular files directly "
             "in the\n"
             "    input directory.\n");

        FATAL("No usable test cases in '%s'", in_dir);
    }

    last_path_time = 0;
    queued_at_start = queued_paths;
}

/* Helper function for load_extras. */

static int compare_extras_len(const void* p1, const void* p2)
{
    struct extra_data *e1 = (struct extra_data*)p1,
                      *e2 = (struct extra_data*)p2;

    return e1->len - e2->len;
}

static int compare_extras_use_d(const void* p1, const void* p2)
{
    struct extra_data *e1 = (struct extra_data*)p1,
                      *e2 = (struct extra_data*)p2;

    return e2->hit_cnt - e1->hit_cnt;
}

/* Read extras from a file, sort by size. */

static void load_extras_file(u8* fname, u32* min_len, u32* max_len,
    u32 dict_level)
{

    FILE* f;
    u8 buf[MAX_LINE];
    u8* lptr;
    u32 cur_line = 0;

    f = fopen(fname, "r");

    if (!f)
        PFATAL("Unable to open '%s'", fname);

    while ((lptr = fgets(buf, MAX_LINE, f))) {

        u8 *rptr, *wptr;
        u32 klen = 0;

        cur_line++;

        /* Trim on left and right. */

        while (isspace(*lptr))
            lptr++;

        rptr = lptr + strlen(lptr) - 1;
        while (rptr >= lptr && isspace(*rptr))
            rptr--;
        rptr++;
        *rptr = 0;

        /* Skip empty lines and comments. */

        if (!*lptr || *lptr == '#')
            continue;

        /* All other lines must end with '"', which we can consume. */

        rptr--;

        if (rptr < lptr || *rptr != '"')
            FATAL("Malformed name=\"value\" pair in line %u.", cur_line);

        *rptr = 0;

        /* Skip alphanumerics and dashes (label). */

        while (isalnum(*lptr) || *lptr == '_')
            lptr++;

        /* If @number follows, parse that. */

        if (*lptr == '@') {

            lptr++;
            if (atoi(lptr) > dict_level)
                continue;
            while (isdigit(*lptr))
                lptr++;
        }

        /* Skip whitespace and = signs. */

        while (isspace(*lptr) || *lptr == '=')
            lptr++;

        /* Consume opening '"'. */

        if (*lptr != '"')
            FATAL("Malformed name=\"keyword\" pair in line %u.", cur_line);

        lptr++;

        if (!*lptr)
            FATAL("Empty keyword in line %u.", cur_line);

        /* Okay, let's allocate memory and copy data between "...", handling
       \xNN escaping, \\, and \". */

        extras = ck_realloc_block(extras, (extras_cnt + 1) * sizeof(struct extra_data));

        wptr = extras[extras_cnt].data = ck_alloc(rptr - lptr);

        while (*lptr) {

            char* hexdigits = "0123456789abcdef";

            switch (*lptr) {

            case 1 ... 31:
            case 128 ... 255:
                FATAL("Non-printable characters in line %u.", cur_line);

            case '\\':

                lptr++;

                if (*lptr == '\\' || *lptr == '"') {
                    *(wptr++) = *(lptr++);
                    klen++;
                    break;
                }

                if (*lptr != 'x' || !isxdigit(lptr[1]) || !isxdigit(lptr[2]))
                    FATAL("Invalid escaping (not \\xNN) in line %u.", cur_line);

                *(wptr++) = ((strchr(hexdigits, tolower(lptr[1])) - hexdigits) << 4) | (strchr(hexdigits, tolower(lptr[2])) - hexdigits);

                lptr += 3;
                klen++;

                break;

            default:

                *(wptr++) = *(lptr++);
                klen++;
            }
        }

        extras[extras_cnt].len = klen;

        if (extras[extras_cnt].len > MAX_DICT_FILE)
            FATAL("Keyword too big in line %u (%s, limit is %s)", cur_line, DMS(klen),
                DMS(MAX_DICT_FILE));

        if (*min_len > klen)
            *min_len = klen;
        if (*max_len < klen)
            *max_len = klen;

        extras_cnt++;
    }

    fclose(f);
}

/* Read extras from the extras directory and sort them by size. */

static void load_extras(u8* dir)
{

    DIR* d;
    struct dirent* de;
    u32 min_len = MAX_DICT_FILE, max_len = 0, dict_level = 0;
    u8* x;

    /* If the name ends with @, extract level and continue. */

    if ((x = strchr((char*)dir, '@'))) {

        *x = 0;
        dict_level = atoi(x + 1);
    }

    ACTF("Loading extra dictionary from '%s' (level %u)...", dir, dict_level);

    d = opendir(dir);

    if (!d) {

        if (errno == ENOTDIR) {
            load_extras_file(dir, &min_len, &max_len, dict_level);
            goto check_and_sort;
        }

        PFATAL("Unable to open '%s'", dir);
    }

    if (x)
        FATAL("Dictionary levels not supported for directories.");

    while ((de = readdir(d))) {

        struct stat st;
        u8* fn = alloc_printf("%s/%s", dir, de->d_name);
        s32 fd;

        if (lstat(fn, &st) || access(fn, R_OK))
            PFATAL("Unable to access '%s'", fn);

        /* This also takes care of . and .. */
        if (!S_ISREG(st.st_mode) || !st.st_size) {

            ck_free(fn);
            continue;
        }

        if (st.st_size > MAX_DICT_FILE)
            FATAL("Extra '%s' is too big (%s, limit is %s)", fn, DMS(st.st_size),
                DMS(MAX_DICT_FILE));

        if (min_len > st.st_size)
            min_len = st.st_size;
        if (max_len < st.st_size)
            max_len = st.st_size;

        extras = ck_realloc_block(extras, (extras_cnt + 1) * sizeof(struct extra_data));

        extras[extras_cnt].data = ck_alloc(st.st_size);
        extras[extras_cnt].len = st.st_size;

        fd = open(fn, O_RDONLY);

        if (fd < 0)
            PFATAL("Unable to open '%s'", fn);

        ck_read(fd, extras[extras_cnt].data, st.st_size, fn);

        close(fd);
        ck_free(fn);

        extras_cnt++;
    }

    closedir(d);

check_and_sort:

    if (!extras_cnt)
        FATAL("No usable files in '%s'", dir);

    qsort(extras, extras_cnt, sizeof(struct extra_data), compare_extras_len);

    OKF("Loaded %u extra tokens, size range %s to %s.", extras_cnt, DMS(min_len),
        DMS(max_len));

    if (max_len > 32)
        WARNF("Some tokens are relatively large (%s) - consider trimming.",
            DMS(max_len));

    if (extras_cnt > MAX_DET_EXTRAS)
        WARNF("More than %u tokens - will use them probabilistically.",
            MAX_DET_EXTRAS);
}

/* Helper function for maybe_add_auto() */

static inline u8 memcmp_nocase(u8* m1, u8* m2, u32 len)
{

    while (len--)
        if (tolower(*(m1++)) ^ tolower(*(m2++)))
            return 1;
    return 0;
}

/* Maybe add automatic extra. */

static void maybe_add_auto(u8* mem, u32 len)
{

    u32 i;

    /* Allow users to specify that they don't want auto dictionaries. */

    if (!MAX_AUTO_EXTRAS || !USE_AUTO_EXTRAS)
        return;

    /* Skip runs of identical bytes. */

    for (i = 1; i < len; i++)
        if (mem[0] ^ mem[i])
            break;

    if (i == len)
        return;

    /* Reject builtin interesting values. */

    if (len == 2) {

        i = sizeof(interesting_16) >> 1;

        while (i--)
            if (*((u16*)mem) == interesting_16[i] || *((u16*)mem) == SWAP16(interesting_16[i]))
                return;
    }

    if (len == 4) {

        i = sizeof(interesting_32) >> 2;

        while (i--)
            if (*((u32*)mem) == interesting_32[i] || *((u32*)mem) == SWAP32(interesting_32[i]))
                return;
    }

    /* Reject anything that matches existing extras. Do a case-insensitive
     match. We optimize by exploiting the fact that extras[] are sorted
     by size. */

    for (i = 0; i < extras_cnt; i++)
        if (extras[i].len >= len)
            break;

    for (; i < extras_cnt && extras[i].len == len; i++)
        if (!memcmp_nocase(extras[i].data, mem, len))
            return;

    /* Last but not least, check a_extras[] for matches. There are no
     guarantees of a particular sort order. */

    auto_changed = 1;

    for (i = 0; i < a_extras_cnt; i++) {

        if (a_extras[i].len == len && !memcmp_nocase(a_extras[i].data, mem, len)) {

            a_extras[i].hit_cnt++;
            goto sort_a_extras;
        }
    }

    /* At this point, looks like we're dealing with a new entry. So, let's
     append it if we have room. Otherwise, let's randomly evict some other
     entry from the bottom half of the list. */

    if (a_extras_cnt < MAX_AUTO_EXTRAS) {

        a_extras = ck_realloc_block(a_extras,
            (a_extras_cnt + 1) * sizeof(struct extra_data));

        a_extras[a_extras_cnt].data = ck_memdup(mem, len);
        a_extras[a_extras_cnt].len = len;
        a_extras_cnt++;
    } else {

        i = MAX_AUTO_EXTRAS / 2 + UR((MAX_AUTO_EXTRAS + 1) / 2);

        ck_free(a_extras[i].data);

        a_extras[i].data = ck_memdup(mem, len);
        a_extras[i].len = len;
        a_extras[i].hit_cnt = 0;
    }

sort_a_extras:

    /* First, sort all auto extras by use count, descending order. */

    qsort(a_extras, a_extras_cnt, sizeof(struct extra_data),
        compare_extras_use_d);

    /* Then, sort the top USE_AUTO_EXTRAS entries by size. */

    qsort(a_extras, MIN(USE_AUTO_EXTRAS, a_extras_cnt), sizeof(struct extra_data),
        compare_extras_len);
}

/* Save automatically generated extras. */

static void save_auto(void)
{

    u32 i;

    if (!auto_changed)
        return;
    auto_changed = 0;

    for (i = 0; i < MIN(USE_AUTO_EXTRAS, a_extras_cnt); i++) {

        u8* fn = alloc_printf("%s/queue/.state/auto_extras/auto_%06u", out_dir, i);
        s32 fd;

        fd = open(fn, O_WRONLY | O_CREAT | O_TRUNC, 0640);

        if (fd < 0)
            PFATAL("Unable to create '%s'", fn);

        ck_write(fd, a_extras[i].data, a_extras[i].len, fn);

        close(fd);
        ck_free(fn);
    }
}

static void init_bug_output_dir()
{
    if (!filesystem::exists("/home/" + dbms_name + "/fuzzing/Bug_Analysis/")) {
        filesystem::create_directory("/home/" + dbms_name + "/fuzzing/Bug_Analysis/");
    }
    if (!filesystem::exists("/home/" + dbms_name + "/fuzzing/Bug_Analysis/detected_bugs")) {
        filesystem::create_directory("/home/" + dbms_name + "/fuzzing/Bug_Analysis/detected_bugs");
    }
    if (!filesystem::exists("/home/" + dbms_name + "/fuzzing/Bug_Analysis/detected_bugs/crashes")) {
        filesystem::create_directory("/home/" + dbms_name + "/fuzzing/Bug_Analysis/detected_bugs/crashes");
    }
    if (!filesystem::exists("/home/" + dbms_name + "/fuzzing/Bug_Analysis/bug_samples")) {
        filesystem::create_directory("/home/" + dbms_name + "/fuzzing/Bug_Analysis/bug_samples");
    }

    const string start_time_file = "/home/" + dbms_name + "/fuzzing/Bug_Analysis/detected_bugs/start_time";
    if (!filesystem::exists(start_time_file)) {
        ofstream outputfile;
        outputfile.open(start_time_file, std::ofstream::out);
        outputfile << (get_cur_time() / 1000);
        outputfile.close();
    }

    const string start_time_file_crash = "/home/" + dbms_name + "/fuzzing/Bug_Analysis/detected_bugs/crashes/start_time";
    if (!filesystem::exists(start_time_file_crash)) {
        ofstream outputfile;
        outputfile.open(start_time_file_crash, std::ofstream::out);
        outputfile << (get_cur_time() / 1000);
        outputfile.close();
    }
}

/* Load automatically generated extras. */

static void load_auto(void)
{

    u32 i;

    for (i = 0; i < USE_AUTO_EXTRAS; i++) {

        u8 tmp[MAX_AUTO_EXTRA + 1];
        u8* fn = alloc_printf("%s/.state/auto_extras/auto_%06u", in_dir, i);
        s32 fd, len;

        fd = open(fn, O_RDONLY, 0640);

        if (fd < 0) {

            if (errno != ENOENT)
                PFATAL("Unable to open '%s'", fn);
            ck_free(fn);
            break;
        }

        /* We read one byte more to cheaply detect tokens that are too
       long (and skip them). */

        len = read(fd, tmp, MAX_AUTO_EXTRA + 1);

        if (len < 0)
            PFATAL("Unable to read from '%s'", fn);

        if (len >= MIN_AUTO_EXTRA && len <= MAX_AUTO_EXTRA)
            maybe_add_auto(tmp, len);

        close(fd);
        ck_free(fn);
    }

    if (i)
        OKF("Loaded %u auto-discovered dictionary tokens.", i);
    else
        OKF("No auto-generated dictionary tokens to reuse.");
}

/* Destroy extras. */

static void destroy_extras(void)
{

    u32 i;

    for (i = 0; i < extras_cnt; i++)
        ck_free(extras[i].data);

    ck_free(extras);

    for (i = 0; i < a_extras_cnt; i++)
        ck_free(a_extras[i].data);

    ck_free(a_extras);
}

inline void print_fuzzer_exec_debug_info()
{
    bool is_debug_info = true;
    if (is_debug_info) {
        // cout << string(30, '\n');
        // printf("\x1b[19A");

        cout << "\n\33[2K \n"
             << "\33[2K total_input_failed:      " << total_input_failed << "\n"
             << "\33[2K total_add_to_queue:      " << total_add_to_queue << "\n"
             << "\33[2K total_mutate_all_failed: " << total_mutate_all_failed
             << "\n"
             << "\33[2K total_mutate_failed:     " << total_mutate_failed << "\n"
             << "\33[2K total_append_failed:     " << total_append_failed << "\n"
             << "\33[2K total good SELECT percentage:       " << debug_good << " / "
             << debug_error + debug_good << " ("
             << debug_good * 100.0 / (debug_error + debug_good) << "%)\n"
             << "\33[2K cockroach_execute_ok:      " << cockroach_execute_ok << "\n"
             << "\33[2K cockroach_execute_error:   " << cockroach_execute_error
             << "\n"
             << "\33[2K cockroach_execute_total:   " << cockroach_execute_total
             << "\n"
             << "\33[2K total_mutate_failed:     "
             << std::to_string(float(total_mutate_failed) / float(total_mutate_num) * 100.0)
             << ": " << total_mutate_failed << " / " << total_mutate_num << "\n"
             << "\33[2K num_valid:               " << num_valid << "\n"
             << "\33[2K num_parse:               " << num_parse << "\n"
             << "\33[2K num_mutate_all:          " << num_mutate_all << "\n"
             << "\33[2K num_reparse:             " << num_reparse << "\n"
             << "\33[2K num_append:              " << num_append << "\n"
             << "\33[2K num_validate:            " << num_validate << "\n";
    }

    return;
}

void stream_output_res(ostream& out, QuerySequenceGenerator* p_query_gen, bool is_debug = false)
{
    if (!is_debug) {
        out << p_query_gen->p_query_sequence->get_query_sequence_str_with_results();
    } else {
        out << p_query_gen->p_query_sequence->get_query_sequence_str_with_results_stmt_by_stmt();
    }
    QuerySequence* previous_seq = p_query_gen->p_query_sequence->previous_sequence;

    while (previous_seq != nullptr) {
        out << previous_seq->get_query_sequence_str_short();
        previous_seq = previous_seq->previous_sequence;
    }

#if defined (mysqldb) || defined(mariadb)
    bool is_read_mysqld_dump = false;
    int trial_times = 0;
    while (!is_read_mysqld_dump && !is_debug && trial_times < 70) { // Retry 70 times, ~ 7 secends?
        if (filesystem::exists("./mysqld_output_to_fuzzer.txt")) {
            is_read_mysqld_dump = true;
            std::ifstream inFile;
            inFile.open("./mysqld_output_to_fuzzer.txt"); //open the input file

            std::stringstream strStream;
            strStream << inFile.rdbuf(); //read the file
            std::string str = strStream.str(); //str holds the content of the file

            out << str;
            inFile.close();
            filesystem::remove("./mysqld_output_to_fuzzer.txt");
            break;
        }
        trial_times++;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
#elif defined(postgresql)
    std::ofstream outfile ("/home/" + dbms_name + "/postgres/bld/fuzzer_to_script_signal_file.txt");
    outfile << "a" << std::endl;
    outfile.close();

    string postgresql_output_to_fuzzer_file = "/home/" + dbms_name + "/postgres/bld/postgresql_output_to_fuzzer.txt";
    bool is_read_postgresql_dump = false;
    int trial_times = 0;
    while (!is_read_postgresql_dump && !is_debug && trial_times < 70) { // Retry 70 times, ~ 7 secends?
        if (filesystem::exists(postgresql_output_to_fuzzer_file.c_str())) {
            is_read_postgresql_dump = true;
            sleep(1); // If triggering bug, wait for 3 seconds for the log to be flushed to the file. 
            std::ifstream inFile;
            inFile.open(postgresql_output_to_fuzzer_file.c_str()); //open the input file

            // Read last 10 lines of the file. 
            std::vector<std::string> last_lines;
            std::string line;
            
            // Read all lines and keep only the last 10
            while (std::getline(inFile, line)) {
                last_lines.push_back(line);
                if (last_lines.size() > 50) {
                    last_lines.erase(last_lines.begin());
                }
            }
            
            // Output the last 10 lines
            for (const auto& line : last_lines) {
                // if (findStringIn(line, "TRAP") || findStringIn(line, "FATAL") || findStringIn(line, "WARNING")) {
                    out << line << "\n";
                // }
            }
            
            inFile.close();
            filesystem::remove(postgresql_output_to_fuzzer_file.c_str());

            break;
        }
        trial_times++;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
#endif

    return;
}

/* The same, but with an adjustable gap. Used for trimming. */

static void write_with_gap(void* mem, u32 len, u32 skip_at, u32 skip_len)
{

    s32 fd = out_fd;
    u32 tail_len = len - skip_at - skip_len;

    if (out_file) {

        unlink(out_file); /* Ignore errors. */

        fd = open(out_file, O_WRONLY | O_CREAT | O_EXCL, 0640);

        if (fd < 0)
            PFATAL("Unable to create '%s'", out_file);
    } else
        lseek(fd, 0, SEEK_SET);

    if (skip_at)
        ck_write(fd, mem, skip_at, out_file);

    if (tail_len)
        ck_write(fd, mem + skip_at + skip_len, tail_len, out_file);

    if (!out_file) {

        if (ftruncate(fd, len - skip_len))
            PFATAL("ftruncate() failed");
        lseek(fd, 0, SEEK_SET);
    } else
        close(fd);
}

static void show_stats(void);

/* Examine map coverage. Called once, for first test case. */

static void check_map_coverage(void)
{

    u32 i;

    if (p_dbms_connector->count_bytes(p_dbms_connector->get_trace_bits()) < 100)
        return;

    for (i = (1 << (MAP_SIZE_POW2 - 1)); i < MAP_SIZE; i++)
        if (p_dbms_connector->get_trace_bits()[i])
            return;

    WARNF("Recompile binary with newer version of afl to improve coverage!");
}

/* Helper function: link() if possible, copy otherwise. */

static void link_or_copy(u8* old_path, u8* new_path)
{

    s32 i = link(old_path, new_path);
    s32 sfd, dfd;
    u8* tmp;

    if (!i)
        return;

    sfd = open(old_path, O_RDONLY);
    if (sfd < 0)
        PFATAL("Unable to open '%s'", old_path);

    dfd = open(new_path, O_WRONLY | O_CREAT | O_EXCL, 0640);
    if (dfd < 0)
        PFATAL("Unable to create '%s'", new_path);

    tmp = ck_alloc(64 * 1024);

    while ((i = read(sfd, tmp, 64 * 1024)) > 0)
        ck_write(dfd, tmp, i, new_path);

    if (i < 0)
        PFATAL("read() failed");

    ck_free(tmp);
    close(sfd);
    close(dfd);
}

static void nuke_resume_dir(void);

#ifndef SIMPLE_FILES

/* Construct a file name for a new test case, capturing the operation
   that led to its discovery. Uses a static buffer. */

static u8* describe_op(u8 hnb)
{

    static u8 ret[256];

    if (syncing_party) {

        sprintf(ret, "sync:%s,src:%06u", syncing_party, syncing_case);
    } else {

        sprintf(ret, "src:%06u", current_entry);

        if (splicing_with >= 0)
            sprintf(ret + strlen(ret), "+%06u", splicing_with);

        sprintf(ret + strlen(ret), ",op:%s", stage_short);

        if (stage_cur_byte >= 0) {

            sprintf(ret + strlen(ret), ",pos:%u", stage_cur_byte);

            if (stage_val_type != STAGE_VAL_NONE)
                sprintf(ret + strlen(ret), ",val:%s%+d",
                    (stage_val_type == STAGE_VAL_BE) ? "be:" : "", stage_cur_val);
        } else
            sprintf(ret + strlen(ret), ",rep:%u", stage_cur_val);
    }

    if (hnb == 2)
        strcat(ret, ",+cov");

    return ret;
}

#endif /* !SIMPLE_FILES */

/* Write a message accompanying the crash directory :-) */

static void write_crash_readme(void)
{

    u8* fn = alloc_printf("%s/crashes/README.txt", out_dir);
    s32 fd;
    FILE* f;

    fd = open(fn, O_WRONLY | O_CREAT | O_EXCL, 0640);
    ck_free(fn);

    /* Do not die on errors here - that would be impolite. */

    if (fd < 0)
        return;

    f = fdopen(fd, "w");

    if (!f) {
        close(fd);
        return;
    }

    fprintf(
        f,
        "Command line used to find this crash:\n\n"

        "%s\n\n"

        "If you can't reproduce a bug outside of afl-fuzz, be sure to set the "
        "same\n"
        "memory limit. The limit used for this fuzzing session was %s.\n\n"

        "Need a tool to minimize test cases before investigating the crashes or "
        "sending\n"
        "them to a vendor? Check out the afl-tmin that comes with the fuzzer!\n\n"

        "Found any cool bugs in open-source tools using afl-fuzz? If yes, please "
        "drop\n"
        "me a mail at <lcamtuf@coredump.cx> once the issues are fixed - I'd love "
        "to\n"
        "add your finds to the gallery at:\n\n"

        "  http://lcamtuf.coredump.cx/afl/\n\n"

        "Thanks :-)\n",

        orig_cmdline, DMS(mem_limit << 20)); /* ignore errors */

    fclose(f);
}

void save_query_sequence_to_queue(QuerySequenceGenerator* p_query_sequence_gen)
{
    // whether to save the whole query sequence into the queue.
    if (p_query_sequence_gen->is_finished_gen && p_query_sequence_gen->is_total_new_coverage) {
        if (p_query_sequence_gen->gen_mode == GenAllFromNew) {
            p_fuzzing_sequence_queue->append_new_sequence_from_scratch(p_query_sequence_gen->p_query_sequence->deep_copy());
        } else {
            p_fuzzing_sequence_queue->append_new_sequence_to_existing_group(p_query_sequence_gen->p_query_sequence->deep_copy());
            p_fuzzing_sequence_queue->repeat_previous_query_seq();
        }
    }
}

/* Check if the result of an execve() during routine fuzzing is interesting,
   save or queue the input test case for further analysis if so. Returns 1 if
   entry is saved, 0 otherwise. */

static u8 save_if_interesting(char** argv, int fault,
    QuerySequenceGenerator* p_query_sequence_gen)
{
    u8* fn = "";
    u8 hnb;
    s32 fd;
    u8 keeping = 0, res;
    vector<IR*> ir_set;

    string latest_query_str = p_query_sequence_gen->p_query_sequence->v_all_query_stmts.back()->to_string();

    if (fault == crash_mode) {

        /* Keep only if there are new bits in the map, add to queue for
       future fuzzing, etc. */

        /* Always check p_dbms_connector->has_new_bits first. */

        // always record the code coverage info.
        hnb = p_dbms_connector->has_new_bits(p_dbms_connector->get_virgin_bits(), latest_query_str);

        if (feedback_mode == FUZZING_NORMAL && !hnb) {
            if (crash_mode)
                total_crashes++;
            rsg->rsg_failed_with_penalty(p_query_sequence_gen);
            return 0;
        } else if (feedback_mode == FUZZING_QUERY_PLAN) {
            // query plan based feedback.
            if (!(p_result_handl->has_new_query_plan())) {
                // no new coverage.
                rsg->rsg_failed_with_penalty(p_query_sequence_gen);
                return 0;
            } else {
                // has new query plan
                // continue the following logic.
            }
        }

        if (feedback_mode == FUZZING_DROP_ALL) { // Disable feedbacks. Drop all queries.
            return keeping;
        }

        /* For evaluation experiments, if we need to disable coverage feedback and
         *randomly drop queries:
         **  1/10 of chances to save the interesting seed.
         **  9/10 of chances to throw away the seed.
         **/
        if ((feedback_mode == FUZZING_RANDOM_SAVE) && get_pct_hit(90)) {
            // Drop query.
            return keeping;
        }

        /* If feedback_mode is save_all, always go through save_if_interesting.
         */

        rsg->rsg_succeed_with_reward(p_query_sequence_gen);
        is_save_curr_query_seq = true;
        p_query_sequence_gen->is_total_new_coverage = true;

        char* tmp_name = stage_name;
        //[modify] add
        // if it is interesting, we update our library with it.
        stage_name = "add_to_library";

        bool is_save_to_queue = false;

        show_stats();
        stage_name = tmp_name;

        //        if (g_mutator.is_stripped_str_in_lib(query_str) || !is_save_to_queue)
        //            return keeping;

#ifndef SIMPLE_FILES

        fn = alloc_printf("%s/queue/id:%06u,%s", out_dir, queued_paths,
            describe_op(hnb));

#else

        fn = alloc_printf("%s/queue/id_%06u", out_dir, queued_paths);

#endif /* ^!SIMPLE_FILES */

        add_to_queue(fn, latest_query_str.size(), 0);

        total_add_to_queue++;

        if (hnb == 2) {
            queue_top->has_new_cov = 1;
            queued_with_cov++;
        }

        queue_top->exec_cksum = hash32(p_dbms_connector->get_trace_bits(), MAP_SIZE, HASH_CONST);

        /* Try to calibrate inline; this also calls update_bitmap_score() when
       successful. */

        // res = calibrate_case(argv, queue_top, stripped_query_string.c_str(),
        //                     queue_cycle - 1, 0);

        // if (res == FAULT_ERROR)
        //   FATAL("Unable to execute target application");

        fd = open(fn, O_WRONLY | O_CREAT | O_EXCL, 0640);
        if (fd < 0)
            PFATAL("Unable to create '%s'", fn);

        ck_write(fd, latest_query_str.c_str(), latest_query_str.size(), fn);
        close(fd);

        keeping = 1;
    }

    switch (fault) {

    case FAULT_SQLERROR: {
        hnb = p_dbms_connector->has_new_bits(p_dbms_connector->get_virgin_bits(), latest_query_str);
        rsg->rsg_failed_with_penalty(p_query_sequence_gen);
        return 0; // return 0 for not keeping.
    }

    case FAULT_TMOUT:

        /* Timeouts are not very interesting, but we're still obliged to keep
     a handful of samples. We use the presence of new bits in the
     hang-specific bitmap as a signal of uniqueness. In "dumb" mode, we
     just keep everything. */

        total_tmouts++;

        memset(p_dbms_connector->get_trace_bits(), 0, MAP_SIZE);

        if (unique_hangs >= KEEP_UNIQUE_HANG)
            return keeping;

        if (!dumb_mode) {

#if defined(__x86_64__) || defined(__arm64__) || defined(__aarch64__)
            p_dbms_connector->simplify_trace((u64*)p_dbms_connector->get_trace_bits());
#else
            p_dbms_connector->simplify_trace((u32*)p_dbms_connector->get_trace_bits());
#endif /* ^__x86_64__ */
        }

        unique_tmouts++;

        unique_hangs++;

        last_hang_time = get_cur_time();

        return 0;
        //    break;

    case FAULT_CRASH:

    keep_as_crash:

        /* This is handled in a manner roughly similar to timeouts,
     except for slightly different limits and no need to re-run test
     cases. */

        total_crashes++;
        memset(p_dbms_connector->get_trace_bits(), 0, MAP_SIZE);

        if (unique_crashes >= KEEP_UNIQUE_CRASH) // 5000
            return keeping;

        if (!dumb_mode) {

#if defined(__x86_64__) || defined(__arm64__) || defined(__aarch64__)
            p_dbms_connector->simplify_trace((u64*)p_dbms_connector->get_trace_bits());
#else
            p_dbms_connector->simplify_trace((u32*)p_dbms_connector->get_trace_bits());
#endif /* ^__x86_64__ */

            //      if (!p_dbms_connector->has_new_bits(virgin_crash, query_str)) // If no new bits.
            //      Return.
            //        return keeping;
        }

        if (!unique_crashes)
            write_crash_readme();

#ifndef SIMPLE_FILES

        fn = alloc_printf("%s/crashes/id:%06llu,sig:%02u,%s", out_dir,
            unique_crashes, kill_signal, describe_op(0));

#else

        fn = alloc_printf("%s/crashes/id_%06llu_%02u", out_dir, unique_crashes,
            kill_signal);

#endif /* ^!SIMPLE_FILES */

        unique_crashes++;

        last_crash_time = get_cur_time();
        last_crash_execs = total_execs;

        break;

    case FAULT_ERROR:
        FATAL("Unable to execute target application");

    default:
        return keeping;
    }

    /* If we're here, we apparently want to save the crash or hang
     test case, too. */

    is_save_curr_query_seq = true;

    string bug_output_dir = "/home/" + dbms_name + "/fuzzing/Bug_Analysis/detected_bugs/crashes/bug:" + to_string(unique_crashes - 1) + ":src:" + to_string(current_entry) + ":core:" + std::to_string(bind_to_core_id) + ".txt";
    // cerr << "Bug output dir is: " << bug_output_dir << endl;
    ofstream outputfile;
    outputfile.open(bug_output_dir, std::ofstream::out | std::ofstream::app);
    stream_output_res(outputfile, p_query_sequence_gen);
    outputfile.close();

    ck_free(fn);

    return keeping;
}

/* The same, but for timeouts. The idea is that when resuming sessions without
   -t given, we don't want to keep auto-scaling the timeout over and over
   again to prevent it from growing due to random flukes. */

static u64 find_timeout(void)
{

    static u8 tmp[4096]; /* Ought to be enough for anybody. */

    u8 *fn, *off;
    s32 fd, i;
    u32 ret;

    if (!resuming_fuzz)
        return 0;

    if (in_place_resume)
        fn = alloc_printf("%s/fuzzer_stats", out_dir);
    else
        fn = alloc_printf("%s/../fuzzer_stats", in_dir);

    fd = open(fn, O_RDONLY);
    ck_free(fn);

    if (fd < 0)
        return 0;

    i = read(fd, tmp, sizeof(tmp) - 1);
    (void)i; /* Ignore errors */
    close(fd);

    off = strstr((const char*)tmp, "exec_timeout      : ");
    if (!off)
        return 0;

    ret = atoi(off + 20);
    if (ret <= 4)
        return 0;

    timeout_given = 3;

    return ret;
}

/* Update stats file for unattended monitoring. */

static void write_stats_file(double bitmap_cvg, double stability, double eps)
{

    static double last_bcvg, last_stab, last_eps;
    static struct rusage usage;

    u8* fn = alloc_printf("%s/fuzzer_stats", out_dir);
    s32 fd;
    FILE* f;

    fd = open(fn, O_WRONLY | O_CREAT | O_TRUNC, 0640);

    if (fd < 0)
        PFATAL("Unable to create '%s'", fn);

    ck_free(fn);

    f = fdopen(fd, "w");

    if (!f)
        PFATAL("fdopen() failed");

    /* Keep last values in case we're called from another context
     where exec/sec stats and such are not readily available. */

    if (!bitmap_cvg && !stability && !eps) {
        bitmap_cvg = last_bcvg;
        stability = last_stab;
        eps = last_eps;
    } else {
        last_bcvg = bitmap_cvg;
        last_stab = stability;
        last_eps = eps;
    }

    fprintf(f,
        "start_time        : %llu\n"
        "last_update       : %llu\n"
        "fuzzer_pid        : %u\n"
        "cycles_done       : %llu\n"
        "execs_done        : %llu\n"
        "execs_per_sec     : %0.02f\n"
        "paths_total       : %u\n"
        "paths_favored     : %u\n"
        "paths_found       : %u\n"
        "paths_imported    : %u\n"
        "max_depth         : %u\n"
        "cur_path          : %u\n" /* Must match find_start_position() */
        "pending_favs      : %u\n"
        "pending_total     : %u\n"
        "variable_paths    : %u\n"
        "stability         : %0.02f%%\n"
        "bitmap_cvg        : %0.02f%%\n"
        "unique_crashes    : %llu\n"
        "unique_hangs      : %llu\n"
        "last_path         : %llu\n"
        "last_crash        : %llu\n"
        "last_hang         : %llu\n"
        "execs_since_crash : %llu\n"
        "exec_timeout      : %u\n" /* Must match find_timeout() */
        "afl_banner        : %s\n"
        "afl_version       : " VERSION "\n"
        "target_mode       : %s%s%s%s%s%s%s\n"
        "command_line      : %s\n"
        "slowest_exec_ms   : %llu\n",
        start_time / 1000, get_cur_time() / 1000, getpid(),
        queue_cycle ? (queue_cycle - 1) : 0, total_execs, eps, queued_paths,
        queued_favored, queued_discovered, queued_imported, max_depth,
        current_entry, pending_favored, pending_not_fuzzed, queued_variable,
        stability, bitmap_cvg, unique_crashes, unique_hangs,
        last_path_time / 1000, last_crash_time / 1000, last_hang_time / 1000,
        total_execs - last_crash_execs, p_dbms_connector->get_exec_tmout(), use_banner,
        qemu_mode ? "qemu " : "", dumb_mode ? " dumb " : "",
        no_forkserver ? "no_forksrv " : "", crash_mode ? "crash " : "",
        persistent_mode ? "persistent " : "",
        deferred_mode ? "deferred " : "",
        (qemu_mode || dumb_mode || no_forkserver || crash_mode || persistent_mode || deferred_mode)
            ? ""
            : "default",
        orig_cmdline, p_dbms_connector->get_slowest_exec_ms());
    /* ignore errors */

    /* Get rss value from the children
     We must have killed the forkserver process and called waitpid
     before calling getrusage */
    if (getrusage(RUSAGE_CHILDREN, &usage)) {
        WARNF("getrusage failed");
    } else if (usage.ru_maxrss == 0) {
        fprintf(f, "peak_rss_mb       : not available while afl is running\n");
    } else {
#ifdef __APPLE__
        fprintf(f, "peak_rss_mb       : %zu\n", usage.ru_maxrss >> 20);
#else
        fprintf(f, "peak_rss_mb       : %zu\n", usage.ru_maxrss >> 10);
#endif /* ^__APPLE__ */
    }

    fclose(f);
}

/* Update the plot file if there is a reason to. */

static void maybe_update_plot_file(double bitmap_cvg, double eps)
{

    static u32 prev_qp, prev_pf, prev_pnf, prev_ce, prev_md;
    static u64 prev_qc, prev_uc, prev_uh;

    //  if (prev_qp == queued_paths && prev_pf == pending_favored &&
    //      prev_pnf == pending_not_fuzzed && prev_ce == current_entry &&
    //      prev_qc == queue_cycle && prev_uc == unique_crashes &&
    //      prev_uh == unique_hangs && prev_md == max_depth)
    //    return;

    prev_qp = queued_paths;
    prev_pf = pending_favored;
    prev_pnf = pending_not_fuzzed;
    prev_ce = current_entry;
    prev_qc = queue_cycle;
    prev_uc = unique_crashes;
    prev_uh = unique_hangs;
    prev_md = max_depth;

    double vm = 0.0, rss = 0.0;
    process_mem_usage(vm, rss);

    /* Fields in the file:

     unix_time, cycles_done, cur_path, paths_total, paths_not_fuzzed,
     favored_not_fuzzed, unique_crashes, unique_hangs, max_depth,
     execs_per_sec */

    fprintf(
        plot_file,
        /* Format */
        "%llu,%llu,%u,%u,%u,%u,%0.02f%%,%llu,%llu,%u,%0.02f,%llu,%llu,"
        "%0.02f%%,%llu,%0.02f,%0.02f,%llu,%llu,%llu,%llu,%llu"
        "\n",
        /* Data */
        get_cur_time() / 1000, queue_cycle - 1, current_entry, queued_paths,
        pending_not_fuzzed, pending_favored, bitmap_cvg, unique_crashes,
        unique_hangs, max_depth, eps, total_execs,
        debug_good, (static_cast<double>(debug_good) * 100.0 / static_cast<double>(debug_error + debug_good)),
        rsg->v_p_cached_reverse_tree.size(),
        vm, rss, // memory usage related.
        num_subquery, num_ORDER_GROUP, num_JOIN, num_CASE, num_pattern_total_exec
        );
    fflush(plot_file); // flush to the file immediately, if print.

    // After some time, sample the current execution for output sampling.
    plot_file_row_num++;
    if (!(plot_file_row_num % 100)) {
        is_sample_current_execution = true;
    }
}

/* A helper function for maybe_delete_out_dir(), deleting all prefixed
   files in a directory. */

static u8 delete_files(u8* path, u8* prefix)
{

    DIR* d;
    struct dirent* d_ent;

    d = opendir(path);

    if (!d)
        return 0;

    while ((d_ent = readdir(d))) {

        if (d_ent->d_name[0] != '.' && (!prefix || !strncmp(d_ent->d_name, prefix, strlen(prefix)))) {

            u8* fname = alloc_printf("%s/%s", path, d_ent->d_name);
            if (unlink(fname))
                PFATAL("Unable to delete '%s'", fname);
            ck_free(fname);
        }
    }

    closedir(d);

    return !!rmdir(path);
}

/* Get the number of runnable processes, with some simple smoothing. */

static double get_runnable_processes(void)
{

    static double res;

#if defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__)

    /* I don't see any portable sysctl or so that would quickly give us the
     number of runnable processes; the 1-minute load average can be a
     semi-decent approximation, though. */

    if (getloadavg(&res, 1) != 1)
        return 0;

#else

    /* On Linux, /proc/stat is probably the best way; load averages are
     computed in funny ways and sometimes don't reflect extremely short-lived
     processes well. */

    FILE* f = fopen("/proc/stat", "r");
    u8 tmp[1024];
    u32 val = 0;

    if (!f)
        return 0;

    while (fgets(tmp, sizeof(tmp), f)) {

        if (!strncmp(tmp, "procs_running ", 14) || !strncmp(tmp, "procs_blocked ", 14))
            val += atoi(tmp + 14);
    }

    fclose(f);

    if (!res) {

        res = val;
    } else {

        res = res * (1.0 - 1.0 / AVG_SMOOTHING) + ((double)val) * (1.0 / AVG_SMOOTHING);
    }

#endif /* ^(__APPLE__ || __FreeBSD__ || __OpenBSD__) */

    return res;
}

/* Delete the temporary directory used for in-place session resume. */

#ifndef SIMPLE_FILES
#define CASE_PREFIX "id:"
#else
#define CASE_PREFIX "id_"
#endif /* ^!SIMPLE_FILES */

static void nuke_resume_dir(void)
{

    u8* fn;

    fn = alloc_printf("%s/_resume/.state/deterministic_done", out_dir);
    if (delete_files(fn, CASE_PREFIX))
        goto dir_cleanup_failed;
    ck_free(fn);

    fn = alloc_printf("%s/_resume/.state/auto_extras", out_dir);
    if (delete_files(fn, "auto_"))
        goto dir_cleanup_failed;
    ck_free(fn);

    fn = alloc_printf("%s/_resume/.state/redundant_edges", out_dir);
    if (delete_files(fn, CASE_PREFIX))
        goto dir_cleanup_failed;
    ck_free(fn);

    fn = alloc_printf("%s/_resume/.state/variable_behavior", out_dir);
    if (delete_files(fn, CASE_PREFIX))
        goto dir_cleanup_failed;
    ck_free(fn);

    fn = alloc_printf("%s/_resume/.state", out_dir);
    if (rmdir(fn) && errno != ENOENT)
        goto dir_cleanup_failed;
    ck_free(fn);

    fn = alloc_printf("%s/_resume", out_dir);
    if (delete_files(fn, CASE_PREFIX))
        goto dir_cleanup_failed;
    ck_free(fn);

    return;

dir_cleanup_failed:

    FATAL("_resume directory cleanup failed");
}

/* Delete fuzzer output directory if we recognize it as ours, if the fuzzer
   is not currently running, and if the last run time isn't too great. */

static void maybe_delete_out_dir(void)
{

    FILE* f;
    u8* fn = alloc_printf("%s/fuzzer_stats", out_dir);

    /* See if the output directory is locked. If yes, bail out. If not,
     create a lock that will persist for the lifetime of the process
     (this requires leaving the descriptor open).*/

    p_dbms_connector->set_out_dir_fd(open(out_dir, O_RDONLY));
    if (p_dbms_connector->get_out_dir_fd() < 0)
        PFATAL("Unable to open '%s'", out_dir);

#ifndef __sun

    if (flock(p_dbms_connector->get_out_dir_fd(), LOCK_EX | LOCK_NB) && errno == EWOULDBLOCK) {

        SAYF("\n" cLRD "[-] " cRST "Looks like the job output directory is being "
             "actively used by another\n"
             "    instance of afl-fuzz. You will need to choose a different %s\n"
             "    or stop the other process first.\n",
            sync_id ? "fuzzer ID" : "output location");

        FATAL("Directory '%s' is in use", out_dir);
    }

#endif /* !__sun */

    f = fopen(fn, "r");

    if (f) {

        u64 start_time, last_update;

        if (fscanf(f,
                "start_time     : %llu\n"
                "last_update    : %llu\n",
                &start_time, &last_update)
            != 2)
            FATAL("Malformed data in '%s'", fn);

        fclose(f);

        /* Let's see how much work is at stake. */

        if (!in_place_resume && last_update - start_time > OUTPUT_GRACE * 60) {

            SAYF("\n" cLRD "[-] " cRST "The job output directory already exists and "
                 "contains the results of more\n"
                 "    than %u minutes worth of fuzzing. To avoid data loss, afl-fuzz "
                 "will *NOT*\n"
                 "    automatically delete this data for you.\n\n"

                 "    If you wish to start a new session, remove or rename the "
                 "directory manually,\n"
                 "    or specify a different output location for this job. To resume "
                 "the old\n"
                 "    session, put '-' as the input directory in the command line "
                 "('-i -') and\n"
                 "    try again.\n",
                OUTPUT_GRACE);

            FATAL("At-risk data found in '%s'", out_dir);
        }
    }

    ck_free(fn);

    /* The idea for in-place resume is pretty simple: we temporarily move the old
     queue/ to a new location that gets deleted once import to the new queue/
     is finished. If _resume/ already exists, the current queue/ may be
     incomplete due to an earlier abort, so we want to use the old _resume/
     dir instead, and we let rename() fail silently. */

    if (in_place_resume) {

        u8* orig_q = alloc_printf("%s/queue", out_dir);

        in_dir = alloc_printf("%s/_resume", out_dir);

        rename(orig_q, in_dir); /* Ignore errors */

        OKF("Output directory exists, will attempt session resume.");

        ck_free(orig_q);
    } else {

        OKF("Output directory exists but deemed OK to reuse.");
    }

    ACTF("Deleting old session data...");

    /* Okay, let's get the ball rolling! First, we need to get rid of the entries
     in <out_dir>/.synced/.../id:*, if any are present. */

    if (!in_place_resume) {

        fn = alloc_printf("%s/.synced", out_dir);
        if (delete_files(fn, NULL))
            goto dir_cleanup_failed;
        ck_free(fn);
    }

    /* Next, we need to clean up <out_dir>/queue/.state/ subdirectories: */

    fn = alloc_printf("%s/queue/.state/deterministic_done", out_dir);
    if (delete_files(fn, CASE_PREFIX))
        goto dir_cleanup_failed;
    ck_free(fn);

    fn = alloc_printf("%s/queue/.state/auto_extras", out_dir);
    if (delete_files(fn, "auto_"))
        goto dir_cleanup_failed;
    ck_free(fn);

    fn = alloc_printf("%s/queue/.state/redundant_edges", out_dir);
    if (delete_files(fn, CASE_PREFIX))
        goto dir_cleanup_failed;
    ck_free(fn);

    fn = alloc_printf("%s/queue/.state/variable_behavior", out_dir);
    if (delete_files(fn, CASE_PREFIX))
        goto dir_cleanup_failed;
    ck_free(fn);

    /* Then, get rid of the .state subdirectory itself (should be empty by now)
     and everything matching <out_dir>/queue/id:*. */

    fn = alloc_printf("%s/queue/.state", out_dir);
    if (rmdir(fn) && errno != ENOENT)
        goto dir_cleanup_failed;
    ck_free(fn);

    fn = alloc_printf("%s/queue", out_dir);
    if (delete_files(fn, CASE_PREFIX))
        goto dir_cleanup_failed;
    ck_free(fn);

    /* All right, let's do <out_dir>/crashes/id:* and <out_dir>/hangs/id:*. */

    if (!in_place_resume) {

        fn = alloc_printf("%s/crashes/README.txt", out_dir);
        unlink(fn); /* Ignore errors */
        ck_free(fn);
    }

    fn = alloc_printf("%s/crashes", out_dir);

    /* Make backup of the crashes directory if it's not empty and if we're
     doing in-place resume. */

    if (in_place_resume && rmdir(fn)) {

        time_t cur_t = time(0);
        struct tm* t = localtime(&cur_t);

#ifndef SIMPLE_FILES

        u8* nfn = alloc_printf("%s.%04u-%02u-%02u-%02u:%02u:%02u", fn,
            t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
            t->tm_hour, t->tm_min, t->tm_sec);

#else

        u8* nfn = alloc_printf("%s_%04u%02u%02u%02u%02u%02u", fn, t->tm_year + 1900,
            t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min,
            t->tm_sec);

#endif /* ^!SIMPLE_FILES */

        rename(fn, nfn); /* Ignore errors. */
        ck_free(nfn);
    }

    if (delete_files(fn, CASE_PREFIX))
        goto dir_cleanup_failed;
    ck_free(fn);

    fn = alloc_printf("%s/hangs", out_dir);

    /* Backup hangs, too. */

    if (in_place_resume && rmdir(fn)) {

        time_t cur_t = time(0);
        struct tm* t = localtime(&cur_t);

#ifndef SIMPLE_FILES

        u8* nfn = alloc_printf("%s.%04u-%02u-%02u-%02u:%02u:%02u", fn,
            t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
            t->tm_hour, t->tm_min, t->tm_sec);

#else

        u8* nfn = alloc_printf("%s_%04u%02u%02u%02u%02u%02u", fn, t->tm_year + 1900,
            t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min,
            t->tm_sec);

#endif /* ^!SIMPLE_FILES */

        rename(fn, nfn); /* Ignore errors. */
        ck_free(nfn);
    }

    if (delete_files(fn, CASE_PREFIX))
        goto dir_cleanup_failed;
    ck_free(fn);

    /* And now, for some finishing touches. */

    fn = alloc_printf("%s/.cur_input", out_dir);
    if (unlink(fn) && errno != ENOENT)
        goto dir_cleanup_failed;
    ck_free(fn);

    fn = alloc_printf("%s/fuzz_bitmap", out_dir);
    if (unlink(fn) && errno != ENOENT)
        goto dir_cleanup_failed;
    ck_free(fn);

    if (!in_place_resume) {
        fn = alloc_printf("%s/fuzzer_stats", out_dir);
        if (unlink(fn) && errno != ENOENT)
            goto dir_cleanup_failed;
        ck_free(fn);
    }

    fn = alloc_printf("%s/plot_data", out_dir);
    if (unlink(fn) && errno != ENOENT)
        goto dir_cleanup_failed;
    ck_free(fn);

    OKF("Output dir cleanup successful.");

    /* Wow... is that all? If yes, celebrate! */

    return;

dir_cleanup_failed:

    SAYF("\n" cLRD "[-] " cRST "Whoops, the fuzzer tried to reuse your output "
         "directory, but bumped into\n"
         "    some files that shouldn't be there or that couldn't be removed - "
         "so it\n"
         "    decided to abort! This happened while processing this path:\n\n"

         "    %s\n\n"
         "    Please examine and manually delete the files, or specify a "
         "different\n"
         "    output location for the tool.\n",
        fn);

    FATAL("Output directory cleanup failed");
}

static void check_term_size(void);

/* A spiffy retro stats screen! This is called every stats_update_freq
   execve() calls, plus in several other circumstances. */

static void show_stats(void)
{
    static u64 last_stats_ms, last_plot_ms, last_ms, last_execs;
    static double avg_exec;
    double t_byte_ratio, stab_ratio;

    u64 cur_ms;
    u32 t_bytes, t_bits;

    u32 banner_len, banner_pad;
    u8 tmp[256];

    cur_ms = get_cur_time();

    /* If not enough time has passed since last UI update, bail out. */

    if (cur_ms - last_ms < 1000 / UI_TARGET_HZ)
        return;

    /* Check if we're past the 10 minute mark. */

    if (cur_ms - start_time > 10 * 60 * 1000)
        run_over10m = 1;

    /* Calculate smoothed exec speed stats. */

    if (!last_execs) {
        avg_exec = ((double)total_execs) * 1000 / (cur_ms - start_time);
    } else {
        double cur_avg = ((double)(total_execs - last_execs)) * 1000 / (cur_ms - last_ms);

        /* If there is a dramatic (5x+) jump in speed, reset the indicator
           more quickly. */

        if (cur_avg * 5 < avg_exec || cur_avg / 5 > avg_exec)
            avg_exec = cur_avg;

        avg_exec = avg_exec * (1.0 - 1.0 / AVG_SMOOTHING) + cur_avg * (1.0 / AVG_SMOOTHING);
    }

    last_ms = cur_ms;
    last_execs = total_execs;

    /* Tell the callers when to contact us (as measured in execs). */

    stats_update_freq = avg_exec / (UI_TARGET_HZ * 10);
    if (!stats_update_freq)
        stats_update_freq = 1;

    /* Do some bitmap stats. */

    t_bytes = p_dbms_connector->count_non_255_bytes(p_dbms_connector->get_virgin_bits());
    t_byte_ratio = ((double)t_bytes * 100) / MAP_SIZE;

    if (t_bytes)
        stab_ratio = 100 - ((double)var_byte_count) * 100 / t_bytes;
    else
        stab_ratio = 100;

    /* Roughly every minute, update fuzzer stats and save auto tokens. */

    if (cur_ms - last_stats_ms > STATS_UPDATE_SEC * 1000) {
        last_stats_ms = cur_ms;
        write_stats_file(t_byte_ratio, stab_ratio, avg_exec);
        save_auto();
        // write_bitmap();
        p_dbms_connector->write_bitmap(out_dir);
    }

    /* Every now and then, write plot data. */

    if (cur_ms - last_plot_ms > PLOT_UPDATE_SEC * 1000) {
        last_plot_ms = cur_ms;
        maybe_update_plot_file(t_byte_ratio, avg_exec);
    }

    /* Honor AFL_EXIT_WHEN_DONE and AFL_BENCH_UNTIL_CRASH. */

    if (!dumb_mode && cycles_wo_finds > 100 && !pending_not_fuzzed && getenv("AFL_EXIT_WHEN_DONE")) {
        stop_soon = 2;
        p_dbms_connector->set_stop_soon(2);
    }

    if (total_crashes && getenv("AFL_BENCH_UNTIL_CRASH")) {
        stop_soon = 2;
        p_dbms_connector->set_stop_soon(2);
    }

    /* If we're not on TTY, bail out. */

    if (not_on_tty)
        return;

    /* Compute some mildly useful bitmap stats. */

    t_bits = (MAP_SIZE << 3) - count_bits(p_dbms_connector->get_virgin_bits());

    /* Now, for the visuals... */

    if (clear_screen) {
        SAYF(TERM_CLEAR CURSOR_HIDE);
        clear_screen = 0;

        check_term_size();
    }

    SAYF(TERM_HOME);

    if (term_too_small) {
        SAYF(cBRI "Your terminal is too small to display the UI.\n"
                  "Please resize terminal window to at least 80x25.\n" cRST);

        return;
    }

    /* Let's start by drawing a centered banner. */

    banner_len = (crash_mode ? 24 : 22) + strlen(VERSION) + strlen(use_banner);
    banner_pad = (80 - banner_len) / 2;
    memset(tmp, ' ', banner_pad);

    sprintf(tmp + banner_pad, "%s " cLCY VERSION cLGN " (%s)", crash_mode ? cPIN "peruvian were-rabbit" : cYEL "american fuzzy lop", use_banner);

    SAYF("\n%s\n\n", tmp);

    /* "Handy" shortcuts for drawing boxes... */

#define bSTG bSTART cGRA
#define bH2 bH bH
#define bH5 bH2 bH2 bH
#define bH10 bH5 bH5
#define bH20 bH10 bH10
#define bH30 bH20 bH10
#define SP5 "     "
#define SP10 SP5 SP5
#define SP20 SP10 SP10

    /* Lord, forgive me this. */

    SAYF(SET_G1 bSTG bLT bH bSTOP cCYA " process timing " bSTG bH30 bH5 bH2 bHB
            bH bSTOP cCYA " overall results " bSTG bH5 bRT "\n");

    if (dumb_mode) {
        strcpy(tmp, cRST);
    } else {
        u64 min_wo_finds = (cur_ms - last_path_time) / 1000 / 60;

        /* First queue cycle: don't stop now! */
        if (queue_cycle == 1 || min_wo_finds < 15)
            strcpy(tmp, cMGN);
        else

            /* Subsequent cycles, but we're still making finds. */
            if (cycles_wo_finds < 25 || min_wo_finds < 30)
                strcpy(tmp, cYEL);
            else

                /* No finds for a long time and no test cases to try. */
                if (cycles_wo_finds > 100 && !pending_not_fuzzed && min_wo_finds > 120)
                    strcpy(tmp, cLGN);

                /* Default: cautiously OK to stop? */
                else
                    strcpy(tmp, cLBL);
    }

    SAYF(bV bSTOP "        run time : " cRST "%-34s " bSTG bV bSTOP
                  "  cycles done : %s%-5s  " bSTG bV "\n",
        DTD(cur_ms, start_time), tmp, DI(queue_cycle - 1));

    /* We want to warn people about not seeing new paths after a full cycle,
       except when resuming fuzzing or running in non-instrumented mode. */

    if (!dumb_mode && (last_path_time || resuming_fuzz || queue_cycle == 1 || in_bitmap || crash_mode)) {
        SAYF(bV bSTOP "   last new path : " cRST "%-34s ",
            DTD(cur_ms, last_path_time));
    } else {
        if (dumb_mode)

            SAYF(bV bSTOP "   last new path : " cPIN "n/a" cRST
                          " (non-instrumented mode)        ");

        else

            SAYF(bV bSTOP "   last new path : " cRST "none yet " cLRD
                          "(odd, check syntax!)      ");
    }

    SAYF(bSTG bV bSTOP "  total paths : " cRST "%-5s  " bSTG bV "\n",
        DI(queued_paths));

    /* Highlight crashes in red if found, denote going over the KEEP_UNIQUE_CRASH
       limit with a '+' appended to the count. */

    sprintf(tmp, "%s%s", DI(unique_crashes),
        (unique_crashes >= KEEP_UNIQUE_CRASH) ? "+" : "");

    SAYF(bV bSTOP " last uniq crash : " cRST "%-34s " bSTG bV bSTOP
                  " uniq crashes : %s%-6s " bSTG bV "\n",
        DTD(cur_ms, last_crash_time), unique_crashes ? cLRD : cRST,
        tmp);

    sprintf(tmp, "%s%s", DI(unique_hangs),
        (unique_hangs >= KEEP_UNIQUE_HANG) ? "+" : "");

    SAYF(bV bSTOP "  last uniq hang : " cRST "%-34s " bSTG bV bSTOP
                  "   uniq hangs : " cRST "%-6s " bSTG bV "\n",
        DTD(cur_ms, last_hang_time), tmp);

    SAYF(bVR bH bSTOP cCYA " cycle progress " bSTG bH20 bHB bH bSTOP cCYA
                           " map coverage " bSTG bH bHT bH20 bH2 bH bVL "\n");

    /* This gets funny because we want to print several variable-length variables
       together, but then cram them into a fixed-width field - so we need to
       put them in a temporary buffer first. */

    sprintf(tmp, "%s%s (%0.02f%%)", DI(current_entry),
        queue_cur->favored ? "" : "*",
        ((double)current_entry * 100) / queued_paths);

    SAYF(bV bSTOP "  now processing : " cRST "%-17s " bSTG bV bSTOP, tmp);

    sprintf(tmp, "%0.02f%% / %0.02f%%", ((double)queue_cur->bitmap_size) * 100 / MAP_SIZE, t_byte_ratio);

    SAYF("    map density : %s%-21s " bSTG bV "\n", t_byte_ratio > 70 ? cLRD : ((t_bytes < 200 && !dumb_mode) ? cPIN : cRST), tmp);

    sprintf(tmp, "%s (%0.02f%%)", DI(cur_skipped_paths),
        ((double)cur_skipped_paths * 100) / queued_paths);

    SAYF(bV bSTOP " paths timed out : " cRST "%-17s " bSTG bV, tmp);

    sprintf(tmp, "%0.02f bits/tuple",
        t_bytes ? (((double)t_bits) / t_bytes) : 0);

    SAYF(bSTOP " count coverage : " cRST "%-21s " bSTG bV "\n", tmp);

    SAYF(bVR bH bSTOP cCYA " stage progress " bSTG bH20 bX bH bSTOP cCYA
                           " findings in depth " bSTG bH20 bVL "\n");

    sprintf(tmp, "%s (%0.02f%%)", DI(queued_favored),
        ((double)queued_favored) * 100 / queued_paths);

    /* Yeah... it's still going on... halp? */

    SAYF(bV bSTOP "  now trying : " cRST "%-21s " bSTG bV bSTOP
                  " favored paths : " cRST "%-22s " bSTG bV "\n",
        stage_name, tmp);

    if (!stage_max) {
        sprintf(tmp, "%s/-", DI(stage_cur));
    } else {
        sprintf(tmp, "%s/%s (%0.02f%%)", DI(stage_cur), DI(stage_max),
            ((double)stage_cur) * 100 / stage_max);
    }

    SAYF(bV bSTOP " stage execs : " cRST "%-21s " bSTG bV bSTOP, tmp);

    sprintf(tmp, "%s (%0.02f%%)", DI(queued_with_cov),
        ((double)queued_with_cov) * 100 / queued_paths);

    SAYF("  new edges on : " cRST "%-22s " bSTG bV "\n", tmp);

    sprintf(tmp, "%s (%s%s unique)", DI(total_crashes), DI(unique_crashes),
        (unique_crashes >= KEEP_UNIQUE_CRASH) ? "+" : "");

    if (crash_mode) {
        SAYF(bV bSTOP " total execs : " cRST "%-21s " bSTG bV bSTOP
                      "   new crashes : %s%-22s " bSTG bV "\n",
            DI(total_execs),
            unique_crashes ? cLRD : cRST, tmp);
    } else {
        SAYF(bV bSTOP " total execs : " cRST "%-21s " bSTG bV bSTOP
                      " total crashes : %s%-22s " bSTG bV "\n",
            DI(total_execs),
            unique_crashes ? cLRD : cRST, tmp);
    }

    /* Show a warning about slow execution. */

    if (avg_exec < 100) {
        sprintf(tmp, "%s/sec (%s)", DF(avg_exec), avg_exec < 20 ? "zzzz..." : "slow!");

        SAYF(bV bSTOP "  exec speed : " cLRD "%-21s ", tmp);
    } else {
        sprintf(tmp, "%s/sec", DF(avg_exec));
        SAYF(bV bSTOP "  exec speed : " cRST "%-21s ", tmp);
    }

    sprintf(tmp, "%s (%s%s unique)", DI(total_tmouts), DI(unique_tmouts),
        (unique_hangs >= KEEP_UNIQUE_HANG) ? "+" : "");

    SAYF(bSTG bV bSTOP "  total tmouts : " cRST "%-22s " bSTG bV "\n", tmp);

    /* Aaaalmost there... hold on! */

    SAYF(bVR bH cCYA bSTOP " fuzzing strategy yields " bSTG bH10 bH bHT bH10
            bH5 bHB bH bSTOP cCYA " path geometry " bSTG bH5 bH2 bH bVL "\n");

    if (skip_deterministic) {
        strcpy(tmp, "n/a, n/a, n/a");
    } else {
        sprintf(tmp, "%s/%s, %s/%s, %s/%s",
            DI(stage_finds[STAGE_FLIP1]), DI(stage_cycles[STAGE_FLIP1]),
            DI(stage_finds[STAGE_FLIP2]), DI(stage_cycles[STAGE_FLIP2]),
            DI(stage_finds[STAGE_FLIP4]), DI(stage_cycles[STAGE_FLIP4]));
    }

    SAYF(bV bSTOP "   bit flips : " cRST "%-37s " bSTG bV bSTOP "    levels : " cRST "%-10s " bSTG bV "\n", tmp, DI(max_depth));

    if (!skip_deterministic)
        sprintf(tmp, "%s/%s, %s/%s, %s/%s",
            DI(stage_finds[STAGE_FLIP8]), DI(stage_cycles[STAGE_FLIP8]),
            DI(stage_finds[STAGE_FLIP16]), DI(stage_cycles[STAGE_FLIP16]),
            DI(stage_finds[STAGE_FLIP32]), DI(stage_cycles[STAGE_FLIP32]));

    SAYF(bV bSTOP "  byte flips : " cRST "%-37s " bSTG bV bSTOP "   pending : " cRST "%-10s " bSTG bV "\n", tmp, DI(pending_not_fuzzed));

    if (!skip_deterministic)
        sprintf(tmp, "%s/%s, %s/%s, %s/%s",
            DI(stage_finds[STAGE_ARITH8]), DI(stage_cycles[STAGE_ARITH8]),
            DI(stage_finds[STAGE_ARITH16]), DI(stage_cycles[STAGE_ARITH16]),
            DI(stage_finds[STAGE_ARITH32]), DI(stage_cycles[STAGE_ARITH32]));

    SAYF(bV bSTOP " arithmetics : " cRST "%-37s " bSTG bV bSTOP "  pend fav : " cRST "%-10s " bSTG bV "\n", tmp, DI(pending_favored));

    if (!skip_deterministic)
        sprintf(tmp, "%s/%s, %s/%s, %s/%s",
            DI(stage_finds[STAGE_INTEREST8]), DI(stage_cycles[STAGE_INTEREST8]),
            DI(stage_finds[STAGE_INTEREST16]), DI(stage_cycles[STAGE_INTEREST16]),
            DI(stage_finds[STAGE_INTEREST32]), DI(stage_cycles[STAGE_INTEREST32]));

    SAYF(bV bSTOP "  known ints : " cRST "%-37s " bSTG bV bSTOP " own finds : " cRST "%-10s " bSTG bV "\n", tmp, DI(queued_discovered));

    if (!skip_deterministic)
        sprintf(tmp, "%s/%s, %s/%s, %s/%s",
            DI(stage_finds[STAGE_EXTRAS_UO]), DI(stage_cycles[STAGE_EXTRAS_UO]),
            DI(stage_finds[STAGE_EXTRAS_UI]), DI(stage_cycles[STAGE_EXTRAS_UI]),
            DI(stage_finds[STAGE_EXTRAS_AO]), DI(stage_cycles[STAGE_EXTRAS_AO]));

    SAYF(bV bSTOP "  dictionary : " cRST "%-37s " bSTG bV bSTOP
                  "  imported : " cRST "%-10s " bSTG bV "\n",
        tmp,
        sync_id ? DI(queued_imported) : (u8*)"n/a");

    sprintf(tmp, "%s/%s, %s/%s",
        DI(stage_finds[STAGE_HAVOC]), DI(stage_cycles[STAGE_HAVOC]),
        DI(stage_finds[STAGE_SPLICE]), DI(stage_cycles[STAGE_SPLICE]));

    SAYF(bV bSTOP "       havoc : " cRST "%-37s " bSTG bV bSTOP, tmp);

    if (t_bytes)
        sprintf(tmp, "%0.02f%%", stab_ratio);
    else
        strcpy(tmp, "n/a");

    SAYF(" stability : %s%-10s " bSTG bV "\n", (stab_ratio < 85 && var_byte_count > 40) ? cLRD : ((queued_variable && (!persistent_mode || var_byte_count > 20)) ? cMGN : cRST), tmp);

    if (!bytes_trim_out) {
        sprintf(tmp, "n/a, ");
    } else {
        sprintf(tmp, "%0.02f%%/%s, ",
            ((double)(bytes_trim_in - bytes_trim_out)) * 100 / bytes_trim_in,
            DI(trim_execs));
    }

    if (!blocks_eff_total) {
        u8 tmp2[128];

        sprintf(tmp2, "n/a");
        strcat(tmp, tmp2);
    } else {
        u8 tmp2[128];

        sprintf(tmp2, "%0.02f%%",
            ((double)(blocks_eff_total - blocks_eff_select)) * 100 / blocks_eff_total);

        strcat(tmp, tmp2);
    }

    SAYF(bV bSTOP "        trim : " cRST "%-37s " bSTG bVR bH20 bH2 bH2 bRB "\n" bLB bH30 bH20 bH2 bH bRB bSTOP cRST RESET_G1, tmp);

    /* Provide some CPU utilization stats. */

    if (cpu_core_count) {
        double cur_runnable = get_runnable_processes();
        u32 cur_utilization = cur_runnable * 100 / cpu_core_count;

        u8* cpu_color = cCYA;

        /* If we could still run one or more processes, use green. */

        if (cpu_core_count > 1 && cur_runnable + 1 <= cpu_core_count)
            cpu_color = cLGN;

        /* If we're clearly oversubscribed, use red. */

        if (!no_cpu_meter_red && cur_utilization >= 150)
            cpu_color = cLRD;

#ifdef HAVE_AFFINITY

        if (cpu_aff >= 0) {

            SAYF(SP10 cGRA "[cpu%03u:%s%3u%%" cGRA "]\r" cRST,
                MIN(cpu_aff, 999), cpu_color,
                MIN(cur_utilization, 999));

        } else {

            SAYF(SP10 cGRA "   [cpu:%s%3u%%" cGRA "]\r" cRST,
                cpu_color, MIN(cur_utilization, 999));
        }

#else

        SAYF(SP10 cGRA "   [cpu:%s%3u%%" cGRA "]\r" cRST,
            cpu_color, MIN(cur_utilization, 999));

#endif /* ^HAVE_AFFINITY */
    } else
        SAYF("\r");

    /* Hallelujah! */

    fflush(0);
    // print_fuzzer_exec_debug_info();
}

/* Display quick statistics at the end of processing the input directory,
   plus a bunch of warnings. Some calibration stuff also ended up here,
   along with several hardcoded constants. Maybe clean up eventually. */

static void show_init_stats(void)
{
    OKF("All set and ready to roll!");
}

/* Find first power of two greater or equal to val (assuming val under
   2^31). */

static u32 next_p2(u32 val)
{

    u32 ret = 1;
    while (val > ret)
        ret <<= 1;
    return ret;
}

/* Helper to choose random block len for block operations in fuzz_one().
   Doesn't return zero, provided that max_len is > 0. */

static u32 choose_block_len(u32 limit)
{

    u32 min_value, max_value;
    u32 rlim = MIN(queue_cycle, 3);

    if (!run_over10m)
        rlim = 1;

    switch (UR(rlim)) {

    case 0:
        min_value = 1;
        max_value = HAVOC_BLK_SMALL;
        break;

    case 1:
        min_value = HAVOC_BLK_SMALL;
        max_value = HAVOC_BLK_MEDIUM;
        break;

    default:

        if (UR(10)) {

            min_value = HAVOC_BLK_MEDIUM;
            max_value = HAVOC_BLK_LARGE;
        } else {

            min_value = HAVOC_BLK_LARGE;
            max_value = HAVOC_BLK_XL;
        }
    }

    if (min_value >= limit)
        min_value = 1;

    return min_value + UR(MIN(max_value, limit) - min_value + 1);
}

/* Calculate case desirability score to adjust the length of havoc fuzzing.
   A helper function for fuzz_one(). Maybe some of these constants should
   go into config.h. */

static u32 calculate_score(struct queue_entry* q)
{

    u32 avg_exec_us = total_cal_us / (total_cal_cycles + 1);
    u32 avg_bitmap_size = total_bitmap_size / (total_bitmap_entries + 1);
    u32 perf_score = 100;

    /* Adjust score based on execution speed of this path, compared to the
     global average. Multiplier ranges from 0.1x to 3x. Fast inputs are
     less expensive to fuzz, so we're giving them more air time. */

    if (q->exec_us * 0.1 > avg_exec_us)
        perf_score = 10;
    else if (q->exec_us * 0.25 > avg_exec_us)
        perf_score = 25;
    else if (q->exec_us * 0.5 > avg_exec_us)
        perf_score = 50;
    else if (q->exec_us * 0.75 > avg_exec_us)
        perf_score = 75;
    else if (q->exec_us * 4 < avg_exec_us)
        perf_score = 300;
    else if (q->exec_us * 3 < avg_exec_us)
        perf_score = 200;
    else if (q->exec_us * 2 < avg_exec_us)
        perf_score = 150;

    /* Adjust score based on bitmap size. The working theory is that better
     coverage translates to better targets. Multiplier from 0.25x to 3x. */

    if (q->bitmap_size * 0.3 > avg_bitmap_size)
        perf_score *= 3;
    else if (q->bitmap_size * 0.5 > avg_bitmap_size)
        perf_score *= 2;
    else if (q->bitmap_size * 0.75 > avg_bitmap_size)
        perf_score *= 1.5;
    else if (q->bitmap_size * 3 < avg_bitmap_size)
        perf_score *= 0.25;
    else if (q->bitmap_size * 2 < avg_bitmap_size)
        perf_score *= 0.5;
    else if (q->bitmap_size * 1.5 < avg_bitmap_size)
        perf_score *= 0.75;

    /* Adjust score based on handicap. Handicap is proportional to how late
     in the game we learned about this path. Latecomers are allowed to run
     for a bit longer until they catch up with the rest. */

    if (q->handicap >= 4) {

        perf_score *= 4;
        q->handicap -= 4;
    } else if (q->handicap) {

        perf_score *= 2;
        q->handicap--;
    }

    /* Final adjustment based on input depth, under the assumption that fuzzing
     deeper test cases is more likely to reveal stuff that can't be
     discovered with traditional fuzzers. */

    switch (q->depth) {

    case 0 ... 3:
        break;
    case 4 ... 7:
        perf_score *= 2;
        break;
    case 8 ... 13:
        perf_score *= 3;
        break;
    case 14 ... 25:
        perf_score *= 4;
        break;
    default:
        perf_score *= 5;
    }

    /* Make sure that we don't go over limit. */

    if (perf_score > HAVOC_MAX_MULT * 100)
        perf_score = HAVOC_MAX_MULT * 100;

    return perf_score;
}

/* Helper function to see if a particular change (xor_val = old ^ new) could
   be a product of deterministic bit flips with the lengths and stepovers
   attempted by afl-fuzz. This is used to avoid dupes in some of the
   deterministic fuzzing operations that follow bit flips. We also
   return 1 if xor_val is zero, which implies that the old and attempted new
   values are identical and the exec would be a waste of time. */

static u8 could_be_bitflip(u32 xor_val)
{

    u32 sh = 0;

    if (!xor_val)
        return 1;

    /* Shift left until first bit set. */

    while (!(xor_val & 1)) {
        sh++;
        xor_val >>= 1;
    }

    /* 1-, 2-, and 4-bit patterns are OK anywhere. */

    if (xor_val == 1 || xor_val == 3 || xor_val == 15)
        return 1;

    /* 8-, 16-, and 32-bit patterns are OK only if shift factor is
     divisible by 8, since that's the stepover for these ops. */

    if (sh & 7)
        return 0;

    if (xor_val == 0xff || xor_val == 0xffff || xor_val == 0xffffffff)
        return 1;

    return 0;
}

/* Helper function to see if a particular value is reachable through
   arithmetic operations. Used for similar purposes. */

static u8 could_be_arith(u32 old_val, u32 new_val, u8 blen)
{

    u32 i, ov = 0, nv = 0, diffs = 0;

    if (old_val == new_val)
        return 1;

    /* See if one-byte adjustments to any byte could produce this result. */

    for (i = 0; i < blen; i++) {

        u8 a = old_val >> (8 * i), b = new_val >> (8 * i);

        if (a != b) {
            diffs++;
            ov = a;
            nv = b;
        }
    }

    /* If only one byte differs and the values are within range, return 1. */

    if (diffs == 1) {

        if ((u8)(ov - nv) <= ARITH_MAX || (u8)(nv - ov) <= ARITH_MAX)
            return 1;
    }

    if (blen == 1)
        return 0;

    /* See if two-byte adjustments to any byte would produce this result. */

    diffs = 0;

    for (i = 0; i < blen / 2; i++) {

        u16 a = old_val >> (16 * i), b = new_val >> (16 * i);

        if (a != b) {
            diffs++;
            ov = a;
            nv = b;
        }
    }

    /* If only one word differs and the values are within range, return 1. */

    if (diffs == 1) {

        if ((u16)(ov - nv) <= ARITH_MAX || (u16)(nv - ov) <= ARITH_MAX)
            return 1;

        ov = SWAP16(ov);
        nv = SWAP16(nv);

        if ((u16)(ov - nv) <= ARITH_MAX || (u16)(nv - ov) <= ARITH_MAX)
            return 1;
    }

    /* Finally, let's do the same thing for dwords. */

    if (blen == 4) {

        if ((u32)(old_val - new_val) <= ARITH_MAX || (u32)(new_val - old_val) <= ARITH_MAX)
            return 1;

        new_val = SWAP32(new_val);
        old_val = SWAP32(old_val);

        if ((u32)(old_val - new_val) <= ARITH_MAX || (u32)(new_val - old_val) <= ARITH_MAX)
            return 1;
    }

    return 0;
}

/* Last but not least, a similar helper to see if insertion of an
   interesting integer is redundant given the insertions done for
   shorter blen. The last param (check_le) is set if the caller
   already executed LE insertion for current blen and wants to see
   if BE variant passed in new_val is unique. */

static u8 could_be_interest(u32 old_val, u32 new_val, u8 blen, u8 check_le)
{

    u32 i, j;

    if (old_val == new_val)
        return 1;

    /* See if one-byte insertions from interesting_8 over old_val could
     produce new_val. */

    for (i = 0; i < blen; i++) {

        for (j = 0; j < sizeof(interesting_8); j++) {

            u32 tval = (old_val & ~(0xff << (i * 8))) | (((u8)interesting_8[j]) << (i * 8));

            if (new_val == tval)
                return 1;
        }
    }

    /* Bail out unless we're also asked to examine two-byte LE insertions
     as a preparation for BE attempts. */

    if (blen == 2 && !check_le)
        return 0;

    /* See if two-byte insertions over old_val could give us new_val. */

    for (i = 0; i < blen - 1; i++) {

        for (j = 0; j < sizeof(interesting_16) / 2; j++) {

            u32 tval = (old_val & ~(0xffff << (i * 8))) | (((u16)interesting_16[j]) << (i * 8));

            if (new_val == tval)
                return 1;

            /* Continue here only if blen > 2. */

            if (blen > 2) {

                tval = (old_val & ~(0xffff << (i * 8))) | (SWAP16(interesting_16[j]) << (i * 8));

                if (new_val == tval)
                    return 1;
            }
        }
    }

    if (blen == 4 && check_le) {

        /* See if four-byte insertions could produce the same result
       (LE only). */

        for (j = 0; j < sizeof(interesting_32) / 4; j++)
            if (new_val == (u32)interesting_32[j])
                return 1;
    }

    return 0;
}

void sample_current_execution(QuerySequenceGenerator* p_query_gen)
{
    ofstream outputfile;
    sample_output_id++;

    string bug_output_dir = "/home/" + dbms_name + "/fuzzing/Bug_Analysis/bug_samples/bug:" + to_string(sample_output_id) + ":src:" + to_string(current_entry) + ":core:" + std::to_string(bind_to_core_id) + ".txt";
    // cerr << "Bug output dir is: " << bug_output_dir << endl;
    outputfile.open(bug_output_dir, std::ofstream::out | std::ofstream::app);

    stream_output_res(outputfile, p_query_gen, /* is_debug = */ true);

    #if defined(mariadb)
    outputfile << "\n\n\nGetting number of reverse statement that match IRTypeSelectStmt: " << rsg->m_cached_reverse_tree[IRTypeSelect].size() << "\n";
    #else
    outputfile << "\n\n\nGetting number of reverse statement that match IRTypeSelectStmt: " << rsg->m_cached_reverse_tree[IRTypeSelectStmt].size() << "\n";
    #endif
    outputfile << "Getting number of saved reverse tree: " << rsg->v_p_cached_reverse_tree.size() << "\n\n\n";
    outputfile << "\nEND output. ";

    outputfile.close();
}

/* Output logical bugs to the bug reporting folder. */
void log_logical_bug(QuerySequenceGenerator* p_query_gen)
{
    ofstream outputfile;
    bug_output_id++;

    string bug_output_dir = "/home/" + dbms_name + "/fuzzing/Bug_Analysis/detected_bugs/bug:" + to_string(bug_output_id) + ":src:" + to_string(current_entry) + ":core:" + std::to_string(bind_to_core_id) + ".txt";
    // cerr << "Bug output dir is: " << bug_output_dir << endl;
    outputfile.open(bug_output_dir, std::ofstream::out | std::ofstream::app);

    stream_output_res(outputfile, p_query_gen);

    outputfile.close();
}

void classify_pattern(string input){
   if (findStringIn(input, "(SELECT") || findStringIn(input, "( SELECT")){
    // cerr << "Getting complex pattern string: " << input << endl;
    num_subquery++;
   }

   if (findStringIn(input, "ORDER BY") || findStringIn(input, "GROUP BY") || findStringIn(input, "HAVING") || findStringIn(input, "WINDOW") || findStringIn(input, "OVER") ){
    num_ORDER_GROUP++;
   }

   if (findStringIn(input, "JOIN")){
    num_JOIN++;
   }

   if (findStringIn(input, "CASE")){
    num_CASE++;
   }

   num_pattern_total_exec++;
}

/* Take the current entry from the queue, fuzz it for a while. This
   function is a tad too long... returns 0 if fuzzed successfully, 1 if
   skipped or bailed out. */

static u8 fuzz_one(char** argv)
{

    s32 len, fd, temp_len, i, j;
    u8 *in_buf, *out_buf, *orig_in, *ex_tmp, *eff_map = 0;
    u64 havoc_queued, orig_hit_cnt, new_hit_cnt;
    u32 splice_cycle = 0, perf_score = 100, orig_perf, prev_cksum, eff_cnt = 1;
    u8 ret_val = 1, doing_det = 0;

    u8 a_collect[MAX_AUTO_EXTRA];
    u32 a_len = 0;
    IR* program;

    vector<IR*> v_ir_stmts;
    v_ir_stmts.clear();

    char* tmp_name = stage_name;
    int skip_count;
    string input;

    if (not_on_tty) {
        ACTF("Fuzzing test case #%u (%u total, %llu uniq crashes found)...",
            current_entry, queued_paths, unique_crashes);
        fflush(stdout);
    }

    /* Map the test case into memory. */

    fd = open(queue_cur->fname, O_RDONLY);

    if (fd < 0)
        PFATAL("Unable to open '%s'", queue_cur->fname);

    len = queue_cur->len;

    orig_in = in_buf = mmap(0, len, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);

    if (orig_in == MAP_FAILED)
        PFATAL("Unable to mmap '%s'", queue_cur->fname);

    close(fd);

    /* We could mmap() out_buf as MAP_PRIVATE, but we end up clobbering every
         single byte anyway, so it wouldn't give us any performance or memory
       usage benefits. */

    out_buf = ck_alloc_nozero(len + 1);

    subseq_tmouts = 0;

    cur_depth = queue_cur->depth;

    memcpy(out_buf, in_buf, len);
    out_buf[len] = '\0';

    //[modify] add
    stage_name = "generate";

    skip_count = 0;

    // Reset the database data.
    // TODO::FIXME, port the reset_database functions to the separated DBMS connector.
    //    p_dbms_connector->reset_database_without_restart(argv, exec_tmout);
    //    reset_database_without_restart(argv, exec_tmout);
    //    p_dbms_connector->run_target(argv, "", 1);
    p_dbms_connector->restart_dbms(argv);

    p_instantiator->reset_data_library();

    /* Always execute this. For bootstrap empty table context */
    auto* query_seq_gen = p_init_query_sequence_gen->deep_copy();

    is_save_curr_query_seq = false;

    // Some configurations to run at the beginning of the fuzzing.
    // Such as SQLite's `PRAGMA`.
    // Empty for DuckDB and CockroachDB now. 
    vector<QueryStmt*> pre_insert_stmt_vec = p_instantiator->construct_custom_pre_insert_stmt();
    for (auto* p_pre_insert_stmt: pre_insert_stmt_vec) {
        p_dbms_connector->run_target(argv, p_pre_insert_stmt->to_string(), 0);
        query_seq_gen->p_query_sequence->append_good_stmt(p_pre_insert_stmt);
        p_pre_insert_stmt->res_str = p_result_handl->get_tmp_cur_res();
        p_dbms_connector->record_code_coverage(argv);
        p_dbms_connector->has_new_bits(p_dbms_connector->get_virgin_bits(), p_pre_insert_stmt->to_string());
    }

    // Execute the already existing good query statements.
    // Pre-inserted by the query sequence generator.
    for (auto* p_pre_init_stmt: query_seq_gen->p_query_sequence->get_good_query_stmts() ) {
        p_dbms_connector->run_target(argv, p_pre_init_stmt->to_string(), 0);
        p_pre_init_stmt->res_str = p_result_handl->get_tmp_cur_res();
        p_dbms_connector->record_code_coverage(argv);
        p_dbms_connector->has_new_bits(p_dbms_connector->get_virgin_bits(), p_pre_init_stmt->to_string());
    }

    auto* first_table_create_stmt_tmp = first_table_create_stmt.deep_copy();
    p_dbms_connector->run_target(argv, first_table_create_stmt.to_string(), 0);
    first_table_create_stmt_tmp->res_str = p_result_handl->get_tmp_cur_res();
    query_seq_gen->p_query_sequence->append_good_stmt(first_table_create_stmt_tmp);
    p_dbms_connector->record_code_coverage(argv);
    p_dbms_connector->has_new_bits(p_dbms_connector->get_virgin_bits(), first_table_create_stmt.to_string());

    auto* first_index_create_stmt_tmp = first_index_create_stmt.deep_copy();
    p_dbms_connector->run_target(argv, first_index_create_stmt.to_string(), 0);
    first_index_create_stmt_tmp->res_str = p_result_handl->get_tmp_cur_res();
    query_seq_gen->p_query_sequence->append_good_stmt(first_index_create_stmt_tmp);
    p_dbms_connector->record_code_coverage(argv);
    p_dbms_connector->has_new_bits(p_dbms_connector->get_virgin_bits(), first_index_create_stmt.to_string());

    auto* first_insert_stmt_tmp = first_insert_stmt.deep_copy();
    p_dbms_connector->run_target(argv, first_insert_stmt.to_string(), 0);
    first_insert_stmt_tmp->res_str = p_result_handl->get_tmp_cur_res();
    query_seq_gen->p_query_sequence->append_good_stmt(first_insert_stmt_tmp);
    p_dbms_connector->record_code_coverage(argv);
    p_dbms_connector->has_new_bits(p_dbms_connector->get_virgin_bits(), first_insert_stmt.to_string());

    for (int stmt_idx = 0; stmt_idx < 40; stmt_idx++) {
        QueryStmt* cur_stmt = query_seq_gen->generate_next_query(rsg, p_fuzzing_sequence_queue);
        if (cur_stmt->stmt_ir->to_string() == "") {
            // Fix some very corner cases, where the generator can return empty SQL string.
            // Directly skip through this to save performance.
            // Also, skipping this would avoid potential crashing if the SQL is too simple.
            delete cur_stmt;
            continue;
        }
        if (cur_stmt->to_string().size() >= 2048) {
            delete cur_stmt;
            continue;
        }

        QueryStmt* mutating_original_stmt = nullptr;
        if (cur_stmt->mutating_original_stmt != nullptr) {
            mutating_original_stmt = cur_stmt->mutating_original_stmt;
        }

        if (cur_stmt == nullptr) {
            total_input_failed++;
            cerr << "For input:idx " << stmt_idx << ", query generation failed. \n\n\n";
            continue;
        }
        num_parse++;

        stage_short = "SQL fuzz";
        stage_max = 1;
        stage_name = "validating";

        stage_val_type = STAGE_VAL_NONE;

        orig_hit_cnt = queued_paths + unique_crashes;

        prev_cksum = queue_cur->exec_cksum;

        int cur_reparse;
        cur_reparse = 0;

        if (stop_soon) {
            delete cur_stmt;
            goto abandon_entry;
        }

        // Prepare for a new statement of instantiation.
        p_instantiator->reset_data_library_single_stmt();

        // Avoid modifying the required nodes for the oracle.
        if (cur_stmt->gen_method == GenAllFromNew || cur_stmt->gen_method == GenMutFromOtherSequence) {
            if (!(p_instantiator->fill_one_stmt(cur_stmt, static_cast<int>(global_instan_idx)))) {
                // instantiation fatal failure.
                delete cur_stmt;
                continue;
            }
        } else {
            if (!(p_instantiator->fill_partial_stmt(cur_stmt, static_cast<int>(global_instan_idx)))) {
                // instantiation fatal failure.
                delete cur_stmt;
                continue;
            }
        }
        // Rolling the global_instan_idx to the current instantiation index.
        query_seq_gen->set_instan_gen_id(p_instantiator->query_instan_data->g_id_counter);
        global_instan_idx = query_seq_gen->instan_idx;

        int ret_res = FAULT_NONE;

        string cur_stmt_str = cur_stmt->to_string();

        // if (cur_stmt->stmt_ir != nullptr && cur_stmt->stmt_ir->get_ir_type() == IRTypeSelectStmt) {
            classify_pattern(cur_stmt_str);
        // }

        if (cur_stmt_str.size() >= 2048) {
            delete cur_stmt;
            continue;
        }

        ret_res = p_dbms_connector->run_target(argv,
            cur_stmt_str, 0);
        cur_stmt->res_str = p_result_handl->get_tmp_cur_res();

#ifdef DEBUG
        cerr << "For stmt: " << cur_stmt_str << "\n";
        cerr << "Getting results: " << p_result_handl->get_tmp_cur_res() << "\n\n\n";
#endif

        // Results handling after run_target
        ResultType res_type = p_result_handl->check_results(cur_stmt->res_str);

        if (res_type == ResultInternalError
        #if defined (mysqldb) || defined(mariadb)
        && p_dbms_connector->get_is_timeout() == false
        #endif
        ) {
            // If the query execution triggers an Internal Error,
            // log the buggy query string.
            p_instantiator->rollback_instan_lib_changes();
            // Here, we are saving append_error_stmt to avoid saving the
            // crashing statement to the queue. However, the context are
            // saved. This is for avoiding too many duplicated bugs, but
            // save the context for easier new bug discovery.
            query_seq_gen->p_query_sequence->append_error_stmt(cur_stmt);
            log_logical_bug(query_seq_gen);
            rsg->rsg_succeed_with_reward(query_seq_gen);
            is_save_curr_query_seq = true;
            break;
        } else if (res_type == ResultError) {
            debug_error++;
            p_instantiator->rollback_instan_lib_changes();

            // Save the error statement to logging.
            query_seq_gen->p_query_sequence->append_error_stmt(cur_stmt);
            ret_res = FAULT_SQLERROR;
            rsg->clear_v_ref_reverse_trees();

        } else if (res_type == ResultNormal) {
            debug_good++;
            if (cur_stmt->query_instan_data != nullptr) {
                delete cur_stmt->query_instan_data;
                cur_stmt->query_instan_data = nullptr;
            }
            cur_stmt->query_instan_data = p_instantiator->get_query_instan_data()->deep_copy();
            query_seq_gen->p_query_sequence->append_good_stmt(cur_stmt);
            total_instan_succeed_num++;
        }
        total_instan_num++;

        p_dbms_connector->record_code_coverage(argv);

        //        p_feedback_mapper->feedback_mapping(v_stmt_ir, p_query_plan_handl);
        //        p_result_handl->clear_results();

        save_if_interesting(argv, ret_res, query_seq_gen);

        if (ret_res == FAULT_SQLERROR) {
            // if SQLERROR, try to re-execute the correct version of the query.
            if (mutating_original_stmt) {
                p_dbms_connector->run_target(argv,
                    mutating_original_stmt->to_string(), 0);
                mutating_original_stmt = mutating_original_stmt->deep_copy();
                mutating_original_stmt->gen_method = GenFallBackUseOriginalSequence;
                // We don't care about the feedback from the re-run
                p_dbms_connector->record_code_coverage(argv);
                p_dbms_connector->has_new_bits(p_dbms_connector->get_virgin_bits(), mutating_original_stmt->to_string());
                query_seq_gen->p_query_sequence->append_good_stmt(mutating_original_stmt);
                rsg->clear_v_ref_reverse_trees();
            }
        }

        total_execs++;
        show_stats();

    } // stmt_idx loop

    if (is_sample_current_execution) {
        sample_current_execution(query_seq_gen);
        is_sample_current_execution = false;
    }

    if (is_save_curr_query_seq) {
        query_seq_gen->p_query_sequence->strip_unnecessary_data();
        query_seq_gen->is_finished_gen = true;
        save_query_sequence_to_queue(query_seq_gen);
    }

    total_execute++;
    stage_cur++;
    stage_cur = stage_max = 0;
    stage_finds[STAGE_FLIP1] += new_hit_cnt - orig_hit_cnt;
    stage_cycles[STAGE_FLIP1] += 40 - skip_count;
    stage_name = tmp_name;

    new_hit_cnt = queued_paths + unique_crashes;

    ret_val = 0;

abandon_entry:

    delete query_seq_gen;
    splicing_with = -1;

    /* Update pending_not_fuzzed count if we made it through the calibration
       cycle and have not seen this entry before. */

    if (!stop_soon && !queue_cur->cal_failed && !queue_cur->was_fuzzed) {
        queue_cur->was_fuzzed = 1;
        pending_not_fuzzed--;
        if (queue_cur->favored)
            pending_favored--;
    }

    munmap(orig_in, queue_cur->len);

    if (in_buf != orig_in)
        ck_free(in_buf);
    ck_free(out_buf);

    ck_free(eff_map);
    // ir_set.clear();

    return ret_val;
}

/* Grab interesting test cases from other fuzzers. */

static void sync_fuzzers(char** argv)
{

    DIR* sd;
    struct dirent* sd_ent;
    u32 sync_cnt = 0;

    sd = opendir(sync_dir);
    if (!sd)
        PFATAL("Unable to open '%s'", sync_dir);

    stage_max = stage_cur = 0;
    cur_depth = 0;

    /* Look at the entries created for every other fuzzer in the sync directory.
     */

    while ((sd_ent = readdir(sd))) {

        static u8 stage_tmp[128];

        DIR* qd;
        struct dirent* qd_ent;
        u8 *qd_path, *qd_synced_path;
        u32 min_accept = 0, next_min_accept;

        s32 id_fd;

        /* Skip dot files and our own output directory. */

        if (sd_ent->d_name[0] == '.' || !strcmp(sync_id, sd_ent->d_name))
            continue;

        /* Skip anything that doesn't have a queue/ subdirectory. */

        qd_path = alloc_printf("%s/%s/queue", sync_dir, sd_ent->d_name);

        if (!(qd = opendir(qd_path))) {
            ck_free(qd_path);
            continue;
        }

        /* Retrieve the ID of the last seen test case. */

        qd_synced_path = alloc_printf("%s/.synced/%s", out_dir, sd_ent->d_name);

        id_fd = open(qd_synced_path, O_RDWR | O_CREAT, 0640);

        if (id_fd < 0)
            PFATAL("Unable to create '%s'", qd_synced_path);

        if (read(id_fd, &min_accept, sizeof(u32)) > 0)
            lseek(id_fd, 0, SEEK_SET);

        next_min_accept = min_accept;

        /* Show stats */

        sprintf(stage_tmp, "sync %u", ++sync_cnt);
        stage_name = stage_tmp;
        stage_cur = 0;
        stage_max = 0;

        /* For every file queued by this fuzzer, parse ID and see if we have looked
       at it before; exec a test case if not. */

        while ((qd_ent = readdir(qd))) {

            u8* path;
            s32 fd;
            struct stat st;

            if (qd_ent->d_name[0] == '.' || sscanf(qd_ent->d_name, CASE_PREFIX "%06u", &syncing_case) != 1 || syncing_case < min_accept)
                continue;

            /* OK, sounds like a new one. Let's give it a try. */

            if (syncing_case >= next_min_accept)
                next_min_accept = syncing_case + 1;

            path = alloc_printf("%s/%s", qd_path, qd_ent->d_name);

            /* Allow this to fail in case the other fuzzer is resuming or so... */

            fd = open(path, O_RDONLY);

            if (fd < 0) {
                ck_free(path);
                continue;
            }

            if (fstat(fd, &st))
                PFATAL("fstat() failed");

            /* Ignore zero-sized or oversized files. */

            if (st.st_size && st.st_size <= MAX_FILE) {

                u8 fault;
                u8* mem = mmap(0, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);

                if (mem == MAP_FAILED)
                    PFATAL("Unable to mmap '%s'", path);

                /* See what happens. We rely on save_if_interesting() to catch major
           errors and save the test case. */

                // write_to_testcase(mem, st.st_size);
                string cmd_str = (char*)mem;

                fault = p_dbms_connector->run_target(argv, cmd_str);

                if (stop_soon)
                    return;

                string saved_str = "";
                for (int output_index = 0; output_index < st.st_size; output_index++) {
                    saved_str += mem[output_index];
                }

                syncing_party = sd_ent->d_name;
                //                ALL_COMP_RES all_comp_res;
                //                vector<int> dump_vec;
                //                all_comp_res.cmd_str = saved_str;
                //                all_comp_res.v_cmd_str.push_back(saved_str);
                //                queued_imported +=
                //                        save_if_interesting(argv, saved_str, fault, all_comp_res);
                syncing_party = 0;

                munmap(mem, st.st_size);

                if (!(stage_cur++ % stats_update_freq))
                    show_stats();
            }

            ck_free(path);
            close(fd);
        }

        ck_write(id_fd, &next_min_accept, sizeof(u32), qd_synced_path);

        close(id_fd);
        closedir(qd);
        ck_free(qd_path);
        ck_free(qd_synced_path);
    }

    closedir(sd);
}

/* Handle stop signal (Ctrl-C, etc). */

static void handle_stop_sig(int sig)
{

    stop_soon = 1;
    p_dbms_connector->set_stop_soon(1);

    if (p_dbms_connector->get_forksrv_pid() > 0) {
        kill(p_dbms_connector->get_forksrv_pid(), SIGKILL);
        int status;
        wait(&status);
    }
}

/* Handle skip request (SIGUSR1). */

static void handle_skipreq(int sig) { skip_requested = 1; }

/* Handle timeout (SIGALRM). */

static void handle_timeout(int sig)
{
    #if defined (cockroachdb)
    p_dbms_connector->set_is_timeout(true);
    child_timed_out = 1;
    p_dbms_connector->set_child_timed_out(1);
    if (p_dbms_connector->get_child_pid() != -1) {
        kill(p_dbms_connector->get_child_pid(), SIGKILL);
        int status;
        waitpid(p_dbms_connector->get_child_pid(), &status, 0);
    }
    if (p_dbms_connector->get_forksrv_pid() != -1) {
        kill(p_dbms_connector->get_forksrv_pid(), SIGKILL);
        int status;
        waitpid(p_dbms_connector->get_forksrv_pid(), &status, 0);
    }
    p_dbms_connector->set_child_pid(-1);
    p_dbms_connector->set_forksrv_pid(-1);
    #elif defined(mysqldb) || defined(mariadb) || defined(postgresql)
    // Not handled here. 
    return;
    #else
    p_dbms_connector->set_is_timeout(true);
    child_timed_out = 1;
    p_dbms_connector->set_child_timed_out(1);
    if (p_dbms_connector->get_child_pid() != -1) {
        kill(p_dbms_connector->get_child_pid(), SIGKILL);
        int status;
        waitpid(p_dbms_connector->get_child_pid(), &status, 0);
    }
    // if (p_dbms_connector->get_forksrv_pid() != -1) {
    //     kill(p_dbms_connector->get_forksrv_pid(), SIGKILL);
    //     int status;
    //     waitpid(p_dbms_connector->get_forksrv_pid(), &status, 0);
    // }
    p_dbms_connector->set_child_pid(-1);
    // p_dbms_connector->set_forksrv_pid(-1);
    #endif
}

/* Do a PATH search and find target binary to see that it exists and
   isn't a shell script - a common and painful mistake. We also check for
   a valid ELF header and for evidence of AFL instrumentation. */

EXP_ST void check_binary(u8* fname)
{

    u8* env_path = 0;
    struct stat st;

    s32 fd;
    u8* f_data;
    u32 f_len = 0;

    ACTF("Validating target binary...");

    if (strchr((char*)fname, '/') || !(env_path = getenv("PATH"))) {

        target_path = ck_strdup(fname);
        if (stat(target_path, &st) || !S_ISREG(st.st_mode) || !(st.st_mode & 0111) || (f_len = st.st_size) < 4)
            FATAL("Program '%s' not found or not executable", fname);
    } else {

        while (env_path) {

            u8 *cur_elem, *delim = strchr((char*)env_path, ':');

            if (delim) {

                cur_elem = ck_alloc(delim - env_path + 1);
                memcpy(cur_elem, env_path, delim - env_path);
                delim++;
            } else
                cur_elem = ck_strdup(env_path);

            env_path = delim;

            if (cur_elem[0])
                target_path = alloc_printf("%s/%s", cur_elem, fname);
            else
                target_path = ck_strdup(fname);

            ck_free(cur_elem);

            if (!stat(target_path, &st) && S_ISREG(st.st_mode) && (st.st_mode & 0111) && (f_len = st.st_size) >= 4)
                break;

            ck_free(target_path);
            target_path = 0;
        }

        if (!target_path)
            FATAL("Program '%s' not found or not executable", fname);
    }

    if (getenv("AFL_SKIP_BIN_CHECK"))
        return;

    /* Check for blatant user errors. */

    if ((!strncmp(target_path, "/tmp/", 5) && !strchr((char*)target_path + 5, '/')) || (!strncmp(target_path, "/var/tmp/", 9) && !strchr((char*)target_path + 9, '/')))
        FATAL("Please don't keep binaries in /tmp or /var/tmp");

    fd = open(target_path, O_RDONLY);

    if (fd < 0)
        PFATAL("Unable to open '%s'", target_path);

    f_data = mmap(0, f_len, PROT_READ, MAP_PRIVATE, fd, 0);

    if (f_data == MAP_FAILED)
        PFATAL("Unable to mmap file '%s'", target_path);

    close(fd);

    if (f_data[0] == '#' && f_data[1] == '!') {

        SAYF("\n" cLRD "[-] " cRST "Oops, the target binary looks like a shell "
             "script. Some build systems will\n"
             "    sometimes generate shell stubs for dynamically linked programs; "
             "try static\n"
             "    library mode (./configure --disable-shared) if that's the "
             "case.\n\n"

             "    Another possible cause is that you are actually trying to use a "
             "shell\n"
             "    wrapper around the fuzzed component. Invoking shell can slow "
             "down the\n"
             "    fuzzing process by a factor of 20x or more; it's best to write "
             "the wrapper\n"
             "    in a compiled language instead.\n");

        FATAL("Program '%s' is a shell script", target_path);
    }

#ifndef __APPLE__

    if (f_data[0] != 0x7f || memcmp(f_data + 1, "ELF", 3))
        FATAL("Program '%s' is not an ELF binary", target_path);

#else

    if (f_data[0] != 0xCF || f_data[1] != 0xFA || f_data[2] != 0xED)
        FATAL("Program '%s' is not a 64-bit Mach-O binary", target_path);

#endif /* ^!__APPLE__ */

    if (!qemu_mode && !dumb_mode && !memmem(f_data, f_len, SHM_ENV_VAR, strlen(SHM_ENV_VAR) + 1)) {

        SAYF(
            "\n" cLRD "[-] " cRST "Looks like the target binary is not "
            "instrumented! The fuzzer depends on\n"
            "    compile-time instrumentation to isolate interesting test cases "
            "while\n"
            "    mutating the input data. For more information, and for tips on "
            "how to\n"
            "    instrument binaries, please see %s/README.\n\n"

            "    When source code is not available, you may be able to leverage "
            "QEMU\n"
            "    mode support. Consult the README for tips on how to enable this.\n"

            "    (It is also possible to use afl-fuzz as a traditional, \"dumb\" "
            "fuzzer.\n"
            "    For that, you can use the -n option - but expect much worse "
            "results.)\n",
            doc_path);

        FATAL("No instrumentation detected");
    }

    if (qemu_mode && memmem(f_data, f_len, SHM_ENV_VAR, strlen(SHM_ENV_VAR) + 1)) {

        SAYF("\n" cLRD "[-] " cRST "This program appears to be instrumented with "
             "afl-gcc, but is being run in\n"
             "    QEMU mode (-Q). This is probably not what you want - this setup "
             "will be\n"
             "    slow and offer no practical benefits.\n");

        FATAL("Instrumentation found in -Q mode");
    }

    if (memmem(f_data, f_len, "libasan.so", 10) || memmem(f_data, f_len, "__msan_init", 11)) {
        uses_asan = 1;
        p_dbms_connector->set_uses_asan(uses_asan);
    }

    /* Detect persistent & deferred init signatures in the binary. */

    if (memmem(f_data, f_len, PERSIST_SIG, strlen(PERSIST_SIG) + 1)) {

        OKF(cPIN "Persistent mode binary detected.");
        setenv(PERSIST_ENV_VAR, "1", 1);
        persistent_mode = 1;
    } else if (getenv("AFL_PERSISTENT")) {

        WARNF("AFL_PERSISTENT is no longer supported and may misbehave!");
    }

    if (memmem(f_data, f_len, DEFER_SIG, strlen(DEFER_SIG) + 1)) {

        OKF(cPIN "Deferred forkserver binary detected.");
        setenv(DEFER_ENV_VAR, "1", 1);
        deferred_mode = 1;
    } else if (getenv("AFL_DEFER_FORKSRV")) {

        WARNF("AFL_DEFER_FORKSRV is no longer supported and may misbehave!");
    }

    if (munmap(f_data, f_len))
        PFATAL("unmap() failed");
}

/* Trim and possibly create a banner for the run. */

static void fix_up_banner(u8* name)
{

    if (!use_banner) {

        if (sync_id) {

            use_banner = sync_id;
        } else {

            u8* trim = strrchr((const char*)name, '/');
            if (!trim)
                use_banner = name;
            else
                use_banner = trim + 1;
        }
    }

    if (strlen(use_banner) > 40) {

        u8* tmp = ck_alloc(44);
        sprintf(tmp, "%.40s...", use_banner);
        use_banner = tmp;
    }
}

/* Check if we're on TTY. */

static void check_if_tty(void)
{

    struct winsize ws;

    if (getenv("AFL_NO_UI")) {
        OKF("Disabling the UI because AFL_NO_UI is set.");
        not_on_tty = 1;
        return;
    }

    if (ioctl(1, TIOCGWINSZ, &ws)) {

        if (errno == ENOTTY) {
            OKF("Looks like we're not running on a tty, so I'll be a bit less "
                "verbose.");
            not_on_tty = 1;
        }

        return;
    }
}

/* Check terminal dimensions after resize. */

static void check_term_size(void)
{

    struct winsize ws;

    term_too_small = 0;

    if (ioctl(1, TIOCGWINSZ, &ws))
        return;

    if (ws.ws_row < 25 || ws.ws_col < 80)
        term_too_small = 1;
}

/* Display usage hints. */

static void usage(u8* argv0)
{

    SAYF(
        "\n%s [ options ] -- /path/to/fuzzed_app [ ... ]\n\n"

        "Required parameters:\n\n"

        "  -i dir        - input directory with test cases\n"
        "  -o dir        - output directory for fuzzer findings\n\n"

        "Execution control settings:\n\n"

        "  -f file       - location read by the fuzzed program (stdin)\n"
        "  -t msec       - timeout for each run (auto-scaled, 50-%u ms)\n"
        "  -m megs       - memory limit for child process (%u MB)\n"
        "  -Q            - use binary-only instrumentation (QEMU mode)\n\n"

        "Fuzzing behavior settings:\n\n"

        "  -d            - quick & dirty mode (skips deterministic steps)\n"
        "  -n            - fuzz without instrumentation (dumb mode)\n"
        "  -x dir        - optional fuzzer dictionary (see README)\n\n"

        "Other stuff:\n\n"

        "  -T text       - text banner to show on the screen\n"
        "  -M / -S id    - distributed mode (see parallel_fuzzing.txt)\n"
        "  -C            - crash exploration mode (the peruvian rabbit thing)\n\n"

        "For additional tips, please consult %s/README.\n\n",

        argv0, EXEC_TIMEOUT, MEM_LIMIT, doc_path);

    exit(1);
}

/* Prepare output directories and fds. */

EXP_ST void setup_dirs_fds(void)
{

    u8* tmp;
    s32 fd;

    ACTF("Setting up output directories...");

    if (sync_id && mkdir(sync_dir, 0750) && errno != EEXIST)
        PFATAL("Unable to create '%s'", sync_dir);

    if (mkdir(out_dir, 0750)) {

        if (errno != EEXIST)
            PFATAL("Unable to create '%s'", out_dir);

        maybe_delete_out_dir();
    } else {

        if (in_place_resume)
            FATAL("Resume attempted but old output directory not found");

        p_dbms_connector->set_out_dir_fd(open(out_dir, O_RDONLY));

#ifndef __sun

        if (p_dbms_connector->get_out_dir_fd() < 0 || flock(p_dbms_connector->get_out_dir_fd(), LOCK_EX | LOCK_NB))
            PFATAL("Unable to flock() output directory.");

#endif /* !__sun */
    }

    /* Queue directory for any starting & discovered paths. */

    tmp = alloc_printf("%s/queue", out_dir);
    if (mkdir(tmp, 0750))
        PFATAL("Unable to create '%s'", tmp);
    ck_free(tmp);

    /* Top-level directory for queue metadata used for session
     resume and related tasks. */

    tmp = alloc_printf("%s/queue/.state/", out_dir);
    if (mkdir(tmp, 0750))
        PFATAL("Unable to create '%s'", tmp);
    ck_free(tmp);

    /* Directory for flagging queue entries that went through
     deterministic fuzzing in the past. */

    tmp = alloc_printf("%s/queue/.state/deterministic_done/", out_dir);
    if (mkdir(tmp, 0750))
        PFATAL("Unable to create '%s'", tmp);
    ck_free(tmp);

    /* Directory with the auto-selected dictionary entries. */

    tmp = alloc_printf("%s/queue/.state/auto_extras/", out_dir);
    if (mkdir(tmp, 0750))
        PFATAL("Unable to create '%s'", tmp);
    ck_free(tmp);

    /* The set of paths currently deemed redundant. */

    tmp = alloc_printf("%s/queue/.state/redundant_edges/", out_dir);
    if (mkdir(tmp, 0750))
        PFATAL("Unable to create '%s'", tmp);
    ck_free(tmp);

    /* The set of paths showing variable behavior. */

    tmp = alloc_printf("%s/queue/.state/variable_behavior/", out_dir);
    if (mkdir(tmp, 0750))
        PFATAL("Unable to create '%s'", tmp);
    ck_free(tmp);

    /* Sync directory for keeping track of cooperating fuzzers. */

    if (sync_id) {

        tmp = alloc_printf("%s/.synced/", out_dir);

        if (mkdir(tmp, 0750) && (!in_place_resume || errno != EEXIST))
            PFATAL("Unable to create '%s'", tmp);

        ck_free(tmp);
    }

    /* All recorded crashes. */

    tmp = alloc_printf("%s/crashes", out_dir);
    if (mkdir(tmp, 0750))
        PFATAL("Unable to create '%s'", tmp);
    ck_free(tmp);

    /* All recorded hangs. */

    tmp = alloc_printf("%s/hangs", out_dir);
    if (mkdir(tmp, 0750))
        PFATAL("Unable to create '%s'", tmp);
    ck_free(tmp);

    /* Gnuplot output file. */

    tmp = alloc_printf("%s/plot_data", out_dir);
    fd = open(tmp, O_WRONLY | O_CREAT | O_EXCL, 0640);
    if (fd < 0)
        PFATAL("Unable to create '%s'", tmp);
    ck_free(tmp);

    plot_file = fdopen(fd, "w");
    p_dbms_connector->set_p_plot_file(plot_file);
    if (!plot_file)
        PFATAL("fdopen() failed");

    fprintf(
        plot_file,
        "unix_time,cycles_done,cur_path,paths_total,"
        "pending_total,pending_favs,map_size,unique_crashes,"
        "unique_hangs,max_depth,execs_per_sec,total_execs,"
        "total_exec_good,total_exec_good_rate,reverse_tree_size,"
        "virtual_mem,resident_set_size_mem,"
        "num_subquery,num_ORDER_GROUP,num_JOIN,num_CASE,num_pattern_total_exec"
        "\n");

    /* ignore errors */
}

/* Setup the output file for fuzzed data, if not using -f. */

EXP_ST void setup_stdio_file(void)
{

    u8* fn = alloc_printf("%s/.cur_input", out_dir);

    unlink(fn); /* Ignore errors */
    out_fd = open(fn, O_RDWR | O_CREAT | O_EXCL, 0640);

    if (out_fd < 0)
        PFATAL("Unable to create '%s'", fn);

    ck_free(fn);
}

/* Make sure that core dumps don't go to a program. */

static void check_crash_handling(void)
{

#ifdef __APPLE__

    /* Yuck! There appears to be no simple C API to query for the state of
     loaded daemons on MacOS X, and I'm a bit hesitant to do something
     more sophisticated, such as disabling crash reporting via Mach ports,
     until I get a box to test the code. So, for now, we check for crash
     reporting the awful way. */

    if (system("launchctl list 2>/dev/null | grep -q '\\.ReportCrash$'"))
        return;

    SAYF(
        "\n" cLRD "[-] " cRST
        "Whoops, your system is configured to forward crash notifications to an\n"
        "    external crash reporting utility. This will cause issues due to "
        "the\n"
        "    extended delay between the fuzzed binary malfunctioning and this "
        "fact\n"
        "    being relayed to the fuzzer via the standard waitpid() API.\n\n"
        "    To avoid having crashes misinterpreted as timeouts, please run the\n"
        "    following commands:\n\n"

        "    SL=/System/Library; PL=com.apple.ReportCrash\n"
        "    launchctl unload -w ${SL}/LaunchAgents/${PL}.plist\n"
        "    sudo launchctl unload -w ${SL}/LaunchDaemons/${PL}.Root.plist\n");

    if (!getenv("AFL_I_DONT_CARE_ABOUT_MISSING_CRASHES"))
        FATAL("Crash reporter detected");

#else

    /* This is Linux specific, but I don't think there's anything equivalent on
     *BSD, so we can just let it slide for now. */

    s32 fd = open("/proc/sys/kernel/core_pattern", O_RDONLY);
    u8 fchar;

    if (fd < 0)
        return;

    ACTF("Checking core_pattern...");

    if (read(fd, &fchar, 1) == 1 && fchar == '|') {

        SAYF(
            "\n" cLRD "[-] " cRST
            "Hmm, your system is configured to send core dump notifications to an\n"
            "    external utility. This will cause issues: there will be an "
            "extended delay\n"
            "    between stumbling upon a crash and having this information "
            "relayed to the\n"
            "    fuzzer via the standard waitpid() API.\n\n"

            "    To avoid having crashes misinterpreted as timeouts, please log in "
            "as root\n"
            "    and temporarily modify /proc/sys/kernel/core_pattern, like so:\n\n"

            "    echo core >/proc/sys/kernel/core_pattern\n");

        if (!getenv("AFL_I_DONT_CARE_ABOUT_MISSING_CRASHES"))
            FATAL("Pipe at the beginning of 'core_pattern'");
    }

    close(fd);

#endif /* ^__APPLE__ */
}

/* Check CPU governor. */

static void check_cpu_governor(void)
{

    FILE* f;
    u8 tmp[128];
    u64 min = 0, max = 0;

    if (getenv("AFL_SKIP_CPUFREQ"))
        return;

    f = fopen("/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor", "r");
    if (!f)
        return;

    ACTF("Checking CPU scaling governor...");

    if (!fgets(tmp, 128, f))
        PFATAL("fgets() failed");

    fclose(f);

    if (!strncmp(tmp, "perf", 4))
        return;

    f = fopen("/sys/devices/system/cpu/cpu0/cpufreq/scaling_min_freq", "r");

    if (f) {
        if (fscanf(f, "%llu", &min) != 1)
            min = 0;
        fclose(f);
    }

    f = fopen("/sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq", "r");

    if (f) {
        if (fscanf(f, "%llu", &max) != 1)
            max = 0;
        fclose(f);
    }

    if (min == max)
        return;

    SAYF("\n" cLRD "[-] " cRST
         "Whoops, your system uses on-demand CPU frequency scaling, adjusted\n"
         "    between %llu and %llu MHz. Unfortunately, the scaling algorithm in "
         "the\n"
         "    kernel is imperfect and can miss the short-lived processes spawned "
         "by\n"
         "    afl-fuzz. To keep things moving, run these commands as root:\n\n"

         "    cd /sys/devices/system/cpu\n"
         "    echo performance | tee cpu*/cpufreq/scaling_governor\n\n"

         "    You can later go back to the original state by replacing "
         "'performance' with\n"
         "    'ondemand'. If you don't want to change the settings, set "
         "AFL_SKIP_CPUFREQ\n"
         "    to make afl-fuzz skip this check - but expect some performance "
         "drop.\n",
        min / 1024, max / 1024);

    FATAL("Suboptimal CPU scaling governor");
}

/* Count the number of logical CPU cores. */

static void get_core_count(void)
{

    u32 cur_runnable = 0;

#if defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__)

    size_t s = sizeof(cpu_core_count);

    /* On *BSD systems, we can just use a sysctl to get the number of CPUs. */

#ifdef __APPLE__

    if (sysctlbyname("hw.logicalcpu", &cpu_core_count, &s, NULL, 0) < 0)
        return;

#else

    int s_name[2] = { CTL_HW, HW_NCPU };

    if (sysctl(s_name, 2, &cpu_core_count, &s, NULL, 0) < 0)
        return;

#endif /* ^__APPLE__ */

#else

#ifdef HAVE_AFFINITY

    cpu_core_count = sysconf(_SC_NPROCESSORS_ONLN);

#else

    FILE* f = fopen("/proc/stat", "r");
    u8 tmp[1024];

    if (!f)
        return;

    while (fgets(tmp, sizeof(tmp), f))
        if (!strncmp(tmp, "cpu", 3) && isdigit(tmp[3]))
            cpu_core_count++;

    fclose(f);

#endif /* ^HAVE_AFFINITY */

#endif /* ^(__APPLE__ || __FreeBSD__ || __OpenBSD__) */

    if (cpu_core_count > 0) {

        cur_runnable = (u32)get_runnable_processes();

#if defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__)

        /* Add ourselves, since the 1-minute average doesn't include that yet. */

        cur_runnable++;

#endif /* __APPLE__ || __FreeBSD__ || __OpenBSD__ */

        OKF("You have %u CPU core%s and %u runnable tasks (utilization: %0.0f%%).",
            cpu_core_count, cpu_core_count > 1 ? "s" : "", cur_runnable,
            cur_runnable * 100.0 / cpu_core_count);

        if (cpu_core_count > 1) {

            if (cur_runnable > cpu_core_count * 1.5) {

                WARNF("System under apparent load, performance may be spotty.");
            } else if (cur_runnable + 1 <= cpu_core_count) {

                OKF("Try parallel jobs - see %s/parallel_fuzzing.txt.", doc_path);
            }
        }
    } else {

        cpu_core_count = 0;
        WARNF("Unable to figure out the number of CPU cores.");
    }
}

/* Validate and fix up out_dir and sync_dir when using -S. */

static void fix_up_sync(void)
{

    u8* x = sync_id;

    if (dumb_mode)
        FATAL("-S / -M and -n are mutually exclusive");

    if (skip_deterministic) {

        if (force_deterministic)
            FATAL("use -S instead of -M -d");
        else
            FATAL("-S already implies -d");
    }

    while (*x) {

        if (!isalnum(*x) && *x != '_' && *x != '-')
            FATAL("Non-alphanumeric fuzzer ID specified via -S or -M");

        x++;
    }

    if (strlen(sync_id) > 32)
        FATAL("Fuzzer ID too long");

    x = alloc_printf("%s/%s", out_dir, sync_id);

    sync_dir = out_dir;
    out_dir = x;

    if (!force_deterministic) {
        skip_deterministic = 1;
        use_splicing = 1;
    }
}

/* Handle screen resize (SIGWINCH). */

static void handle_resize(int sig) { clear_screen = 1; }

/* Check ASAN options. */

static void check_asan_opts(void)
{
    u8* x = getenv("ASAN_OPTIONS");

    if (x) {

        if (!strstr((char*)x, "abort_on_error=1"))
            FATAL("Custom ASAN_OPTIONS set without abort_on_error=1 - please fix!");

        if (!strstr((char*)x, "symbolize=0"))
            FATAL("Custom ASAN_OPTIONS set without symbolize=0 - please fix!");
    }

    x = getenv("MSAN_OPTIONS");

    if (x) {

        if (!strstr((char*)x, "exit_code=" STRINGIFY(MSAN_ERROR)))
            FATAL("Custom MSAN_OPTIONS set without exit_code=" STRINGIFY(
                MSAN_ERROR) " - please fix!");

        if (!strstr((char*)x, "symbolize=0"))
            FATAL("Custom MSAN_OPTIONS set without symbolize=0 - please fix!");
    }
}

/* Detect @@ in args. */

EXP_ST void detect_file_args(char** argv)
{

    u32 i = 0;
    u8* cwd = getcwd(NULL, 0);

    if (!cwd)
        PFATAL("getcwd() failed");

    while (argv[i]) {

        u8* aa_loc = strstr((const char*)argv[i], "@@");

        if (aa_loc) {

            u8 *aa_subst, *n_arg;

            /* If we don't have a file name chosen yet, use a safe default. */

            if (!out_file)
                out_file = alloc_printf("%s/.cur_input", out_dir);

            /* Be sure that we're always using fully-qualified paths. */

            if (out_file[0] == '/')
                aa_subst = out_file;
            else
                aa_subst = alloc_printf("%s/%s", cwd, out_file);

            /* Construct a replacement argv value. */

            *aa_loc = 0;
            n_arg = alloc_printf("%s%s%s", argv[i], aa_subst, aa_loc + 2);
            argv[i] = n_arg;
            *aa_loc = '@';

            if (out_file[0] != '/')
                ck_free(aa_subst);
        }

        i++;
    }

    free(cwd); /* not tracked */
}

/* Set up signal handlers. More complicated that needs to be, because libc on
   Solaris doesn't resume interrupted reads(), sets SA_RESETHAND when you call
   siginterrupt(), and does other unnecessary things. */

EXP_ST void setup_signal_handlers(void)
{

    struct sigaction sa;

    sa.sa_handler = NULL;
    sa.sa_flags = SA_RESTART;
    sa.sa_sigaction = NULL;

    sigemptyset(&sa.sa_mask);

    /* Various ways of saying "stop". */

    sa.sa_handler = handle_stop_sig;
    sigaction(SIGHUP, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    /* Exec timeout notifications. */

    sa.sa_handler = handle_timeout;
    sigaction(SIGALRM, &sa, NULL);

    /* Window resize */

    sa.sa_handler = handle_resize;
    sigaction(SIGWINCH, &sa, NULL);

    /* SIGUSR1: skip entry */

    sa.sa_handler = handle_skipreq;
    sigaction(SIGUSR1, &sa, NULL);

    /* Things we don't care about. */

    sa.sa_handler = SIG_IGN;
    sigaction(SIGTSTP, &sa, NULL);
    sigaction(SIGPIPE, &sa, NULL);
}

/* Rewrite argv for QEMU. */

static char** get_qemu_argv(u8* own_loc, char** argv, int argc)
{

    char** new_argv = reinterpret_cast<char**>(ck_alloc(sizeof(char*) * (argc + 4)));
    u8 *tmp, *cp, *rsl, *own_copy;

    /* Workaround for a QEMU stability glitch. */

    setenv("QEMU_LOG", "nochain", 1);

    memcpy(new_argv + 3, argv + 1, sizeof(char*) * argc);

    new_argv[2] = target_path;
    new_argv[1] = "--";

    /* Now we need to actually find the QEMU binary to put in argv[0]. */

    tmp = getenv("AFL_PATH");

    if (tmp) {

        cp = alloc_printf("%s/afl-qemu-trace", tmp);

        if (access(cp, X_OK))
            FATAL("Unable to find '%s'", tmp);

        target_path = new_argv[0] = cp;
        return new_argv;
    }

    own_copy = ck_strdup(own_loc);
    rsl = strrchr((const char*)own_copy, '/');

    if (rsl) {

        *rsl = 0;

        cp = alloc_printf("%s/afl-qemu-trace", own_copy);
        ck_free(own_copy);

        if (!access(cp, X_OK)) {

            target_path = new_argv[0] = cp;
            return new_argv;
        }
    } else
        ck_free(own_copy);

    if (!access(BIN_PATH "/afl-qemu-trace", X_OK)) {

        target_path = new_argv[0] = ck_strdup(BIN_PATH "/afl-qemu-trace");
        return new_argv;
    }

    SAYF("\n" cLRD "[-] " cRST "Oops, unable to find the 'afl-qemu-trace' "
         "binary. The binary must be built\n"
         "    separately by following the instructions in qemu_mode/README.qemu. "
         "If you\n"
         "    already have the binary installed, you may need to specify "
         "AFL_PATH in the\n"
         "    environment.\n\n"

         "    Of course, even without QEMU, afl-fuzz can still work with "
         "binaries that are\n"
         "    instrumented at compile time with afl-gcc. It is also possible to "
         "use it as a\n"
         "    traditional \"dumb\" fuzzer by specifying '-n' in the command "
         "line.\n");

    FATAL("Failed to locate 'afl-qemu-trace'.");
}

/* Make a copy of the current command line. */

static void save_cmdline(u32 argc, char** argv)
{

    u32 len = 1, i;
    u8* buf;

    for (i = 0; i < argc; i++)
        len += strlen(argv[i]) + 1;

    buf = orig_cmdline = ck_alloc(len);

    for (i = 0; i < argc; i++) {

        u32 l = strlen(argv[i]);

        memcpy(buf, argv[i], l);
        buf += l;

        if (i != argc - 1)
            *(buf++) = ' ';
    }

    *buf = 0;
}

#ifndef AFL_LIB
int main(int argc, char** argv)
{

    dump_library = 0;
    disable_dyn_instan = false;
    disable_rsg_generator = false;

    double mab_epsilon = 0.3;

    u64 tmp_exec_tmout = 0;

    s32 opt;
    u64 prev_queued = 0;
    u32 sync_interval_cnt = 0, seek_to = 0;
    u8* extras_dir = 0;
    u8 mem_limit_given = 0;
    u8 exit_1 = !!getenv("AFL_BENCH_JUST_ONE");
    char** use_argv;

    struct timeval tv;
    struct timezone tz;

    SAYF(cCYA "afl-fuzz " cBRI VERSION cRST " by <lcamtuf@google.com>\n");

    doc_path = access(DOC_PATH, F_OK) ? "docs" : DOC_PATH;

    gettimeofday(&tv, &tz);
    srandom(tv.tv_sec ^ tv.tv_usec ^ getpid());

    while ((opt = getopt(argc, argv, "+i:o:f:m:t:T:dnCB:S:M:x:QDc:lP:F:XRGE:K:s:")) > 0)

        switch (opt) {
        case 'l': /* initial input list */
            g_library_path = "./empty";
            break;

        case 'i': /* input dir */

            if (in_dir)
                FATAL("Multiple -i options not supported");
            in_dir = optarg;

            if (!strcmp(in_dir, "-"))
                in_place_resume = 1;

            break;

        case 'o': /* output dir */

            if (out_dir)
                FATAL("Multiple -o options not supported");
            out_dir = optarg;
            break;

        case 'M': { /* master sync ID */

            u8* c;

            if (sync_id)
                FATAL("Multiple -S or -M options not supported");
            sync_id = ck_strdup(optarg);

            if ((c = strchr((char*)sync_id, ':'))) {

                *c = 0;

                if (sscanf(c + 1, "%u/%u", &master_id, &master_max) != 2 || !master_id || !master_max || master_id > master_max || master_max > 1000000)
                    FATAL("Bogus master ID passed to -M");
            }

            force_deterministic = 1;
        }

        break;

        case 'S':

            if (sync_id)
                FATAL("Multiple -S or -M options not supported");
            sync_id = ck_strdup(optarg);
            break;

        case 'f': /* target file */

            if (out_file)
                FATAL("Multiple -f options not supported");
            out_file = optarg;
            break;

        case 'x': /* dictionary */

            if (extras_dir)
                FATAL("Multiple -x options not supported");
            extras_dir = optarg;
            break;

        case 't': { /* timeout */

            u8 suffix = 0;

            if (timeout_given)
                FATAL("Multiple -t options not supported");

            if (sscanf(optarg, "%u%c", &tmp_exec_tmout, &suffix) < 1 || optarg[0] == '-')
                FATAL("Bad syntax used for -t");

            if (tmp_exec_tmout < 5)
                FATAL("Dangerously low value of -t");

            if (suffix == '+')
                timeout_given = 2;
            else
                timeout_given = 1;

            break;
        }

        case 'm': { /* mem limit */

            u8 suffix = 'M';

            if (mem_limit_given)
                FATAL("Multiple -m options not supported");
            mem_limit_given = 1;

            if (!strcmp(optarg, "none")) {

                mem_limit = 0;
                break;
            }

            if (sscanf(optarg, "%llu%c", &mem_limit, &suffix) < 1 || optarg[0] == '-')
                FATAL("Bad syntax used for -m");

            switch (suffix) {

            case 'T':
                mem_limit *= 1024 * 1024;
                break;
            case 'G':
                mem_limit *= 1024;
                break;
            case 'k':
                mem_limit /= 1024;
                break;
            case 'M':
                break;
            default:
                FATAL("Unsupported suffix or bad syntax for -m");
            }

            if (mem_limit < 5)
                FATAL("Dangerously low value of -m");

            if (sizeof(rlim_t) == 4 && mem_limit > 2000)
                FATAL("Value of -m out of range on 32-bit systems");
        }

        break;

        case 'F': { /* coverage feedback */
            string arg = string(optarg);
            if (arg == "code_coverage") {
                cout << "\033[1;31m Warning: Using pure code coverage as feedback. "
                        "\033[0m \n\n\n";
                feedback_mode = FUZZING_DROP_ALL;
            } else if (arg == "drop_all") {
                cout << "\033[1;31m Warning: Ignoring feedbacks. Drop all mutated "
                        "queries. \033[0m \n\n\n";
                feedback_mode = FUZZING_DROP_ALL;
            } else if (arg == "random_save") {
                cout << "\033[1;31m Warning: Ignoring feedbacks. Randomly saved "
                        "mutated queries. \033[0m \n\n\n";
                feedback_mode = FUZZING_RANDOM_SAVE;
            } else if (arg == "save_all") {
                cout << "\033[1;31m Warning: Ignoring feedbacks. Save all mutated "
                        "queries. \033[0m \n\n\n";
                feedback_mode = FUZZING_SAVE_ALL;
            } else {
                FATAL("Error: Ignoring feedbacks parameters not recognized. \n");
            }
        } break;

        case 'E': {
            mab_epsilon = stod(optarg);
            cout << "\033[1;31m Warning: Using custom MAB epsilon value: " << mab_epsilon
                 << ". \033[0m \n\n\n";
        } break;

        case 'X': {
            disable_dyn_instan = true;
            cout << "\033[1;31m Warning: Disabling query dynamic instantiation based "
                    "on the query error messages. "
                    "\033[0m \n\n\n";
        } break;

        case 'G': {
            disable_rsg_feedback = true;
            cout << "\033[1;31m Warning: Disabling coverage feedback for the RSG module. "
                    "\033[0m \n\n\n";
        } break;

        case 'R': {
            disable_rsg_generator = true;
            cout << "\033[1;31m Warning: Disabling RSG (Random Statement Generator). "
                    "\033[0m \n\n\n";
        } break;

        case 'd': /* skip deterministic */

            if (skip_deterministic)
                FATAL("Multiple -d options not supported");
            skip_deterministic = 1;
            use_splicing = 1;
            break;

        case 'D': /* dump squirrel libraries */

            dump_library = 1;
            break;

        case 'c': /* bind to specific CPU core num */
            bind_to_core_id = atoi(optarg);
            break;

        case 'P': /* Accessing cockroach using port num.  */
            bind_to_port = atoi(optarg);
            break;

        case 'K':
        {
            string arg = string(optarg);
            socket_path = arg;
        }
        break;

        case 's':
            shm_size = atoi(optarg);
            break;

        case 'B': /* load bitmap */

            /* This is a secret undocumented option! It is useful if you find
       an interesting test case during a normal fuzzing process, and want
       to mutate it without rediscovering any of the test cases already
       found during an earlier run.

       To use this mode, you need to point -B to the fuzz_bitmap produced
       by an earlier run for the exact same binary... and that's it.

       I only used this once or twice to get variants of a particular
       file, so I'm not making this an official setting. */

            if (in_bitmap)
                FATAL("Multiple -B options not supported");

            in_bitmap = optarg;
            break;

        case 'C': /* crash mode */

            if (crash_mode)
                FATAL("Multiple -C options not supported");
            crash_mode = FAULT_CRASH;
            break;

        case 'n': /* dumb mode */

            if (dumb_mode)
                FATAL("Multiple -n options not supported");
            if (getenv("AFL_DUMB_FORKSRV"))
                dumb_mode = 2;
            else
                dumb_mode = 1;

            break;

        case 'T': /* banner */

            if (use_banner)
                FATAL("Multiple -T options not supported");
            use_banner = optarg;
            break;

        case 'Q': /* QEMU mode */

            if (qemu_mode)
                FATAL("Multiple -Q options not supported");
            qemu_mode = 1;

            if (!mem_limit_given)
                mem_limit = MEM_LIMIT_QEMU;

            break;

        default:

            usage(argv[0]);
        }

    /* Finish setup RSG; */
    string grammar_str = read_file_to_str(FuzzerConfigurations::grammar_file_path);
    if (grammar_str.empty()) {
        cerr << "Error: Cannot read grammar_str from file: " << FuzzerConfigurations::grammar_file_path << "\n\n\n";
        abort();
    }

#if defined(cockroachdb)
    p_ir_wrapper = new CockroachDBIRWrapper();
    p_query_importer = new QueryImporter();
    rsg = get_new_rsg(grammar_str, "cockroachdb", "stmt_without_legacy_transaction", FuzzerConfigurations::epsilon, FuzzingMode::FuzzingModeNormal,
        p_query_importer,
        cockroachdb_comp_expr_filter,
        cockroachdb_keyword_handl, remove_unimpl_cockroachdb,
        cockroachdb_comp_rule_terminator,
        cockroachdb_ir_context_setup,
        p_ir_wrapper);
    p_instantiator = new CockroachDBQueryInstantiator();
    p_result_handl = new CockroachDBResultHandler();
    p_query_plan_handl = new CockroachDBQueryPlanHandl();
    p_dbms_connector = new CockroachDBConnector(p_result_handl);
    p_feedback_mapper = new FeedbackMapper(
        rsg, p_ir_wrapper, p_dbms_connector, p_instantiator); // use the default one.
    p_init_query_sequence_gen = new CockroachDBQuerySequenceGenerator();
    dbms_name = "cockroachdb";
    /* CockroachDB does not use shared memory to communicate between DB server and this fuzzer.
     * The code coverage is written to the file system, and then read by the fuzzer.
     * Not ideal, but suffice for the current testing.
     */
#elif defined(duckdb)
    p_ir_wrapper = new DuckDBIRWrapper();
    p_query_importer = new QueryImporter();
    rsg = get_new_rsg(grammar_str, "duckdb", "stmt", FuzzerConfigurations::epsilon, FuzzingMode::FuzzingModeNormal,
        p_query_importer,
        duckdb_comp_expr_filter,
        duckdb_keyword_handl, remove_unimpl_duckdb,
        duckdb_comp_rule_terminator,
        duckdb_ir_context_setup,
        p_ir_wrapper);
    p_instantiator = new DuckDBQueryInstantiator();
    p_result_handl = new DuckDBResultHandler();
    p_query_plan_handl = new DuckDBQueryPlanHandl();
    p_dbms_connector = new DuckDBConnector(p_result_handl);
    p_feedback_mapper = new FeedbackMapper(
        rsg, p_ir_wrapper, p_dbms_connector, p_instantiator); // use the default one.
    p_init_query_sequence_gen = new DuckDBQuerySequenceGenerator();
    p_dbms_connector->setup_actual_shm(); // Use the shared memory to communicate the code coverage between duckdb
                                          // and this fuzzer. This is the preferred and traditional way.

    dbms_name = "duckdb";
#elif defined(sqlite)
    p_ir_wrapper = new SQLiteIRWrapper();
    p_query_importer = new QueryImporter();
    rsg = get_new_rsg(grammar_str, "sqlite", "cmd", FuzzerConfigurations::epsilon, FuzzingMode::FuzzingModeNormal,
        p_query_importer,
        sqlite_comp_expr_filter,
        sqlite_keyword_handl, remove_unimpl_sqlite,
        sqlite_comp_rule_terminator,
        sqlite_ir_context_setup,
        p_ir_wrapper);
    p_instantiator = new SQLiteQueryInstantiator();
    p_result_handl = new SQLiteResultHandler();
    p_query_plan_handl = new SQLiteQueryPlanHandl();
    p_dbms_connector = new SQLiteConnector(p_result_handl);
    p_feedback_mapper = new FeedbackMapper(
        rsg, p_ir_wrapper, p_dbms_connector, p_instantiator); // use the default one.
    p_init_query_sequence_gen = new SQLiteQuerySequenceGenerator();
    p_dbms_connector->setup_actual_shm(); // Use the shared memory to communicate the code coverage between sqlite
                                          // and this fuzzer. This is the preferred and traditional way.

    dbms_name = "sqlite";

#elif defined(mysqldb)
    p_ir_wrapper = new MySQLIRWrapper();
    p_query_importer = new QueryImporter();
    rsg = get_new_rsg(grammar_str, "mysql", "simple_statement", FuzzerConfigurations::epsilon, FuzzingMode::FuzzingModeNormal,
        p_query_importer,
        mysql_comp_expr_filter,
        mysql_keyword_handl, remove_unimpl_mysql,
        mysql_comp_rule_terminator,
        mysql_ir_context_setup,
        p_ir_wrapper);
    p_instantiator = new MySQLQueryInstantiator();
    p_result_handl = new MySQLResultHandler();
    p_query_plan_handl = new MySQLQueryPlanHandl();
    p_dbms_connector = new MySQLConnector(p_result_handl);
    p_feedback_mapper = new FeedbackMapper(
        rsg, p_ir_wrapper, p_dbms_connector, p_instantiator); // use the default one.
    p_init_query_sequence_gen = new MySQLQuerySequenceGenerator();
    p_dbms_connector->set_socket_path(socket_path);
    p_dbms_connector->set_bind_to_port(bind_to_port);
    p_dbms_connector->set_shm_size(shm_size);
    p_dbms_connector->setup_actual_shm(); // Use the shared memory to communicate the code coverage between sqlite
                                          // and this fuzzer. This is the preferred and traditional way.
    dbms_name = "mysql";
    mysql_library_init(0, nullptr, nullptr); // Used by the MySQL linked library. 
#elif defined(mariadb)
    p_ir_wrapper = new MariaDBIRWrapper();
    p_query_importer = new QueryImporter();
    rsg = get_new_rsg(grammar_str, "mariadb", "verb_clause", FuzzerConfigurations::epsilon, FuzzingMode::FuzzingModeNormal,
        p_query_importer,
        mariadb_comp_expr_filter,
        mariadb_keyword_handl, remove_unimpl_mariadb,
        mariadb_comp_rule_terminator,
        mariadb_ir_context_setup,
        p_ir_wrapper);
    p_instantiator = new MariaDBQueryInstantiator();
    p_result_handl = new MariaDBResultHandler();
    p_query_plan_handl = new MariaDBQueryPlanHandl();
    p_dbms_connector = new MariaDBConnector(p_result_handl);
    p_feedback_mapper = new FeedbackMapper(
        rsg, p_ir_wrapper, p_dbms_connector, p_instantiator); // use the default one.
    p_init_query_sequence_gen = new MariaDBQuerySequenceGenerator();
    p_dbms_connector->set_socket_path(socket_path);
    p_dbms_connector->set_bind_to_port(bind_to_port);
    p_dbms_connector->set_shm_size(shm_size);
    p_dbms_connector->setup_actual_shm(); // Use the shared memory to communicate the code coverage between sqlite
                                          // and this fuzzer. This is the preferred and traditional way.
    dbms_name = "mariadb";
    mysql_library_init(0, nullptr, nullptr); // MariaDB uses the same library as MySQL. Used by the MySQL linked library. 
#elif defined(postgresql)
    p_ir_wrapper = new PostgreSQLIRWrapper();
    p_query_importer = new QueryImporter();
    rsg = get_new_rsg(grammar_str, "postgresql", "stmt", FuzzerConfigurations::epsilon, FuzzingMode::FuzzingModeNormal,
        p_query_importer,
        postgresql_comp_expr_filter,
        postgresql_keyword_handl, remove_unimpl_postgresql,
        postgresql_comp_rule_terminator,
        postgresql_ir_context_setup,
        p_ir_wrapper);
    p_instantiator = new PostgreSQLQueryInstantiator();
    p_result_handl = new PostgreSQLResultHandler();
    p_query_plan_handl = new PostgreSQLQueryPlanHandl();
    p_dbms_connector = new PostgreSQLConnector(p_result_handl);
    p_feedback_mapper = new FeedbackMapper(
        rsg, p_ir_wrapper, p_dbms_connector, p_instantiator); // use the default one.
    p_init_query_sequence_gen = new PostgreSQLQuerySequenceGenerator();
    p_dbms_connector->set_socket_path(socket_path);
    p_dbms_connector->set_bind_to_port(bind_to_port);
    p_dbms_connector->set_shm_size(shm_size);
    p_dbms_connector->setup_actual_shm(); // Use the shared memory to communicate the code coverage between sqlite
                                          // and this fuzzer. This is the preferred and traditional way.
    dbms_name = "postgresql";
#else
#error "Error: Cannot recognize dbms_name: none of cockroachdb, duckdb or sqlite. "
#endif
    p_dbms_connector->read_bitmap((u8*)in_bitmap);
    p_dbms_connector->set_mem_limit(mem_limit);
    p_dbms_connector->set_doc_path(doc_path);
    p_dbms_connector->set_exec_tmout(tmp_exec_tmout);
    p_dbms_connector->set_dump_library(dump_library);
    p_instantiator->p_rsg = rsg;
    p_fuzzing_sequence_queue = new FuzzingSequenceQueue();
    rsg->cache_reverse_tree();

    // Added new feature.
    rsg->is_use_additive_mutation = true;

    if (optind == argc || !in_dir || !out_dir)
        usage(argv[0]);

    setup_signal_handlers();
    check_asan_opts();

    if (sync_id)
        fix_up_sync();

    if (!strcmp(in_dir, out_dir))
        FATAL("Input and output directories can't be the same");

    if (dumb_mode) {

        if (crash_mode)
            FATAL("-C and -n are mutually exclusive");
        if (qemu_mode)
            FATAL("-Q and -n are mutually exclusive");
    }

    if (getenv("AFL_NO_CPU_RED"))
        no_cpu_meter_red = 1;
    if (getenv("AFL_NO_ARITH"))
        no_arith = 1;
    if (getenv("AFL_SHUFFLE_QUEUE"))
        shuffle_queue = 1;
    if (getenv("AFL_FAST_CAL"))
        fast_cal = 1;

    if (getenv("AFL_HANG_TMOUT")) {
        p_dbms_connector->set_hang_tmout(atoi(getenv("AFL_HANG_TMOUT")));
        if (!p_dbms_connector->get_hang_tmout())
            FATAL("Invalid value of AFL_HANG_TMOUT");
    }

    if (dumb_mode == 2 && no_forkserver)
        FATAL("AFL_DUMB_FORKSRV and AFL_NO_FORKSRV are mutually exclusive");

    if (getenv("AFL_PRELOAD")) {
        setenv("LD_PRELOAD", getenv("AFL_PRELOAD"), 1);
        setenv("DYLD_INSERT_LIBRARIES", getenv("AFL_PRELOAD"), 1);
    }

    if (getenv("AFL_LD_PRELOAD"))
        FATAL("Use AFL_PRELOAD instead of AFL_LD_PRELOAD");

    save_cmdline(argc, argv);

    fix_up_banner(argv[optind]);

    check_if_tty();

    get_core_count();

#ifdef HAVE_AFFINITY
    bind_to_free_cpu(bind_to_core_id);
#endif /* HAVE_AFFINITY */

    p_dbms_connector->set_bind_to_core_id(bind_to_core_id);

    check_crash_handling();
    check_cpu_governor();

    setup_post();

    setup_dirs_fds();
    read_testcases();
    load_auto();

    if (extras_dir)
        load_extras(extras_dir);

    if (!timeout_given) {
        tmp_exec_tmout = find_timeout();
        if (tmp_exec_tmout != 0) {
            p_dbms_connector->set_exec_tmout(tmp_exec_tmout);
        }
    }

    detect_file_args(argv + optind + 1);

    if (!out_file)
        setup_stdio_file();

    // check_binary(argv[optind]);

    start_time = get_cur_time();

    if (qemu_mode)
        use_argv = get_qemu_argv(argv[0], argv + optind, argc - optind);
    else
        use_argv = argv + optind;

    u64 start_time = get_cur_time();
    p_instantiator->init_data_library();
    cerr << "init_data_library() takes "
         << (get_cur_time() - start_time) / 1000 << " seconds\n";

    show_init_stats();

    write_stats_file(0, 0, 0);
    save_auto();

    if (stop_soon)
        goto stop_fuzzing;

    /* Woop woop woop */

    if (!not_on_tty) {
        sleep(4);
        start_time += 4000;
        if (stop_soon)
            goto stop_fuzzing;
    }

    init_bug_output_dir();

    while (1) {

        u8 skipped_fuzz;

        if (!queue_cur) {

            queue_cycle++;
            current_entry = 0;
            cur_skipped_paths = 0;
            queue_cur = queue;

            while (seek_to) {
                current_entry++;
                seek_to--;
                queue_cur = queue_cur->next;
            }

            show_stats();

            if (not_on_tty) {
                ACTF("Entering queue cycle %llu.", queue_cycle);
                fflush(stdout);
            }

            /* If we had a full queue cycle with no new finds, try
         recombination strategies next. */

            if (queued_paths == prev_queued) {

                if (use_splicing)
                    cycles_wo_finds++;
                else
                    use_splicing = 1;
            } else
                cycles_wo_finds = 0;

            prev_queued = queued_paths;

            if (sync_id && queue_cycle == 1 && getenv("AFL_IMPORT_FIRST"))
                sync_fuzzers(use_argv);
        }

        skipped_fuzz = fuzz_one(use_argv);

        if (!stop_soon && sync_id && !skipped_fuzz) {

            if (!(sync_interval_cnt++ % SYNC_INTERVAL))
                sync_fuzzers(use_argv);
        }

        if (!stop_soon && exit_1) {
            stop_soon = 2;
            p_dbms_connector->set_stop_soon(2);
        }

        if (stop_soon)
            break;

        queue_cur = queue_cur->next;
        current_entry++;
    }

    if (queue_cur)
        show_stats();

    /* If we stopped programmatically, we kill the forkserver and the current
     runner. If we stopped manually, this is done by the signal handler. */
    if (stop_soon == 2) {
        if (p_dbms_connector->get_forksrv_pid() > 0) {
            if (p_dbms_connector->get_child_pid() != -1) {
                kill(p_dbms_connector->get_child_pid(), SIGKILL);
                int status;
                waitpid(p_dbms_connector->get_child_pid(), &status, 0);
            }
            if (p_dbms_connector->get_forksrv_pid() != -1) {
                kill(p_dbms_connector->get_forksrv_pid(), SIGKILL);
                int status;
                waitpid(p_dbms_connector->get_forksrv_pid(), &status, 0);
            }
            p_dbms_connector->set_child_pid(-1);
            p_dbms_connector->set_forksrv_pid(-1);
        }
    }
    /* Now that we've killed the forkserver, we wait for it to be able to get
     * rusage stats. */
    if (waitpid(p_dbms_connector->get_forksrv_pid(), NULL, 0) <= 0) {
        WARNF("error waitpid\n");
    }

    p_dbms_connector->write_bitmap(out_dir);
    write_stats_file(0, 0, 0);
    save_auto();

stop_fuzzing:

    SAYF(CURSOR_SHOW cLRD "\n\n+++ Testing aborted %s +++\n" cRST,
        stop_soon == 2 ? "programmatically" : "by user");

    fclose(plot_file);
    p_dbms_connector->set_p_plot_file(nullptr);
    destroy_queue();
    destroy_extras();

    delete p_instantiator;
    delete p_result_handl;
    delete p_query_plan_handl;
    delete p_dbms_connector;
    delete rsg;
    delete p_feedback_mapper;
    delete p_ir_wrapper;
    delete p_init_query_sequence_gen;
    delete p_fuzzing_sequence_queue;
    ck_free(target_path);
    ck_free(sync_id);

    alloc_report();

    OKF("We're done here. Have a nice day!\n");

    exit(0);
}

#endif /* !AFL_LIB */
