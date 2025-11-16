//
// Created by XXX on 2/29/24.
//

#include "../headers/parse.h"
#include <iostream>

using std::cerr;

void Tree::start_parse(const string& text_in)
{
    this->lex = new Lexer(this->dbms_name, text_in);
    this->text = text_in;
}
void Tree::stop_parse()
{
    if (this->lex) {
        delete (this->lex);
        this->lex = nullptr;
    }
    this->text.clear();
}

int Tree::parse(const string& text_in)
{
    this->start_parse(text_in);
    this->text = text_in;
    if (this->dbms_name == "sqlite") {
        this->parse_lemon_internal();
    } else {
        this->parse_internal();
    }
    this->stop_parse();
    return 0;
}

LexTokenItem Tree::next()
{
    if (this->peek_count > 0) {
        this->peek_count--;
    } else {
        this->token[0] = this->lex->nextItem();
    }
    return this->token[this->peek_count];
}

void Tree::backup()
{
    this->peek_count++;
}

LexTokenItem Tree::peek()
{
    if (this->peek_count > 0) {
        return this->token[this->peek_count - 1];
    }
    this->peek_count = 1;
    this->token[0] = this->lex->nextItem();
    return this->token[0];
}

void Tree::parse_error(const std::string& error_str)
{
    // Encounter error, directly abort.
    cerr << "Getting parsing error: " << error_str << "\nFrom line: " << this->lex->get_cur_line_number() << "\nAbort. \n";
    abort();
}

LexTokenItem Tree::expect(LexTokenItemTyp itemTyp, const string& context)
{
    LexTokenItem cur_token = this->next();
    if (cur_token.typ != itemTyp) {
        this->unexpected(cur_token, context);
    }
    return cur_token;
}

void Tree::unexpected(LexTokenItem cur_token, const std::string& context)
{
    const string err_msg = "Unexpected token: " + cur_token.to_string() + " in " + context;
    // This function will terminate the program execution.
    this->parse_error(err_msg);
}

void Tree::parse_internal()
{
    for (;;) {
        LexTokenItem cur_token = this->next();
        if (cur_token.typ == itemIdent) {
            ProductionNode* p = get_new_production_node(cur_token.pos, cur_token.val);
            this->parse_production(p);
            this->productions.push_back(p);
        } else if (cur_token.typ == itemEOF) {
            return;
        }
    }
}

void Tree::parse_lemon_internal()
{
    bool is_comment = false;
    for (;;) {
        LexTokenItem cur_token = this->next();
        switch (cur_token.typ) {
        case itemNL: {
            is_comment = false;
            // break the switch
            break;
        }
        case itemComment: {
            is_comment = true;
            // break the switch
            break;
        }
        case itemIdent: {
            if (is_comment) {
                // break the switch
                break;
            }
            ProductionNode* p = nullptr;
            if (this->m_prods.count(cur_token.val)) {
                p = this->m_prods[cur_token.val];
            } else {
                p = get_new_production_node(cur_token.pos, cur_token.val);
            }
            this->parse_production_lemon(p);
            if (!(this->m_prods.count(cur_token.val))) {
                this->productions.push_back(p);
                this->m_prods[cur_token.val] = p;
            }
            break;
        }
        case itemEOF: {
            return;
        }
        default: {
            // Do nothing. Ignored.
        }
        }
    }
}

void Tree::parse_production(ProductionNode* p)
{
    const string context = "production";
    this->expect(itemColon, context);
    if (this->peek().typ == itemNL) {
        this->next();
    }
    bool expect_expr = true;

    for (;;) {
        const LexTokenItem cur_token = this->next();
        switch (cur_token.typ) {
        case (itemComment):
            [[fallthrough]];
        case (itemNL):
            // Ignore the current token. Continue to read the next.
            break;
        case (itemPipe): {
            if (expect_expr) {
                ExpressionNode* e = get_new_expression(cur_token.pos);
                p->all_exprs.push_back(e);
            }
            expect_expr = true;
            break;
        }
        default: {
            this->backup();
            if (!expect_expr) {
                return;
            }
            ExpressionNode* e = get_new_expression(cur_token.pos);
            this->parse_expression(e);
            p->all_exprs.push_back(e);
            expect_expr = false;
        }
        }
    }
}

