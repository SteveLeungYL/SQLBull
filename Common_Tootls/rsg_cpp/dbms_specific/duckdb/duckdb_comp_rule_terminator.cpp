//
// Created by XXX on 10/14/24.
//

#include "duckdb_comp_rule_terminator.h"

// used in SQL IR generation.
// the return is whether we should continue parsing/generating the sub-node of this ir.
// if the current ir modification does not impact the sub-ir-tree, return FALSE.
// otherwise, handle everything in this function, and return TRUE.
bool duckdb_comp_rule_terminator(RSG* rsg, ProductionNode*& cur_prod, IR*& cur_ir)
{
    // If directly simplified as literal or identifier, remember to balance between the identifier and the literals.

    IRTYPE ir_typ = cur_ir->get_ir_type();
    if (ir_typ == IRTypeSelectNoParens || ir_typ == IRTypeSelectStmt) {
        cur_ir->set_symbol_type(SymbolNonTerm);
        cur_prod = rsg->get_prod_from_ir_type(IRTypeSelectNoParens);
        cur_ir->set_mapped_expr_node(cur_prod->all_exprs.front());
        cur_ir->set_mapped_prod_node(cur_prod);
        cur_ir->set_is_favor(IsFavor::favor);

        IR* sub_ir = new IR(SymbolTerm, IRTypeUnknownType, string("SELECT"),
            nullptr, nullptr, nullptr);
        cur_ir->free_children();
        cur_ir->add_one_child(sub_ir, 0);

        IR* a_expr_ir = new IR(SymbolNonTerm, IRTypeAExpr, string(""), rsg->get_token_from_ir_type(IRTypeAExpr), &dump_simple_expr_node, rsg->get_prod_from_ir_type(IRTypeAExpr));
        IR* string_literal_ir = new IR(SymbolLit, IRTypeUnknownType, string("'string'"), nullptr, nullptr, nullptr);
        a_expr_ir->add_one_child(string_literal_ir, 0);

        cur_ir->add_one_child(a_expr_ir, 1);

        cur_ir->set_mapped_expr_node(&dump_simple_expr_node);

        return true;
    } else if (ir_typ == IRTypeAExpr || ir_typ == IRTypeBExpr || ir_typ == IRTypeCExpr || ir_typ == IRTypeDExpr) {
        cur_ir->free_children();
        IR* child_ir = nullptr;
        if (get_pct_hit(50)) {
            child_ir = new IR(SymbolLit, IRTypeUnknownType, string("'string'"), nullptr, nullptr, nullptr);
        } else {
            child_ir = new IR(SymbolIden, IRTypeColId, string("c01"), rsg->get_token_from_ir_type(IRTypeColId), nullptr, nullptr);
            child_ir->set_data_type(DataColumnName);
            child_ir->set_data_flag(ContextUse);
        }
        cur_ir->add_one_child(child_ir, 0);
        cur_ir->set_mapped_expr_node(&dump_simple_expr_node);

        return true;
    } else if (ir_typ == IRTypeTableRef) {

        vector<IR*> new_children;
        IR* table_name_ir = new IR(SymbolIden, IRTypeIDENT, "v00", nullptr, nullptr, nullptr);
        table_name_ir->set_data_type(DataTableName);
        table_name_ir->set_data_flag(ContextUse);

        IR* alias_as = new IR(SymbolLit, IRTypeUnknownType, "AS ", nullptr, nullptr, nullptr);

        IR* alias_ir = new IR(SymbolIden, IRTypeIDENT, "a00", nullptr, nullptr, nullptr);
        alias_ir->set_data_type(DataTableAliasName);
        alias_ir->set_data_flag(ContextDefine);

        new_children.push_back(table_name_ir);
        new_children.push_back(alias_as);
        new_children.push_back(alias_ir);

        cur_ir->set_symbol_type(SymbolNonTerm);
        cur_ir->free_children();
        cur_ir->set_children_nodes(new_children);

        cur_ir->set_mapped_expr_node(&dump_simple_expr_node);

        return true;
    }

    // Did not handle the terminating type,
    // continue parsing the sub-ir-tree.
    return false;
};