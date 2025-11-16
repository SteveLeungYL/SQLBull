//
// Created by XXX on 3/8/24.
//

#ifndef RSG_CPP_IR_WRAPPER_H
#define RSG_CPP_IR_WRAPPER_H

#include "ir.h"
#include <algorithm>
#include <vector>

using namespace std;

/*
 * Functions that need to be implemented in the inherit class:
 * IRTYPE get_cur_stmt_type_from_sub_ir(IR* cur_ir);
 * bool is_in_subquery(IR* cur_stmt, IR* check_node, bool output_debug = false);
 */

enum ExpressionTyp {
    ExpTypUnknown = -1,
    ExpTypTerm = 0,
    ExpTypNormPrefered = 1,
    ExpTypNorm = 2,
    ExpTypNonRecurComp = 3,
    ExpTypRecComp = 4,
    ExpTypComp = 4, // Same as RecComp.
};

class IRWrapper {
public:
    IRWrapper()
        : ir_root(nullptr)
    {
    }
    IRWrapper(IR* root_in)
        : ir_root(root_in)
    {
    }

    virtual ~IRWrapper() = default;

    virtual void set_ir_root(IR* in)
    {
        // Call every time the class pass in a new ir_root.
        // Can be invoked by every modification function with cur_root as
        // the first argument.
        this->ir_root = in;
    }
    IR* get_ir_root() { return this->ir_root; }

    IR* get_first_stmtlist_from_root();
    IR* get_first_stmtlist_from_root(IR* cur_root)
    {
        set_ir_root(cur_root);
        return this->get_first_stmtlist_from_root();
    }

    IR* get_last_stmtlist_from_root();
    IR* get_last_stmtlist_from_root(IR* cur_root)
    {
        set_ir_root(cur_root);
        return this->get_last_stmtlist_from_root();
    }

    IR* get_first_stmt_from_root();
    IR* get_first_stmt_from_root(IR* cur_root)
    {
        set_ir_root(cur_root);
        return this->get_first_stmt_from_root();
    }

    IR* get_last_stmt_from_root();
    IR* get_last_stmt_from_root(IR* cur_root)
    {
        set_ir_root(cur_root);
        return this->get_last_stmt_from_root();
    }

    IR* get_root_from_ir_tree(vector<IR*>& ir_tree)
    {
        return ir_tree.back();
    }

    // gather all the ir nodes in the current ir tree.
    // root is ir_tree.back();
    // ensure the query string sequence order.
    // left node appears before the right node in to_string();
    vector<IR*> get_all_ir_node();
    vector<IR*> get_all_ir_node(IR* cur_ir_root)
    {
        this->set_ir_root(cur_ir_root);
        return this->get_all_ir_node();
    }

    vector<IR*> get_all_non_term_ir_node(const bool is_only_save_favor);
    vector<IR*> get_all_non_term_ir_node(ExpressionTyp max_comp_level = ExpTypComp);
    vector<IR*> get_all_non_term_ir_node(IR* cur_ir_root, const bool is_only_save_favor)
    {
        this->set_ir_root(cur_ir_root);
        return this->get_all_non_term_ir_node(is_only_save_favor);
    }
    vector<IR*> get_all_non_term_ir_node(IR* cur_ir_root, ExpressionTyp max_comp_level = ExpTypComp)
    {
        this->set_ir_root(cur_ir_root);
        return this->get_all_non_term_ir_node(max_comp_level);
    }
    vector<IR*> get_all_non_term_ir_node_breadth_first();
    vector<IR*> get_all_non_term_ir_node_breadth_first(IR* cur_ir_root)
    {
        this->set_ir_root(cur_ir_root);
        return this->get_all_non_term_ir_node_breadth_first();
    }

    virtual IRTYPE get_cur_stmt_type_from_sub_ir(IR* cur_ir) = 0;

    template <typename T>
    bool is_exist_ir_node_in_stmt_with_type(IR* ir_root, T type, int stmt_idx, bool is_subquery = false,
        bool ignore_is_subquery = false)
    {
        this->set_ir_root(ir_root);
        return this->is_exist_ir_node_in_stmt_with_type(type, stmt_idx, is_subquery, ignore_is_subquery);
    }
    template <typename T>
    bool is_exist_ir_node_in_stmt_with_type(T type, int stmt_idx, bool is_subquery = false,
        bool ignore_is_subquery = false);

