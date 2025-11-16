//
// Created by XXX on 2/26/24.
//

#ifndef RSG_CPP_LEX_H
#define RSG_CPP_LEX_H

#include "node.h"
#include <string>
#include <utility>

using std::string;
constexpr int eof = -1;

enum LexTokenItemTyp {
    itemError, // error occurred; value is text of error
    itemEOF,
    itemComment,
    itemPct,
    itemDoublePct,
    itemIdent,
    itemColon,
    itemLiteral,
    itemExpr,
    itemPipe,
    itemNL,
    itemSemicolon,
    itemAssign,
    itemTerm,
    itemNop // No operation for this read. Continue to the next.
};

struct LexTokenItem {
    enum LexTokenItemTyp typ;
    int pos;
    string val;

    string to_string()
    {
        switch (this->typ) {
        case (itemEOF): {
            return "ESCAPE_EOF";
        };
        case (itemError): {
            return this->val;
        }
        default: {
            return this->val;
        }
        }
    }

    LexTokenItem(enum LexTokenItemTyp typ_in, int pos_in, string val_in)
        : typ(typ_in)
        , pos(pos_in)
        , val(std::move(val_in)) {};
    LexTokenItem()
        : typ(itemNop)
        , pos(0)
        , val("") {};
};

// Ahead declaration for recursive reference.
class Lexer;
void* lex_start_lemon(Lexer*);
void* lex_start(Lexer*);

class Lexer {
private:
    string name; // the name of the input; used only for error reports
    string input; // the string being scanned
    void* (*state_func)(Lexer*); // the current lexing function to enter. Return the parsed lex token item.
    void* (*next_state_func)(Lexer*); // the next lexing function to enter. The same as state_func and setup inside the state_func.
    int pos; // current position in the input
    int start; // start position of this item. Used for emit.
    //    int width; // width of last rune read from input. Do not consider unicode, always 1.
    int lastPos; // position of most recent item returned by nextItem
    struct LexTokenItem cur_token_item; // current scanned items

public:
    Lexer(const string& name_in, const string& input_in)
        : name(name_in)
        , input(input_in)
        , pos(0)
        , start(0)
        , lastPos(0)
    {
        if (name == "sqlite") {
            this->state_func = lex_start_lemon;
            this->next_state_func = lex_start_lemon; // placeholder
        } else {
            this->state_func = lex_start;
            this->next_state_func = lex_start; // placeholder
        }
    }

    void set_cur_token_item(LexTokenItem in) { this->cur_token_item = in; }
    const string get_lexer_name() const { return this->name; }

    LexTokenItem run_step(); // Main workflow entry. Run step.
    const string next(); // next returns the next token in the input.
    const string peek(); // peek returns but does not consume the next token in the input./
    void backup(); // backup steps back one token. Can only be called once per call of next.
    LexTokenItem emit(enum LexTokenItemTyp t);
    void ignore();
    struct LexTokenItem nextItem();

    // Debug logging related.
    int get_cur_line_number();
    void lex_error(string error_str); // Will directly abort.
};

#endif // RSG_CPP_LEX_H
