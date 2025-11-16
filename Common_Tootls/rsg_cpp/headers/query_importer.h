//
// Created by XXX on 5/17/24.
//

#ifndef RSG_CPP_QUERY_IMPORTER_H
#define RSG_CPP_QUERY_IMPORTER_H

#include <map>

#include "fuzzer_configurations.h"
#include "node.h"
#include "utils.h"

using namespace std;

class RSG;

class QueryImporter {
public:
    // helper function
    vector<IR*> parse_grammar_dump(const string in);

    // main entry
    vector<IR*> import_grammar(const string file_name);

    void set_rsg(RSG* in) { this->rsg = in; }

private:
    IR* parse_one_stmt(vector<string>);

    // walk the grammar based on pre-parsed query string.
    // v_pair is keyword string to exercised rule index.
    vector<IR*> generate_ir_vec_from_expr(ExpressionNode* cur_expr, vector<pair<string, pair<int, string>>>& v_pair, unsigned long cur_idx);
    IR* generate_ir_from_prod(ProductionNode* cur_prod, vector<pair<string, pair<int, string>>>& v_pair, unsigned long cur_idx);

    int find_mapped_v_pair_idx(const vector<pair<string, pair<int, string>>>& v_pair, string, int, bool);

    // dependent data struct
    RSG* rsg;
};

#endif // RSG_CPP_QUERY_IMPORTER_H
