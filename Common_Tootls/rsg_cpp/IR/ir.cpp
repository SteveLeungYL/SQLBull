//
// Created by XXX on 3/6/24.
//

#include "../headers/ir.h"

#include <string>

#include "../headers/utils.h"

using std::string;

string IR::to_string()
{
    string res;
    this->to_string_helper(res);
    trim_string(res);
    return res;
}

void IR::to_string_helper(std::string& res)
{
#define str_or_custom(v)                 \
    do {                                 \
        if (!(this->str_val_.empty())) { \
            res += this->str_val_;       \
        } else {                         \
            res += std::to_string(v);    \
        }                                \
    } while (0)

    switch (this->symbol_type_) {
    case SymbolLit: {
        switch (this->ir_type_) {
#if defined(sqlite) || defined(mysqldb) || defined(mariadb) || defined(postgresql)
        case IRTypeINTEGER: {
#else
        case IRTypeICONST: {
#endif
            str_or_custom(this->int_val_);
            return;
        }
#if defined(sqlite) || defined(mysqldb) || defined(mariadb) || defined(postgresql)
        case IRTypeFLOAT: {
#else
        case IRTypeFCONST: {
#endif
            str_or_custom(this->float_val_);
            return;
        }
        case IRTypeBOOLEAN: {
            if (!(this->str_val_.empty())) {
                res += this->str_val_;
            } else if (this->bool_val_) {
                res += "TRUE";
            } else {
                res += "FALSE";
            }
            return;
        }
        default: {
            if (!this->str_val_.empty()) {
                res += this->str_val_;
            }
            return;
        }
        }
        break;
    } // case SymbolLit;
    case SymbolIden:
        [[fallthrough]];
    case SymbolTerm: {
        res += this->str_val_;
        return;
    }
    case SymbolCustomTerm: {
        if (this->p_custom_type) {
            this->p_custom_type->to_string_helper(res);
        } else {
            res += this->str_val_;
        }
        return;
    }
    case SymbolNonTerm: {
        for (IR* cur_child : this->children_) {
            res += cur_child->to_string() + " ";
        }
        return;
    }
    default: {
        cerr << "Error: IR::to_string_helper(). Facing unexpected Symbol type "
            "from IR. "
            "Getting "
            << get_string_by_symbol_type(this->symbol_type_) << "\n\n\n";
        abort();
    }
    }
#undef str_or_custom
}

void IR::drop()
{
    // self destruct.
    if (this->p_data_affinity) {
        delete p_data_affinity;
    }
    if (this->p_custom_type) {
        delete p_custom_type;
    }
    delete this;
}

void IR::deep_drop()
{
    for (IR* cur_child : this->children_) {
        cur_child->deep_drop();
    }
    this->drop();
}

IR* IR::deep_copy()
{
    switch (this->symbol_type_) {
    case SymbolLit:
        [[fallthrough]];
    case SymbolCustomTerm:
        [[fallthrough]];
    case SymbolIden:
        [[fallthrough]];
    case SymbolTerm: {
        IR* res_node = new IR(*this);
        res_node->set_parent_node(nullptr);
        res_node->set_is_favor(this->get_is_favor());
        return res_node;
    }
    case SymbolNonTerm: {
        vector<IR*> res_children;
        for (IR*& old_child : this->get_children()) {
            IR* new_child = old_child->deep_copy();
            res_children.push_back(new_child);
        }
        IR* new_node = new IR(*this);
        new_node->set_parent_node(nullptr);
        new_node->set_children_nodes(res_children);
        new_node->set_is_favor(this->get_is_favor());
        return new_node;
    }
    default: {
        cerr << "Error: IR::deep_copy(). Facing unexpected Symbol type from IR. "
                "Getting "
             << get_string_by_symbol_type(this->symbol_type_) << "\n\n\n";
        abort();
    }
    }
}

IR* IR::get_root()
{
    IR* res = this;
    while (res->get_parent_node()) {
        res = res->get_parent_node();
    }
    return res;
}

void IR::free_children()
{
    for (IR*& cur_children : this->children_) {
        cur_children->deep_drop();
    }
    this->children_.clear();
}

void IR::detach_children()
{
    for (IR*& cur_children : this->children_) {
        cur_children->set_parent_node(nullptr);
    }
    this->children_.clear();
}

bool IR::detach_one_child(IR* rov_node)
{
    vector<IR*> res_children;

    bool is_found = false;
    for (IR*& cur_child : this->children_) {
        if (cur_child == rov_node) {
            cur_child->set_parent_node(nullptr);
            is_found = true;
            continue;
        } else {
            res_children.push_back(cur_child);
        }
    }

    if (!is_found) {
        return false;
    }

    this->set_children_nodes(res_children);

    return true;
}

bool IR::add_one_child(IR* app_node, int app_idx)
{
    if (app_idx < 0 || app_idx > this->get_children_ref().size()) {
        return false;
    }
    vector<IR*> res_children;
    vector<IR*> ori_children = this->get_children_ref();
    res_children.reserve(ori_children.size() + 1);
    for (int idx = 0; idx < ori_children.size(); idx++) {
        if (idx == app_idx) {
            res_children.push_back(app_node);
        }
        res_children.push_back(ori_children[idx]);
    }

    if (app_idx == ori_children.size()) {
        // append to the end
        res_children.push_back(app_node);
    }

    // set up the new children and remap their parents.
    this->set_children_nodes(res_children);
    return true;
}

bool IR::swap_one_child(IR* old_node, IR* new_node, bool is_free)
{
    int targeted_idx = -1;
    for (int i = 0; i < this->children_.size(); i++) {
        if (this->children_[i] == old_node) {
            targeted_idx = i;
            break;
        }
    }
    // If not found.
    if (targeted_idx == -1) {
        return false;
    }
    // If found.
    if (is_free) {
        this->children_[targeted_idx]->deep_drop();
    } else {
        this->children_[targeted_idx]->set_parent_node(nullptr);
    }
    this->children_[targeted_idx] = new_node;
    new_node->set_parent_node(this);
    return true;
}

bool IR::is_empty()
{
    switch (this->symbol_type_) {
    case SymbolLit:
        return false;
    case SymbolCustomTerm:
        return false;
    case SymbolIden:
        [[fallthrough]];
    case SymbolTerm: {
        return this->str_val_.empty();
    }
    case SymbolNonTerm: {
        return this->children_.empty();
    }
    default: {
        cerr << "Error: IR::is_empty(). Facing unexpected Symbol type "
            "from IR. "
            "Getting "
            << get_string_by_symbol_type(this->symbol_type_) << "\n\n\n";
        abort();
    }
    }
}

void IR::mutate_literal_helper()
{
    DataAffinity* p = this->get_p_data_affinity();
#ifdef DEBUG
    if (p == nullptr) {
        cerr << "Error: Calling IR::mutate_literal_helper() with IR: " << this->print_info() << ", but without p_data_affinity\n\n\n";
        abort();
    }
#endif
    this->set_str_val(p->get_mutated_literal());
}

void IR::debug(ostream& out, int depth)
{
    if (depth > 0) {
        for (int depth_idx = 0; depth_idx < depth; depth_idx++) {
            out << "--";
        }
    }

    out << this << "," << get_string_by_symbol_type(this->get_symbol_type()) << "," << get_string_by_ir_type(this->get_ir_type())
        << "," << get_string_by_data_type(this->get_data_type()) << ","
        << get_string_by_data_flag(this->get_data_flag()) << ","
        << get_string_by_data_affi(this->get_data_affinity_type())
        << "," << this->get_is_favor()
        << "," << this->get_str_val() << ".\n";

    for (IR* cur_child : this->children_) {
        cur_child->debug(out, depth + 1);
    }
}

uint64_t IR::hash_tree() const
{
    vector<IRTYPE> v_ir_tree_types;
    v_ir_tree_types.reserve(100);

    this->_hash_tree_helper(v_ir_tree_types);
    assert(v_ir_tree_types.size());
    return fuzzing_hash(v_ir_tree_types.data(), (v_ir_tree_types.size() * sizeof(v_ir_tree_types.front())));
}

void IR::_hash_tree_helper(vector<IRTYPE>& v_ir_tree_types) const
{

    if (this->get_is_empty()) {
        // Do not count the empty keywords that doesn't contribute to the query string.
        return;
    }

    v_ir_tree_types.emplace_back(this->get_ir_type());
    for (auto& child : this->get_children()) {
        child->_hash_tree_helper(v_ir_tree_types);
    }
}

bool IR::get_is_empty() const
{

    switch (this->symbol_type_) {
    case SymbolLit: {
        return false;
    }
    case SymbolIden:
        [[fallthrough]];
    case SymbolTerm: {
        if (this->str_val_.empty()) {
            return true;
        } else {
            return false;
        }
    }
    case SymbolCustomTerm: {
        if (this->p_custom_type) {
            return false;
        } else {
            return this->str_val_.empty();
        }
    }
    case SymbolNonTerm: {
        return this->children_.empty();
    }
    default: {
        cerr << "Logic error. Getting default in IR::get_is_empty()?";
        abort();
    }
    }
}