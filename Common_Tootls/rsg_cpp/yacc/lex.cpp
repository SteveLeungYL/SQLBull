//
// Created by XXX on 2/26/24.
//

#include "../headers/lex.h"
#include <algorithm>
#include <cctype>
#include <iostream>
#include <string>

using std::cerr;
using std::string;

void* lex_start(Lexer*);
void* lex_start_lemon(Lexer*);
void* lex_comment(Lexer*);
void* lex_pct(Lexer*);
void* lex_expr(Lexer*);
void* lex_ident(Lexer*);
void* lex_literal(Lexer*);

inline bool is_space(const string& in)
{
    for (const char& c : in) {
        if (c != ' ' && c != '\t') {
            return false;
        }
    }
    return true;
}

inline bool is_ident(const string& in)
{
    for (const char& c : in) {
        if (!std::isalpha(c) && !std::isdigit(c) && c != '_') {
            return false;
        }
    }
    return true;
}

void* lex_start(Lexer* lexer)
{
    // Infinite loop. break by return.
    for (;;) {
        const string r = lexer->next();
        if (r == "ESCAPE_EOF") {
            lexer->set_cur_token_item(lexer->emit(itemEOF));
            return nullptr;
        } else if (r == "/") {
            return (void*)lex_comment;
        } else if (r == "%") {
            return (void*)lex_pct;
        } else if (r == "\n") {
            lexer->set_cur_token_item(lexer->emit(itemNL));
            return (void*)lex_start;
        } else if (r == ":") {
            lexer->set_cur_token_item(lexer->emit(itemColon));
            return (void*)lex_start;
        } else if (r == "|") {
            lexer->set_cur_token_item(lexer->emit(itemPipe));
            return (void*)lex_start;
        } else if (r == "{") {
            return (void*)lex_expr;
        } else if (is_space(r)) {
            // isSpace
            // Directly go to the next lex character.
            lexer->ignore();
            continue;
        } else if (is_ident(r)) {
            return (void*)lex_ident;
        } else if (r == ";") {
            lexer->set_cur_token_item(lexer->emit(itemSemicolon));
            return (void*)lex_start;
        } else if (r == "'") {
            return (void*)lex_literal;
        } else {
            string tmp_err_msg = "Invalid character: " + r;
            // this lex_error function will terminate the program execution.
            lexer->lex_error(tmp_err_msg);
            return nullptr;
        }
    }
}

void* lex_start_lemon(Lexer* lexer)
{
    // Infinite loop. break by return.
    int assign_count = 0;
    for (;;) {
        const string r = lexer->next();
        if (r == "ESCAPE_EOF") {
            lexer->set_cur_token_item(lexer->emit(itemEOF));
            return nullptr;
        } else if (r == "/") {
            return (void*)lex_comment;
        } else if (r == "\n") {
            lexer->set_cur_token_item(lexer->emit(itemNL));
            return (void*)lex_start_lemon;
        } else if (r == ":") {
            // Only valid case: "::=". cooperate with r == "="
            if (assign_count < 2 && assign_count >= 0) {
                assign_count++;
                continue;
            } else {
                string tmp_err_msg = "Invalid character: " + r;
                // this lex_error function will terminate the program execution.
                lexer->lex_error(tmp_err_msg);
                return nullptr;
            }
        } else if (r == "=") {
            // Only valid case: "::=". cooperate with r == ":"
            if (assign_count == 2) {
                lexer->set_cur_token_item(lexer->emit(itemAssign));
                return (void*)lex_start_lemon;
            } else {
                string tmp_err_msg = "Invalid character: " + r;
                // this lex_error function will terminate the program execution.
                lexer->lex_error(tmp_err_msg);
                return nullptr;
            }
        } else if (r == "|") {
            lexer->set_cur_token_item(lexer->emit(itemPipe));
            return (void*)lex_start_lemon;
        } else if (r == ".") {
            lexer->set_cur_token_item(lexer->emit(itemTerm));
            return (void*)lex_start_lemon;
        } else if (is_space(r)) {
            // isSpace
            // Directly go to the next lex character.
            lexer->ignore();
            continue;
        } else if (is_ident(r)) {
            return (void*)lex_ident;
        } else if (r == "'") {
            return (void*)lex_literal;
        } else if (r == "[" || r == "]") {
            // Directly go to the next lex character.
            lexer->ignore();
            continue;
        } else {
            string tmp_err_msg = "Invalid character: " + r;
            // this lex_error function will terminate the program execution.
            lexer->lex_error(tmp_err_msg);
            return nullptr;
        }
    }
}

