//
// Created by XXX on 3/4/24.
//

#ifndef RSG_CPP_DATA_AFFINITY_H
#define RSG_CPP_DATA_AFFINITY_H

#include "ir_types_common.h"
#include <map>
#include <memory>
#include <string>
#include <vector>

using namespace std;

DATAAFFINITYTYPE get_random_affinity_type(bool is_basic_type_only = true, bool is_no_array = false);
string get_random_affinity_type_str(bool is_basic_type_only = true);
string get_random_affinity_type_str_formal(bool is_basic_type_only = true);
string get_affinity_type_str_formal(DATAAFFINITYTYPE);

DATAAFFINITYTYPE get_data_affinity_by_idx(int idx);

class DataAffinity;
DataAffinity get_data_affinity_struct_by_string(string s);

class DataAffinity {

private:
    // Data Structure.
    DATAAFFINITYTYPE data_affinity;
    bool is_range;
    bool is_enum;
    long long int_min;
    long long int_max;
    double float_min;
    double float_max;
    vector<string> v_enum_str;

    /* For various data types inside the Tuple.  */
    vector<shared_ptr<DataAffinity>> v_tuple_types;

    /* Helper functions. */
    [[nodiscard]] bool is_str_collation(const string& str_in);
    [[nodiscard]] string get_rand_collation_str();

    [[nodiscard]] DATAAFFINITYTYPE detect_numerical_type(const string&);
    [[nodiscard]] DATAAFFINITYTYPE detect_string_type(const string&);

    /* Compact data types */
    [[nodiscard]] string mutate_affi_tuple();
    [[nodiscard]] string mutate_affi_array();

    /* Basic data types */
    [[nodiscard]] string mutate_affi_int(); // and also for serial.
    [[nodiscard]] string mutate_affi_oid(); // unsigned oid.
    [[nodiscard]] string mutate_affi_float(); // decimal and float.
    [[nodiscard]] string mutate_affi_collate();
    [[nodiscard]] string mutate_affi_bool();
    [[nodiscard]] string mutate_affi_onoff();
    [[nodiscard]] string mutate_affi_onoffauto();
    [[nodiscard]] string mutate_affi_bit();
    [[nodiscard]] string mutate_affi_byte();
    [[nodiscard]] string mutate_affi_jsonb(bool is_cast = true);
    [[nodiscard]] string mutate_affi_interval(bool is_cast = true);
    [[nodiscard]] string mutate_affi_intervaltz(bool is_cast = true);
    [[nodiscard]] string mutate_affi_date(bool is_cast = true);
    [[nodiscard]] string mutate_affi_timestamp(bool is_cast = true);
    [[nodiscard]] string mutate_affi_timestamptz(bool is_cast = true);
    [[nodiscard]] string mutate_affi_time(bool is_cast = true);
    [[nodiscard]] string mutate_affi_timetz(bool is_cast = true);
    [[nodiscard]] string mutate_affi_uuid(bool is_cast = true);
    [[nodiscard]] string mutate_affi_enum();
    [[nodiscard]] string mutate_affi_inet(bool is_cast = true);
    [[nodiscard]] string mutate_affi_string();

    // TODO
    // Spatial types.
    /* Seems not implemented. */
    //    string mutate_affi_box2d();
    //    string mutate_affi_void();
    //    string mutate_affi_point();
    //    string mutate_affi_linestring();
    //    string mutate_affi_polygon();
    //    string mutate_affi_multipoint();
    //    string mutate_affi_multilinestring();
    //    string mutate_affi_multipolygon();
    //    string mutate_affi_geometrycollection();

    [[nodiscard]] string get_rand_alphabet_num();
    [[nodiscard]] string get_rand_hex_num();
    [[nodiscard]] string add_random_time_zone();
    [[nodiscard]] DATAAFFINITYTYPE transfer_array_to_normal_type(DATAAFFINITYTYPE in_type);

public:
    DataAffinity()
        : data_affinity(AFFIUNKNOWN)
        , is_range(false)
        , is_enum(false)
        , int_min(0)
        , int_max(0)
        , float_min(0.0)
        , float_max(0.0)
    {
        // No need to init v_enum_str;
    }

    DataAffinity(const DATAAFFINITYTYPE type_in)
        : data_affinity(type_in)
        , is_range(false)
        , is_enum(false)
        , int_min(0)
        , int_max(0)
        , float_min(0.0)
        , float_max(0.0)
    {
    }

    // Copy constructor.
    DataAffinity(const DataAffinity& copy_in)
        : data_affinity(copy_in.get_data_affinity())
        , is_range(copy_in.get_is_range())
        , is_enum(copy_in.get_is_enum())
        , int_min(copy_in.get_int_min())
        , int_max(copy_in.get_int_max())
        , float_min(copy_in.get_float_min())
        , float_max(copy_in.get_float_max())
        , v_enum_str(copy_in.get_v_enum_str())
        , v_tuple_types(copy_in.get_v_tuple_types())
    {
    }

    [[nodiscard]] DATAAFFINITYTYPE recognize_data_type(const string& str_in); // Return `this` pointer.

    [[nodiscard]] DATAAFFINITYTYPE get_data_affinity() const { return this->data_affinity; }
    void set_data_affinity(DATAAFFINITYTYPE in) { this->data_affinity = in; }

    void set_is_range(bool in) { this->is_range = in; }
    [[nodiscard]] bool get_is_range() const { return this->is_range; }

    void set_is_enum(bool in) { this->is_enum = in; }
    [[nodiscard]] bool get_is_enum() const { return this->is_enum; }

    void set_v_enum_str(const vector<string>& in) { this->v_enum_str = in; }
    [[nodiscard]] vector<string> get_v_enum_str() const { return this->v_enum_str; }

    [[nodiscard]] vector<shared_ptr<DataAffinity>> get_v_tuple_types() const { return this->v_tuple_types; }

    void set_int_range(long long min, long long max)
    {
        this->int_min = min;
        this->int_max = max;
    }
    [[nodiscard]] long long get_int_max() const { return this->int_max; }
    [[nodiscard]] long long get_int_min() const { return this->int_min; }

    void set_float_range(double min, double max)
    {
        this->float_min = min;
        this->float_max = max;
    }
    [[nodiscard]] double get_float_max() const { return this->float_max; }
    [[nodiscard]] double get_float_min() const { return this->float_min; }

    [[nodiscard]] string get_mutated_literal(DATAAFFINITYTYPE type_in = AFFIUNKNOWN);

    template <typename T>
    void set_range(T min, T max, DATAAFFINITYTYPE data_affi = AFFIINT)
    {
        switch (data_affi) {
        case AFFIINT:
        case AFFIARRAYINT:
        case AFFIOID:
        case AFFIARRAYOID:
        case AFFISERIAL:
        case AFFIARRAYSERIAL:
            set_int_range(min, max);
            return;
        case AFFIDECIMAL:
        case AFFIARRAYDECIMAL:
        case AFFIFLOAT:
        case AFFIARRAYFLOAT:
            set_float_range(min, max);
            return;
        default:
            set_int_range(min, max);
            return;
        }
    }

    void push_new_v_tuple_types(const DataAffinity& data_affi_in)
    {
        this->v_tuple_types.push_back(make_shared<DataAffinity>(DataAffinity(data_affi_in)));
    }

    void clean_up_v_tuple_types()
    {
        this->v_tuple_types.clear();
    }

    [[nodiscard]] unsigned long long calc_hash();
};

struct FunctionSignature {
    DataAffinity return_data_affi = DataAffinity();
    vector<DataAffinity> param_data_affi;
};

#endif // RSG_CPP_DATA_AFFINITY_H
