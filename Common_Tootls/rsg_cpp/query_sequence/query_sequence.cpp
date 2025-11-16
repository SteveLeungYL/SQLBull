//
// Created by XXX on 9/4/24.
//
#include "../headers/query_sequence.h"
#include "../headers/query_instantiator.h"

[[nodiscard]] QueryStmt*
QuerySequenceGenerator::generate_next_query(RSG* p_rsg,
    FuzzingSequenceQueue* p_queue)
{
    /* Does not save to the v_query_stmts array. */
    // Repeat generating the same statement 10 time max.
    for (int i = 0; i < 10; i++) {
        QueryStmt* res = this->generate_next_query_helper(p_rsg, p_queue);
        if (res == nullptr) {
            continue;
        }
        if (res->gen_method == GenMut || res->gen_method == GenMutFromOtherSequence || this->gen_mode == GenMut) {
            if (this->p_saved_query_sequence != nullptr) {
                // Check whether instan_id reach the maximum number used, if not, update.
                // If yes, update again +100 gap.
                this->set_instan_gen_id(
                    this->p_saved_query_sequence->v_good_query_stmts.back()
                        ->query_instan_data->g_id_counter);
            }
        }
        if (this->gen_mode == GenMut) {
            if (this->p_saved_query_sequence != nullptr && this->p_query_sequence->previous_sequence == nullptr) {
                // Connect the query sequence mutation chain. For debugging purpose.
                this->p_query_sequence->previous_sequence = this->p_saved_query_sequence;
            }
        }
        return res;
    }

    return nullptr;
}

void QueryStmt::set_query_instan_data(QueryInstantiatorData* in)
{
    if (this->query_instan_data != nullptr) {
        delete this->query_instan_data;
    }
    this->query_instan_data = in;
}

void QueryStmt::strip_unnecessary_data() {
    // Call before saving to queue. Strip unnecessary data such as execution results.
    this->res_str.clear();
    this->mutating_irs.clear();
}

string QueryStmt::to_string() const
{
    if (this->query_stmt_type == TYPEIR) {
        return this->stmt_ir->to_string() + ";";
    } else {
        return this->stmt_str;
    }
}

[[nodiscard]] QueryStmt* QueryStmt::deep_copy() const
{
    QueryStmt* res = this->query_stmt_type == TYPEIR ? new QueryStmt(this->stmt_ir->deep_copy()) : new QueryStmt(this->stmt_str);
    res->res_str = this->res_str;
    if (this->query_instan_data != nullptr) {
        res->query_instan_data = this->query_instan_data->deep_copy();
    }
    res->gen_method = this->gen_method;
    res->mutating_irs = {};
    return res;
}

QueryStmt::~QueryStmt()
{
    if (this->query_instan_data != nullptr) {
        delete this->query_instan_data;
    }
    if (this->query_stmt_type == TYPEIR && this->stmt_ir != nullptr) {
        this->stmt_ir->deep_drop();
    }
}

string QuerySequence::get_query_sequence_str_with_results_stmt_by_stmt() const
{
    string res = "Beging debug printing stmt by stmt: \n";
    for (int idx = 0; idx < this->v_all_query_stmts.size(); idx++) {
        res += "Query: " + this->v_all_query_stmts[idx]->to_string() + "\n";
        res += "Result: " + this->v_all_query_stmts[idx]->res_str + "\n";
    }
    res += "End debug printing. \n";
    return res;
}

void QuerySequence::append_good_stmt(QueryStmt* in)
{
    v_good_query_stmts.push_back(in->deep_copy());
    v_all_query_stmts.push_back(in);
    in = nullptr;
}

void QuerySequence::append_error_stmt(QueryStmt* in)
{
    v_all_query_stmts.push_back(in);
    in = nullptr;
}

QuerySequence* QuerySequence::deep_copy()
{
    auto* res = new QuerySequence();
    for (auto& cur_stmt : this->v_good_query_stmts) {
        res->v_good_query_stmts.push_back(cur_stmt->deep_copy());
    }
    for (auto& cur_stmt : this->v_all_query_stmts) {
        res->v_all_query_stmts.push_back(cur_stmt->deep_copy());
    }
    res->previous_sequence = this->previous_sequence;
    return res;
}

void QuerySequence::strip_unnecessary_data()
{
    for (auto& cur_stmt : this->v_good_query_stmts) {
        cur_stmt->strip_unnecessary_data();
    }
    for (auto& cur_stmt : this->v_all_query_stmts) {
        cur_stmt->strip_unnecessary_data();
    }
}

string QuerySequence::get_all_query_stmts_str() const
{
    string res = "";

    for (auto& cur_stmt : this->v_all_query_stmts) {
        res += cur_stmt->to_string();
    }

    return res;
}

string QuerySequence::get_query_sequence_str_with_results() const
{
    string res = "Query: \n";
    for (int idx = 0; idx < this->v_all_query_stmts.size(); idx++) {
        res += this->v_all_query_stmts[idx]->to_string() + "\n";
    }
    res += "\n\n\nResults: \n";
    for (int idx = 0; idx < this->v_all_query_stmts.size(); idx++) {
        res += this->v_all_query_stmts[idx]->res_str + "\n";
    }

    return res;
}

string QuerySequence::get_query_sequence_str_short() const
{
    string res = "Query Short: \n";
    for (int idx = 0; idx < this->v_good_query_stmts.size(); idx++) {
        res += this->v_good_query_stmts[idx]->to_string() + "\n";
    }
    res += "\n\n\n";
    return res;
}

QuerySequence::~QuerySequence()
{
    for (auto& cur_stmt : this->v_good_query_stmts) {
        delete cur_stmt;
    }
    for (auto& cur_stmt : this->v_all_query_stmts) {
        delete cur_stmt;
    }
};

void QuerySequenceGenerator::set_instan_gen_id(unsigned long long in)
{
    // add the gap 100, so the next getting is safe from id collision.
    if (in > this->instan_idx) {
        this->instan_idx = in;
    }
    this->instan_idx = this->instan_idx - (this->instan_idx % 100) + 100;
}

QuerySequenceGenerator* QuerySequenceGenerator::deep_copy_helper(QuerySequenceGenerator* res) const
{
    /* Cannot call deep copy directly because of pure-virtual class. */
    /* Let the derived class handle the deep_copy function. */
    res->gen_mode = this->gen_mode;
    res->instan_idx = this->instan_idx;
    if (res->p_query_sequence != nullptr) {
        delete res->p_query_sequence;
        res->p_query_sequence = nullptr;
    }
    if (res->p_saved_query_sequence != nullptr) {
        delete res->p_saved_query_sequence;
        res->p_saved_query_sequence = nullptr;
    }
    if (this->p_query_sequence != nullptr) {
        res->p_query_sequence = this->p_query_sequence->deep_copy();
    }
    res->p_saved_query_sequence = this->p_saved_query_sequence; // Do not own.
    return res;
}

QuerySequenceGenerator::QuerySequenceGenerator()
{
    this->p_query_sequence = new QuerySequence();
    this->p_saved_query_sequence = nullptr;
};

QuerySequenceGenerator::~QuerySequenceGenerator()
{
    if (this->p_query_sequence != nullptr) {
        delete this->p_query_sequence;
    }
    this->p_saved_query_sequence = nullptr;
}