void* lex_literal(Lexer* lexer)
{
    // Infinite loop. break by return.
    for (;;) {
        const string r = lexer->next();
        if (r == "'") {
            lexer->set_cur_token_item(lexer->emit(itemLiteral));
            if (lexer->get_lexer_name() == "sqlite") {
                return (void*)lex_start_lemon;
            } else {
                return (void*)lex_start;
            }
        } else if (r == "ESCAPE_EOF") {
            string tmp_err_msg = "Literal reaching EOF: " + r;
            // this lex_error function will terminate the program execution.
            lexer->lex_error(tmp_err_msg);
            return nullptr;
        }
        continue; // explicit continue;
    } // infinite for loop while reading the whole literal length.
}

void* lex_expr(Lexer* lexer)
{
    int ct = 1;
    // Infinite loop. break by return.
    // Read the expr. Drop inputs until the end of the grammar rule expression section.
    for (;;) {
        const string r = lexer->next();
        if (r == "{") {
            ct++;
            continue;
        } else if (r == "}") {
            ct--;
            if (ct == 0) {
                lexer->set_cur_token_item(lexer->emit(itemExpr));
                if (lexer->get_lexer_name() == "sqlite") {
                    return (void*)lex_start_lemon;
                } else {
                    return (void*)lex_start;
                }
            }
        }
    }
}

void* lex_comment(Lexer* lexer)
{
    const string r = lexer->next();
    if (r == "/") {
        for (;;) {
            const string rr = lexer->next();
            if (rr == "\n") {
                lexer->backup();
                lexer->set_cur_token_item(lexer->emit(itemComment));
                if (lexer->get_lexer_name() == "sqlite") {
                    return (void*)lex_start_lemon;
                } else {
                    return (void*)lex_start;
                }
            }
        }
    } else if (r == "*") {
        for (;;) {
            const string rr = lexer->next();
            if (rr == "*") {
                if (lexer->peek() == "/") {
                    lexer->next();
                    lexer->set_cur_token_item(lexer->emit(itemComment));
                    if (lexer->get_lexer_name() == "sqlite") {
                        return (void*)lex_start_lemon;
                    } else {
                        return (void*)lex_start;
                    }
                }
            }
        }
    } else {
        string tmp_err_msg = "Invalid character, expected comment: " + r;
        // this lex_error function will terminate the program execution.
        lexer->lex_error(tmp_err_msg);
        return nullptr;
    }
}

