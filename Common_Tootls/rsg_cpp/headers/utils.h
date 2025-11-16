#ifndef __UTILS_H__
#define __UTILS_H__

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <dirent.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

using namespace std;

#define vector_rand_ele_safe(a) \
    (a.size() != 0 ? a[get_rand_int(a.size())] : gen_id_name())

#define vector_rand_ele(a) (a[get_rand_int(a.size())])

static std::random_device rdd; // random device engine, usually based on
// /dev/random on UNIX-like systems
// initialize Mersennes' twister using rdd to generate the seed
static std::mt19937 rng { rdd() };

// #define get_rand_int(range) rand() % (range)
inline int get_rand_int(const int range)
{
    if (range != 0) {
        std::uniform_int_distribution<int> uid(0, range - 1);
        return uid(rng);
    } else
        return 0;
}
inline int get_rand_int(const int start, const int end)
{
    int range = end - start;
    if (range > 0) {
        std::uniform_int_distribution<int> uid(0, range - 1);
        int res = uid(rng);
        res += start;
        return res;
    } else {
        return 0;
    }
}

static inline bool get_pct_hit(const int pct) {
    return get_rand_int(100) < pct;
}

inline long long get_rand_long_long(long long range)
{

    if (range > 0) {
        std::uniform_int_distribution<long long> uid(0, range - 1);
        return uid(rng);
    } else {
        return 0;
    }
}

inline long long get_rand_long_long(long long start, long long end)
{
    long long range = end - start;
    if (range > 0) {
        std::uniform_int_distribution<long long> uid(0, range - 1);
        long long res = uid(rng);
        res += start;
        return res;
    } else {
        return 0;
    }
}

inline float get_rand_float(float min, float max)
{
    if ((max - min) < 0) {
        return 0.0;
    } else if ((max - min) == 0) {
        return min;
    }
    int rand_int = get_rand_int(RAND_MAX);
    return ((max - min) * ((float)rand_int / RAND_MAX)) + min;
}
inline float get_rand_float(float max) { return get_rand_float(0, max); }

inline double get_rand_double(double min, double max)
{
    if ((max - min) < 0) {
        return 0.0;
    } else if ((max - min) == 0) {
        return min;
    }
    int rand_int = get_rand_int(RAND_MAX);
    return ((max - min) * ((double)rand_int / RAND_MAX)) + min;
}
inline double get_rand_double(double max) { return get_rand_double(0, max); }

uint64_t fuzzing_hash(const void* key, int len);
void trim_string(string&);
std::string trim(const std::string& s);
vector<string> get_all_files_in_dir(const char* dir_name);
string magic_string_generator(string& s);
void ensure_semicolon_at_query_end(string&);
std::vector<string> string_splitter(const string& input_string,
    const char delimiter_re);
std::vector<string> string_splitter(string input_string,
    string delimiter_re);
bool is_str_empty(string input_str);

string str_toupper(string str_in);
string str_tolower(string str_in);

string::const_iterator findStringIter(const std::string& strHaystack,
    const std::string& strNeedle);
bool findStringIn(const std::string& strHaystack, const std::string& strNeedle);

string gen_string();

double gen_float();

long gen_long();

int gen_int();

inline bool is_digits(string str)
{
    if (str == "NaN")
        return true;
    return str.find_first_not_of("0123456789. ") == std::string::npos;
}

static inline std::string replace_str(std::string str, const std::string& substr1, const std::string& substr2)
{
    for (size_t index = str.find(substr1, 0); index != std::string::npos && substr1.length(); index = str.find(substr1, index + substr2.length())) {
        str.replace(index, substr1.length(), substr2);
    }
    return str;
}

static inline std::string read_file_to_str(const string& in)
{
    string res_str;
    if (filesystem::exists(in)) {
        ifstream res_in(in, ios::in);
        if (res_in) {
            // get length of file:
            res_in.seekg(0, res_in.end);
            int length = res_in.tellg();
            res_in.seekg(0, res_in.beg);

            char* tmp_res = new char[length];
            res_in.read(tmp_res, length);
            res_str = string(tmp_res, length);
            delete[] tmp_res;
        }
        res_in.close();
    } else {
        cerr << "Error: file: " << in << " not existed. string read_file_to_str()\n\n\n";
        abort();
    }
    return res_str;
}

#include "types.h"

#if defined(__x86_64__) || defined(__arm64__) || defined(__aarch64__)

#define ROL64(_x, _r) ((((u64)(_x)) << (_r)) | (((u64)(_x)) >> (64 - (_r))))

static inline u32 hash32(const void* key, u32 len, u32 seed)
{

    const u64* data = (u64*)key;
    u64 h1 = seed ^ len;

    len >>= 3;

    while (len--) {

        u64 k1 = *data++;

        k1 *= 0x87c37b91114253d5ULL;
        k1 = ROL64(k1, 31);
        k1 *= 0x4cf5ad432745937fULL;

        h1 ^= k1;
        h1 = ROL64(h1, 27);
        h1 = h1 * 5 + 0x52dce729;
    }

    h1 ^= h1 >> 33;
    h1 *= 0xff51afd7ed558ccdULL;
    h1 ^= h1 >> 33;
    h1 *= 0xc4ceb9fe1a85ec53ULL;
    h1 ^= h1 >> 33;

    return h1;
}

#else

#define ROL32(_x, _r) ((((u32)(_x)) << (_r)) | (((u32)(_x)) >> (32 - (_r))))

static inline u32 hash32(const void* key, u32 len, u32 seed)
{

    const u32* data = (u32*)key;
    u32 h1 = seed ^ len;

    len >>= 2;

    while (len--) {

        u32 k1 = *data++;

        k1 *= 0xcc9e2d51;
        k1 = ROL32(k1, 15);
        k1 *= 0x1b873593;

        h1 ^= k1;
        h1 = ROL32(h1, 13);
        h1 = h1 * 5 + 0xe6546b64;
    }

    h1 ^= h1 >> 16;
    h1 *= 0x85ebca6b;
    h1 ^= h1 >> 13;
    h1 *= 0xc2b2ae35;
    h1 ^= h1 >> 16;

    return h1;
}

#endif /* ^__x86_64__ */

void process_mem_usage(double& vm_usage, double& resident_set);

int max(int a, int b);
unsigned int max(unsigned int a, unsigned int b);

#endif // __UTILS_H__
