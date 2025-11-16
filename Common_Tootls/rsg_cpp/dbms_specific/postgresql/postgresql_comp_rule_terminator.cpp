//
// Created by XXX on 10/14/24.
//

#include "../../headers/fuzzer_configurations.h"
#include "postgresql_comp_rule_terminator.h"

// used in SQL IR generation.
// the return is whether we should continue parsing/generating the sub-node of this ir.
// if the current ir modification does not impact the sub-ir-tree, return FALSE.
// otherwise, handle everything in this function, and return TRUE.
bool postgresql_comp_rule_terminator(RSG* rsg, ProductionNode*& cur_prod, IR*& cur_ir)
{
    // If directly simplified as literal or identifier, remember to balance between the identifier and the literals.

    // TODO:: FIXME:: Implement this later.

    // ATTENTION, MySQL has expr, simple_expr, common_table_expr, signal_allowed_expr, bit_expr, sum_expr. All handled differently.

    // ignore sum_expr, and common_table_expr

    IRTYPE ir_typ = cur_ir->get_ir_type();

    if (ir_typ == IRTypeAExpr || ir_typ == IRTypeBExpr || ir_typ == IRTypeCExpr) {
        // Can be simplified as simple_expr.

        cur_ir->free_children();
        IR* child_ir = nullptr;

        string str = "TRUE";
        if (get_pct_hit(50)) {
            str = "FALSE";
        }

        child_ir = new IR(SymbolLit, IRTypeBOOLEAN, str, nullptr, nullptr, nullptr);

        cur_ir->add_one_child(child_ir, 0);
        cur_ir->set_mapped_expr_node(&dump_simple_expr_node);
        cur_ir->set_is_favor(IsFavor::favor);

        return true;

    } else if (ir_typ == IRTypeFuncExpr) {
        // Can be simplified as sum_expr.

        cur_ir->free_children();
        IR* child_ir = new IR(SymbolTerm, IRTypeUnknownType, string("COUNT (*)"), nullptr, nullptr, nullptr);

        cur_ir->add_one_child(child_ir, 0);
        cur_ir->set_mapped_expr_node(&dump_simple_expr_node);
        cur_ir->set_is_favor(IsFavor::favor);

        return true;

    } else if (ir_typ == IRTypeSelectWithParens && cur_ir->get_parent_node() && rsg->p_ir_wrapper->is_in_subquery(nullptr, cur_ir)) {
        // Can be simplified as subquery.

        cur_ir->free_children();

        cur_ir->set_symbol_type(SymbolNonTerm);
        cur_prod = rsg->get_prod_from_ir_type(IRTypeSelectNoParens);
        cur_ir->set_mapped_prod_node(cur_prod);

        string sub_str = "( SELECT TRUE )";
        if (get_pct_hit(50)) {
            sub_str = "( SELECT FALSE )";
        }
        IR* sub_ir = new IR(SymbolTerm, IRTypeUnknownType, sub_str, nullptr, nullptr, nullptr);
        cur_ir->add_one_child(sub_ir, 0);

        cur_ir->set_mapped_expr_node(&dump_simple_expr_node);
        cur_ir->set_is_favor(IsFavor::favor);

        return true;

    } else if (ir_typ == IRTypeTableRef) {
        // Can be simplified as table_ref.

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