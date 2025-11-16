//
// Created by yuliang on 11/26/24.
//

#ifndef REVERSE_TREE_H
#define REVERSE_TREE_H

#include "node.h"

class ReverseTreeNode {
public:
    ProductionNode *prod_node;
    uint64_t chosen_rule_idx;

    ExpressionNode *expr_node;
    uint64_t node_idx_in_expr;

    ReverseTreeNode(ProductionNode *prod_node_in, uint64_t rule_idx_in, ExpressionNode *expr_node_in,
                    uint64_t expr_idx_in): prod_node(prod_node_in),
                                           chosen_rule_idx(rule_idx_in),
                                           expr_node(expr_node_in),
                                           node_idx_in_expr(expr_idx_in) {
    }

    ReverseTreeNode(const ReverseTreeNode &in) = default;

    [[nodiscard]] ProductionNode* get_prod_node() const {
        return prod_node;
    }

    [[nodiscard]] ExpressionNode* get_expr_node() const {
        return expr_node;
    }

};

class ReverseTree {
public:
    vector<ReverseTreeNode> v_nodes;

    ReverseTree() = default;
    ~ReverseTree() = default;

    ReverseTree(const ReverseTree &in) {
        this->v_nodes.reserve(in.v_nodes.size());
        for (auto &node: in.v_nodes) {
            this->v_nodes.emplace_back(node);
        }
    }

    [[nodiscard]] string debug_print() const {
        string res;
        int idx = 0;
        for (const auto& cur_node: this->v_nodes) {
            res += to_string(this->v_nodes.size() - 1 - idx) + ": " + cur_node.prod_node->get_name() + ", ";
            if (cur_node.expr_node) {
                res += cur_node.expr_node->to_string() + "\n";
            } else {
                res += "NULL\n";
            }
            idx++;
        }
        return res;
    }

    void append_node(const ReverseTreeNode &node_in) {
        this->v_nodes.emplace_back(node_in);
    }

    void append_node(ProductionNode *prod_in, uint64_t rule_idx, ExpressionNode *expr_in, uint64_t expr_idx) {
        this->v_nodes.emplace_back(prod_in, rule_idx, expr_in, expr_idx);
    }

    void reserve(uint64_t num) {
        this->v_nodes.reserve(num);
    }

    [[nodiscard]] uint64_t size() const {
        return this->v_nodes.size();
    }

    void clear() {
        this->v_nodes.clear();
    }

    [[nodiscard]] bool is_empty() const {
        return this->v_nodes.empty();
    }

    [[nodiscard]] ReverseTreeNode get_reverse_node_on_idx(const uint64_t idx) const {
        if (idx >= this->v_nodes.size()) {
            cerr << "Error: idx bigger than this->v_nodes. \n";
            abort();
        }
        return this->v_nodes[idx];
    }
    [[nodiscard]] ProductionNode *get_prod_node_on_idx(const uint64_t idx) const {
        return this->get_reverse_node_on_idx(idx).prod_node;
    }
    [[nodiscard]] uint64_t get_rule_idx_on_tree_idx(const uint64_t idx) const {
        return this->get_reverse_node_on_idx(idx).chosen_rule_idx;
    }
    [[nodiscard]] ExpressionNode *get_expr_node_on_idx(const uint64_t idx) const {
        return this->get_reverse_node_on_idx(idx).expr_node;
    }
    [[nodiscard]] uint64_t get_expr_idx_on_tree_idx(const uint64_t idx) const {
        return this->get_reverse_node_on_idx(idx).node_idx_in_expr;
    }
    [[nodiscard]] IRTYPE get_ir_type_on_idx(const uint64_t idx) const {
        return this->get_prod_node_on_idx(idx)->get_cached_ir()->get_ir_type();
    }

    vector<ReverseTreeNode> get_sub_tree_up_to_idx(uint64_t idx_in) {
        if (idx_in >= this->v_nodes.size()) {
            cerr << "Error: Getting idx_in: " << idx_in << " exceeding reverse tree size: " << v_nodes.size() << ".\n";
            abort();
        }

        vector<ReverseTreeNode> res_tree;
        res_tree.reserve(idx_in + 1);

        for (uint64_t idx = 0; idx < idx_in; idx++) {
            res_tree.emplace_back(this->v_nodes[idx]);
        }

        return res_tree;
    }
};

class ReverseTreeMatch {
public:
    ReverseTree* tree; // Reverse tree that at least one node is matching the given IR type.
    uint64_t matched_tree_idx; // For the matched tree, which idx of the node is matching the given IR type.
    IRTYPE matched_ir_type;

    ReverseTreeMatch(ReverseTree* tree_in, const uint64_t matched_idx_in, const IRTYPE type_in):
        tree(tree_in),
        matched_tree_idx(matched_idx_in),
        matched_ir_type(type_in) {}

    ReverseTreeMatch(const ReverseTreeMatch &in) = default;
    ~ReverseTreeMatch() = default;
};

#endif //REVERSE_TREE_H
