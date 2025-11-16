//
// Created by XXX on 2/26/24.
//

// This file hold the grammar expression structures from the RSG module.

#ifndef RSG_CPP_NODE_H
#define RSG_CPP_NODE_H

#include "ir.h"
#include "ir_wrapper.h"
#include <cassert>
#include <string>
#include <utility>
#include <vector>

using std::string, std::vector;

enum TokenNodeTyp {
    TypNonTermKeyword = 0,
    TypTermKeyword = 1,
    TypLiteral = 2
};

// ExpressionTyp is defined in ir_wrapper.h

class ProductionNode;

// Represent one token. Has copy constructor.
class TokenNode {
public:
    string str_value;
    enum TokenNodeTyp item_typ;
    IR *cached_ir;
    vector<ProductionNode *> mapped_children_prods;
    ExpressionNode *parent_expression_node = nullptr;

    TokenNode(string str_in, enum TokenNodeTyp typ_in)
        : str_value(std::move(str_in))
          , item_typ(typ_in)
          , cached_ir(nullptr) {
    };

    TokenNode(string str_in, enum TokenNodeTyp typ_in, IR *ir_in)
        : str_value(std::move(str_in))
          , item_typ(typ_in)
          , cached_ir(ir_in) {
    };

    TokenNode(const TokenNode &in)
        : str_value(in.get_string())
          , item_typ(in.get_type())
          , cached_ir(in.get_cached_ir()) // no deep_copy!
    {
    };

    [[nodiscard]] TokenNodeTyp get_type() const { return this->item_typ; }
    [[nodiscard]] string get_string() const { return this->str_value; }
    [[nodiscard]] IR *get_cached_ir() const { return this->cached_ir; }

    [[nodiscard]] vector<ProductionNode *> get_mapped_children_prods() const {
        assert(this->item_typ == TypNonTermKeyword);
        return this->mapped_children_prods;
    }

    [[nodiscard]] string print_info() const {
        string res;
        res += to_string(this->item_typ) + " " + this->get_string();
        if (cached_ir) {
            res += " " + this->get_cached_ir()->to_string();
        }
        return res;
    }

    [[nodiscard]] ExpressionNode *get_parent_expression_node() const {
        return this->parent_expression_node;
    }

    void set_type(TokenNodeTyp in) { this->item_typ = in; }
    void set_string(string in) { this->str_value = in; }
    void set_cached_ir(IR *in) { this->cached_ir = in; }

    void set_mapped_children_prods(const vector<ProductionNode *> &in) {
        if (in.empty()) {
            // If empty. Do not override TypNonTermKeyword.
            return;
        }
        this->mapped_children_prods = in;
        this->item_typ = TypNonTermKeyword;
    }

    void set_parent_expression_node(ExpressionNode *in) {
        this->parent_expression_node = in;
    }
};

// ExpressionNode hold a single grammar expression, consisted of multiple grammar keywords/tokens.
// no copy constructor. Use new delete to manage the memory.
class ExpressionNode {
public:
    int pos;
    vector<TokenNode *> tokens;
    ProductionNode *parent_production_node = nullptr;
    string command; // Parsing related.
    int HitCount = 0; // For calculating rewards. And for debugging.
    double reward_score = 0.0; // Rewards for the MAB algorithm.
    unsigned int unique_hash = 0; // Unique hash, used to calculate grammar edge information.
    ExpressionTyp expression_type = ExpTypUnknown; // Expression complexity type.

    explicit ExpressionNode(int pos_in)
        : pos(pos_in) {
    };

    explicit ExpressionNode(int pos_in, ExpressionTyp typ_in)
        : pos(pos_in),
        expression_type(typ_in) {
    };

    ~ExpressionNode() {
        for (TokenNode *cur_token: this->tokens) {
            delete cur_token;
        }
    }

    [[nodiscard]] ExpressionNode* deep_copy() const {
        ExpressionNode* res_expr = new ExpressionNode(this->get_pos());
        vector<TokenNode*> res_v_token_;
        for (auto* token: this->tokens) {
            res_v_token_.push_back(new TokenNode(*token));
        }
        res_expr->set_token(res_v_token_);
        return res_expr;
    }

    [[nodiscard]] string to_string() const {
        string res;
        int idx = 0;
        for (TokenNode *cur_token: this->tokens) {
            if (idx++ > 0) {
                res += " ";
            }
            res += cur_token->get_string();
        }
        return res;
    }

    [[nodiscard]] int get_pos() const {
        return this->pos;
    }

    [[nodiscard]] vector<TokenNode *> get_tokens() const {
        return this->tokens;
    }

    [[nodiscard]] string get_command() const {
        return this->command;
    }

    [[nodiscard]] int get_hit_count() const {
        return this->HitCount;
    }

    [[nodiscard]] double get_reward_score() const {
        return this->reward_score;
    }

    [[nodiscard]] unsigned int get_unique_hash() const {
        return this->unique_hash;
    }

    [[nodiscard]] ExpressionTyp get_expression_type() const {
        return this->expression_type;
    }

    [[nodiscard]] ProductionNode *get_parent_production_node() const {
        return this->parent_production_node;
    }

    // setter
    void set_pos(int pos_in) {
        this->pos = pos_in;
    }

    void set_token(vector<TokenNode *> &token_in) {
        this->tokens = token_in;
    }

