//
// Created by XXX on 3/14/24.
//

#include "cockroachdb_keyword_handling.h"
#include "../../headers/utils.h"

void cockroachdb_keyword_handl(TokenNode*& cur_token)
{
    IR* cur_ir = cur_token->get_cached_ir();
    string cur_str = cur_ir->get_str_val();
    if (cur_str.find("_LA") != string::npos) {
        cur_str = replace_str(cur_str, "_LA", "");
    }

    else if (cur_str.find("NOTHING_AFTER_RETURNING") != string::npos) {
        cur_str = "NOTHING";
    }

    else if (cur_str.find("'IDENT'") != string::npos) {
        cur_str = "IDENT";
        cur_ir->set_symbol_type(SymbolIden);
        cur_ir->set_ir_type(IRTypeName);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    }

    else if (cur_str == "RESET_ALL") {
        cur_str = "RESET";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_ir->set_ir_type(IRTypeRESET);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    }

    else if (cur_str.find("INDEX_BEFORE_PAREN") != string::npos
        || cur_str.find("INDEX_BEFORE_NAME_THEN_PAREN") != string::npos
        || cur_str.find("INDEX_AFTER_ORDER_BY_BEFORE_AT") != string::npos) {
        cur_str = "INDEX";
        // cur_ir->set_str_val("INDEX");
    }

    else if (cur_str == "IDENT") {
        cur_str = "ident";
        cur_ir->set_symbol_type(SymbolIden);
        cur_ir->set_ir_type(IRTypeName);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    }

    else if (cur_str == "SCONST") {
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

    else if (cur_str == "BITCONST") {
        cur_str = "B'10010'";
        cur_ir->set_symbol_type(SymbolLit);
        cur_ir->set_ir_type(IRTypeBITCONST);
        cur_ir->set_data_affinity_type(AFFIBIT);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    }

    else if (cur_str == "substr_from") {
        cur_ir->set_symbol_type(SymbolNonTerm);
        // cur_ir->set_str_val("");
        cur_str = "";
        IR* from_tmp_ir = new IR(SymbolTerm, IRTypeFROM, string("FROM"), cur_token);
        IR* str_tmp_ir = new IR(IRTypeSCONST, string("'string'"), AFFISTRING, cur_token);

        vector<IR*> v_tmp_children_node { from_tmp_ir, str_tmp_ir };
        cur_ir->set_children_nodes(v_tmp_children_node);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    }

    else if (cur_str == "substr_for") {
        cur_ir->set_symbol_type(SymbolNonTerm);
        // cur_ir->set_str_val("");
        cur_str = "";
        IR* from_tmp_ir = new IR(SymbolTerm, IRTypeFOR, string("FOR"), cur_token);
        IR* str_tmp_ir = new IR(IRTypeSCONST, string("'string'"), AFFISTRING, cur_token);

        vector<IR*> v_tmp_children_node { from_tmp_ir, str_tmp_ir };
        cur_ir->set_children_nodes(v_tmp_children_node);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    }

    else if (cur_str == "overlay_placing") {
        cur_ir->set_symbol_type(SymbolNonTerm);
        // cur_ir->set_str_val("");
        cur_str = "";
        IR* from_tmp_ir = new IR(SymbolTerm, IRTypePLACING, string("PLACING"), cur_token);
        IR* str_tmp_ir = new IR(IRTypeSCONST, string("'string'"), AFFISTRING, cur_token);

        vector<IR*> v_tmp_children_node { from_tmp_ir, str_tmp_ir };
        cur_ir->set_children_nodes(v_tmp_children_node);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    }

    else if (cur_str == "error") {
        // cur_ir->set_str_val("");
        cur_str = "";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    }

    else if (cur_str == "CLUSTER_ALL") {
        // cur_ir->set_str_val("CLUSTER");
        cur_str = "CLUSTER";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    }

    else if (cur_str == "TENANT_ALL") {
        // cur_ir->set_str_val("TENANT");
        cur_str = "TENANT";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    }

    else if (cur_str == "SET_TRACING") {
        // cur_ir->set_str_val("SET");
        cur_str = "SET";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    }

    else if (cur_str == "generated_always_as") {
        // cur_ir->set_str_val(" GENERATED ALWAYS AS ");
        cur_str = " GENERATED ALWAYS AS ";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    }

    else if (cur_str == "GENERATED_BY_DEFAULT") {
        // cur_ir->set_str_val("GENERATED");
        cur_str = "GENERATED";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    }

    else if (cur_str == "ROLE_ALL") {
        // cur_ir->set_str_val("ROLE");
        cur_str = "ROLE";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    }

    else if (cur_str == "USER_ALL") {
        // cur_ir->set_str_val("USER");
        cur_str = "USER";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    }

    else if (cur_str == "NOT_EQUALS") {
        // cur_ir->set_str_val("!=");
        cur_str = "!=";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    }

    else if (cur_str == "NEG_INNER_PRODUCT") {
        // cur_ir->set_str_val("<#>");
        cur_str = "<#>";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    }

    else if (cur_str == "DISTANCE") {
        // cur_ir->set_str_val("->");
        cur_str = "->";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    }

    else if (cur_str == "HELPTOKEN") {
        // cur_ir->set_str_val("??");
        cur_str = "??";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    }

    else if (cur_str == "DOT_DOT") {
        // cur_ir->set_str_val("..");
        cur_str = "..";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    }

    else if (cur_str == "NOT_REGIMATCH") {
        // cur_ir->set_str_val("!~*");
        cur_str = "!~*";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    }

    else if (cur_str == "NOT_REGMATCH") {
        // cur_ir->set_str_val("!~");
        cur_str = "!~";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    }

    else if (cur_str == "JSON_SOME_EXISTS") {
        // cur_ir->set_str_val("?|");
        cur_str = "?|";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    }

    else if (cur_str == "JSON_ALL_EXISTS") {
        // cur_ir->set_str_val("?&");
        cur_str = "?&";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    }

    else if (cur_str == "INET_CONTAINED_BY_OR_EQUALS") {
        // cur_ir->set_str_val("<<=");
        cur_str = "<<=";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    }

    else if (cur_str == "LSHIFT") {
        // cur_ir->set_str_val("<<");
        cur_str = "<<";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    }

    else if (cur_str == "COS_DISTANCE") {
        // cur_ir->set_str_val("<=>");
        cur_str = "<=>";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    }

    else if (cur_str == "LESS_EQUALS") {
        // cur_ir->set_str_val("<=");
        cur_str = "<=";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    }

    else if (cur_str == "CONTAINED_BY") {
        // cur_ir->set_str_val("<@");
        cur_str = "<@";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    }

    else if (cur_str == "DISTANCE") {
        // cur_ir->set_str_val("<->");
        cur_str = "<->";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    }

    else if (cur_str == "INET_CONTAINS_OR_EQUALS") {
        // cur_ir->set_str_val(">>=");
        cur_str = ">>=";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    }

    else if (cur_str == "RSHIFT") {
        // cur_ir->set_str_val(">>");
        cur_str = ">>";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    }

    else if (cur_str == "GREATER_EQUALS") {
        // cur_ir->set_str_val(">=");
        cur_str = ">=";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    }

    else if (cur_str == "TYPEANNOTATE") {
        // cur_ir->set_str_val(":::");
        cur_str = ":::";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    }

    else if (cur_str == "TYPECAST") {
        cur_ir->set_str_val("::");
        cur_str = "::";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    }

    else if (cur_str == "CBRT") {
        // cur_ir->set_str_val("||/");
        cur_str = "||/";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    }

    else if (cur_str == "CONCAT") {
        // cur_ir->set_str_val("||");
        cur_str = "||";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    }

    else if (cur_str == "SQRT") {
        // cur_ir->set_str_val("|/");
        cur_str = "|/";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    }

    else if (cur_str == "FLOORDIV") {
        // cur_ir->set_str_val("//");
        cur_str = "//";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    }

    else if (cur_str == "REGIMATCH") {
        // cur_ir->set_str_val("~*");
        cur_str = "~*";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    }

    else if (cur_str == "CONTAINS") {
        // cur_ir->set_str_val("@>");
        cur_str = "@>";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    }

    else if (cur_str == "AT_AT") {
        // cur_ir->set_str_val("@@");
        cur_str = "@@";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    }

    else if (cur_str == "AND_AND") {
        // cur_ir->set_str_val("&&");
        cur_str = "&&";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    }

    else if (cur_str == "FETCHTEXT") {
        // cur_ir->set_str_val("->>");
        cur_str = "->>";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    }

    else if (cur_str == "FETCHVAL") {
        // cur_ir->set_str_val("->");
        cur_str = "->";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    }

    else if (cur_str == "FETCHTEXT_PATH") {
        // cur_ir->set_str_val("#>>");
        cur_str = "#>>";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    }

    else if (cur_str == "FETCHVAL_PATH") {
        // cur_ir->set_str_val("#>");
        cur_str = "#>";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    }

    else if (cur_str == "REMOVE_PATH") {
        // cur_ir->set_str_val("#-");
        cur_str = "#-";
        cur_ir->set_symbol_type(SymbolTerm);
        cur_token->set_mapped_children_prods({});
        cur_token->set_type(TypTermKeyword);
    }

    cur_ir->set_str_val(cur_str);
    //    cur_token->set_string(cur_str);
}