    template <typename TYPE>
    bool is_exist_ir_node_in_stmt_with_type(IR* cur_stmt, TYPE node_type,
        bool is_subquery = false,
        bool ignore_is_subquery = false);

    template <typename TYPE>
    vector<IR*> get_ir_node_in_stmt_with_type(IR* cur_stmt, TYPE node_type,
        bool is_subquery = false,
        bool ignore_is_subquery = false);

    int get_num_stmt(IR* cur_root)
    {
        this->set_ir_root(cur_root);
        return this->get_num_stmt();
    }
    int get_num_stmt()
    {
        return this->get_stmt_ir_vec().size();
    };

    /* Appending statements.
     * idx = 0 means appending to the very beginning.
     * idx = size(stmt_size) means appending to the very end.
     * */
    bool append_stmt_at_idx(IR* app_stmt, int idx)
    {
        return this->append_stmt_at_idx_helper(app_stmt, idx);
    };
    bool append_stmt_at_idx(IR* cur_root, IR* app_stmt, int idx)
    {
        this->set_ir_root(cur_root);
        return this->append_stmt_at_idx(app_stmt, idx);
    };

    bool append_stmt_at_begin(IR* cur_root, IR* app_stmt)
    {
        this->set_ir_root(cur_root);
        return this->append_stmt_at_begin(app_stmt);
    };
    bool append_stmt_at_begin(IR* app_stmt)
    {
        return this->append_stmt_at_idx(app_stmt, 0);
    };

    bool append_stmt_at_end(IR* cur_root, IR* app_stmt)
    {
        this->set_ir_root(cur_root);
        return this->append_stmt_at_end(app_stmt);
    };
    bool append_stmt_at_end(IR* app_stmt)
    {
        return this->append_stmt_at_idx(app_stmt, this->get_num_stmt());
    };

    bool remove_stmt_and_free(IR* cur_root, IR* rov_stmt)
    {
        this->set_ir_root(cur_root);
        return this->remove_stmt_and_free(rov_stmt);
    }
    bool remove_stmt_and_free(IR* rov_stmt)
    {
        int res_idx = this->get_stmt_idx(rov_stmt);
        return this->remove_stmt_and_free(res_idx);
    };

    bool remove_stmt_and_free(IR* cur_root, int idx)
    {
        this->set_ir_root(cur_root);
        return this->remove_stmt_and_free(idx);
    }
    bool remove_stmt_and_free(int res_idx)
    {
        if (this->get_num_stmt() <= 1) {
            return false;
        }
        if (res_idx >= 0 && res_idx < this->get_num_stmt()) {
            return this->remove_stmt_at_idx_and_free_helper(res_idx);
        }
        return false;
    };

    bool replace_stmt_and_free(IR* cur_root, IR* old_stmt, IR* new_stmt)
    {
        this->set_ir_root(cur_root);
        return this->replace_stmt_and_free(old_stmt, new_stmt);
    }
    bool replace_stmt_and_free(IR* old_stmt, IR* new_stmt)
    {
        int res_idx = this->get_stmt_idx(old_stmt);
        return this->replace_stmt_and_free(res_idx, new_stmt);
    }

    bool replace_stmt_and_free(IR* cur_root, int idx, IR* new_stmt)
    {
        this->set_ir_root(cur_root);
        return this->replace_stmt_and_free(idx, new_stmt);
    }
    bool replace_stmt_and_free(int res_idx, IR* new_stmt)
    {
        if (res_idx >= 0 && res_idx < this->get_num_stmt()) {
            return this->remove_stmt_and_free(res_idx) && this->append_stmt_at_idx(new_stmt, res_idx);
        }
        return false;
    }

    int get_stmt_idx(IR* cur_root, IR* stmt_in)
    {
        this->set_ir_root(cur_root);
        return this->get_stmt_idx(stmt_in);
    }
    int get_stmt_idx(IR* stmt_in)
    {
        vector<IR*> stmt_list = this->get_stmt_ir_vec();
        int res_idx = 0;
        for (IR*& cur_stmt_ir : stmt_list) {
            if (cur_stmt_ir == stmt_in) {
                return res_idx;
            }
            res_idx++;
        }

        return -1; // Return -1 for not found or error.
    }

    IR* get_ir_node_for_stmt_with_idx(IR* cur_root, int idx)
    {
        this->set_ir_root(cur_root);
        return this->get_ir_node_for_stmt_with_idx(idx);
    }
    IR* get_ir_node_for_stmt_with_idx(int res_idx)
    {
        if (res_idx >= 0 && res_idx < this->get_num_stmt()) {
            return this->get_stmt_ir_vec()[res_idx];
        }
        cerr << "Inside IRWrapper::get_ir_node_for_stmt_with_idx, getting stmt_idx: " << res_idx
             << ", which appears to be invalid compared to stmt size: " << this->get_num_stmt() << "\n\n\n";
        abort();
    }

