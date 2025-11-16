//
// Created by XXX on 12/11/24.
//

#include "sqlite_ir_context_setup.h"
#include "sqlite_fuzzer_configurations.h"
#include <initializer_list>

#define BEGIN vector<IR*> children = cur_ir->get_children();

inline string get_random_collation_name() {
    auto rand_int = get_rand_int(100);
    if (rand_int < 33) {
        return "NOCASE";
    } else if (rand_int < 66) {
        return "BINARY";
    } else {
        return "RTRIM";
    }
}

inline void handle_joinop(IR*& cur_ir) {
    BEGIN;

    /* This is a complete rewrite. 
     * Original expression: 
     * Expr: COMMA
     * Expr: JOIN
     * Expr: JOIN_KW JOIN
     * Expr: JOIN_KW nm JOIN
     * Expr: JOIN_KW nm nm JOIN
     * For any expressions longer than 1, we need to explicitly handle the joinop. 
     * Link to the join rule: https://www.sqlite.org/syntax/join-operator.html
     */

    if (children.size() == 1) {
        return;
    }

    cur_ir->free_children();

    if (get_pct_hit(20)) {
        // CROSS JOIN
        IR* join_op_0 = new IR(SymbolTerm, IRTypeUnknownType, "CROSS", nullptr, nullptr, nullptr);
        cur_ir->add_one_child(join_op_0, 0);

        IR* join_op_1 = new IR(SymbolTerm, IRTypeUnknownType, "JOIN", nullptr, nullptr, nullptr);
        cur_ir->add_one_child(join_op_1, 1);
        return;
    }

    string natural_str = get_pct_hit(50) ? "NATURAL" : "";
    IR* join_op_0 = new IR(SymbolTerm, IRTypeUnknownType, natural_str, nullptr, nullptr, nullptr);

    cur_ir->add_one_child(join_op_0, 0);

    int rand_int = get_rand_int(100);
    bool is_possible_outer = false;
    if (rand_int < 25) {
        // <NATURAL> LEFT <OUTER> JOIN
        is_possible_outer = true;
        IR* join_op_1 = new IR(SymbolTerm, IRTypeUnknownType, "LEFT", nullptr, nullptr, nullptr);
        cur_ir->add_one_child(join_op_1, 1);
    } else if (rand_int < 50) {
        // <NATURAL> RIGHT <OUTER> JOIN
        is_possible_outer = true;
        IR* join_op_1 = new IR(SymbolTerm, IRTypeUnknownType, "RIGHT", nullptr, nullptr, nullptr);
        cur_ir->add_one_child(join_op_1, 1);
    } else if (rand_int < 75) {
        // <NATURAL> FULL <OUTER> JOIN
        is_possible_outer = true;
        IR* join_op_1 = new IR(SymbolTerm, IRTypeUnknownType, "FULL", nullptr, nullptr, nullptr);
        cur_ir->add_one_child(join_op_1, 1);
    } else {
        // <NATURAL> INNER JOIN
        is_possible_outer = false;
        IR* join_op_1 = new IR(SymbolTerm, IRTypeUnknownType, "INNER", nullptr, nullptr, nullptr);
        cur_ir->add_one_child(join_op_1, 1);
    }

    if (is_possible_outer && get_pct_hit(50)) {
        // Whether to add OUTER. 
        IR* join_op_2 = new IR(SymbolTerm, IRTypeUnknownType, "OUTER", nullptr, nullptr, nullptr);
        cur_ir->add_one_child(join_op_2, 2);
    }

    // The LAST JOIN keyword. 
    IR* join_op_3 = new IR(SymbolTerm, IRTypeUnknownType, "JOIN", nullptr, nullptr, nullptr);
    cur_ir->add_one_child(join_op_3, cur_ir->get_children_ref().size());

}