    void set_command(const string &in) {
        this->command = in;
    }

    void set_hit_count(int in) {
        this->HitCount = in;
    }

    void set_reward_score(double in) {
        this->reward_score = in;
    }

    void set_unique_hash(unsigned int hash_in) {
        this->unique_hash = hash_in;
    }

    void set_expression_type(ExpressionTyp in) {
        this->expression_type = in;
    }

    void set_parent_production_node(ProductionNode *in) {
        this->parent_production_node = in;
    }
};

// ProductionNode holds is a named production/non-terminal keyword of multiple expressions.
class ProductionNode {
public:
    int pos;
    bool is_additive_mutation_preferred = false;
    string name;
    vector<ExpressionNode *> all_exprs;
    vector<pair<ExpressionNode *, int> > v_parent_expression_nodes;
    // If referenced in other syntax expression, saved here.
    IR *cached_ir; // the cached IRs are all SymbolNonTerm IRs.
    // but no children/subtree are constructed yet.
    // shared with TokenNode.cached_ir.

    // rule categorization.
    // prioritize from top to button if we need to force terminate the grammar-based generation.
    vector<ExpressionNode *> term_exprs; // the rule would DEFINITELY terminate in x depth.
    vector<ExpressionNode *> norm_preferred_exprs; // the expression contains a path that would lead to termination.
    vector<ExpressionNode *> norm_exprs; // we don't know whether these grammar rule would lead to termination or not
    vector<ExpressionNode *> non_recur_comp_exprs; // complex rules. Defined by users. Non-recursive rules.
    vector<ExpressionNode *> recur_comp_exprs;
    // recursive complex rules. The expression references the top keyword, forming a loop.

    vector<ExpressionNode *> comp_exprs;
    // for debugging purpose. Contains both non_recur_comp_exprs and recur_comp_exprs.

    explicit ProductionNode(int pos_in, string name_in)
        : pos(pos_in)
          , name(std::move(name_in))
          , cached_ir(nullptr) {
    };

    ~ProductionNode() {
        // leave cached_ir to be freed by the RSG.
        for (int i = 0; i < this->all_exprs.size(); i++) {
            // Make sure all ExpressionNode are come from new allocator!
            delete this->all_exprs[i];
        }
    };

    [[nodiscard]] int get_pos() const {
        return this->pos;
    }

    [[nodiscard]] string get_name() const {
        return this->name;
    }

    [[nodiscard]] vector<ExpressionNode *> get_exprs() const {
        return this->all_exprs;
    }

    [[nodiscard]] bool get_is_categorized() const {
        return (!this->term_exprs.empty())
               || (!this->norm_exprs.empty())
               || (!this->norm_preferred_exprs.empty())
               || (!this->comp_exprs.empty())
               || (!this->recur_comp_exprs.empty())
               || (!this->non_recur_comp_exprs.empty());
    }

    [[nodiscard]] IR *get_cached_ir() const {
        return this->cached_ir;
    }

    [[nodiscard]] vector<ExpressionNode *> get_term_exprs() const {
        return this->term_exprs;
    }

    [[nodiscard]] vector<ExpressionNode *> get_norm_preferred_exprs() const {
        return this->norm_preferred_exprs;
    }

    [[nodiscard]] vector<ExpressionNode *> get_norm_exprs() const {
        return this->norm_exprs;
    }

    [[nodiscard]] vector<ExpressionNode *> get_non_recur_comp_exprs() const {
        return this->non_recur_comp_exprs;
    }

    [[nodiscard]] vector<ExpressionNode *> get_recur_comp_exprs() const {
        return this->recur_comp_exprs;
    }

    [[nodiscard]] vector<ExpressionNode *> get_comp_exprs() const {
        return this->comp_exprs;
    }

    [[nodiscard]] vector<pair<ExpressionNode *, int> > get_parent_exprs() const {
        return this->v_parent_expression_nodes;
    }

    [[nodiscard]] int get_expr_idx(ExpressionNode *expr_in) const {
        for (int idx = 0; idx < this->all_exprs.size(); idx++) {
            if (this->all_exprs[idx] == expr_in) {
                return idx;
            }
        }

        // Not found.
        return -1;
    }

    void set_pos(int in) { this->pos = in; }
    void set_name(string &in) { this->name = in; }
    void set_exprs(vector<ExpressionNode *> &in) { this->all_exprs = in; }
    void set_cached_ir(IR *in) { this->cached_ir = in; }

    void set_v_parent_expression_nodes(const vector<pair<ExpressionNode *, int> > &in) {
        this->v_parent_expression_nodes = in;
    }

    void append_v_parent_expression_nodes(ExpressionNode *&in, int token_idx) {
        this->v_parent_expression_nodes.push_back(make_pair(in, token_idx));
    }
};

ProductionNode* get_new_production_node(int pos, const string name);
ExpressionNode* get_new_expression(int pos);
TokenNode* get_new_token_node(const string str, TokenNodeTyp typ);

// Dump and easy to use pre-init nodes.

// This is used when we explicitly modified (e.g., force terminate the syntax tree generation),
// the dump expression is ExpTypTerm so the additive mutation can pick this up later.
extern ExpressionNode dump_simple_expr_node;

#endif // RSG_CPP_NODE_H