    bool is_ir_before(IR* f, IR* l); // Check is IR f before IR l in query string.
    bool is_ir_after(IR* f, IR* l) // Check is IR f after IR l in query string.
    {
        return !(this->is_ir_before(f, l));
    }

    template <typename TYPE>
    bool is_ir_in(IR*, TYPE);

    vector<IRTYPE> get_all_stmt_ir_type();

    vector<IR*> get_stmt_ir_vec(IR* cur_root)
    {
        this->set_ir_root(cur_root);
        return std::move(this->get_stmt_ir_vec());
    }
    vector<IR*> get_stmt_ir_vec()
    {
        if (this->get_ir_root() == nullptr) {
            cerr << "Getting ir_root == nullptr in IRWrapper. \n\n\n";
            abort();
        }

        IR* stmt_list_ir = this->get_stmtlist_ir();
        return stmt_list_ir->get_children();
    }

    IR* get_stmtlist_ir(IR* ir_root)
    {
        this->set_ir_root(ir_root);
        return std::move(this->get_stmtlist_ir());
    }

    IR* get_stmtlist_ir()
    {
        // Assumption, The IRAllStmtList node is the only child for IRTypeRoot.
        if (this->get_ir_root() == nullptr) {
            cerr << "Getting ir_root == nullptr in IRWrapper. \n\n\n";
            abort();
        }

        if (this->ir_root->get_children().empty()) {
            cerr << "Error: the root does not contain stmtlist node. \n\n\n";
            abort();
        } else if (this->ir_root->get_children().front()->get_ir_type() != IRTypeAllStmtList) {
            cerr << "Error: the root does not contain stmtlist node. \n\n\n";
            abort();
        } else {
            return this->ir_root->get_children().front();
        }
    }

    // No need to setup ir root for this function.
    virtual bool is_in_subquery(IR* cur_stmt, IR* check_node, bool output_debug = false) = 0;

    IRTYPE get_parent_type(IR* cur_node, int depth = 0);

    template <typename T>
    IR* get_ancestor_node_matching_type(IR* cur_IR, T type);

    // Iterate all the child node from the input cur_node. For each child node,
    // call the handler function from its input function pointer.
    typedef void (*handler_t)(IR*, void*, void*);
    void iter_sub_nodes_with_handler(IR* cur_node, void*, void*, handler_t);

    template <typename T, typename U>
    IR* find_closest_nearby_IR_with_type(IR* cur_node, const T mat_type, const U& term_type);
    template <typename T>
    IR* find_closest_nearby_IR_with_type(IR* cur_node, const T mat_type)
    {
        vector<T> dummy_vec;
        return std::move(this->find_closest_nearby_IR_with_type(cur_node, mat_type, dummy_vec));
    }

    virtual bool remove_stmt_at_idx_and_free_helper(unsigned int rov_idx)
    {
        IR* stmtlist_ir = this->get_stmtlist_ir();
        vector<IR*> v_ori_all_stmt = stmtlist_ir->get_children();

        vector<IR*> v_new_res_stmt;
        v_new_res_stmt.reserve(v_ori_all_stmt.size());
        bool is_found = false;
        for (int i = 0; i < v_ori_all_stmt.size(); i++) {
            if (i == rov_idx) {
                v_ori_all_stmt[i]->deep_drop();
                is_found = true;
                continue;
            } else {
                v_new_res_stmt.push_back(v_ori_all_stmt[i]);
            }
        }

        if (!is_found) {
            // the stmt to remove is not found.
            return false;
        }

        stmtlist_ir->set_children_nodes(v_new_res_stmt);
        return true;
    }
    virtual bool append_stmt_at_idx_helper(IR* app_stmt, int in_idx)
    {
        IR* stmtlist_ir = this->get_stmtlist_ir();
        vector<IR*> v_ori_all_stmt = stmtlist_ir->get_children();

        vector<IR*> v_new_res_stmt;
        v_new_res_stmt.reserve(v_ori_all_stmt.size() + 1);
        bool is_inserted = false;

        for (int i = 0; i < v_ori_all_stmt.size(); i++) {
            if (i == in_idx) {
                v_new_res_stmt.push_back(app_stmt);
                is_inserted = true;
                // continue execution.
            }
            v_new_res_stmt.push_back(v_ori_all_stmt[i]);
        }

        if (in_idx == v_ori_all_stmt.size()) {
            is_inserted = true;
            v_new_res_stmt.push_back(app_stmt);
        }

        if (!is_inserted) {
            // the stmt to insert is not correct.
            cerr << "Error: Cannot insert into statement with idx: " << in_idx << " for size: "
                 << this->get_num_stmt() << "\n\n\n";
            abort();
        }

        // connect child and parent.
        stmtlist_ir->set_children_nodes(v_new_res_stmt);
        return true;
    }

