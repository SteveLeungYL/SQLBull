//
// Created by XXX on 10/14/24.
//

#include "../../headers/fuzzer_configurations.h"
#include "mysql_comp_rule_terminator.h"

// used in SQL IR generation.
// the return is whether we should continue parsing/generating the sub-node of this ir.
// if the current ir modification does not impact the sub-ir-tree, return FALSE.
// otherwise, handle everything in this function, and return TRUE.
bool mysql_comp_rule_terminator(RSG* rsg, ProductionNode*& cur_prod, IR*& cur_ir)
{
    // If directly simplified as literal or identifier, remember to balance between the identifier and the literals.

    // TODO:: FIXME:: Implement this later.

    // ATTENTION, MySQL has expr, simple_expr, common_table_expr, signal_allowed_expr, bit_expr, sum_expr. All handled differently.

    // ignore sum_expr, and common_table_expr

    IRTYPE ir_typ = cur_ir->get_ir_type();

    if (ir_typ == IRTypeExpr || ir_typ == IRTypeSimpleExpr || ir_typ == IRTypeSignalAllowedExpr || ir_typ == IRTypeBitExpr) {
        // Can be simplified as simple_expr.

        cur_ir->free_children();
        IR* child_ir = nullptr;

        string str = "TRUE";
        if (get_pct_hit(50)) {
            str = "FALSE";
        }

        child_ir = new IR(SymbolLit, IRTypeBOOLEAN, str, nullptr, nullptr, nullptr);

        // if (get_pct_hit(50)) {
            // child_ir = new IR(SymbolLit, IRTypeUnknownType, string("TRUE"), nullptr, nullptr, nullptr);
        // } else {
        //     child_ir = new IR(SymbolIden, IRTypeIDENT, string("c01"), nullptr, nullptr, nullptr);
        //     child_ir->set_data_type(DataColumnName);
        //     child_ir->set_data_flag(ContextUse);
        // }

        cur_ir->add_one_child(child_ir, 0);
        cur_ir->set_mapped_expr_node(&dump_simple_expr_node);
        cur_ir->set_is_favor(IsFavor::favor);

        return true;
    } else if (ir_typ == IRTypeSumExpr) {
        // Can be simplified as sum_expr.

        cur_ir->free_children();
        IR* child_ir = new IR(SymbolTerm, IRTypeUnknownType, string("COUNT (*)"), nullptr, nullptr, nullptr);

        cur_ir->add_one_child(child_ir, 0);
        cur_ir->set_mapped_expr_node(&dump_simple_expr_node);

        return true;
    } else if (ir_typ == IRTypeSubquery) {
        // Can be simplified as subquery.

        cur_ir->free_children();

        cur_ir->set_symbol_type(SymbolNonTerm);
        cur_prod = rsg->get_prod_from_ir_type(IRTypeSubquery);
        cur_ir->set_mapped_prod_node(cur_prod);

        IR* left_paren_ir = new IR(SymbolTerm, IRTypeUnknownType, string("("), nullptr, nullptr, nullptr);
        cur_ir->add_one_child(left_paren_ir, 0);

        IR* sub_ir = new IR(SymbolTerm, IRTypeUnknownType, string("SELECT"),
            nullptr, nullptr, nullptr);
        cur_ir->add_one_child(sub_ir, 1);

        IR* a_expr_ir = new IR(SymbolNonTerm, IRTypeExpr, string(""), rsg->get_token_from_ir_type(IRTypeExpr), &dump_simple_expr_node, rsg->get_prod_from_ir_type(IRTypeExpr));
        IR* string_literal_ir = new IR(SymbolLit, IRTypeUnknownType, string("'string'"), nullptr, nullptr, nullptr);
        a_expr_ir->add_one_child(string_literal_ir, 0);

        cur_ir->add_one_child(a_expr_ir, 2);

        IR* right_paren_ir = new IR(SymbolTerm, IRTypeUnknownType, string(")"), nullptr, nullptr, nullptr);
        cur_ir->add_one_child(right_paren_ir, 3);

        cur_ir->set_mapped_expr_node(&dump_simple_expr_node);
        cur_ir->set_is_favor(IsFavor::favor);

        return true;
    } else if (ir_typ == IRTypeTableFactor) {
        // Can be simplified as table_factor.

        cur_ir->free_children();
        cur_ir->set_mapped_expr_node(&dump_simple_expr_node);

        IR* table_name_ir = new IR(IRTypeIDENT, string("v00"), DataTableName, ContextUseTop, nullptr);
        cur_ir->add_one_child(table_name_ir, 0);

        cur_ir->set_mapped_expr_node(&dump_simple_expr_node);
        cur_ir->set_is_favor(IsFavor::favor);

        return true;
    }

    // Did not handle the terminating type,
    // continue parsing the sub-ir-tree.
    return false;
};