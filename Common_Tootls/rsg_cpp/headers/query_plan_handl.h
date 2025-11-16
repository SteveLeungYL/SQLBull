//
// Created by XXX on 3/22/24.
//

#ifndef RSG_CPP_QUERY_PLAN_HANDL_H
#define RSG_CPP_QUERY_PLAN_HANDL_H

#include "utils.h"
#include <string>
#include <vector>

using namespace std;

class QueryPlanHandler {
public:
    virtual vector<string> retrieve_query_plan(const string&, const string&)
    {
        cerr << "Error: Calling the base class version of the retrieve_query_plan. \n\n\n";
        abort();
    }

    virtual uint64_t hash_query_plan(std::vector<string> const& vec_in)
    {

        string res_str;

        for (const string& in : vec_in) {
            res_str += in + ",";
        }

        uint64_t res = fuzzing_hash(res_str.c_str(), res_str.size());

#ifdef DEBUG
        cerr << "Saving query plan string: " << res << ", getting hash: " << res << "\n\n\n";
#endif

        return res;
    }

    virtual ~QueryPlanHandler() = default;
};

#endif // RSG_CPP_QUERY_PLAN_HANDL_H
