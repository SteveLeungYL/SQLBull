//
// Created by XXX on 10/21/24.
//

#include "duckdb_keyword_handling.h"
#include "../../headers/node.h"
#include "../../headers/utils.h"

void duckdb_keyword_handl(TokenNode*& cur_token)
{
    // Modify the cur_str, not the IR itself.

    IR* cur_ir = cur_token->get_cached_ir();
    string cur_str = cur_ir->get_str_val();

    if (cur_str == "IDENT") {
        cur_str = "ident";
        cur_ir->set_symbol_type(SymbolIden);
        cur_ir->set_ir_type(IRTypeName);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    } else if (cur_str == "SCONST") {
        cur_str = "'string'";
        cur_ir->set_symbol_type(SymbolLit);
        cur_ir->set_ir_type(IRTypeSCONST);
        cur_ir->set_data_affinity_type(AFFISTRING);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    }

    else if (cur_str == "ICONST") {
        cur_str = "0";
        cur_ir->set_lit_int_value(0);
        cur_ir->set_symbol_type(SymbolLit);
        cur_ir->set_ir_type(IRTypeICONST);
        cur_ir->set_data_affinity_type(AFFIINT);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    }

    else if (cur_str == "FCONST") {
        cur_str = "0.0";
        cur_ir->set_lit_float_value(0.0);
        cur_ir->set_symbol_type(SymbolLit);
        cur_ir->set_ir_type(IRTypeFCONST);
        cur_ir->set_data_affinity_type(AFFIFLOAT);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    }

    else if (cur_str == "BCONST") {
        // byte, not bool.
        cur_str = "b'bypte'";
        cur_ir->set_symbol_type(SymbolLit);
        cur_ir->set_ir_type(IRTypeBCONST);
        cur_ir->set_data_affinity_type(AFFIBYTES);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    }

    else if (cur_str == "XCONST") {
        cur_str = "B'10010'";
        cur_ir->set_symbol_type(SymbolLit);
        cur_ir->set_ir_type(IRTypeXCONST);
        cur_ir->set_data_affinity_type(AFFIBIT);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    }

    else if (cur_str == "Op") {
        cur_str = "+";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_ir->set_ir_type(IRTypeUnknownType);
        cur_ir->set_data_affinity_type(AFFIUNKNOWN);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    }

    else if (cur_str == "PARAMS") {
        cur_str = "?";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_ir->set_ir_type(IRTypeUnknownType);
        cur_ir->set_data_affinity_type(AFFIUNKNOWN);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    }

    else if (cur_str == "TYPECAST") {
        cur_str = "::";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_ir->set_ir_type(IRTypeUnknownType);
        cur_ir->set_data_affinity_type(AFFIUNKNOWN);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    }

    else if (cur_str == "DOT_DOT") {
        cur_str = "..";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_ir->set_ir_type(IRTypeUnknownType);
        cur_ir->set_data_affinity_type(AFFIUNKNOWN);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    }

    else if (cur_str == "COLON_EQUALS") {
        cur_str = ":=";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_ir->set_ir_type(IRTypeUnknownType);
        cur_ir->set_data_affinity_type(AFFIUNKNOWN);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    }

    else if (cur_str == "EQUALS_GREATER") {
        cur_str = "=>";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_ir->set_ir_type(IRTypeUnknownType);
        cur_ir->set_data_affinity_type(AFFIUNKNOWN);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    }

    else if (cur_str == "INTEGER_DIVISION") {
        cur_str = "//";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_ir->set_ir_type(IRTypeUnknownType);
        cur_ir->set_data_affinity_type(AFFIUNKNOWN);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    }

    else if (cur_str == "POWER_OF") {
        cur_str = "**";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_ir->set_ir_type(IRTypeUnknownType);
        cur_ir->set_data_affinity_type(AFFIUNKNOWN);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    }

    else if (cur_str == "LAMBDA_ARROW") {
        cur_str = "->";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_ir->set_ir_type(IRTypeUnknownType);
        cur_ir->set_data_affinity_type(AFFIUNKNOWN);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    }

    else if (cur_str == "DOUBLE_ARROW") {
        cur_str = "->>";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_ir->set_ir_type(IRTypeUnknownType);
        cur_ir->set_data_affinity_type(AFFIUNKNOWN);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    }

    else if (cur_str == "LESS_EQUALS") {
        cur_str = "<=";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_ir->set_ir_type(IRTypeUnknownType);
        cur_ir->set_data_affinity_type(AFFIUNKNOWN);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    }

    else if (cur_str == "GREATER_EQUALS") {
        cur_str = ">=";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_ir->set_ir_type(IRTypeUnknownType);
        cur_ir->set_data_affinity_type(AFFIUNKNOWN);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    }

    else if (cur_str == "NOT_EQUALS") {
        cur_str = "!=";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_ir->set_ir_type(IRTypeUnknownType);
        cur_ir->set_data_affinity_type(AFFIUNKNOWN);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    }

    else if (cur_str == "ISNULL") {
        cur_str = "IS NULL";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_ir->set_ir_type(IRTypeUnknownType);
        cur_ir->set_data_affinity_type(AFFIUNKNOWN);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    }

    else if (cur_str == "NOTNULL") {
        cur_str = "NOTNULL"; // NOT CHANGED.
        cur_ir->set_symbol_type(SymbolTerm);
        cur_ir->set_ir_type(IRTypeUnknownType);
        cur_ir->set_data_affinity_type(AFFIUNKNOWN);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    }

    else if (cur_str == "GLOBAL") {
        cur_str = "";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_ir->set_ir_type(IRTypeGLOBAL);
        cur_ir->set_data_affinity_type(AFFIUNKNOWN);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    }

    else if (cur_str == "GLOBAL") {
        cur_str = "";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_ir->set_ir_type(IRTypeGLOBAL);
        cur_ir->set_data_affinity_type(AFFIUNKNOWN);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    }

    cur_str = replace_str(cur_str, "_P", "");
    cur_str = replace_str(cur_str, "_LA", "");

    cur_ir->set_str_val(cur_str);
}