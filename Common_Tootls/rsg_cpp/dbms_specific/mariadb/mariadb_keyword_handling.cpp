//
// Created by XXX on 10/21/24.
//

#include "mariadb_keyword_handling.h"
#include "../../headers/ir_types_common.h"
#include "../../headers/node.h"
#include "../../headers/utils.h"

void mariadb_keyword_handl(TokenNode*& cur_token)
{
    // Modify the cur_str, not the IR itself.

    IR* cur_ir = cur_token->get_cached_ir();
    string cur_str = cur_ir->get_str_val();

    if (cur_str == "END_OF_INPUT") {
        cur_str = "";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_ir->set_ir_type(IRTypeENDOFINPUT);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    } else if (cur_str == "AND_AND_SYM") {
        cur_str = "&&";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_ir->set_ir_type(IRTypeANDANDSYM);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    } else if (cur_str == "LE") {
        cur_str = "<=";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_ir->set_ir_type(IRTypeLE);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    } else if (cur_str == "NE") {
        cur_str = "!=";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_ir->set_ir_type(IRTypeNE);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    } else if (cur_str == "GE") {
        cur_str = ">=";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_ir->set_ir_type(IRTypeGE);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    } else if (cur_str == "SHIFT_LEFT") {
        cur_str = "<<";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_ir->set_ir_type(IRTypeSHIFTLEFT);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    } else if (cur_str == "SHIFT_RIGHT") {
        cur_str = ">>";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_ir->set_ir_type(IRTypeSHIFTRIGHT);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    } else if (cur_str == "EQUAL_SYM") {
        cur_str = "<=>";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_ir->set_ir_type(IRTypeEQUALSYM);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    } else if (cur_str == "CURDATE") {
        cur_str = "CURRENT_DATE";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_ir->set_ir_type(IRTypeCURDATE);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    } else if (cur_str == "CURTIME") {
        cur_str = "CURRENT_TIME";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_ir->set_ir_type(IRTypeCURTIME);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    } else if (cur_str == "NOW_SYM") {
        cur_str = "CURRENT_TIMESTAMP";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_ir->set_ir_type(IRTypeNOWSYM);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    } else if (cur_str == "TIMESTAMP_ADD") {
        cur_str = "TIMESTAMPADD";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_ir->set_ir_type(IRTypeTIMESTAMPADD);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    } else if (cur_str == "TIMESTAMP_DIFF") {
        cur_str = "TIMESTAMPDIFF";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_ir->set_ir_type(IRTypeTIMESTAMPDIFF);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    } else if (cur_str == "OR2_SYM") {
        cur_str = "||";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_ir->set_ir_type(IRTypeOR2SYM);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    // } else if (cur_str == "USER") {
    //     cur_str = "SESSION_USER";
    //     cur_ir->set_symbol_type(SymbolTerm);
    //     cur_ir->set_ir_type(IRTypeUSER);
    //     cur_token->set_mapped_children_prods({});
    //     cur_token->set_type(TypTermKeyword);
    // } else if (cur_str == "GRAMMAR_SELECTOR_PART") {
    //     cur_str = "";
    //     cur_ir->set_symbol_type(SymbolTerm);
    //     cur_ir->set_ir_type(IRTypeGRAMMARSELECTORPART);
    //     cur_token->set_mapped_children_prods({});
    //     cur_token->set_type(TypTermKeyword);
    // } else if (cur_str == "GRAMMAR_SELECTOR_GCOL") {
    //     cur_str = "";
    //     cur_ir->set_symbol_type(SymbolTerm);
    //     cur_ir->set_ir_type(IRTypeGRAMMARSELECTORGCOL);
    //     cur_token->set_mapped_children_prods({});
    //     cur_token->set_type(TypTermKeyword);
    // } else if (cur_str == "GRAMMAR_SELECTOR_CTE") {
    //     cur_str = "";
    //     cur_ir->set_symbol_type(SymbolTerm);
    //     cur_ir->set_ir_type(IRTypeGRAMMARSELECTORCTE);
    //     cur_token->set_mapped_children_prods({});
    //     cur_token->set_type(TypTermKeyword);
    // } else if (cur_str == "GRAMMAR_SELECTOR_DERIVED_EXPR") {
    //     cur_str = "";
    //     cur_ir->set_symbol_type(SymbolTerm);
    //     cur_ir->set_ir_type(IRTypeGRAMMARSELECTORDERIVEDEXPR);
    //     cur_token->set_mapped_children_prods({});
    //     cur_token->set_type(TypTermKeyword);
    } else if (cur_str == "AUTO_INC") {
        cur_str = "AUTO_INCREMENT";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_ir->set_ir_type(IRTypeAUTOINC);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    } else if (cur_str == "AND_AND_SYM") {
        cur_str = "&&";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_ir->set_ir_type(IRTypeANDANDSYM);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    } else if (cur_str == "FALSE_SYM") {
        cur_str = "FALSE";
        cur_ir->set_symbol_type(SymbolLit);
        cur_ir->set_ir_type(IRTypeBOOLEAN);
        cur_ir->set_data_affinity_type(AFFIBOOL);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    } else if (cur_str == "TRUE_SYM") {
        cur_str = "TRUE";
        cur_ir->set_symbol_type(SymbolLit);
        cur_ir->set_ir_type(IRTypeBOOLEAN);
        cur_ir->set_data_affinity_type(AFFIBOOL);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    } else if (cur_str == "HEX_NUM") {
        cur_str = "0x1234";
        cur_ir->set_symbol_type(SymbolLit);
        cur_ir->set_ir_type(IRTypeINTEGER);
        cur_ir->set_data_affinity_type(AFFIINT);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    } else if (cur_str == "BIN_NUM") {
        cur_str = "'1100'";
        cur_ir->set_symbol_type(SymbolLit);
        cur_ir->set_ir_type(IRTypeINTEGER);
        cur_ir->set_data_affinity_type(AFFIBIT);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    } else if (cur_str == "UNDERSCORE_CHARSET") {
        // Not the best way to handle this.
        // But it's a quick fix.
        cur_str = "";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_ir->set_ir_type(IRTypeUNDERSCORECHARSET);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    } else if (cur_str == "DECIMAL_NUM") {
        cur_str = "1234567890";
        cur_ir->set_symbol_type(SymbolLit);
        cur_ir->set_ir_type(IRTypeINTEGER);
        cur_ir->set_data_affinity_type(AFFIINT);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    } else if (cur_str == "FLOAT_NUM") {
        cur_str = "123.456";
        cur_ir->set_symbol_type(SymbolLit);
        cur_ir->set_ir_type(IRTypeFLOAT);
        cur_ir->set_data_affinity_type(AFFIFLOAT);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    } else if (cur_str == "NUM") {
        cur_str = "1234567890";
        cur_ir->set_symbol_type(SymbolLit);
        cur_ir->set_ir_type(IRTypeINTEGER);
        cur_ir->set_data_affinity_type(AFFIINT);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    } else if (cur_str == "LONG_NUM") {
        cur_str = "1234567890";
        cur_ir->set_symbol_type(SymbolLit);
        cur_ir->set_ir_type(IRTypeINTEGER);
        cur_ir->set_data_affinity_type(AFFIINT);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    } else if (cur_str == "ULONGLONG_NUM") {
        cur_str = "1234567890";
        cur_ir->set_symbol_type(SymbolLit);
        cur_ir->set_ir_type(IRTypeINTEGER);
        cur_ir->set_data_affinity_type(AFFIINT);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    } else if (cur_str == "TEXT_STRING") {
        cur_str = "'string'";
        cur_ir->set_symbol_type(SymbolLit);
        cur_ir->set_ir_type(IRTypeSTRING);
        cur_ir->set_data_affinity_type(AFFISTRING);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    } else if (cur_str == "NCHAR_STRING") {
        cur_str = "'string'";
        cur_ir->set_symbol_type(SymbolLit);
        cur_ir->set_ir_type(IRTypeSTRING);
        cur_ir->set_data_affinity_type(AFFISTRING);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    } else if (cur_str == "NOT2_SYM") {
        cur_str = "NOT";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_ir->set_ir_type(IRTypeNOT2SYM);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    } else if (cur_str == "SET_VAR") {
        cur_str = ":=";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_ir->set_ir_type(IRTypeSETVAR);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    } else if (cur_str == "WITH_ROLLUP_SYM") {
        cur_str = "WITH ROLLUP";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_ir->set_ir_type(IRTypeWITHROLLUPSYM);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    } else if (cur_str == "JSON_SEPARATOR_SYM") {
        cur_str = "->>";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_ir->set_ir_type(IRTypeWITHROLLUPSYM);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    } else if (cur_str == "JSON_UNQUOTED_SEPARATOR_SYM") {
        cur_str = "->";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_ir->set_ir_type(IRTypeWITHROLLUPSYM);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    } else if (cur_str == "PARAM_MARKER") {
        cur_str = "?";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_ir->set_ir_type(IRTypePARAMMARKER);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    } else if (cur_str == "OR2_SYM") {
        cur_str = "||";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_ir->set_ir_type(IRTypeOR2SYM);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    } else if (cur_str == "IDENT_sys") {
        cur_str = "v00";
        cur_ir->set_symbol_type(SymbolIden);
        cur_ir->set_ir_type(IRTypeIDENT);
        cur_ir->set_data_type(DataTableName);
        cur_ir->set_data_flag(ContextUse);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    } else if (cur_str == "ident") {
        cur_str = "c01";
        cur_ir->set_symbol_type(SymbolIden);
        cur_ir->set_ir_type(IRTypeIDENT);
        cur_ir->set_data_type(DataColumnName);
        cur_ir->set_data_flag(ContextUse);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    } else if (cur_str == "Ident") {
        cur_str = "c01";
        cur_ir->set_symbol_type(SymbolIden);
        cur_ir->set_ir_type(IRTypeIDENT);
        cur_ir->set_data_type(DataColumnName);
        cur_ir->set_data_flag(ContextUse);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    } else if (cur_str == "END_OF_INPUT") {
        cur_str = "";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_ir->set_ir_type(IRTypeENDOFINPUT);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    }

    // The identifiers are handled in the ir_context_setup code.

    //////////////////////////////////////////////

    cur_str = replace_str(cur_str, "_SYM", "");
    cur_ir->set_str_val(cur_str);
}