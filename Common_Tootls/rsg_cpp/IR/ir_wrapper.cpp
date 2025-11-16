//
// Created by XXX on 3/11/24.
//

#include "../headers/ir_wrapper.h"
#include "../headers/node.h"

IR* IRWrapper::get_first_stmtlist_from_root()
{
    // Assuming we stmtlist is the direct parent node for stmt.
    vector<IR*> v_stmt = this->get_stmt_ir_vec();
    if (v_stmt.empty()) {
        return nullptr;
    }

    return v_stmt.front()->get_parent_node();
}

IR* IRWrapper::get_last_stmtlist_from_root()
{
    // Assuming we stmtlist is the direct parent node for stmt.
    vector<IR*> v_stmt = this->get_stmt_ir_vec();
    if (v_stmt.empty()) {
        return nullptr;
    }

    return v_stmt.back()->get_parent_node();
}

IR* IRWrapper::get_first_stmt_from_root()
{
    vector<IR*> v_stmt = this->get_stmt_ir_vec();
    if (v_stmt.empty()) {
        return nullptr;
    }

    return v_stmt.front();
}

IR* IRWrapper::get_last_stmt_from_root()
{
    vector<IR*> v_stmt = this->get_stmt_ir_vec();
    if (v_stmt.empty()) {
        return nullptr;
    }

    return v_stmt.back();
}

vector<IR*> IRWrapper::get_all_ir_node()
{
    // Gather all the ir nodes in the current ir tree.
    vector<IR*> v_res;
    this->get_all_ir_node_helper(v_res, this->ir_root);
    return v_res;
}

void IRWrapper::get_all_ir_node_helper(vector<IR*>& v_res, IR*& cur_root)
{
    // Helper to gather all the ir node in the current ir tree.
    for (IR*& cur_child : cur_root->get_children()) {
        this->get_all_ir_node_helper(v_res, cur_child);
    }

    v_res.push_back(cur_root);
    return;
}

vector<IR*> IRWrapper::get_all_non_term_ir_node(const bool is_only_save_favor)
{
    // Gather all the ir nodes in the current ir tree.
    vector<IR*> v_res;
    this->get_all_non_term_ir_node_helper(v_res, this->ir_root, ExpTypComp, is_only_save_favor); // Max complexity is ExpTypComp, meaning we don't care about complexity here.
    return v_res;
}

vector<IR*> IRWrapper::get_all_non_term_ir_node(ExpressionTyp max_comp_level)
{
    // Gather all the ir nodes in the current ir tree.
    vector<IR*> v_res;
    this->get_all_non_term_ir_node_helper(v_res, this->ir_root, max_comp_level, false);
    return v_res;
}

vector<IR*> IRWrapper::get_all_non_term_ir_node_breadth_first()
{
    // Gather all the ir nodes in the current ir tree.
    vector<IR*> v_res;
    this->get_all_non_term_ir_node_breadth_first_helper(v_res, this->ir_root);
    return v_res;
}

void IRWrapper::get_all_non_term_ir_node_helper(vector<IR*>& v_res, IR*& cur_root, ExpressionTyp max_comp_level, const bool is_only_save_favor)
{
    /*
     * Attention:
     * If is_only_save_favor is enabled, then max_comp_level would be ignored!
     */
    // Helper to gather all the ir node in the current ir tree.
    for (IR*& cur_child : cur_root->get_children()) {
        this->get_all_non_term_ir_node_helper(v_res, cur_child, max_comp_level, is_only_save_favor);
    }

    if (is_only_save_favor) {
        if (cur_root->get_is_favor() != non_favor) {
            v_res.push_back(cur_root);
        }
        return;
    }

    // else, for non-favored, check whether symbol exceed max_comp_level.
    if (cur_root->get_symbol_type() == SymbolNonTerm && cur_root->get_mapped_expr_node() != nullptr && // For custom introduced IRs, may not have expr node attached.
        cur_root->get_mapped_expr_node()->get_expression_type() <= max_comp_level) {
        v_res.push_back(cur_root);
        // return;
    }
}

void IRWrapper::get_all_non_term_ir_node_breadth_first_helper(vector<IR*>& v_res, IR*& cur_root)
{
    // Helper to gather all the ir node in the current ir tree.
    // breadth first.
    if (cur_root->get_symbol_type() == SymbolNonTerm) {
        v_res.push_back(cur_root);
    }
    for (IR*& cur_child : cur_root->get_children()) {
        bool tmp = false;
        this->get_all_non_term_ir_node_helper(v_res, cur_child, ExpTypComp, tmp);
    }
}

IRTYPE IRWrapper::get_parent_type(IR* cur_node, int depth)
{
    // depth <= 0 means immediate parent.
    // depth 1 means grandparent.
    // so on so forth.
#ifdef DEBUG
    if (cur_node == nullptr) {
        cerr << "Getting nullptr from cur_node in IRWrapper::get_parent_type. \n\n\n";
        abort();
    }
#endif
    IRTYPE res_type;
    do {
        cur_node = cur_node->get_parent_node();
        if (cur_node == nullptr) {
            return IRTypeUnknownType;
        }
        res_type = cur_node->get_ir_type();
    } while ((depth-- < 0));
    return res_type;
}

void IRWrapper::iter_sub_nodes_with_handler(IR* cur_node, void* var_1, void* var_2, handler_t handler)
{
    // Recursive function.
    // Depth first search.
    if (cur_node == NULL || handler == NULL) {
        return;
    }

    for (IR*& cur_child : cur_node->get_children()) {
        this->iter_sub_nodes_with_handler(cur_child, var_1, var_2, handler);
    }

    // Call the handler function to modify all the searched nodes.
    handler(cur_node, var_1, var_2);
}

bool IRWrapper::is_ir_before(IR* l, IR* r)
{
    vector<IR*> v_all_node = this->get_all_ir_node();
    auto l_idx = std::find(v_all_node.begin(), v_all_node.end(), l);
    auto r_idx = std::find(v_all_node.begin(), v_all_node.end(), r);

    if (l_idx == v_all_node.end() || r_idx == v_all_node.end()) {
        cerr << "Error: cannot find IR in root: " << l->print_info() << " " << r->print_info() << "\n\n\n";
        abort();
    }

    return (l_idx <= r_idx);
}

vector<IRTYPE> IRWrapper::get_all_stmt_ir_type()
{
    vector<IR*> v_stmt = this->get_stmt_ir_vec();

    vector<IRTYPE> res;
    for (IR*& cur_stmt : v_stmt) {
        res.push_back(cur_stmt->get_ir_type());
    }

    return res;
}
