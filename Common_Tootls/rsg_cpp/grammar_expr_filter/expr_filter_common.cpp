//
// Created by XXX on 3/1/24.
//

#include "../headers/expr_filter_common.h"
#include "../headers/fuzzer_configurations.h"
#include "../headers/rsg.h"
#include <string>
#include <vector>

using std::string;
using std::vector;

namespace GrammarExprFilter {

bool find_substr_from_vec_helper(const string& str, vector<string>& v_sub)
{
    for (string& cur_sub : v_sub) {
        if (str.find(cur_sub) != string::npos) {
            return true;
        }
    }
    return false;
}

// Helper function for classify_grammar_exprs_helper

static inline bool
call_comp_filter_func_with_vec(const ExpressionNode* expr_in, bool (*comp_filter_func)(const string&))
{
    for (const TokenNode* cur_token : expr_in->get_tokens()) {
        const string& cur_str = cur_token->get_string();
        if (comp_filter_func(cur_str)) {
            // detect complicated non-term keyword.
            return true;
        }
    }

    return false;
}

static inline bool is_expr_recursive_comp(const ExpressionNode* expr_in, const string& root)
{
    for (const TokenNode* cur_token : expr_in->get_tokens()) {
        const string& cur_str = cur_token->get_string();
        if (root == cur_str) {
            // detect complicated non-term keyword.
            return true;
        }
    }

    return false;
}

static inline bool is_expr_recursive_comp(const ExpressionNode* expr_in, const vector<string>& v_root)
{
    for (const TokenNode* cur_token : expr_in->get_tokens()) {
        const string& cur_str = cur_token->get_string();
        for (const string& root : v_root) {
            if (root == cur_str) {
                // detect complicated non-term keyword.
                return true;
            }
        }
    }

    return false;
}

static inline bool is_expr_term(RSG* rsg, const ExpressionNode* expr_in)
{
#if defined(mysqldb)
    // FIXME:: TODO: Dirty patch. 
    if (expr_in->get_tokens().size() == 1 && expr_in->get_tokens()[0]->get_string() == "table_factor") {
        return true;
    }
#endif

    for (const TokenNode* cur_token : expr_in->get_tokens()) {
        const string& cur_str = cur_token->get_string();
        if (rsg->m_all_prods_str.count(cur_str) != 0) {
            return false;
        }
    }
    return true;
}

// static helper functions.
static inline void set_term(ProductionNode*& cur_prod, ExpressionNode*& cur_expr)
{
    cur_expr->expression_type = ExpressionTyp::ExpTypTerm;
    cur_prod->term_exprs.push_back(cur_expr);
}

static inline void set_norm_preferred(ProductionNode*& cur_prod, ExpressionNode*& cur_expr)
{
    cur_expr->expression_type = ExpressionTyp::ExpTypNormPrefered;
    cur_prod->norm_preferred_exprs.push_back(cur_expr);
}

static inline void set_norm(ProductionNode*& cur_prod, ExpressionNode*& cur_expr)
{
    cur_expr->expression_type = ExpressionTyp::ExpTypNorm;
    cur_prod->norm_exprs.push_back(cur_expr);
}

static inline void set_comp(ProductionNode*& cur_prod, ExpressionNode*& cur_expr)
{
    cur_expr->expression_type = ExpressionTyp::ExpTypComp; // Treat it as the more complicated case.
    cur_prod->comp_exprs.push_back(cur_expr);
}

static inline void set_recur_comp(ProductionNode*& cur_prod, ExpressionNode*& cur_expr)
{
    cur_expr->expression_type = ExpressionTyp::ExpTypRecComp;
    cur_prod->recur_comp_exprs.push_back(cur_expr);
}

static inline void set_non_recur_comp(ProductionNode*& cur_prod, ExpressionNode*& cur_expr)
{
    cur_expr->expression_type = ExpressionTyp::ExpTypNonRecurComp;
    cur_prod->non_recur_comp_exprs.push_back(cur_expr);
}

enum SUBNODECOMPTYPE {
    all_term = 0,
    has_term = 1,
    all_recur_comp = 2,
    all_comp = 3,
    mixed = 4
};

static inline SUBNODECOMPTYPE summarize_node_type_within_one_expr(vector<SUBNODECOMPTYPE>& in)
{
    // Scan through one expression. The result is the worse case from all the tokens.
    SUBNODECOMPTYPE res = all_term; // default
    for (const auto& cur_child : in) {
        if (cur_child == all_recur_comp) {
            // worse case, directly return.
            return all_recur_comp;
        } else if (cur_child == all_comp) {
            res = all_comp;
        } else if (cur_child == mixed && res != all_comp) {
            // Could have comp later.
            res = mixed;
        } else if (cur_child == has_term && res == all_term) {
            // could be more complex later.
            res = has_term;
        } else {
            // == all_term. do nothing.
            // if all keywords are all_term, then return all_term by default.
        }
    }
    return res;
}

static inline SUBNODECOMPTYPE summarize_node_type_across_exprs(vector<SUBNODECOMPTYPE>& in)
{
    // Scan through across expressions. The result could be benefit from the one term expression (norm_preferred).
    bool is_all_recur_comp = true;
    bool is_all_comp = true;
    bool is_all_term = true;
    bool is_has_term = false;

    for (const auto& cur_type : in) {
        switch (cur_type) {
        case all_comp: {
            is_all_term = false;
            is_all_recur_comp = false;
        } break;
        case all_recur_comp: {
            is_all_term = false;
        } break;
        case has_term: {
            // can also be treated as conditional term.
            is_has_term = true;
            // no longer all_term now. has_term is conditional term.
            // is_all_term = false; // fallthrough
        }
            [[fallthrough]];
        case mixed: {
            // mixed. meaning that we don't have confident term or complex
            is_all_term = false;
            is_all_comp = false;
            is_all_recur_comp = false;
        } break;
        case all_term: {
            is_all_recur_comp = false;
            is_all_comp = false;
            is_has_term = true;
        } break;
        }
    }

    if (is_all_term) {
        // all term
        return all_term;
    } else if (is_has_term) {
        // has both term and comp.
        return has_term;
    } else if (is_all_recur_comp) {
        // all recur_comp
        return all_recur_comp;
    } else if (is_all_comp) {
        // has non-recur-comp
        return all_comp;
    } else {
        // only mixed and comp, no term.
        return mixed;
    }
}

/*
 * Helper function for classify_grammar_exprs_helper.
 * If classify_grammar_exprs_helper cannot detect the expression type, then it should go
 * deeper and look into every non-terminal tokens presented in the interested grammar expression.
 * Intuitively, if all sub-rule-expressions from one non-terminal token are Terminating rules, then
 * we can safely declare that this token would lead to termination. If all tokens within the current
 * expression are terminating tokens, then the current rule is terminating rule.
 * On the other hand, if one token in the expression that doomed to lead to complex rules(recursive/non-recursive),
 * then the current expression is complex expression.
 * This is a recursive function that check the above logic up to FuzzerConstant::GrammarRuleCatScanDepth
 * depth. If still cannot decide whether it is term or comp, then return norm(unknown).
 * */
static enum SUBNODECOMPTYPE
classify_multi_step_exprs_helper(RSG* rsg, vector<string>& v_grand_root, const string& cur_token_str,
    bool (*comp_filter_func)(const string&), unsigned int depth)
{
    // recursively one-depth check whether node is complex node. no need to check

    if (rsg->m_all_prods_str.count(cur_token_str) == 0) {
        // terminating keyword.
        return all_term;
    }

    depth--;

    vector<ProductionNode*>& cur_v_prod = rsg->m_all_prods_str[cur_token_str];
    vector<SUBNODECOMPTYPE> v_cross_exprs_node_type;

    // Scan across different expressions from the same non-terminal keyword.
    for (ProductionNode*& cur_prod : cur_v_prod) {
        // dump vector. In rare cases one non-terminal keyword would have 2 prods available.
        for (ExpressionNode*& cur_expr : cur_prod->all_exprs) {

            enum SUBNODECOMPTYPE cur_expr_node_type;
            if (is_expr_recursive_comp(cur_expr, v_grand_root)) {
                cur_expr_node_type = all_recur_comp;
            } else if (is_expr_term(rsg, cur_expr)) {
                cur_expr_node_type = all_term;
            } else if (call_comp_filter_func_with_vec(cur_expr, comp_filter_func)) {
                cur_expr_node_type = all_comp;
            } else if (depth != 0) {
                // normal rule but depth not reached. go deeper and see.
                // iterate non-terminal keywords within one expression.
                vector<string> v_tmp_grant_root = v_grand_root; // deep-copy
                v_tmp_grant_root.push_back(cur_token_str);
                vector<SUBNODECOMPTYPE> v_children_res;
                for (const TokenNode* cur_non_term : cur_expr->get_tokens()) {
                    SUBNODECOMPTYPE child_res = classify_multi_step_exprs_helper(rsg, v_tmp_grant_root,
                        cur_non_term->get_string(),
                        comp_filter_func, depth);
                    v_children_res.push_back(child_res);
                }
                cur_expr_node_type = summarize_node_type_within_one_expr(v_children_res);
            } else {
                // default mixed.
                cur_expr_node_type = mixed;
            }
            v_cross_exprs_node_type.push_back(cur_expr_node_type);
        }
    }

    return summarize_node_type_across_exprs(v_cross_exprs_node_type);
}

/*
 * See comments in the header declaration file.
 * This function handles all the expressions that present directly given the
 * current root non-terminal keyword.
 * It is responsible for detecting each grammar expression type, and label these
 * expressions within ProductionNode structure.
 * */
void classify_grammar_exprs_helper(RSG* rsg, bool (*comp_filter_func)(const string&))
{
    for (auto& productions_map : rsg->m_all_prods_str) {
        const string& cur_root = productions_map.first;
        vector<ProductionNode*>& cur_v_prod = productions_map.second;
        for (ProductionNode*& cur_prod : cur_v_prod) {
            bool is_contain_term = false;
            bool is_contain_non_term = false;

            for (ExpressionNode*& cur_expr : cur_prod->all_exprs) {
                if (is_expr_term(rsg, cur_expr)) {
                    // simple terminating rule.
                    set_term(cur_prod, cur_expr);
                    is_contain_term = true;
                    continue;
                } else if (is_expr_recursive_comp(cur_expr, cur_root)) {
                    // recursive complex rule.
                    set_recur_comp(cur_prod, cur_expr);
                    set_comp(cur_prod, cur_expr);
                    is_contain_non_term = true;
                    // This rule is determined to be recursive complex rule.
                    // Continue to the next rule.
                    continue;
                } else if (call_comp_filter_func_with_vec(cur_expr, comp_filter_func)) {
                    // marked complex rule.
                    set_non_recur_comp(cur_prod, cur_expr);
                    set_comp(cur_prod, cur_expr);
                    is_contain_non_term = true;
                    continue;
                } else {
                    // not able to determine the expression type here.
                    // Assuming normal/mixed rule for the moment.
                    // Look deeper recursively with depth, check the subtree for more information.
                    vector<string> v_tmp_grant_root = { cur_root };
                    vector<SUBNODECOMPTYPE> v_non_term_keyword_types;

                    // iterate every keyword in the current rule expression,
                    // look into their (subtree) grammar expressions
                    // with depth FuzzerConstant::GrammarRuleCatScanDepth
                    for (const TokenNode* cur_token : cur_expr->get_tokens()) {
                        auto cur_type = classify_multi_step_exprs_helper(rsg, v_tmp_grant_root,
                            cur_token->get_string(), comp_filter_func,
                            FuzzerConfigurations::GrammarRuleCatScanDepth);
                        v_non_term_keyword_types.push_back(cur_type);
                        if (cur_type == all_recur_comp || cur_type == all_comp) {
                            // already see doomed complex expression. no need to look further.
                            break;
                        }
                    }
                    SUBNODECOMPTYPE res_type = summarize_node_type_within_one_expr(v_non_term_keyword_types);
                    switch (res_type) {
                    case all_term: {
                        set_term(cur_prod, cur_expr);
                        is_contain_term = true;
                    } break;
                    case has_term: {
                        set_norm_preferred(cur_prod, cur_expr);
                        is_contain_non_term = true;
                    } break;
                    case all_recur_comp: {
                        set_comp(cur_prod, cur_expr);
                        set_recur_comp(cur_prod, cur_expr);
                        is_contain_non_term = true;
                    } break;
                    case all_comp: {
                        set_comp(cur_prod, cur_expr);
                        set_non_recur_comp(cur_prod, cur_expr);
                        is_contain_non_term = true;
                    } break;
                    case mixed: {
                        set_norm(cur_prod, cur_expr);
                        is_contain_non_term = true;
                    } break;
                    }
                    continue;
                }
            } // for exprs

            if (is_contain_non_term && is_contain_term) {
                cur_prod->is_additive_mutation_preferred = true;
#ifdef DEBUG
                cerr << "Getting is_additive_mutation_preferred node: " << cur_prod->get_name() << "\n";
#endif
            }
        }
    }

    // for verification purpose.
    for (auto& productions_map : rsg->m_all_prods_str) {
        vector<ProductionNode*>& cur_v_prod = productions_map.second;
        for (const ProductionNode* cur_prod : cur_v_prod) {
            int all_expr_size = cur_prod->get_exprs().size();
            int term_expr_size = cur_prod->get_term_exprs().size();
            int norm_preferred_exprs_size = cur_prod->get_norm_preferred_exprs().size();
            int norm_exprs_size = cur_prod->get_norm_exprs().size();
            int non_recur_comp_exprs_size = cur_prod->get_non_recur_comp_exprs().size();
            int recur_comp_exprs_size = cur_prod->get_recur_comp_exprs().size();
            int comp_exprs_size = cur_prod->get_comp_exprs().size();

            if (
                all_expr_size != (term_expr_size + norm_preferred_exprs_size + norm_exprs_size + comp_exprs_size)
                || comp_exprs_size != (non_recur_comp_exprs_size + recur_comp_exprs_size)) {
                cerr << "Error: For Production Node: " << cur_prod->get_name() << ", classification information mismatched. \n\n\n";
                abort();
            }

#ifdef DEBUG
            cerr << "Debug: For Production node: " << cur_prod->get_name() << "\n";

            cerr << "----Term Expr: \n";
            for (ExpressionNode* cur_expr : cur_prod->get_term_exprs()) {
                cerr << "--------";
                int idx = 0;
                for (TokenNode* cur_token : cur_expr->get_tokens()) {
                    if (idx > 0) {
                        cerr << ", ";
                    }
                    cerr << cur_token->get_string();
                    idx++;
                }
                cerr << "\n";
            }
            cerr << "\n\n";

            cerr << "----Norm Preferred Expr: \n";
            for (ExpressionNode* cur_expr : cur_prod->get_norm_preferred_exprs()) {
                cerr << "--------";
                int idx = 0;
                for (TokenNode* cur_token : cur_expr->get_tokens()) {
                    if (idx > 0) {
                        cerr << ", ";
                    }
                    cerr << cur_token->get_string();
                    idx++;
                }
                cerr << "\n";
            }
            cerr << "\n\n";

            cerr << "----Norm Expr: \n";
            for (ExpressionNode* cur_expr : cur_prod->get_norm_exprs()) {
                cerr << "--------";
                int idx = 0;
                for (TokenNode* cur_token : cur_expr->get_tokens()) {
                    if (idx > 0) {
                        cerr << ", ";
                    }
                    cerr << cur_token->get_string();
                    idx++;
                }
                cerr << "\n";
            }
            cerr << "\n\n";

            cerr << "----Non-Recur Comp Expr: \n";
            for (ExpressionNode* cur_expr : cur_prod->get_non_recur_comp_exprs()) {
                cerr << "--------";
                int idx = 0;
                for (TokenNode* cur_token : cur_expr->get_tokens()) {
                    if (idx > 0) {
                        cerr << ", ";
                    }
                    cerr << cur_token->get_string();
                    idx++;
                }
                cerr << "\n";
            }

            cerr << "\n\n";
            cerr << "----Recur Comp Expr: \n";
            for (ExpressionNode* cur_expr : cur_prod->get_recur_comp_exprs()) {
                cerr << "--------";
                int idx = 0;
                for (TokenNode* cur_token : cur_expr->get_tokens()) {
                    if (idx > 0) {
                        cerr << ", ";
                    }
                    cerr << cur_token->get_string();
                    idx++;
                }
                cerr << "\n";
            }
            cerr << "\n\n\n\n\n";
#endif
        }
    }
}
} // namespace GrammarExprFilter