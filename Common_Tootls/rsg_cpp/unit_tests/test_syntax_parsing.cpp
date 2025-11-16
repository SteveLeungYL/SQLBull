//
// Created by yuliang on 12/9/24.
//

#include "../headers/config.h"
#include "../headers/dbms_connector.h"
#include "../headers/debug.h"
#include "../headers/feedback_mapper.h"
#include "../headers/fuzzer_configurations.h"
#include "../headers/query_importer.h"
#include "../headers/query_instantiator.h"
#include "../headers/query_plan_handl.h"
#include "../headers/results_handler.h"
#include "../headers/rsg.h"
#include "../headers/types.h"
#include "../headers/utils.h"
#include "../headers/parse.h"
#include "../headers/fuzzer_configurations.h"

#include "unistd.h"
#include "pwd.h"

int main(int argc, char** argv)
{
    string dbms_name = getpwuid(getuid())->pw_name; 
    string grammar_str = read_file_to_str(FuzzerConfigurations::grammar_file_path);

    // The entry function for the RSG construction. also set up the rule filtering and classification etc.
    Tree* grammar_tree = new Tree("sql", dbms_name);
    grammar_tree->parse(grammar_str);

    vector<ProductionNode*> parsed_prods = grammar_tree->transfer_parsed_productions();
    delete grammar_tree;

    cerr << "Getting parsed syntax: \n\n";
    for (auto* prod_node: parsed_prods) {
        cerr << "\n\n\nProduction node: " << prod_node->get_name() << "\n";

        for (auto* expr_node: prod_node->get_exprs()) {
            cerr << "Expr: " << expr_node->to_string() << "\n";
        }
    }


    for (auto* prod_node: parsed_prods) {
        delete prod_node;
    }

    cerr << "END\n\n\n";

    return 0;
}