void* lex_pct(Lexer* lexer)
{
    const string r = lexer->next();
    if (r == "%") {
        lexer->set_cur_token_item(lexer->emit(itemDoublePct));
        return (void*)lex_start;
    } else if (r == "{") {
        for (;;) {
            const string rr = lexer->next();
            if (r == "%") {
                if (lexer->peek() == "}") {
                    lexer->next();
                    lexer->set_cur_token_item(lexer->emit(itemPct));
                    if (lexer->get_lexer_name() == "sqlite") {
                        return (void*)lex_start_lemon;
                    } else {
                        return (void*)lex_start;
                    }
                }
            } else if (r == "ESCAPE_EOF") {
                string tmp_err_msg = "Invalid character, expected itemPct: " + r;
                // this lex_error function will terminate the program execution.
                lexer->lex_error(tmp_err_msg);
                return nullptr;
            }
        }
    } else if (r == "p") {
        // Dirty implementation.
        if (lexer->next() != "r" || lexer->next() != "e" || lexer->next() != "c" || !is_space(lexer->next())) {
            // this lex_error function will terminate the program execution.
            lexer->lex_error("Expected %prec, but get error token. ");
            return nullptr;
        }
        bool is_inside_quoted_ident = false;
        for (;;) {
            const string rr = lexer->next();
            if (rr == "'") {
                // For handling quoted identifiers, such as: %prec '%'
                is_inside_quoted_ident = !is_inside_quoted_ident;
            } else if (is_inside_quoted_ident) {
                continue;
            } else if (is_ident(rr)) {
                // absorb the whole ident and ignore.
            } else {
                lexer->backup();
                lexer->set_cur_token_item(lexer->emit(itemPct));
                if (lexer->get_lexer_name() == "sqlite") {
                    return (void*)lex_start_lemon;
                } else {
                    return (void*)lex_start;
                }
            }
        }
    } else {
        int ct = 0;
        for (;;) {
            const string rr = lexer->next();
            if (rr == " " || rr == "{") {
                ct++;
            } else if (rr == "}") {
                ct--;
                if (ct == 0) {
                    lexer->set_cur_token_item(lexer->emit(itemPct));
                    if (lexer->get_lexer_name() == "sqlite") {
                        return (void*)lex_start_lemon;
                    } else {
                        return (void*)lex_start;
                    }
                }
            } else if (rr == "\n") {
                if (ct == 0) {
                    lexer->set_cur_token_item(lexer->emit(itemPct));
                    if (lexer->get_lexer_name() == "sqlite") {
                        return (void*)lex_start_lemon;
                    } else {
                        return (void*)lex_start;
                    }
                }
            } else if (rr == "ESCAPE_EOF") {
                // this lex_error function will terminate the program execution.
                lexer->lex_error("Expected itemPct contents, but get EOF. ");
                return nullptr;
            }
        }
    }
}

void* lex_ident(Lexer* lexer)
{
    for (;;) {
        const string r = lexer->next();
        if (is_ident(r)) {
            // absorb
        } else {
            lexer->backup();
            lexer->set_cur_token_item(lexer->emit(itemIdent));
            if (lexer->get_lexer_name() == "sqlite") {
                return (void*)lex_start_lemon;
            } else {
                return (void*)lex_start;
            }
        }
    }
}

const string Lexer::next()
{
    if (this->pos >= this->input.size()) {
        return "ESCAPE_EOF";
    }
    const string ret(1, this->input[pos]);
    ++(this->pos);
    return ret;
}

const string Lexer::peek()
{
    const string ret = this->next();
    this->backup();
    return ret;
}

void Lexer::backup()
{
    this->pos--;
}

void Lexer::ignore()
{
    this->start = this->pos;
}

int Lexer::get_cur_line_number()
{
    string sub_str = this->input.substr(0, this->lastPos);
    return std::count(sub_str.begin(), sub_str.end(), '\n');
}

LexTokenItem Lexer::nextItem()
{
    LexTokenItem res_item = this->run_step();
    this->lastPos = this->pos;
    return res_item;
}

LexTokenItem Lexer::emit(enum LexTokenItemTyp t)
{
    LexTokenItem ret_item = LexTokenItem(t, this->start, this->input.substr(this->start, (this->pos - this->start)));
    this->start = this->pos;
    return ret_item;
}

LexTokenItem Lexer::run_step()
{
    this->cur_token_item = LexTokenItem(); // By default, NOP.
    // Continue running until the returned cur_token_item is no longer the itemNop, stands for No Operation.
    while (this->cur_token_item.typ == itemNop && this->state_func != nullptr) {
        this->next_state_func = (void* (*)(Lexer*))this->state_func(this); // cast back to state function pointer.
        this->state_func = this->next_state_func;
    }
    return this->cur_token_item;
}

void Lexer::lex_error(string error_str)
{
    // Encounter error, directly abort.
    cerr << "Getting Lex tokenizing error: " << error_str << "\nFrom line: " << this->get_cur_line_number() << "\nAbort. \n";
    abort();
}