    virtual bool is_ir_statement_typed(IRTYPE ir_type) { return false; }

protected: // protected members. accessible in the inherited child class.
    IR* ir_root = nullptr;

    // Helper function for find_closest_nearby_IR_with_type();

    inline bool comp_type(IR* cur_node, IR* comp_node)
    {
        return (
            cur_node->get_id() == comp_node->get_id()
            && cur_node->get_symbol_type() == comp_node->get_symbol_type()
            && cur_node->get_ir_type() == comp_node->get_ir_type()
            && cur_node->get_data_flag() == comp_node->get_data_flag()
            && comp_type(cur_node->get_p_data_affinity(), comp_node->get_p_data_affinity())
            && comp_type(cur_node->get_p_custom_type(), comp_node->get_p_custom_type())
            && cur_node->get_node_fixed() == comp_node->get_node_fixed()
            && cur_node->get_is_instantiated() == comp_node->get_is_instantiated()
            && cur_node->get_str_val() == comp_node->get_str_val());
    }

    inline bool comp_type(DataAffinity* a, DataAffinity* b)
    {
        if (a == nullptr && b == nullptr) {
            return true;
        } else if (a == nullptr || b == nullptr) {
            return false;
        } else {
            // a != nullptr && b != nullptr
            auto a_tuple_types = a->get_v_tuple_types();
            auto b_tuple_types = b->get_v_tuple_types();
            if (a_tuple_types.size() != b_tuple_types.size()) {
                return false;
            }
            for (int idx = 0; idx < a_tuple_types.size(); idx++) {
                if (!(this->comp_type((a_tuple_types[idx]), (b_tuple_types[idx])))) {
                    return false;
                }
            }

            return (
                a->get_data_affinity() == b->get_data_affinity()
                && a->get_is_range() == b->get_is_range()
                && a->get_is_enum() == b->get_is_enum()
                && a->get_int_min() == b->get_int_min()
                && a->get_int_max() == b->get_int_max()
                && a->get_float_min() == b->get_float_min()
                && a->get_float_max() == b->get_float_max()
                && a->get_v_enum_str() == b->get_v_enum_str());
        }
    }

    inline bool comp_type(shared_ptr<DataAffinity> a, shared_ptr<DataAffinity> b)
    {
        if (a == nullptr && b == nullptr) {
            return true;
        } else if (a == nullptr || b == nullptr) {
            return false;
        } else {
            // a != nullptr && b != nullptr
            auto a_tuple_types = a->get_v_tuple_types();
            auto b_tuple_types = b->get_v_tuple_types();
            if (a_tuple_types.size() != b_tuple_types.size()) {
                return false;
            }
            for (int idx = 0; idx < a_tuple_types.size(); idx++) {
                if (!(this->comp_type((a_tuple_types[idx]), (b_tuple_types[idx])))) {
                    return false;
                }
            }

            return (
                a->get_data_affinity() == b->get_data_affinity()
                && a->get_is_range() == b->get_is_range()
                && a->get_is_enum() == b->get_is_enum()
                && a->get_int_min() == b->get_int_min()
                && a->get_int_max() == b->get_int_max()
                && a->get_float_min() == b->get_float_min()
                && a->get_float_max() == b->get_float_max()
                && a->get_v_enum_str() == b->get_v_enum_str());
        }
    }

    inline bool comp_type(CustomTokenType* a, CustomTokenType* b)
    {
        if (a == nullptr && b == nullptr) {
            return true;
        } else if (a == nullptr || b == nullptr) {
            return false;
        } else {
            return a->is_same_with(b);
        }
    }

