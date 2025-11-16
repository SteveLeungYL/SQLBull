//
// Created by XXX on 10/21/24.
//

#include "postgresql_keyword_handling.h"
#include "../../headers/ir_types_common.h"
#include "../../headers/node.h"
#include "../../headers/utils.h"

void postgresql_keyword_handl(TokenNode*& cur_token)
{
    // Modify the cur_str, not the IR itself.

    IR* cur_ir = cur_token->get_cached_ir();
    string cur_str = cur_ir->get_str_val();

    if (cur_str == "SCONST") {
        cur_str = "";
        cur_ir->set_symbol_type(SymbolLit);
        cur_ir->set_ir_type(IRTypeSTRING);
        cur_ir->set_data_type(DataLiteral);
        cur_ir->set_data_affinity_type(AFFISTRING);
        cur_ir->set_str_val("'string'");
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    } else if (cur_str == "ICONST") {
        cur_str = "0";
        cur_ir->set_symbol_type(SymbolLit);
        cur_ir->set_ir_type(IRTypeINTEGER);
        cur_ir->set_data_type(DataLiteral);
        cur_ir->set_data_affinity_type(AFFIINT);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    } else if (cur_str == "FCONST") {
        cur_str = "0.0";
        cur_ir->set_symbol_type(SymbolLit);
        cur_ir->set_ir_type(IRTypeFLOAT);
        cur_ir->set_data_type(DataLiteral);
        cur_ir->set_data_affinity_type(AFFIFLOAT);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    } else if (cur_str == "BCONST") {
        cur_str = "B'101'";
        cur_ir->set_symbol_type(SymbolLit);
        cur_ir->set_ir_type(IRTypeBIT);
        cur_ir->set_data_type(DataLiteral);
        cur_ir->set_data_affinity_type(AFFIBIT);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    } else if (cur_str == "XCONST") {
        // TREAT IT as float. 
        cur_str = "0.0";
        cur_ir->set_symbol_type(SymbolLit);
        cur_ir->set_ir_type(IRTypeFLOAT);
        cur_ir->set_data_type(DataLiteral);
        cur_ir->set_data_affinity_type(AFFIFLOAT);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    } else if (cur_str == "LESS_EQUALS") {
        cur_str = "<=";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    } else if (cur_str == "GREATER_EQUALS") {
        cur_str = ">=";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    } else if (cur_str == "NOT_EQUALS") {
        cur_str = "<>";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    } else if (cur_str == "DOT_DOT") {
        cur_str = "..";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    } else if (cur_str == "COLON_EQUALS") {
        cur_str = ":=";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    } else if (cur_str == "EQUALS_GREATER") {
        cur_str = "=>";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    } else if (cur_str == "TYPECAST") {
        cur_str = "::";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    } else if (cur_str == "Op") {
        // Treat it as =.
        cur_str = "=";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    } else if (cur_str == "TRUE_P") {
        cur_ir->set_symbol_type(SymbolLit);
        cur_ir->set_ir_type(IRTypeBOOLEAN);
        cur_ir->set_data_type(DataLiteral);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    } else if (cur_str == "FALSE_P") {
        cur_ir->set_symbol_type(SymbolLit);
        cur_ir->set_ir_type(IRTypeBOOLEAN);
        cur_ir->set_data_type(DataLiteral);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    }

    // The identifiers are handled in the ir_context_setup code.

    //////////////////////////////////////////////

    cur_str = replace_str(cur_str, "_P", "");
    cur_str = replace_str(cur_str, "_LA", "");
    cur_ir->set_str_val(cur_str);
}