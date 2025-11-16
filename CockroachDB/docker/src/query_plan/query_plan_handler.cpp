//
// Created by XXX on 2/20/24.
//

#include "query_plan_handler.h"
#include "../include/utils.h"
#include <regex>
#include <fstream>
#include <filesystem>

using namespace std;

static unsigned int cmd_str_out_idx = 0;

static string clean_up_operand_names(const string& cur_line) {
    static regex re("(v\\d+)|(c\\d+)|(i\\d+)|(s\\d+)|(seq\\d+)|"
                    "(view\\d+)|(par\\d+)|(view_c\\d+)|(constraint_\\d+)|(family_\\d+)");
    return regex_replace(cur_line, re, "");
}

#define DEBUG

vector<string> retrieve_query_plan(const string query_in, const string res_in) {
    vector<string> res_out;
    for (string cur_line : string_splitter(res_in, "\n")) {
        if (findStringIn(cur_line, "• ")) {
            // This is a main optimization line
            cur_line = string_splitter(cur_line, "• ").back();
#ifdef DEBUG
            cerr << "Getting triggered optimization: " << cur_line << "\n\n\n\n";
#endif
            res_out.push_back(cur_line);
        }
    }
#ifdef DEBUG
    if (res_out.empty()) {
        return res_out;
    }
    ofstream outputfile("query_plan_optimization.txt", std::ofstream::out | std::ofstream::app);
    outputfile << "[";
    for (const string& cur_res : res_out) {
        outputfile << cur_res << ", ";
    }
    outputfile << "]:./query_plan_cmd/" << to_string(cmd_str_out_idx) << ".sql\n";
    outputfile.close();

    if (!filesystem::exists("./query_plan_cmd/")) {
        filesystem::create_directories("./query_plan_cmd/");
    }
    string cmd_out_file_name = "./query_plan_cmd/" + to_string(cmd_str_out_idx) + ".sql";
    ofstream cmd_out_file(cmd_out_file_name, std::ofstream::out);
    cmd_out_file.write(query_in.c_str(), query_in.size());
    cmd_out_file.close();

    cmd_str_out_idx++;

#endif

    return res_out;
}

vector<string> retrieve_query_plan_opt(const string in) {
    vector<string> res_out;
    bool is_opt_line = false;
    for (string cur_line : string_splitter(in, "\n")) {
//        if (findStringIn(cur_line, "----")) {
//            // This is the beginning of the opt output.
//            is_opt_line = true;
//            continue;
//        } else if (findStringIn(cur_line, "rows)")) {
//            // This is the ending of the opt output.
//            is_opt_line = false;
//            continue;
//        } else if (is_opt_line) {
        if (findStringIn(cur_line, "─ ")) {
            cur_line = string_splitter(cur_line, "─ ").back();
            res_out.push_back(clean_up_operand_names(cur_line));
        }
//    }
    }
#ifdef DEBUG
    if (res_out.empty()) {
        return res_out;
    }
    ofstream outputfile("query_plan_optimization.txt", std::ofstream::out | std::ofstream::app);
    outputfile << "[";
    for (const string& cur_res : res_out) {
        outputfile << cur_res << ", ";
    }
    outputfile << "]\n";
    outputfile.close();
#endif

    return res_out;
}