inline void handle_as(IR*& cur_ir, DATATYPE data_type, DATAFLAG data_flag) {
    BEGIN;

    for (auto* child : children) {
        if (child->get_ir_type() == IRTypeNm) {
            IR* new_name = new IR(IRTypeIDENT, "c01", data_type, data_flag, child->get_mapped_token_node());
            cur_ir->swap_one_child(child, new_name);
        } else if (child->get_ir_type() == IRTypeids) {
            IR* new_name = new IR(IRTypeIDENT, "c01", data_type, data_flag, child->get_mapped_token_node());
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_defer_subclause(IR*& cur_ir) {
    BEGIN;

    for (auto* child : children) {
        if (child->get_ir_type() == IRTypeids) {
            IR* new_name = new IR(IRTypeIDENT, get_random_collation_name(), DataCollationName, ContextUse, child->get_mapped_token_node());
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_nm(IR*& cur_ir) {
    BEGIN;

    // Just placeholder. 
    if (children.size() == 1) {
        IR* new_name = new IR(IRTypeIDENT, "v00", DataTableName, ContextUse, children.back()->get_mapped_token_node());
        cur_ir->swap_one_child(children.back(), new_name);
    }
}

inline void handle_table_option(IR*& cur_ir) {
    BEGIN;

    // WITHOUT ROWID
    if (children.size() == 2) {
        IR* new_name = new IR(SymbolTerm, IRTypeUnknownType, "ROWID", nullptr, nullptr, nullptr);
        cur_ir->swap_one_child(children.back(), new_name);
    } else {
        // STRICT (size == 1)
        IR* new_name = new IR(SymbolTerm, IRTypeUnknownType, "STRICT", nullptr, nullptr, nullptr);
        cur_ir->swap_one_child(children.back(), new_name);
    }
}

inline void handle_create_table(IR*& cur_ir) {
    BEGIN;

    for (auto* child : children) {
        if (child->get_ir_type() == IRTypeNm) {
            IR* new_name = new IR(IRTypeIDENT, "v00", DataTableName, ContextDefine, child->get_mapped_token_node());
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_trans_opt(IR*& cur_ir, DATAFLAG data_flag)
{
    BEGIN;

    for (auto* child : children) {
        if (child->get_ir_type() == IRTypeNm) {
            IR* new_name = new IR(IRTypeIDENT, "transaction_name_0", DataTransactionName, data_flag, child->get_mapped_token_node());
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_indexed_by(IR*& cur_ir) {
    BEGIN;

    for (auto* child : children) {
        if (child->get_ir_type() == IRTypeNm) {
            IR* new_name = new IR(IRTypeIDENT, "i03", DataIndexName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_selcollist(IR*& cur_ir) {
    BEGIN;

    // Only handling: sclp scanpt nm DOT STAR
    // Column is handled by expr. 
    // Expr: sclp scanpt nm DOT STAR
    for (auto* child : children) {
        if (child->get_ir_type() == IRTypeNm) {
            IR* new_name = new IR(IRTypeIDENT, "v00", DataTableName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }

        else if (child->get_ir_type() == IRTypeAs) {
            handle_as(child, DataColumnAliasName, ContextDefine);
        }
    }

    // Ignored expres: 
    // Expr: sclp scanpt expr scanpt as
    // Expr: sclp scanpt STAR
}

inline void handle_eidlist(IR*& cur_ir, DATATYPE data_type, DATAFLAG data_flag) {
    BEGIN;

    //eidlist::= eidlist COMMA nm collate sortorder .
    //eidlist::= nm collate sortorder .

    for (auto* child : children) {
        if (child->get_ir_type() == IRTypeNm) {
            IR* new_name = new IR(IRTypeIDENT, "c01", data_type, data_flag, child->get_mapped_token_node());
            cur_ir->swap_one_child(child, new_name);
        }
        else if (child->get_ir_type() == IRTypeEidlist) {
            handle_eidlist(child, data_type, data_flag);
        }
    }
}

inline void handle_eidlist_opt(IR*& cur_ir, DATATYPE data_type, DATAFLAG data_flag) {
    BEGIN;

    // eidlist_opt::= LP eidlist RP .
    // eidlist_opt::= .

    for (auto* child : children) {
        if (child->get_ir_type() == IRTypeEidlist) {
            handle_eidlist(child, data_type, data_flag);
        }
    }
}

inline void handle_idlist(IR*& cur_ir, DATATYPE data_type, DATAFLAG data_flag) {
    BEGIN;

    for (auto* child : children) {
        if (child->get_ir_type() == IRTypeNm) {
            IR* new_name = new IR(IRTypeIDENT, "c01", data_type, data_flag, child->get_mapped_token_node());
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_idlist_opt(IR*& cur_ir, DATATYPE data_type, DATAFLAG data_flag) {
    BEGIN;

    for (auto* child : children) {
        if (child->get_ir_type() == IRTypeIdlist) {
            handle_idlist(child, data_type, data_flag);
        }
    }
}

inline void handle_setlist(IR*& cur_ir) {
    BEGIN;

    for (auto* child : children) {
        if (child->get_ir_type() == IRTypeNm) {
            IR* new_name = new IR(IRTypeIDENT, "c01", DataColumnName, ContextUse, nullptr);
            cur_ir->swap_one_child(child, new_name);
        } else if (child->get_ir_type() == IRTypeIdlist) {
            handle_idlist(child, DataColumnName, ContextUse);
        }
    }
}

inline void handle_dbnm(IR*& cur_ir) {
    BEGIN;
    cur_ir->free_children();
}

inline void handle_on_using(IR*& cur_ir) {
    BEGIN;

    for (auto* child : children) {
        if (child->get_ir_type() == IRTypeIdlist) {
            handle_idlist(child, DataColumnName, ContextUse);
        }
    }
}

inline void handle_stl_prefix(IR*& cur_ir, DATAFLAG data_flag);

inline void handle_seltablist(IR*& cur_ir, DATAFLAG data_flag) {
    BEGIN;

    // seltablist::= stl_prefix nm dbnm as on_using .
    // seltablist::= stl_prefix nm dbnm as indexed_by on_using .
    // seltablist::= stl_prefix nm dbnm LP exprlist RP as on_using .
    // seltablist::= stl_prefix LP select RP as on_using .
    // seltablist::= stl_prefix LP seltablist RP as on_using .

    for (auto* child : children) {
        if (child->get_ir_type() == IRTypeNm) {
            IR* new_name = new IR(IRTypeIDENT, "v00", DataTableName, data_flag, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
        else if (child->get_ir_type() == IRTypeAs) {
            if (data_flag == ContextUse) {
                handle_as(child, DataTableAliasName, ContextDefine);
            } else {
                child->free_children();
            }
        }
        else if (child->get_ir_type() == IRTypeSeltablist) {
            handle_seltablist(child, data_flag);
        }
        else if (child->get_ir_type() == IRTypeStlPrefix) {
            handle_stl_prefix(child, data_flag);
        }
    }
}


inline void handle_stl_prefix(IR*& cur_ir, DATAFLAG data_flag = ContextUse) {
    BEGIN;

    // stl_prefix::= seltablist joinop .
    // stl_prefix::= .

    for (auto* child : children) {
        if (child->get_ir_type() == IRTypeSeltablist) {
            handle_seltablist(child, data_flag);
        }
    }
}

inline void handle_trigger_event(IR*& cur_ir) {
    BEGIN;

    for (auto* child : children) {
        if (child->get_ir_type() == IRTypeIdlist) {
            handle_idlist(child, DataColumnName, ContextUse);
        }
    }
}

inline void handle_trigger_decl(IR*& cur_ir) {
    BEGIN;

    // ONLY ONE RULE. 

    for (auto* child : children) {
        if (child->get_ir_type() == IRTypeNm) {
            IR* new_name = new IR(IRTypeIDENT, "trigger_name_0", DataTriggerName, ContextDefine, child->get_mapped_token_node());
            cur_ir->swap_one_child(child, new_name);
        } else if (child->get_ir_type() == IRTypeFullname) {
            IR* new_name = new IR(IRTypeIDENT, "v00", DataTableName, ContextUse, child->get_mapped_token_node());
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_trnm(IR*& cur_ir, DATATYPE data_type, DATAFLAG data_flag) {
    BEGIN;

    IR* new_name = new IR(IRTypeIDENT, "trigger_name_0", data_type, data_flag, children.back()->get_mapped_token_node());
    cur_ir->free_children();
    cur_ir->add_one_child(new_name, 0);
}

inline void handle_tridxby(IR*& cur_ir) {
    BEGIN;

    for (auto* child : children) {
        if (child->get_ir_type() == IRTypeNm) {
            IR* new_name = new IR(IRTypeIDENT, "index_0", DataIndexName, ContextUse, child->get_mapped_token_node());
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_trigger_cmd(IR*& cur_ir) {
    BEGIN;

    for (auto* child : children) {
        if (child->get_ir_type() == IRTypeTrnm) {
            handle_trnm(child, DataTableName, ContextUse);
        }
        // No need to handle tridxby.
        else if (child->get_ir_type() == IRTypeIdlistOpt) {
            handle_idlist_opt(child, DataColumnName, ContextUse);
        }
    }
}

inline void handle_columnname(IR*& cur_ir) {
    BEGIN;

    // This is actually only used in the column creation. 

    for (auto* child : children) {
        if (child->get_ir_type() == IRTypeNm) {
            IR* new_name = new IR(IRTypeIDENT, "c01", DataColumnName, ContextDefine, child->get_mapped_token_node());
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_tcons(IR*& cur_ir) {
    BEGIN;

    // Expr: CONSTRAINT nm
    if (children.front()->get_ir_type() == IRTypeCONSTRAINT) {
        IR* new_name = new IR(IRTypeIDENT, "constraint_name_0", DataConstraintName, ContextDefine, children.back()->get_mapped_token_node());
        cur_ir->swap_one_child(children.back(), new_name);
    }

    // Expr: FOREIGN KEY LP eidlist RP REFERENCES nm eidlist_opt refargs defer_subclause_opt
    if (children.front()->get_ir_type() == IRTypeFOREIGN) {
        for (auto* child : children) {
            if (child->get_ir_type() == IRTypeNm) {
                IR* new_name = new IR(IRTypeIDENT, "v00", DataTableName, ContextUse, child->get_mapped_token_node());
                cur_ir->swap_one_child(child, new_name);
            }

            else if (child->get_ir_type() == IRTypeEidlist) {
                handle_eidlist(child, DataColumnName, ContextUse);
            }

            else if (child->get_ir_type() == IRTypeEidlistOpt) {
                handle_eidlist_opt(child, DataColumnName, ContextUse);
            }
        }
    }

    // No need to handle. 
    // Expr: PRIMARY KEY LP sortlist autoinc RP onconf
    // Expr: UNIQUE LP sortlist RP onconf
    // Expr: CHECK LP expr RP onconf

}



inline void handle_limit_opt(IR*& cur_ir) {
    // Empty. No need to handle.
    // Column names are handled by expr. 
}

inline void handle_sortlist(IR*& cur_ir) {
    // Empty. No need to handle.
    // Column names are handled by expr. 
}

inline void handle_upsert(IR*& cur_ir) {
    // Empty. No need to handle.
}

inline void handle_create_vtab(IR*& cur_ir) {
    // Expr: createkw VIRTUAL TABLE ifnotexists nm dbnm USING nm
    BEGIN;

    IR* new_name = new IR(IRTypeIDENT, "v00", DataTableName, ContextDefine, children[4]->get_mapped_token_node());
    cur_ir->swap_one_child(children[4], new_name);

    if (get_pct_hit(50)) {
        new_name = new IR(IRTypeIDENT, "rtree", DataVirtualTableModuleName, ContextUse, children.back()->get_mapped_token_node());
        cur_ir->swap_one_child(children.back(), new_name);
    } else {
        if (get_pct_hit(50)) {
            new_name = new IR(IRTypeIDENT, "fts4", DataVirtualTableModuleName, ContextUse, children.back()->get_mapped_token_node());
            cur_ir->swap_one_child(children.back(), new_name);
        } else {
            new_name = new IR(IRTypeIDENT, "fts5", DataVirtualTableModuleName, ContextUse, children.back()->get_mapped_token_node());
            cur_ir->swap_one_child(children.back(), new_name);
        }
    }
}

inline void handle_vtabarglist(IR*& cur_ir) {
    BEGIN;

    cur_ir->free_children();

    auto* col_0 = new IR(IRTypeIDENT, "c01", DataColumnName, ContextDefine, nullptr);
    auto* col_1 = new IR(IRTypeIDENT, "c02", DataColumnName, ContextDefine, nullptr);
    auto* col_2 = new IR(IRTypeIDENT, "c03", DataColumnName, ContextDefine, nullptr);
    cur_ir->add_one_child(col_0, 0);
    cur_ir->add_one_child(col_1, 1);
    cur_ir->add_one_child(col_2, 2);
}

inline void handle_nmnum(IR*& cur_ir, DATATYPE data_type, DATAFLAG data_flag) {
    BEGIN;

    for (auto* child : children) {
        if (child->get_ir_type() == IRTypeNm) {
            IR* new_name = new IR(IRTypeIDENT, "c1", data_type, data_flag, child->get_mapped_token_node());
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_add_column_fullname(IR*& cur_ir, DATATYPE data_type, DATAFLAG data_flag) {
    BEGIN;

    for (auto* child : children) {
        if (child->get_ir_type() == IRTypeFullname) {
            IR* new_name = new IR(IRTypeIDENT, "v00", data_type, data_flag, child->get_mapped_token_node());
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_refarg(IR*& cur_ir) {
    BEGIN;

    for (auto* child : children) {
        if (child->get_ir_type() == IRTypeNm) {
            string match_string;
            int random_number = get_rand_int(100);
            if (random_number < 33) {
                match_string = "FULL";
            } else if (random_number < 66) {    
                match_string = "SIMPLE";
            } else {
                match_string = "PARTIAL";
            }
            IR* new_name = new IR(SymbolTerm, IRTypeUnknownType, match_string, nullptr, nullptr, nullptr);
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_generated(IR*& cur_ir) {
    BEGIN;

    for (auto* child : children) {
        if (child->get_ir_type() == IRTypeid) {
            if (get_pct_hit(50)) {
                IR* new_name = new IR(SymbolTerm, IRTypeVIRTUAL, "VIRTUAL", nullptr, nullptr, nullptr);
                cur_ir->swap_one_child(child, new_name);
            } else {
                IR* new_name = new IR(SymbolTerm, IRTypeUnknownType, "STORED", nullptr, nullptr, nullptr);
                cur_ir->swap_one_child(child, new_name);
            }
        }
    }
}

inline void handle_withnm(IR*& cur_ir) {
    BEGIN;

    for (auto* child : children) {
        if (child->get_ir_type() == IRTypeNm) {
            IR* new_name = new IR(IRTypeIDENT, "va_03", DataTableAliasName, ContextDefine, child->get_mapped_token_node());
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_typetoken(IR*& cur_ir) {
    BEGIN;

    // For all expressions: 
    // Expr: <empty>
    // Expr: typename
    // Expr: typename LP signed RP
    // Expr: typename LP signed COMMA signed RP
    for (auto* child : children) {
        if (child->get_ir_type() == IRTypeTypename) {
            IR* new_name = new IR(IRTypeIDENT, "INT", DataTypeName, ContextUse, child->get_mapped_token_node());
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_ccons(IR*& cur_ir) {
    BEGIN;

    if (children.front()->get_ir_type() == IRTypeCONSTRAINT) {
        // Expr: CONSTRAINT nm
        IR* new_name = new IR(IRTypeIDENT, "constraint_name_0", DataConstraintName, ContextDefine, children.back()->get_mapped_token_node());
        cur_ir->swap_one_child(children.back(), new_name);
    }

    else if (children.front()->get_ir_type() == IRTypeDEFAULT) {
        // Expr: DEFAULT scantok term
        // Expr: DEFAULT LP expr RP
        // Expr: DEFAULT PLUS scantok term
        // Expr: DEFAULT MINUS scantok term
        // Expr: DEFAULT scantok id

        // No need to handle term and scantok. 
        for (auto* child : children) {
            if (child->get_ir_type() == IRTypeid) {
                // FIXME:: Not sure if this is correct. 
                IR* new_name = new IR(IRTypeIDENT, "c01", DataColumnName, ContextUse, child->get_mapped_token_node());
                cur_ir->swap_one_child(child, new_name);
            }
        }
    }

    // Expr: REFERENCES nm eidlist_opt refargs
    else if (children.front()->get_ir_type() == IRTypeREFERENCES) {
        for (auto* child : children) {
            if (child->get_ir_type() == IRTypeNm) {
                IR* new_name = new IR(IRTypeIDENT, "v00", DataTableName, ContextUse, child->get_mapped_token_node());
                cur_ir->swap_one_child(child, new_name);
            }
            else if (child->get_ir_type() == IRTypeEidlistOpt) {
                handle_eidlist_opt(child, DataColumnName, ContextUse);
            }
        }
    }

    // Expr: COLLATE ids
    else if (children.front()->get_ir_type() == IRTypeCollate) {
        for (auto* child : children) {
            if (child->get_ir_type() == IRTypeid) {
                IR* new_name = new IR(IRTypeIDENT, get_random_collation_name(), DataCollationName, ContextUse, child->get_mapped_token_node());
                cur_ir->swap_one_child(child, new_name);
            }
        }
    }

    // Ignore expres:
    // Expr: NULL onconf
    // Expr: NOT NULL onconf
    // Expr: PRIMARY KEY sortorder onconf autoinc
    // Expr: UNIQUE onconf
    // Expr: CHECK LP expr RP
    // Expr: defer_subclause
}

inline void handle_wqitem(IR*& cur_ir) {
    BEGIN;

    // Expr: withnm eidlist_opt wqas LP select RP
    // wqas and withnm don't need to be handled. 
    for (auto* child : children) {
        if (child->get_ir_type() == IRTypeEidlistOpt) {
            handle_eidlist_opt(child, DataColumnAliasName, ContextUse);
        }
    }
}

inline void handle_cmd(IR*& cur_ir) {
    BEGIN;

    // Expr: BEGIN transtype trans_opt
    if (children.size() == 3 && children[0]->get_ir_type() == IRTypeBEGIN && children.back()->get_ir_type() == IRTypeTransOpt) {
        handle_trans_opt(children.back(), ContextDefine);

    // Expr: COMMIT trans_opt
    } else if (children.size() == 2 && children[0]->get_ir_type() == IRTypeCOMMIT) {
        handle_trans_opt(children[1], ContextUse);

    // Expr: END trans_opt
    } else if (children.size() == 2 && children[0]->get_ir_type() == IRTypeEND) {
        handle_trans_opt(children[1], ContextUndefine);

    // Expr: ROLLBACK trans_opt
    } else if (children.size() == 2 && children[0]->get_ir_type() == IRTypeROLLBACK) {
        handle_trans_opt(children.back(), ContextUse);

    // Expr: SAVEPOINT nm
    } else if (children.size() == 2 && children[0]->get_ir_type() == IRTypeSAVEPOINT) {
        IR* new_name = new IR(IRTypeIDENT, "checkpoint_name_0", DataCheckPointName, ContextDefine, nullptr);
        cur_ir->swap_one_child(children[1], new_name);

    // Expr: RELEASE savepoint_opt nm
    } else if (children.size() == 3 && children[0]->get_ir_type() == IRTypeRELEASE && children[1]->get_ir_type() == IRTypeSavepointOpt) {
        IR* new_name = new IR(IRTypeIDENT, "checkpoint_name_0", DataCheckPointName, ContextUse, nullptr);
        cur_ir->swap_one_child(children.back(), new_name);

    // Expr: ROLLBACK trans_opt TO savepoint_opt nm
    } else if (children.size() == 5 && children[0]->get_ir_type() == IRTypeROLLBACK && children.back()->get_ir_type() == IRTypeNm && children[1]->get_ir_type() == IRTypeTransOpt) {
        IR* new_name = new IR(IRTypeIDENT, "checkpoint_name_0", DataCheckPointName, ContextUse, nullptr);
        cur_ir->swap_one_child(children.back(), new_name);

        handle_trans_opt(children[1], ContextUse);

    // Expr: DROP TABLE ifexists fullname
    } else if (children.size() > 3 && children[0]->get_ir_type() == IRTypeDROP && children[1]->get_ir_type() == IRTypeTABLE && children.back()->get_ir_type() == IRTypeFullname) {
        IR* new_name = new IR(IRTypeIDENT, "v00", DataTableName, ContextUndefine, children.back()->get_mapped_token_node());
        cur_ir->swap_one_child(children.back(), new_name);


    // Expr: createkw temp VIEW ifnotexists nm dbnm eidlist_opt AS select
    } else if (children.size() > 6 && children.front()->get_ir_type() == IRTypeCreatekw && children[2]->get_ir_type() == IRTypeVIEW) {
        for (auto* child : children) {
            if (child->get_ir_type() == IRTypeNm) {
                IR* new_name = new IR(IRTypeIDENT, "view_name_0", DataViewName, ContextDefine, child->get_mapped_token_node());
                cur_ir->swap_one_child(child, new_name);
            } else if (child->get_ir_type() == IRTypeEidlistOpt) {
                handle_eidlist_opt(child, DataViewColumnName, ContextDefine);
            }
        }

    // Expr: DROP VIEW ifexists fullname
    } else if (children.size() == 4 && children.front()->get_ir_type() == IRTypeDROP && children[1]->get_ir_type() == IRTypeVIEW) {
        for (auto* child : children) {
            if (child->get_ir_type() == IRTypeFullname) {
                IR* new_name = new IR(IRTypeIDENT, "view_name_0", DataViewName, ContextUndefine, child->get_mapped_token_node());
                cur_ir->swap_one_child(child, new_name);
            }
        }

    // Expr: with DELETE FROM xfullname indexed_opt where_opt_ret orderby_opt limit_opt
    } else if (children.size() > 6 && children[1]->get_ir_type() == IRTypeDELETE && children[2]->get_ir_type() == IRTypeFROM) {
        for (auto* child : children) {
            if (child->get_ir_type() == IRTypeXfullname) {
                IR* new_name = new IR(IRTypeIDENT, "v00", DataTableName, ContextUse, child->get_mapped_token_node());
                cur_ir->swap_one_child(child, new_name);
            }
        }

    // Expr: with UPDATE orconf xfullname indexed_opt SET setlist from where_opt_ret orderby_opt limit_opt
    } else if (children.size() > 6 && children[1]->get_ir_type() == IRTypeUPDATE && children[2]->get_ir_type() == IRTypeOrconf) {
        for (auto* child : children) {
            if (child->get_ir_type() == IRTypeXfullname) {
                IR* new_name = new IR(IRTypeIDENT, "v00", DataTableName, ContextUse, child->get_mapped_token_node());
                cur_ir->swap_one_child(child, new_name);
            }
        }

        // Expr: with insert_cmd INTO xfullname idlist_opt select upsert
        // Expr: with insert_cmd INTO xfullname idlist_opt DEFAULT VALUES returning
    } else if (children.size() > 6 && children[1]->get_ir_type() == IRTypeInsertCmd && children[2]->get_ir_type() == IRTypeINTO) {
        for (auto* child : children) {
            if (child->get_ir_type() == IRTypeXfullname) {
                IR* new_name = new IR(IRTypeIDENT, "v00", DataTableName, ContextUse, child->get_mapped_token_node());
                cur_ir->swap_one_child(child, new_name);
            } else if (child->get_ir_type() == IRTypeIdlistOpt) {
                handle_idlist_opt(child, DataColumnName, ContextUse);
            }
        }

        // Expr: createkw uniqueflag INDEX ifnotexists nm dbnm ON nm LP sortlist RP where_opt
    } else if (children.size() > 10 && children.front()->get_ir_type() == IRTypeCreatekw && children[1]->get_ir_type() == IRTypeUniqueflag && children[2]->get_ir_type() == IRTypeINDEX) {
        if (children[4]->get_ir_type() != IRTypeNm || children[7]->get_ir_type() != IRTypeNm) {
            cerr << "Error: Failed to match rule: createkw uniqueflag INDEX ifnotexists nm dbnm ON nm LP sortlist RP where_opt" << endl;
            abort();
        }

        IR* new_name = new IR(IRTypeIDENT, "index_0", DataIndexName, ContextDefine, children[4]->get_mapped_token_node());
        cur_ir->swap_one_child(children[4], new_name);

        new_name = new IR(IRTypeIDENT, "v00", DataTableName, ContextUse, children[7]->get_mapped_token_node());
        cur_ir->swap_one_child(children[7], new_name);

    // Expr: DROP INDEX ifexists fullname
    } else if (children.size() == 4 && children.front()->get_ir_type() == IRTypeDROP && children[1]->get_ir_type() == IRTypeINDEX) {
        for (auto* child : children) {
            if (child->get_ir_type() == IRTypeFullname) {
                IR* new_name = new IR(IRTypeIDENT, "index_0", DataIndexName, ContextUndefine, child->get_mapped_token_node());
                cur_ir->swap_one_child(child, new_name);
            }
        }

    // Expr: VACUUM vinto
    // Expr: VACUUM nm vinto
    } else if (children.size() >= 2 && children.front()->get_ir_type() == IRTypeVACUUM) {
        for (auto* child : children) {
            if (child->get_ir_type() == IRTypeNm) {
                IR* new_name = new IR(IRTypeIDENT, "main", DataDatabaseName, ContextUse, child->get_mapped_token_node());
                cur_ir->swap_one_child(child, new_name);
            }
        }

    // Expr: PRAGMA nm dbnm
    // Expr: PRAGMA nm dbnm EQ nmnum
    // Expr: PRAGMA nm dbnm LP nmnum RP
    // Expr: PRAGMA nm dbnm EQ minus_num
    // Expr: PRAGMA nm dbnm LP minus_num RP
    } else if (children.size() > 2 && (children.front()->get_ir_type() == IRTypePRAGMA || children.front()->get_ir_type() == IRTypeVACUUM)) {
        for (auto* child : children) {
            if (child->get_ir_type() == IRTypeNm) {
                IR* new_name = new IR(IRTypeIDENT, "pragma_name_0", DataPragmaName, ContextUse, child->get_mapped_token_node());
                cur_ir->swap_one_child(child, new_name);
            } else if (child->get_ir_type() == IRTypeNmnum) {
                handle_nmnum(child, DataPragmaValue, ContextUse);
            }
        }

        // Expr: createkw trigger_decl BEGIN trigger_cmd_list END
    } else if (children.size() > 3 && children.front()->get_ir_type() == IRTypeCreatekw && children[1]->get_ir_type() == IRTypeTriggerDecl) {
        // No need to handle.


        // Expr: DROP TRIGGER ifexists fullname
    } else if (children.size() == 4 && children.front()->get_ir_type() == IRTypeDROP && children[1]->get_ir_type() == IRTypeTRIGGER) {
        for (auto* child : children) {
            if (child->get_ir_type() == IRTypeFullname) {
                IR* new_name = new IR(IRTypeIDENT, "trigger_name_0", DataTriggerName, ContextUndefine, child->get_mapped_token_node());
                cur_ir->swap_one_child(child, new_name);
            }
        }

        // Expr: REINDEX nm dbnm
        // Expr: ANALYZE
        // Expr: ANALYZE nm dbnm
    } else if (children.size() > 2 && (children.front()->get_ir_type() == IRTypeREINDEX || children.front()->get_ir_type() == IRTypeANALYZE)) {
        for (auto* child : children) {
            if (child->get_ir_type() == IRTypeNm) {
                IR* new_name = new IR(IRTypeIDENT, "v00", DataTableName, ContextUse, child->get_mapped_token_node());
                cur_ir->swap_one_child(child, new_name);
            }
        }

        // Expr: ALTER TABLE fullname RENAME TO nm
    } else if (children.size() == 6 && children.front()->get_ir_type() == IRTypeALTER && children[1]->get_ir_type() == IRTypeTABLE && children[2]->get_ir_type() == IRTypeFullname && children[3]->get_ir_type() == IRTypeRENAME && children[4]->get_ir_type() == IRTypeTO && children[5]->get_ir_type() == IRTypeNm) {
        for (auto* child : children) {
            if (child->get_ir_type() == IRTypeFullname) {
                IR* new_name = new IR(IRTypeIDENT, "v00", DataTableName, ContextReplaceUndefine, child->get_mapped_token_node());
                cur_ir->swap_one_child(child, new_name);
            } else if (child->get_ir_type() == IRTypeNm) {
                IR* new_name = new IR(IRTypeIDENT, "v00", DataTableName, ContextReplaceDefine, child->get_mapped_token_node());
                cur_ir->swap_one_child(child, new_name);
            }
        }

        // Expr: ALTER TABLE add_column_fullname ADD kwcolumn_opt columnname carglist
    } else if (children.size() == 7 && children.front()->get_ir_type() == IRTypeALTER && children[1]->get_ir_type() == IRTypeTABLE && children[2]->get_ir_type() == IRTypeAddColumnFullname && children[3]->get_ir_type() == IRTypeADD && children[4]->get_ir_type() == IRTypeKwcolumnOpt && children[5]->get_ir_type() == IRTypeColumnname) {
        for (auto* child : children) {
            if (child->get_ir_type() == IRTypeAddColumnFullname) {
                handle_add_column_fullname(child, DataTableName, ContextUse);
            }
        }

    // Expr: ALTER TABLE fullname DROP kwcolumn_opt nm
    } else if (children.size() == 6 && children.front()->get_ir_type() == IRTypeALTER && children[1]->get_ir_type() == IRTypeTABLE && children[2]->get_ir_type() == IRTypeFullname && children[3]->get_ir_type() == IRTypeDROP && children[4]->get_ir_type() == IRTypeKwcolumnOpt && children[5]->get_ir_type() == IRTypeNm) {

        IR* new_name = new IR(IRTypeIDENT, "c01", DataColumnName, ContextUndefine, children.back()->get_mapped_token_node());
        cur_ir->swap_one_child(children.back(), new_name);

        new_name = new IR(IRTypeIDENT, "v00", DataTableName, ContextUse, children[2]->get_mapped_token_node());
        cur_ir->swap_one_child(children[2], new_name);

        // Expr: ALTER TABLE fullname RENAME kwcolumn_opt nm TO nm
    } else if (children.size() == 8 && children.front()->get_ir_type() == IRTypeALTER && children[1]->get_ir_type() == IRTypeTABLE && children[2]->get_ir_type() == IRTypeFullname && children[3]->get_ir_type() == IRTypeRENAME && children[4]->get_ir_type() == IRTypeKwcolumnOpt && children[5]->get_ir_type() == IRTypeNm && children[6]->get_ir_type() == IRTypeTO && children[7]->get_ir_type() == IRTypeNm) {
        IR* new_name = new IR(IRTypeIDENT, "c01", DataColumnName, ContextReplaceDefine, children.back()->get_mapped_token_node());
        cur_ir->swap_one_child(children.back(), new_name);

        new_name = new IR(IRTypeIDENT, "c01", DataColumnName, ContextReplaceUndefine, children[5]->get_mapped_token_node());
        cur_ir->swap_one_child(children[5], new_name);

        new_name = new IR(IRTypeIDENT, "v00", DataTableName, ContextUse, children[2]->get_mapped_token_node());
        cur_ir->swap_one_child(children[2], new_name);
    }


    // Ignore rules. 
    // Expr: ATTACH database_kw_opt expr AS expr key_opt
    // Expr: DETACH database_kw_opt expr
    // Expr: REINDEX
    // Expr: create_vtab

}

inline void handle_oneselect(IR*& cur_ir) {
    // Everything handled by itself.

    // Expr: SELECT distinct selcollist from where_opt groupby_opt having_opt orderby_opt limit_opt
    // Expr: SELECT distinct selcollist from where_opt groupby_opt having_opt window_clause orderby_opt limit_opt
    // Expr: values
    // Expr: mvalues
}

inline void handle_windowdefn(IR*& cur_ir) {
    BEGIN;

    // Expr: nm AS LP window RP
    // window is handled by itself.
    for (auto* child : children) {
        if (child->get_ir_type() == IRTypeNm) {
            IR* new_name = new IR(IRTypeIDENT, "window_0", DataWindowName, ContextDefine, child->get_mapped_token_node());
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_window(IR*& cur_ir) {
    BEGIN;

    // For all expressions: 
    // Expr: PARTITION BY nexprlist orderby_opt frame_opt
    // Expr: nm PARTITION BY nexprlist orderby_opt frame_opt
    // Expr: ORDER BY sortlist frame_opt
    // Expr: nm ORDER BY sortlist frame_opt
    // Expr: frame_opt
    // Expr: nm frame_opt

    for (auto* child : children) {
        if (child->get_ir_type() == IRTypeNm) {
            IR* new_name = new IR(IRTypeIDENT, "window_0", DataWindowName, ContextUse, child->get_mapped_token_node());
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_over_clause(IR*& cur_ir) {
    BEGIN;

    for (auto* child : children) {
        if (child->get_ir_type() == IRTypeNm) {
            IR* new_name = new IR(IRTypeIDENT, "window_0", DataWindowName, ContextUse, child->get_mapped_token_node());
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_expr(IR*& cur_ir) {
    BEGIN;
    if (children.front()->get_ir_type() == IRTypeidj) {
        // Expr: idj LP distinct exprlist RP
        // Expr: idj LP distinct exprlist ORDER BY sortlist RP
        // Expr: idj LP STAR RP
        // Expr: idj LP distinct exprlist RP filter_over
        // Expr: idj LP distinct exprlist ORDER BY sortlist RP filter_over
        // Expr: idj LP STAR RP filter_over
        IR* new_name = new IR(IRTypeIDENT, "func_name_any()", DataFunctionExpr, ContextUse, children.front()->get_mapped_token_node());
        cur_ir->swap_one_child(children.front(), new_name);
        cur_ir->set_ir_type(IRTypeExpr);

        // Remove anything between LP and RP.
        vector<IR*> new_children;
        bool is_in_lp = false;
        for (auto* child : cur_ir->get_children()) {
            if (child->get_ir_type() == IRTypeLP) {
                is_in_lp = true;
                child->deep_drop();
            } else if (child->get_ir_type() == IRTypeRP) {
                is_in_lp = false;
                child->deep_drop();
            } else if (!is_in_lp) {
                new_children.push_back(child);
            } else {
                child->deep_drop();
            }
        }

        cur_ir->set_children_nodes(new_children);
        return;
    }

    for (auto* child : children) {
        if (child->get_ir_type() == IRTypeidj) {
            // idj related.
            if (children.size() == 1) {
                // Expr: idj
                IR* new_name = new IR(IRTypeIDENT, "c01", DataColumnName, ContextUse, child->get_mapped_token_node());
                cur_ir->swap_one_child(child, new_name);
                break;
            }
        } else if (child->get_ir_type() == IRTypeTypetoken) {
            // Expr: CAST LP expr AS typetoken RP
            IR* new_name = new IR(IRTypeIDENT, "INT", DataTypeName, ContextUse, child->get_mapped_token_node());
            cur_ir->swap_one_child(child, new_name);
            return;
        }
    }

    if (children.size() >= 3 && children[1]->get_ir_type() == IRTypeDOT) {
        // Expr: nm DOT nm
        // Expr: nm DOT nm DOT nm
        cur_ir->free_children();
        IR* new_name = new IR(IRTypeIDENT, "c01", DataColumnName, ContextUse, nullptr);
        cur_ir->add_one_child(new_name, 0);
    }

    if (children.size() == 3 && children[1]->get_ir_type() == IRTypeCOLLATE) {
        IR* new_name = new IR(IRTypeIDENT, get_random_collation_name(), DataCollationName, ContextUse, nullptr);
        cur_ir->swap_one_child(children.back(), new_name);
    }
}

inline void handle_collate(IR*& cur_ir) {
    BEGIN;

    for (auto* child : children) {
        if (child->get_ir_type() == IRTypeids) {
            IR* new_name = new IR(IRTypeIDENT, get_random_collation_name(), DataCollationName, ContextUse, child->get_mapped_token_node());
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_nmnum(IR*& cur_ir) {
    BEGIN;

    for (auto* child : children) {
        if (child->get_ir_type() == IRTypeNm) {
            IR* new_name = new IR(IRTypeIDENT, "c01", DataPragmaValue, ContextUse, child->get_mapped_token_node());
            cur_ir->swap_one_child(child, new_name);
        }
    }
}

inline void handle_create_view_stmt(IR*& cur_ir) {
    BEGIN;

    // Expr: createkw temp VIEW ifnotexists nm dbnm eidlist_opt AS select
    for (auto* child : children) {
        if (child->get_ir_type() == IRTypeNm) {
            IR* new_name = new IR(IRTypeIDENT, "view_name_0", DataViewName, ContextDefine, child->get_mapped_token_node());
            cur_ir->swap_one_child(child, new_name);
        } else if (child->get_ir_type() == IRTypeEidlistOpt) {
            handle_eidlist_opt(child, DataViewColumnName, ContextDefine);
        }
    }
}

inline void handle_create_table_stmt(IR*& cur_ir) {
    /* no need to handle */

    // create_table create_table_args .
}

inline void handle_from(IR*& cur_ir) {
    BEGIN;

    // The terminating rule is: from::= FROM IDENT. handled by itself.

    // from::= nm COMMA seltablist .
    for (auto* child : children) {
        if (child->get_ir_type() == IRTypeNm) {
            // ContextUseTop is used for referencing table into the FROM clause.
            IR* new_name = new IR(IRTypeIDENT, "v00", DataTableName, ContextUseTop, child->get_mapped_token_node());
            cur_ir->swap_one_child(child, new_name);
        } else if (child->get_ir_type() == IRTypeSeltablist) {
            handle_seltablist(child, ContextUseTop);
        }
    }
}

inline void handle_create_index_stmt(IR*& cur_ir) {
    BEGIN;

    // createkw uniqueflag INDEX ifnotexists nm dbnm ON nm LP sortlist RP where_opt .

    if (children[4]->get_ir_type() != IRTypeNm || children[7]->get_ir_type() != IRTypeNm) {
        cerr << "Error: Failed to match rule: createkw uniqueflag INDEX ifnotexists nm dbnm ON nm LP sortlist RP where_opt" << endl;
        abort();
    }

    IR* new_name = new IR(IRTypeIDENT, "index_0", DataIndexName, ContextDefine, children[4]->get_mapped_token_node());
    cur_ir->swap_one_child(children[4], new_name);

    new_name = new IR(IRTypeIDENT, "v00", DataTableName, ContextUse, children[7]->get_mapped_token_node());
    cur_ir->swap_one_child(children[7], new_name);
}

inline void handle_create_trigger_stmt(IR*& cur_ir) {
    // create_trigger_stmt::= createkw trigger_decl BEGIN trigger_cmd_list END .
    // no need to handle.
}

inline void handle_insert_stmt(IR*& cur_ir) {
    BEGIN;

    // Expr: with insert_cmd INTO xfullname idlist_opt select upsert
    // Expr: with insert_cmd INTO xfullname idlist_opt DEFAULT VALUES returning
    for (auto* child : children) {
        if (child->get_ir_type() == IRTypeXfullname) {
            IR* new_name = new IR(IRTypeIDENT, "v00", DataTableName, ContextUse, child->get_mapped_token_node());
            cur_ir->swap_one_child(child, new_name);
        } else if (child->get_ir_type() == IRTypeIdlistOpt) {
            handle_idlist_opt(child, DataColumnName, ContextUse);
        }
    }
}

inline void handle_alter_stmt(IR*& cur_ir) {
    BEGIN;

    // Expr: ALTER TABLE fullname RENAME TO nm
    if (children.size() == 6 && children.front()->get_ir_type() == IRTypeALTER && children[1]->get_ir_type() == IRTypeTABLE && children[2]->get_ir_type() == IRTypeFullname && children[3]->get_ir_type() == IRTypeRENAME && children[4]->get_ir_type() == IRTypeTO && children[5]->get_ir_type() == IRTypeNm) {
        for (auto* child : children) {
            if (child->get_ir_type() == IRTypeFullname) {
                IR* new_name = new IR(IRTypeIDENT, "v00", DataTableName, ContextReplaceUndefine, child->get_mapped_token_node());
                cur_ir->swap_one_child(child, new_name);
            } else if (child->get_ir_type() == IRTypeNm) {
                IR* new_name = new IR(IRTypeIDENT, "v00", DataTableName, ContextReplaceDefine, child->get_mapped_token_node());
                cur_ir->swap_one_child(child, new_name);
            }
        }

        // Expr: ALTER TABLE add_column_fullname ADD kwcolumn_opt columnname carglist
    } else if (children.size() == 7 && children.front()->get_ir_type() == IRTypeALTER && children[1]->get_ir_type() == IRTypeTABLE && children[2]->get_ir_type() == IRTypeAddColumnFullname && children[3]->get_ir_type() == IRTypeADD && children[4]->get_ir_type() == IRTypeKwcolumnOpt && children[5]->get_ir_type() == IRTypeColumnname) {
        for (auto* child : children) {
            if (child->get_ir_type() == IRTypeAddColumnFullname) {
                handle_add_column_fullname(child, DataTableName, ContextUse);
            }
        }

    // Expr: ALTER TABLE fullname DROP kwcolumn_opt nm
    } else if (children.size() == 6 && children.front()->get_ir_type() == IRTypeALTER && children[1]->get_ir_type() == IRTypeTABLE && children[2]->get_ir_type() == IRTypeFullname && children[3]->get_ir_type() == IRTypeDROP && children[4]->get_ir_type() == IRTypeKwcolumnOpt && children[5]->get_ir_type() == IRTypeNm) {

        IR* new_name = new IR(IRTypeIDENT, "c01", DataColumnName, ContextUndefine, children.back()->get_mapped_token_node());
        cur_ir->swap_one_child(children.back(), new_name);

        new_name = new IR(IRTypeIDENT, "v00", DataTableName, ContextUse, children[2]->get_mapped_token_node());
        cur_ir->swap_one_child(children[2], new_name);

        // Expr: ALTER TABLE fullname RENAME kwcolumn_opt nm TO nm
    } else if (children.size() == 8 && children.front()->get_ir_type() == IRTypeALTER && children[1]->get_ir_type() == IRTypeTABLE && children[2]->get_ir_type() == IRTypeFullname && children[3]->get_ir_type() == IRTypeRENAME && children[4]->get_ir_type() == IRTypeKwcolumnOpt && children[5]->get_ir_type() == IRTypeNm && children[6]->get_ir_type() == IRTypeTO && children[7]->get_ir_type() == IRTypeNm) {
        IR* new_name = new IR(IRTypeIDENT, "c01", DataColumnName, ContextReplaceDefine, children.back()->get_mapped_token_node());
        cur_ir->swap_one_child(children.back(), new_name);

        new_name = new IR(IRTypeIDENT, "c01", DataColumnName, ContextReplaceUndefine, children[5]->get_mapped_token_node());
        cur_ir->swap_one_child(children[5], new_name);

        new_name = new IR(IRTypeIDENT, "v00", DataTableName, ContextUse, children[2]->get_mapped_token_node());
        cur_ir->swap_one_child(children[2], new_name);
    }

}

void sqlite_ir_context_setup(RSG* rsg, IR*& cur_ir)
{
    IRTYPE cur_ir_type = cur_ir->get_ir_type();

    switch (cur_ir_type) {

    case (IRTypeIndexedBy):
        handle_indexed_by(cur_ir);
        break;

    case (IRTypeSelcollist):
        handle_selcollist(cur_ir);
        break;

    case (IRTypeCmd):
        handle_cmd(cur_ir);
        break;

    case (IRTypeSetlist):
        handle_setlist(cur_ir);
        break;

    case (IRTypeIdlist):
        // Just default data type and data flag. 
        handle_idlist(cur_ir, DataColumnName, ContextUse);
        break;

    case (IRTypeDbnm):
        handle_dbnm(cur_ir);
        break;

    // case (IRTypeSeltablist):
    //     handle_seltablist(cur_ir, ContextUse);
    //     break;

    case (IRTypeOnUsing):
        handle_on_using(cur_ir);
        break;

    case (IRTypeSortlist):
        handle_sortlist(cur_ir);
        break;

    case (IRTypeLimitOpt):
        handle_limit_opt(cur_ir);
        break;

    case (IRTypeUpsert):
        handle_upsert(cur_ir);
        break;

    case (IRTypeTriggerEvent):
        handle_trigger_event(cur_ir);
        break;

    case (IRTypeTriggerDecl):
        handle_trigger_decl(cur_ir);
        break;

    case (IRTypeTriggerCmd):
        handle_trigger_cmd(cur_ir);
        break;

    case (IRTypeTridxby):
        handle_tridxby(cur_ir);
        break;

    case (IRTypeColumnname):
        handle_columnname(cur_ir);
        break;

    case (IRTypeCcons):
        handle_ccons(cur_ir);
        break;

    case (IRTypeRefarg):
        handle_refarg(cur_ir);
        break;

    case (IRTypeGenerated):
        handle_generated(cur_ir);
        break;

    case (IRTypeCreateVtab):
        handle_create_vtab(cur_ir);
        break;

    case (IRTypeVtabarglist):
        handle_vtabarglist(cur_ir);
        break;

    case (IRTypeCreateTable):
        handle_create_table(cur_ir);
        break;

    case (IRTypeTcons):
        handle_tcons(cur_ir);
        break;

    case (IRTypeTableOption):
        handle_table_option(cur_ir);
        break;

    case (IRTypeNm):
        handle_nm(cur_ir);
        break;

    case (IRTypeTypetoken):
        handle_typetoken(cur_ir);
        break;

    case (IRTypeDeferSubclause):
        handle_defer_subclause(cur_ir);
        break;

    case (IRTypeWithnm):
        handle_withnm(cur_ir);
        break;

    case (IRTypeWqitem):
        handle_wqitem(cur_ir);
        break;

    case (IRTypeOneselect):
        handle_oneselect(cur_ir);
        break;

    case (IRTypeWindowdefn):
        handle_windowdefn(cur_ir);
        break;

    case (IRTypeWindow):
        handle_window(cur_ir);
        break;

    // case (IRTypeAs):
    //     handle_as(cur_ir);
    //     break;

    case (IRTypeJoinop):
        handle_joinop(cur_ir);
        break;

    case (IRTypeExpr):
        handle_expr(cur_ir);
        cur_ir->set_mapped_expr_node(&dump_simple_expr_node);
        cur_ir->set_is_favor(IsFavor::favor);
        break;

    case (IRTypeOverClause):
        handle_over_clause(cur_ir);
        break;

    case (IRTypeCollate):
        handle_collate(cur_ir);
        break;

    case (IRTypeNmnum):
        handle_nmnum(cur_ir);
        break;

    case (IRTypeCaseExprlist):
        cur_ir->set_is_favor(IsFavor::favor);
        break;

    case (IRTypeCaseElse):
        cur_ir->set_is_favor(IsFavor::favor);
        break;

    case (IRTypeSelectStmt):
        // cur_ir->set_is_favor(IsFavor::favor);
        break;

    case (IRTypeCreateViewStmt):
        handle_create_view_stmt(cur_ir);
        break;

    case (IRTypeCreateTableStmt):
        handle_create_table_stmt(cur_ir);
        break;

    case (IRTypeCreateIndexStmt):
        handle_create_index_stmt(cur_ir);
        break;

    case (IRTypeCreateTriggerStmt):
        handle_create_trigger_stmt(cur_ir);
        break;

    case (IRTypeInsertStmt):
        handle_insert_stmt(cur_ir);
        break;

    case (IRTypeAlterStmt):
        handle_alter_stmt(cur_ir);
        break;

    case (IRTypeFrom):
        handle_from(cur_ir);
        break;

    default:
        break;
    }

    // IMPORTANT!!!
    for (auto& interesting_type : SQLiteFuzzerConfigurations::sqlite_interesting_ir_types) {
        if (interesting_type == IRTypeSelectStmt) {
            continue;
        }
        if (interesting_type == cur_ir_type) {
            cur_ir->set_is_favor(IsFavor::favor);
            break;
        }
    }

}