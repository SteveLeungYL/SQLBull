//
// Created by XXX on 2/29/24.
//

#ifndef RSG_CPP_PARSE_H
#define RSG_CPP_PARSE_H

#include "lex.h"

// Parsing tree
class Tree {
private:
    const string name; // name of the template represented by the tree.
    const string dbms_name;
    vector<ProductionNode*> productions; // The current saved grammar rule expressions.
    map<string, ProductionNode*> m_prods; // Used by lemon parser.
    string text; // text parsed to create the template (or its parent)
    // Parsing only, cleared after parse.
    Lexer* lex;
    LexTokenItem token[2]; // one-token lookahead for parser. The second one is just an empty placeholder for debugging purpose.
    int peek_count;

    LexTokenItem next();
    void backup();
    LexTokenItem peek();
    void parse_error(const string&); // will terminate the program.
    LexTokenItem expect(LexTokenItemTyp, const string&);
    void unexpected(LexTokenItem, const string&);
    // Omit the Recover function from the Go implementation.
    void start_parse(const string& text_in);
    void stop_parse();

    void parse_internal(); // helper function for parse.
    void parse_lemon_internal(); // helper function for parse.
    void parse_production(ProductionNode*); // the function that handle one non-terminal token rules parsing.
    void parse_production_lemon(ProductionNode*); // the function that handle one non-terminal token rules parsing.
    void parse_expression(ExpressionNode*); // the function that handle one grammar rules parsing.
    vector<ExpressionNode*> parse_expression_lemon(int pos); // the function that handle one grammar rules parsing.

public:
    int parse(const string&); // Parsing entry.

    Tree(const string name_in, const string dbms_name_in)
        : name(std::move(name_in))
        , dbms_name(std::move(dbms_name_in))
        , lex(nullptr)
        , peek_count(0)
    {
    }
    ~Tree()
    {
        if (this->lex) {
            delete (this->lex);
        }
        for (ProductionNode* cur_prod : this->productions) {
            delete cur_prod;
        }
    }

    vector<ProductionNode*> transfer_parsed_productions()
    {
        // Transfer the parsed grammar rules to external structure.
        // Clear the current parsing context.
        this->text.clear();
        vector<ProductionNode*> ret = this->productions;
        this->productions.clear();
        return std::move(ret);
    }
};

#endif // RSG_CPP_PARSE_H