    inline bool comp_type(IR* cur_node, IRTYPE ir_type)
    {
        if (cur_node->get_ir_type() == ir_type) {
            return true;
        } else {
            return false;
        }
    }
    inline bool comp_type(IR* cur_node, DATATYPE data_type)
    {
        if (cur_node->get_data_type() == data_type) {
            return true;
        } else {
            return false;
        }
    }
    inline bool comp_type(IR* cur_node, string in_str)
    {
        if (cur_node->to_string() == in_str) {
            return true;
        }
        return false;
    }
    inline bool comp_type(IR* cur_node, vector<IRTYPE> v_ir_type)
    {
        for (auto& ir_type : v_ir_type) {
            if (cur_node->get_ir_type() == ir_type) {
                return true;
            }
        }
        return false;
    }
    inline bool comp_type(IR* cur_node, vector<DATATYPE> v_data_type)
    {
        for (auto& data_type : v_data_type) {
            if (cur_node->get_data_type() == data_type) {
                return true;
            }
        }
        return false;
    }
    inline bool comp_type(IR* cur_node, vector<DATAFLAG> v_data_flag)
    {
        for (auto& data_flag : v_data_flag) {
            if (cur_node->get_data_flag() == data_flag) {
                return true;
            }
        }
        return false;
    }
    inline bool comp_type(IR* cur_node, vector<DATAAFFINITYTYPE> v_data_affi)
    {
        for (auto& data_affi : v_data_affi) {
            if (cur_node->get_data_affinity_type() == data_affi) {
                return true;
            }
        }
        return false;
    }
    inline bool comp_type(IR* cur_node, vector<string> v_in_str)
    {
        for (string& in_str : v_in_str) {
            if (cur_node->to_string() == in_str) {
                return true;
            }
        }
        return false;
    }

    template <typename T>
    inline bool comp_type(T a, T b)
    {
        if (a == b) {
            return true;
        } else {
            return false;
        }
    }

    // Helper function.
    // Iterate all the child node from the input cur_node.
    // Find the closest matching node from the start searching point.
    // Used to find the closest matching type node from one node.
    template <typename T, typename U>
    IR* find_closest_nearby_IR_with_type_helper(IR* cur_node, const T& mat_type, const U& term_type, const bool& is_prioritize_left);

    void get_all_ir_node_helper(vector<IR*>&, IR*&);
    void get_all_non_term_ir_node_helper(vector<IR*>&, IR*&, ExpressionTyp max_comp_level, const bool);
    void get_all_non_term_ir_node_breadth_first_helper(vector<IR*>&, IR*&);
};

// template functions that have to be implemented in the header file to avoid linking issues.

template <typename T>
bool IRWrapper::is_exist_ir_node_in_stmt_with_type(T type, int stmt_idx, bool is_subquery,
    bool ignore_is_subquery)
{
#ifdef DEBUG
    if (stmt_idx < 0 || stmt_idx >= this->get_num_stmt()) {
        cerr << "Error: In IRWrapper::is_exist_ir_node_in_stmt_with_type, getting stmt_idx: "
             << stmt_idx << " larger than the total stmt number: " << this->get_num_stmt() << "\n\n\n";
        abort();
    }
#endif
    IR* cur_stmt = this->get_ir_node_for_stmt_with_idx(stmt_idx);
    return this->is_exist_ir_node_in_stmt_with_type(cur_stmt, type, is_subquery, ignore_is_subquery);
}

template <typename TYPE>
bool IRWrapper::is_exist_ir_node_in_stmt_with_type(IR* cur_stmt, TYPE node_type,
    bool is_in_subquery,
    bool ignore_is_subquery)
{
    return !((this->get_ir_node_in_stmt_with_type(cur_stmt, node_type, is_in_subquery, ignore_is_subquery)).empty());
}

template <typename TYPE>
vector<IR*> IRWrapper::get_ir_node_in_stmt_with_type(IR* cur_stmt, TYPE node_type,
    bool is_in_subquery,
    bool ignore_is_subquery)
{
    // get all nodes in the stmt.
    vector<IR*> v_all_nodes;
    this->get_all_ir_node_helper(v_all_nodes, cur_stmt);

    vector<IR*> v_res_node;
    for (IR*& cur_node : v_all_nodes) {
        if (comp_type(cur_node, node_type)) {
            v_res_node.push_back(cur_node);
        }
    }

    if (ignore_is_subquery) {
        return v_res_node;
    }

    // filter by the subquery node option.
    vector<IR*> v_res_filtered_node;
    for (IR* cur_node : v_res_node) {
        if (this->is_in_subquery(cur_stmt, cur_node) == is_in_subquery) {
            v_res_filtered_node.push_back(cur_node);
        }
    }

    return v_res_filtered_node;
}

