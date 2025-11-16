//
// Created by XXX on 10/21/24.
//

#include "sqlite_keyword_handling.h"
#include "../../headers/node.h"
#include "../../headers/utils.h"
#include "../../headers/ir_types_common.h"

void sqlite_keyword_handl(TokenNode*& cur_token)
{
    // Modify the cur_str, not the IR itself.

    IR* cur_ir = cur_token->get_cached_ir();
    string cur_str = cur_ir->get_str_val();

    if (cur_str == "idj") {
        cur_str = "v00";
        cur_ir->set_symbol_type(SymbolIden);
        cur_ir->set_ir_type(IRTypeidj);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    } else if (cur_str == "LP") {
        cur_str = "(";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_ir->set_ir_type(IRTypeLP);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    } else if (cur_str == "RP") {
        cur_str = ")";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_ir->set_ir_type(IRTypeRP);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    } else if (cur_str == "COMMA") {
        cur_str = ",";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_ir->set_ir_type(IRTypeCOMMA);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    } else if (cur_str == "STRING") {
        cur_str = "'string'";
        cur_ir->set_symbol_type(SymbolLit);
        cur_ir->set_ir_type(IRTypeSTRING);
        cur_ir->set_data_type(DataLiteral);
        cur_ir->set_data_affinity_type(AFFISTRING);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    } else if (cur_str == "FLOAT") {
        cur_str = "0.0";
        cur_ir->set_symbol_type(SymbolLit);
        cur_ir->set_ir_type(IRTypeFLOAT);
        cur_ir->set_data_affinity_type(AFFIFLOAT);
        cur_ir->set_data_type(DataLiteral);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    } else if (cur_str == "BLOB") {
        cur_str = "blob";
        cur_ir->set_symbol_type(SymbolLit);
        cur_ir->set_ir_type(IRTypeBLOB);
        cur_ir->set_data_type(DataLiteral);
        // Treat it as STRING.
        cur_ir->set_data_affinity_type(AFFISTRING);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    } else if (cur_str == "INTEGER") {
        cur_str = "0";
        cur_ir->set_symbol_type(SymbolLit);
        cur_ir->set_ir_type(IRTypeINTEGER);
        cur_ir->set_data_affinity_type(AFFIINT);
        cur_ir->set_data_type(DataLiteral);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    } else if (cur_str == "CTIME_KW") {
        cur_str = "'2024-10-01'";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_ir->set_ir_type(IRTypeCTIME_KW);
        cur_ir->set_data_type(DataLiteral);
        cur_ir->set_data_affinity_type(AFFITIME);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    } else if (cur_str == "QNUMBER") {
        cur_str = "100";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_ir->set_ir_type(IRTypeINTEGER);
        cur_ir->set_data_affinity_type(AFFIINT);
        cur_ir->set_data_type(DataLiteral);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    } else if (cur_str == "PLUS") {
        cur_str = "+";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_ir->set_ir_type(IRTypePLUS);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    } else if (cur_str == "MINUS") {
        cur_str = "-";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_ir->set_ir_type(IRTypePLUS);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    } else if (cur_str == "ID") {
        // Ignore the ID keyword. Only used in the generated clause.
        cur_str = "";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_ir->set_ir_type(IRTypeTerm);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    } else if (cur_str == "AUTOINCR") {
        cur_str = "AUTOINCREMENT";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_ir->set_ir_type(IRTypeTerm);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    } else if (cur_str == "STAR") {
        cur_str = " * ";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_ir->set_ir_type(IRTypeTerm);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    } else if (cur_str == "DOT") {
        cur_str = " . ";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_ir->set_ir_type(IRTypeTerm);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    } else if (cur_str == "EQ") {
        cur_str = " = ";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_ir->set_ir_type(IRTypeTerm);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    } else if (cur_str == "LT") {
        cur_str = "<";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_ir->set_ir_type(IRTypeTerm);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    } else if (cur_str == "GT") {
        cur_str = ">";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_ir->set_ir_type(IRTypeTerm);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    } else if (cur_str == "GE") {
        cur_str = ">=";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_ir->set_ir_type(IRTypeTerm);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    } else if (cur_str == "LE") {
        cur_str = "<=";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_ir->set_ir_type(IRTypeTerm);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    } else if (cur_str == "NE") {
        cur_str = "<>";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_ir->set_ir_type(IRTypeTerm);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    } else if (cur_str == "BITAND") {
        cur_str = "&";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_ir->set_ir_type(IRTypeTerm);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    } else if (cur_str == "BITOR") {
        cur_str = "|";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_ir->set_ir_type(IRTypeTerm);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    } else if (cur_str == "LSHIFT") {
        cur_str = "<<";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_ir->set_ir_type(IRTypeTerm);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    } else if (cur_str == "RSHIFT") {
        cur_str = ">>";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_ir->set_ir_type(IRTypeTerm);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    } else if (cur_str == "SLASH") {
        cur_str = "/";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_ir->set_ir_type(IRTypeTerm);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    } else if (cur_str == "REM") {
        cur_str = "%";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_ir->set_ir_type(IRTypeTerm);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    } else if (cur_str == "CONCAT") {
        cur_str = "||";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_ir->set_ir_type(IRTypeTerm);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    } else if (cur_str == "BITNOT") {
        cur_str = "~";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_ir->set_ir_type(IRTypeTerm);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    } else if (cur_str == "PTR") {
        cur_str = "->";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_ir->set_ir_type(IRTypeTerm);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    } else if (cur_str == "ISNULL") {
        cur_str = "IS NULL";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_ir->set_ir_type(IRTypeTerm);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    } else if (cur_str == "NOTNULL") {
        cur_str = "NOT NULL";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_ir->set_ir_type(IRTypeTerm);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    } else if (cur_str == "number") {
        cur_str = "0.0";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_ir->set_ir_type(IRTypeTerm);
        if(get_pct_hit(50)) {
            cur_ir->set_data_affinity_type(AFFIFLOAT);
        } else {
            cur_ir->set_data_affinity_type(AFFIINT);
        }
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    } else if (cur_str == "SEMI") {
        cur_str = ";";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_ir->set_ir_type(IRTypeSEMI);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    }

    cur_str = replace_str(cur_str, "_KW", "");
    cur_str = replace_str(cur_str, "COLUMNKW", "COLUMN");

    cur_ir->set_str_val(cur_str);
}