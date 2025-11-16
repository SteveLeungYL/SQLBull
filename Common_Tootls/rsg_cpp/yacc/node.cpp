//
// Created by XXX on 3/22/24.
//

#include "../headers/node.h"

ProductionNode* get_new_production_node(int pos, const string name) { return new ProductionNode(pos, name); }
ExpressionNode* get_new_expression(int pos) { return new ExpressionNode(pos); }
TokenNode* get_new_token_node(const string str_in, TokenNodeTyp typ) { return new TokenNode(str_in, typ); }

ExpressionNode dump_simple_expr_node(-1, ExpTypTerm);