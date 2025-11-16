//
// Created by XXX on 10/14/24.
//

#include "sqlite_comp_rule_terminator.h"

// used in SQL IR generation.
// the return is whether we should continue parsing/generating the sub-node of this ir.
// if the current ir modification does not impact the sub-ir-tree, return FALSE.
// otherwise, handle everything in this function, and return TRUE.
bool sqlite_comp_rule_terminator(RSG* rsg, ProductionNode*& cur_prod, IR*& cur_ir)
{
    // If directly simplified as literal or identifier, remember to balance between the identifier and the literals.

    IRTYPE ir_typ = cur_ir->get_ir_type();
    if (ir_typ == IRTypeSelectStmt) {
        cur_ir->set_symbol_type(SymbolNonTerm);
        // Should we favor the SELECT statement?
        cur_ir->set_is_favor(IsFavor::favor);
        cur_ir->free_children();

        IR* sub_ir = new IR(SymbolTerm, IRTypeUnknownType, string("SELECT"),
            nullptr, nullptr, nullptr);
        cur_ir->add_one_child(sub_ir, 0);

        if (get_pct_hit(50)) {
            IR* a_expr_ir = new IR(SymbolNonTerm, IRTypeExpr, string(""), rsg->get_token_from_ir_type(IRTypeExpr), &dump_simple_expr_node, rsg->get_prod_from_ir_type(IRTypeExpr));
            IR* string_literal_ir = new IR(SymbolLit, IRTypeSTRING, string("'string'"), nullptr, nullptr, nullptr);
            string_literal_ir->set_data_type(DataLiteral);
            a_expr_ir->add_one_child(string_literal_ir, 0);
            a_expr_ir->set_mapped_expr_node(&dump_simple_expr_node);
            a_expr_ir->set_is_favor(favor);

            cur_ir->add_one_child(a_expr_ir, 1);
            cur_ir->set_mapped_expr_node(&dump_simple_expr_node);
            cur_ir->set_is_favor(favor);
        } else {
            auto* column_name_ir = new IR(SymbolIden, IRTypeIDENT, string("c01"), nullptr, nullptr, nullptr);
            column_name_ir->set_data_type(DataColumnName);
            column_name_ir->set_data_flag(ContextUse);

            auto* sel_column_list_ir = new IR(SymbolNonTerm, IRTypeSelcollist, string(""), rsg->get_token_from_ir_type(IRTypeSelcollist), &dump_simple_expr_node, rsg->get_prod_from_ir_type(IRTypeSelcollist));
            sel_column_list_ir->add_one_child(column_name_ir, 0);
            sel_column_list_ir->set_is_favor(favor);

            cur_ir->add_one_child(sel_column_list_ir, 1);

            auto* from_ir = new IR(SymbolNonTerm, IRTypeFrom, string(""), rsg->get_token_from_ir_type(IRTypeFrom), &dump_simple_expr_node, rsg->get_prod_from_ir_type(IRTypeFrom));

            auto* from_keyword_ir = new IR(SymbolTerm, IRTypeFROM, string("FROM"), nullptr, nullptr, nullptr);

            auto* table_name_ir = new IR(SymbolIden, IRTypeIDENT, string("v00"), nullptr, nullptr, nullptr);
            table_name_ir->set_data_type(DataTableName);
            table_name_ir->set_data_flag(ContextUse);

            // auto* comma_ir = new IR(SymbolTerm, IRTypeCOMMA, string(","), rsg->get_token_from_ir_type(IRTypeCOMMA), nullptr, nullptr);

            // auto* sel_table_list_ir = new IR(SymbolNonTerm, IRTypeSeltablist, string(""), rsg->get_token_from_ir_type(IRTypeSeltablist), &dump_simple_expr_node, rsg->get_prod_from_ir_type(IRTypeSeltablist));
            // sel_table_list_ir->set_is_favor(favor);

            from_ir->add_one_child(from_keyword_ir, 0);
            from_ir->add_one_child(table_name_ir, 1);
            // from_ir->add_one_child(comma_ir, 2);
            // from_ir->add_one_child(sel_table_list_ir, 3);
            from_ir->set_is_favor(favor);

            cur_ir->add_one_child(from_ir, 2);
            cur_ir->set_mapped_expr_node(&dump_simple_expr_node);
            cur_ir->set_is_favor(favor);
        }

        return true;
    } else if (ir_typ == IRTypeExpr) {
        cur_ir->free_children();
        IR* child_ir = nullptr;
        if (get_pct_hit(50)) {
            auto child_ir_type = IRTypeSTRING;
            auto child_data_affinity = AFFIANY;
            int rand_int = get_rand_int(100);
            if (rand_int < 30) {
                child_ir_type = IRTypeSTRING;
            } else if (rand_int < 60) {
                child_ir_type = IRTypeINTEGER;
            } else if (rand_int < 95) {
                child_ir_type = IRTypeFLOAT;
            } else {
                child_ir_type = IRTypeBOOLEAN;
            }
            child_ir = new IR(SymbolLit, child_ir_type, string("'string'"), nullptr, nullptr, nullptr);
            child_ir->set_data_type(DataLiteral);
            child_ir->set_data_affinity(child_data_affinity);
        } else {
            child_ir = new IR(SymbolIden, IRTypeIDENT, string("c01"), nullptr, nullptr, nullptr);
            child_ir->set_data_type(DataColumnName);
            child_ir->set_data_flag(ContextUse);
        }
        cur_ir->add_one_child(child_ir, 0);
        cur_ir->set_mapped_expr_node(&dump_simple_expr_node);
        cur_ir->set_is_favor(favor);

        return true;
    }

    // Did not handle the terminating type,
    // continue parsing the sub-ir-tree.
    return false;
};