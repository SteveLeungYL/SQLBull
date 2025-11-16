//
// Created by XXX on 3/22/24.
//

#ifndef RSG_CPP_RESULTS_HANDLER_H
#define RSG_CPP_RESULTS_HANDLER_H

#include "ir.h"
#include <set>
#include <string>
#include <vector>

using namespace std;

enum ResultType {
    ResultNormal = 0,
    ResultInteresting = 1,
    ResultError = 2,
    ResultAllError = 2, // same as ResultError. Used for whole query sequence results.
    ResultInternalError = 3,
    ResultCrash = 4
};

class ResultHandler {

protected:
    // helper functions
    virtual bool is_res_error(const string&);
    // DBMS returns unexpected errors.
    virtual bool is_dbms_internal_error(const string&);

    // query plan feedback related.
    set<uint64_t> trig_query_plan;

public:
    // main entry
    // does not handle ResultCrash in this function.
    virtual ResultType check_results(string&);

    // getter
    [[nodiscard]] virtual vector<ResultType> get_v_results() const
    {
        return this->v_results;
    }
    [[nodiscard]] virtual vector<IR*> get_v_stmt_ir() const
    {
        return this->v_stmt_ir;
    }
    [[nodiscard]] virtual vector<string> get_v_stmt_str() const
    {
        return this->v_stmt_str;
    }

    [[nodiscard]] virtual vector<string> get_v_results_str() const
    {
        return this->v_results_str;
    }

    [[nodiscard]] virtual ResultType get_final_res_flag() const
    {
        return this->final_res;
    }

    [[nodiscard]] virtual vector<string> get_v_query_plan() const
    {
        return this->v_query_plan;
    }

    [[nodiscard]] virtual uint64_t get_query_plan_hash() const
    {
        return this->query_plan_hash;
    }

    [[nodiscard]] virtual string get_tmp_cur_res() const
    {
        return this->tmp_cur_res;
    }

    // setter
    virtual void set_v_stmt_str(vector<string>& v_stmt_str_in)
    {
        this->v_stmt_str = v_stmt_str_in;
    }
    virtual void set_v_stmt_ir(vector<IR*>& v_stmt_ir_in)
    {
        this->v_stmt_ir = v_stmt_ir_in;
    }
    virtual void set_v_result_str(vector<string>& v_res_str_in)
    {
        this->v_results_str = v_res_str_in;
    }

    virtual void set_v_query_plan(vector<string>& v_in)
    {
        this->v_query_plan = v_in;
    }

    virtual void set_query_plan_hash(uint64_t in)
    {
        this->query_plan_hash = in;
    }

    virtual void set_final_res_flag(ResultType final_flag_in)
    {
        this->final_res = final_flag_in;
    }

    virtual void set_tmp_cur_res(const string& res_in)
    {
        this->tmp_cur_res = res_in;
    }

    virtual void clear_results()
    {
        this->optimizations.clear();
        this->v_results.clear();
        this->v_stmt_ir.clear(); // not freed!
        this->v_stmt_str.clear();
        this->v_query_plan.clear();
        this->query_plan_hash = 0;
    }

    virtual bool is_res_empty()
    {
        if (this->v_results.empty()) {
            return true;
        } else {
            return false;
        }
    }

    virtual int has_new_query_plan()
    {
        if (this->query_plan_hash == 0) {
            return false;
        } else {
            if (this->trig_query_plan.count(this->query_plan_hash)) {
                return true;
            } else {
                return false;
            }
        }
    }

    virtual ~ResultHandler() = default;

private:
    // TODO::FIXME:: the API could be changed later.
    vector<string> optimizations;
    vector<ResultType> v_results;
    vector<string> v_results_str;
    vector<IR*> v_stmt_ir;
    vector<string> v_stmt_str;
    vector<string> v_query_plan;
    uint64_t query_plan_hash;
    ResultType final_res;

    string tmp_cur_res; // used for dbms_connector.
};

#endif // RSG_CPP_RESULTS_HANDLER_H