template <typename T>
IR* IRWrapper::get_ancestor_node_matching_type(IR* cur_IR, T type)
{
    cur_IR = cur_IR->get_parent_node();
    while (cur_IR != nullptr) {
        if (this->comp_type(cur_IR, type)) {
            return cur_IR;
        }
        cur_IR = cur_IR->get_parent_node();
    }
    // Error. Ancestor node not found.
    return nullptr;
}

template <typename T, typename U>
IR* IRWrapper::find_closest_nearby_IR_with_type_helper(IR* cur_node, const T& mat_type, const U& term_type, const bool& is_prioritize_left)
{
    // recursive function.
    if (this->comp_type(cur_node, term_type)) {
        // reach term_type. do not continue searching the sub nodes.
        return nullptr;
    } else if (this->comp_type(cur_node, mat_type)) {
        // successfully matching node, return the matched node.
        return cur_node;
    } else {
        // failed to match. continue searching the sub nodes.
        const vector<IR*> cur_children = cur_node->get_children();
        IR* match_ir = nullptr;
        if (is_prioritize_left) {
            // prioritize leading node.
            for (auto iter = cur_children.begin(); iter != cur_children.end(); iter++) {
                match_ir = this->find_closest_nearby_IR_with_type_helper(*iter, mat_type, term_type, is_prioritize_left);
                if (match_ir != nullptr) {
                    return match_ir;
                }
            }
        } else {
            // prioritize tailing node.
            for (auto iter = cur_children.rbegin(); iter != cur_children.rend(); iter++) { // reverse iterator.
                match_ir = this->find_closest_nearby_IR_with_type_helper(*iter, mat_type, term_type, is_prioritize_left);
                if (match_ir != nullptr) {
                    return match_ir;
                }
            }
        }
    }
    // not found matching node in this subtree. return null.
    return nullptr;
}

template <typename T>
bool IRWrapper::is_ir_in(IR* cur_ir, T typ)
{
    while (cur_ir->get_parent_node() != nullptr) {
        cur_ir = cur_ir->get_parent_node();
        if (comp_type(cur_ir, typ)) {
            return true;
        }
    }

    return false;
}

template <typename T, typename U>
IR* IRWrapper::find_closest_nearby_IR_with_type(IR* cur_node, const T mat_type, const U& term_type)
{
    // find the closest node that matching the given type.
    // only check parents and neighbor nodes, do not check sub nodes.
#ifdef DEBUG
    if (cur_node == nullptr) {
        cerr << "Getting nullptr from cur_node in  IRWrapper::find_closest_nearby_IR_with_type. \n\n\n";
        abort();
    }
#endif

    IR* cur_parent = cur_node->get_parent_node();
    while (cur_parent != nullptr) {
        const vector<IR*> parent_children = cur_parent->get_children();
        auto match_iter = std::find(parent_children.begin(), parent_children.end(), cur_node);

        if (match_iter == parent_children.end()) {
            /* ERROR HANDLING. IR Tree construction error. Cannot find child node in parent node children vector. */
            cerr << "Error: Cannot find child node: " << cur_node->print_info() << " from parent node: " << cur_parent->print_info() << ". \nGetting all child node info: \n";
            for (IR* const cur_child : parent_children) {
                cerr << cur_child->print_info() << ", ";
            }
            cerr << "\n\n\n";
            abort();
        }

        int match_idx = match_iter - parent_children.begin();

        for (int search_idx = 1;
             (
                 match_idx - search_idx >= 0 || match_idx + search_idx < parent_children.size());
             search_idx++) {
            if (match_idx - search_idx >= 0) {
                // searching query left side. searching from right to left for closest.
                IR* match_ir = this->find_closest_nearby_IR_with_type_helper(parent_children[match_idx - search_idx], mat_type, term_type, false);
                if (match_ir != nullptr) {
                    return match_ir;
                }
            }
            if (match_idx + search_idx < parent_children.size()) {
                // searching query right side. searching from left to right for closest.
                IR* match_ir = this->find_closest_nearby_IR_with_type_helper(parent_children[match_idx + search_idx], mat_type, term_type, true);
                if (match_ir != nullptr) {
                    return match_ir;
                }
            }
            // not found. continue further search.
        }

        cur_node = cur_parent;
        cur_parent = cur_parent->get_parent_node();
    }

    return nullptr;
}

#endif // RSG_CPP_IR_WRAPPER_H