void Tree::parse_production_lemon(ProductionNode* p)
{
    const string context = "production";
    this->expect(itemAssign, context);
    if (this->peek().typ == itemNL) {
        this->next();
    }
    bool expect_expr = true;

    for (;;) {
        const LexTokenItem cur_token = this->next();
        switch (cur_token.typ) {
        case (itemComment):
            [[fallthrough]];
        case (itemNL):
            // Ignore the current token. Continue to read the next.
            // For the lemon rules, every grammar rule is in one line.
            // No comments are in the way.
            return;
        case (itemPipe): {
            // TODO: This rule doesn't seem to be handled correctly.
            // FIXME: SHOULD FIX LATER.
            if (expect_expr) {
                this->unexpected(cur_token, context);
            }
            expect_expr = true;
            break;
        }
        default: {
            this->backup();
            if (!expect_expr) {
                return;
            }
            vector<ExpressionNode*> v_e = this->parse_expression_lemon(cur_token.pos);
            for (auto* e: v_e) {
                p->all_exprs.push_back(e);
            }
            expect_expr = false;
        }
        }
    }
}

void Tree::parse_expression(ExpressionNode* e)
{
    const string context = "expression";
    for (;;) {
        const LexTokenItem cur_token = this->next();
        switch (cur_token.typ) {
        case (itemPipe): {
            this->backup();
            return;
        }
        case (itemSemicolon): {
            return;
        }
        case (itemNL): {
            LexTokenItemTyp peek_type = this->peek().typ;
            if (peek_type == itemPipe || peek_type == itemNL || peek_type == itemSemicolon) {
                return;
            }
            break;
        }
        case (itemIdent): {
            e->tokens.push_back(get_new_token_node(cur_token.val, TypTermKeyword));
            break;
        }
        case (itemLiteral): {
            e->tokens.push_back(get_new_token_node(cur_token.val, TypLiteral));
            break;
        }
        case (itemExpr): {
            e->command = cur_token.val;
            if (this->peek().typ == itemNL) {
                this->next();
            }
            break;
        }
        case (itemPct):
            [[fallthrough]];
        case (itemComment):
            break;
        default: {
            this->unexpected(cur_token, context);
        }
        }
    }
}

vector<ExpressionNode*> Tree::parse_expression_lemon(int pos)
{
    const string context = "expression";
    vector<ExpressionNode*> v_e;
    v_e.push_back(get_new_expression(pos));

    for (;;) {
        const LexTokenItem cur_token = this->next();
        switch (cur_token.typ) {
        case (itemNL): {
            // all expressions are in one line
            // as formatted by our generate_ir_types_sqlite.py.
            // Return empty new expression.
            return v_e;
        }
        case (itemIdent): {
            for (auto* e : v_e) {
                e->tokens.push_back(get_new_token_node(cur_token.val, TypTermKeyword));
            }
            break;
        }
        case (itemLiteral): {
            for (auto* e : v_e) {
                e->tokens.push_back(get_new_token_node(cur_token.val, TypLiteral));
            }

            break;
        }
        case (itemExpr): {
            for (auto* e : v_e) {
                e->command = cur_token.val;
            }
            if (this->peek().typ == itemNL) {
                this->next();
            }
            return v_e;
        }
        case (itemComment):
            break;
        case (itemTerm): {
            // if encounter te termination period, ignore all other text until the end of line.
            for (LexTokenItem next_token = this->next(); next_token.typ != itemNL && next_token.typ != itemEOF; next_token = this->next()) { }
            // Backup the last new line of EOF token.
            this->backup();
            return v_e;
        }
        case (itemPipe): {
            ExpressionNode* new_expr_node = v_e.back()->deep_copy();

            // If encounter itemPipe, meaning that there is one token that have already
            // been duplicate pushed to both previous and this new expression. Remove it.
            TokenNode* node_to_rov = new_expr_node->tokens.back();
            new_expr_node->tokens.pop_back();
            delete node_to_rov;

            const LexTokenItem pipe_next_token = this->next();

            if (pipe_next_token.typ == itemIdent) {
                new_expr_node->tokens.push_back(get_new_token_node(pipe_next_token.val, TypTermKeyword));
            } else if (pipe_next_token.typ == itemLiteral) {
                new_expr_node->tokens.push_back(get_new_token_node(pipe_next_token.val, TypLiteral));
            }

            v_e.push_back(new_expr_node);

            break;
        }
        default: {
            this->unexpected(cur_token, context);
        }
        }
    }
}