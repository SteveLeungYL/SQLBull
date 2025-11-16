//
// Created by XXX on 3/6/24.
//

#ifndef RSG_CPP_DATA_CUSTOM_TYPE_H
#define RSG_CPP_DATA_CUSTOM_TYPE_H

#include <string>

using std::string;

// Just data declaration. No implementation yet.
// For future usage.
class CustomTokenType {
public:
    virtual unsigned long long calc_hash() = 0;
    virtual void* set_data(void*) = 0;
    virtual void mutate_cur_token() = 0;
    virtual void to_string_helper(string&) = 0;

    // Placeholder to enforce the copy constructor implementation.
    virtual CustomTokenType* deep_copy() = 0;

    virtual bool is_same_with(CustomTokenType*) = 0;

    virtual ~CustomTokenType() = default;
};

#endif // RSG_CPP_DATA_CUSTOM_TYPE_H
