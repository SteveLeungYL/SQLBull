//
// Created by XXX on 3/4/24.
//

#ifndef RSG_CPP_IR_H
#define RSG_CPP_IR_H

#include "custom_token_type.h"
#include "data_affinity.h"
#include "ir_types_common.h"

#include <string>

using std::string;

class ProductionNode;
class ExpressionNode;
class TokenNode;

enum IsFavor {
    non_favor = 0,
    favor_select = 1,
    favor_other = 1,
    favor = 1,
};

class IR {
public:
    // general
    IR(SYMBOLTYPE sym_type_in, IRTYPE ir_type_in, const string str_in,
        TokenNode* mapped_token_in,
        ExpressionNode* mapped_expr_in = nullptr, ProductionNode* mapped_production_in = nullptr)
        : id_(0)
        , symbol_type_(sym_type_in)
        , ir_type_(ir_type_in)
        , data_type_(DataNone)
        , data_flag_(ContextUnknown)
        , p_data_affinity(nullptr)
        , p_custom_type(nullptr)
        , parent_(NULL)
        , mapped_prod_node(mapped_production_in)
        , mapped_expr_node(mapped_expr_in)
        , mapped_token_node(mapped_token_in)
        , str_val_(str_in)
    {
    }

    IR(SYMBOLTYPE sym_type_in, IRTYPE ir_type_in, const char* str_in, TokenNode* mapped_token_in,
        ExpressionNode* mapped_expr_in = nullptr, ProductionNode* mapped_production_in = nullptr)
        : id_(0)
        , symbol_type_(sym_type_in)
        , ir_type_(ir_type_in)
        , data_type_(DataNone)
        , data_flag_(ContextUnknown)
        , p_data_affinity(nullptr)
        , p_custom_type(nullptr)
        , parent_(NULL)
        , mapped_prod_node(mapped_production_in)
        , mapped_expr_node(mapped_expr_in)
        , mapped_token_node(mapped_token_in)
        , str_val_(str_in)
    {
    }

    // identifier
    IR(IRTYPE ir_type_in, const string str_in,
        const DATATYPE data_type_in, const DATAFLAG data_flag_in, TokenNode* mapped_node_in)
        : id_(0)
        , symbol_type_(SymbolIden)
        , ir_type_(ir_type_in)
        , data_type_(data_type_in)
        , data_flag_(data_flag_in)
        , p_data_affinity(nullptr)
        , p_custom_type(nullptr)
        , parent_(NULL)
        , mapped_prod_node(nullptr)
        , mapped_expr_node(nullptr)
        , mapped_token_node(mapped_node_in)
        , str_val_(str_in)
    {
    }

    IR(IRTYPE ir_type_in, const char* str_in,
        const DATATYPE data_type_in, const DATAFLAG data_flag_in, TokenNode* mapped_node_in)
        : id_(0)
        , symbol_type_(SymbolIden)
        , ir_type_(ir_type_in)
        , data_type_(data_type_in)
        , data_flag_(data_flag_in)
        , p_data_affinity(nullptr)
        , p_custom_type(nullptr)
        , parent_(NULL)
        , mapped_prod_node(nullptr)
        , mapped_expr_node(nullptr)
        , mapped_token_node(mapped_node_in)
        , str_val_(str_in)
    {
    }

    // For literal
    IR(IRTYPE ir_type_in, const string str_in,
        const DATAAFFINITYTYPE data_affi_in, TokenNode* mapped_node_in)
        : id_(0)
        , symbol_type_(SymbolLit)
        , ir_type_(ir_type_in)
        , data_type_(DataNone)
        , data_flag_(ContextUnknown)
        , p_custom_type(nullptr)
        , parent_(NULL)
        , mapped_prod_node(nullptr)
        , mapped_expr_node(nullptr)
        , mapped_token_node(mapped_node_in)
        , str_val_(str_in)
    {
        this->p_data_affinity = new DataAffinity(data_affi_in);
    }

    IR(IRTYPE ir_type_in, const char* str_in,
        const DATAAFFINITYTYPE data_affi_in, TokenNode* mapped_node_in)
        : id_(0)
        , symbol_type_(SymbolLit)
        , ir_type_(ir_type_in)
        , data_type_(DataNone)
        , data_flag_(ContextUnknown)
        , p_custom_type(nullptr)
        , parent_(NULL)
        , mapped_prod_node(nullptr)
        , mapped_expr_node(nullptr)
        , mapped_token_node(mapped_node_in)
        , str_val_(str_in)
    {
        this->p_data_affinity = new DataAffinity(data_affi_in);
    }

    IR(IRTYPE ir_type_in, const int int_in,
        const DATAAFFINITYTYPE data_affi_in, TokenNode* mapped_node_in)
        : id_(0)
        , symbol_type_(SymbolLit)
        , ir_type_(ir_type_in)
        , data_type_(DataNone)
        , data_flag_(ContextUnknown)
        , p_custom_type(nullptr)
        , parent_(NULL)
        , mapped_prod_node(nullptr)
        , mapped_expr_node(nullptr)
        , mapped_token_node(mapped_node_in)
        , int_val_(int_in)
    {
        this->p_data_affinity = new DataAffinity(data_affi_in);
    }

    IR(IRTYPE ir_type_in, const double float_in,
        const DATAAFFINITYTYPE data_affi_in, TokenNode* mapped_node_in)
        : id_(0)
        , symbol_type_(SymbolLit)
        , ir_type_(ir_type_in)
        , data_type_(DataNone)
        , data_flag_(ContextUnknown)
        , p_custom_type(nullptr)
        , parent_(NULL)
        , mapped_prod_node(nullptr)
        , mapped_expr_node(nullptr)
        , mapped_token_node(mapped_node_in)
        , float_val_(float_in)
    {
        this->p_data_affinity = new DataAffinity(data_affi_in);
    }

    IR(IRTYPE ir_type_in, const bool bool_in,
        const DATAAFFINITYTYPE data_affi_in, TokenNode* mapped_node_in)
        : id_(0)
        , symbol_type_(SymbolLit)
        , ir_type_(ir_type_in)
        , data_type_(DataNone)
        , data_flag_(ContextUnknown)
        , p_custom_type(nullptr)
        , parent_(NULL)
        , mapped_prod_node(nullptr)
        , mapped_expr_node(nullptr)
        , mapped_token_node(mapped_node_in)
        , bool_val_(bool_in)
    {
        this->p_data_affinity = new DataAffinity(data_affi_in);
    }

    IR(const DATAAFFINITYTYPE data_affi, TokenNode* mapped_node_in)
        : id_(0)
        , symbol_type_(SymbolLit)
        , ir_type_(IRTypeUnknownType)
        , data_type_(DataNone)
        , data_flag_(ContextUnknown)
        , p_custom_type(nullptr)
        , parent_(NULL)
        , mapped_prod_node(nullptr)
        , mapped_expr_node(nullptr)
        , mapped_token_node(mapped_node_in)
    {
        this->set_data_affinity(data_affi);
        this->mutate_literal_helper();
        this->symbol_type_ = SymbolLit;
    }

    IR(CustomTokenType* custom_type_in, TokenNode* mapped_node_in)
        : id_(0)
        , symbol_type_(SymbolLit)
        , ir_type_(IRTypeUnknownType)
        , data_type_(DataNone)
        , data_flag_(ContextUnknown)
        , p_data_affinity(nullptr)
        , parent_(NULL)
        , mapped_prod_node(nullptr)
        , mapped_expr_node(nullptr)
        , mapped_token_node(mapped_node_in)
    {
        // Cannot use any copy constructor here.
        // The passed in instance is an inheritance class
        // from the base custom type class.
        this->p_custom_type = custom_type_in;
        this->symbol_type_ = SymbolCustomTerm;
    }

    // for non-terminal type.
    IR(vector<IR*> children_nodes, TokenNode* mapped_token_in, ExpressionNode* mapped_expr_in, ProductionNode* mapped_prod_in)
        : id_(0)
        , symbol_type_(SymbolNonTerm)
        , ir_type_(IRTypeUnknownType)
        , data_type_(DataNone)
        , data_flag_(ContextUnknown)
        , p_data_affinity(nullptr)
        , p_custom_type(nullptr)
        , parent_(NULL)
        , mapped_prod_node(mapped_prod_in)
        , mapped_expr_node(mapped_expr_in)
        , mapped_token_node(mapped_token_in)
    {
        this->symbol_type_ = SymbolNonTerm;
        this->children_ = children_nodes;
        for (IR* cur_child : this->children_) {
            cur_child->set_parent_node(this);
        }
    }

    /* copy_constructor */
    IR(const IR& ir_in)
        : id_(ir_in.get_id())
        , symbol_type_(ir_in.get_symbol_type())
        , ir_type_(ir_in.get_ir_type())
        , data_type_(ir_in.get_data_type())
        , data_flag_(ir_in.get_data_flag())
        //        , p_data_affinity(new DataAffinity(
        //              *(ir_in.get_p_data_affinity()))) // construct a new data affinity instance
        //        ,p_custom_type(
        //            ir_in.get_p_custom_type()
        //                ->deep_copy()) // construct a new custom type instance
        , parent_(ir_in.get_parent_node())
        , // FIXME: Should we do this (copying
          // parent node address)?
        children_(ir_in.get_children())
        , // FIXME: Should we do this (copying
          // children node address)?
        is_node_fixed(ir_in.get_node_fixed())
        , mutated_times_(ir_in.get_mutated_times())
        , is_instantiated(ir_in.get_is_instantiated())
        , mapped_prod_node(ir_in.get_mapped_prod_node())
        , mapped_expr_node(ir_in.get_mapped_expr_node())
        , mapped_token_node(ir_in.get_mapped_token_node())
        , str_val_(ir_in.get_str_val())
    {
        if (ir_in.get_symbol_type() == SymbolLit) {
            switch (ir_in.get_ir_type()) {
#if defined(cockroachdb) || defined(duckdb)
            case IRTypeICONST:
                [[fallthrough]];
#endif
            case IRTypeINTEGER: {
                this->int_val_ = ir_in.get_lit_int_val();
                break;
            }

#if defined(cockroachdb) || defined(duckdb)
            case IRTypeFCONST:
                [[fallthrough]];
#endif
            case IRTypeFLOAT: {
                this->float_val_ = ir_in.get_lit_float_val();
                break;
            }
            case IRTypeBOOLEAN: {
                this->bool_val_ = ir_in.get_lit_bool_val();
                break;
            }
            default: {
                break;
            }
            }
        }

        if (ir_in.get_p_data_affinity() != nullptr) {
            this->p_data_affinity = new DataAffinity(*(ir_in.get_p_data_affinity()));
        } else {
            this->p_data_affinity = nullptr;
        }

        if (ir_in.get_p_custom_type() != nullptr) {
            this->p_custom_type = get_p_custom_type()->deep_copy();
        } else {
            this->p_custom_type = nullptr;
        }
    }

    const string print_info() const
    {
        string res;
        res += "Symbol type: " + get_string_by_symbol_type(this->get_symbol_type());
        res += ", IR type: " + get_string_by_ir_type(this->get_ir_type());
        res += ", Data type: " + get_string_by_data_type(this->get_data_type());
        res += ", Data flag: " + get_string_by_data_flag(this->get_data_flag());
        res += ", Data affinity: " + get_string_by_data_affi(this->get_data_affinity_type());
        return res;
    }

    const string print_info_short() const
    {
        string res;
        res += get_string_by_symbol_type(this->get_symbol_type());
        res += " " + get_string_by_ir_type(this->get_ir_type());
        res += " " + get_string_by_data_type(this->get_data_type());
        res += " " + get_string_by_data_flag(this->get_data_flag());
        res += " " + get_string_by_data_affi(this->get_data_affinity_type());
        return res;
    }

    // Helper function.
    // Setter
    void set_id(const int in) { this->id_ = in; }
    void set_symbol_type(const SYMBOLTYPE in) { this->symbol_type_ = in; }
    void set_ir_type(const IRTYPE in) { this->ir_type_ = in; }
    void set_data_type(const DATATYPE in) { this->data_type_ = in; }
    void set_data_flag(const DATAFLAG in) { this->data_flag_ = in; }
    void set_data_affinity_type(const DATAAFFINITYTYPE in)
    {
        if (this->p_data_affinity != nullptr) {
            this->p_data_affinity->set_data_affinity(in);
        } else {
            this->p_data_affinity = new DataAffinity(in);
        }
    }
    void set_data_affinity(DataAffinity in)
    {
        if (this->p_data_affinity != nullptr) {
            // Use the copy constructor.
            *(this->p_data_affinity) = in;
        } else {
            this->p_data_affinity = new DataAffinity(in);
        }
    }
    void set_custom_type(CustomTokenType* custom_type_in)
    {
        this->p_custom_type = custom_type_in;
    }
    void set_str_val(const string in) { this->str_val_ = in; }
    void set_parent_node(IR* in) { this->parent_ = in; }
    void set_node_fixed(const bool in) { this->is_node_fixed = in; }
    void set_mutated_times(const unsigned int in) { this->is_instantiated = in; }
    void set_is_instantiated(const bool in) { this->is_instantiated = in; }
    void set_type(DATATYPE data_type_in, // Set type regardless of its node type.
        DATAFLAG data_flag_in,
        DATAAFFINITYTYPE data_affi_in = AFFIUNKNOWN)
    {
        this->data_type_ = data_type_in;
        this->data_flag_ = data_flag_in;
        this->p_data_affinity->set_data_affinity(data_affi_in);
    }
    void set_children_nodes(vector<IR*> children_nodes)
    {
        this->children_ = children_nodes;
        for (IR* cur_child : this->children_) {
            cur_child->set_parent_node(this);
        }
    }
    void set_lit_int_value(const int in) { this->int_val_ = in; }
    void set_lit_float_value(const double in) { this->float_val_ = in; }
    void set_lit_bool_value(const bool in) { this->bool_val_ = in; }
    void set_mapped_prod_node(ProductionNode* in) { this->mapped_prod_node = in; }
    void set_mapped_expr_node(ExpressionNode* in) { this->mapped_expr_node = in; }
    void set_mapped_token_node(TokenNode* in) { this->mapped_token_node = in; }

    // Getter
    unsigned int get_id() const { return this->id_; }
    [[nodiscard]] SYMBOLTYPE get_symbol_type() const { return this->symbol_type_; }
    [[nodiscard]] IRTYPE get_ir_type() const { return this->ir_type_; }
    [[nodiscard]] DATATYPE get_data_type() const { return this->data_type_; }
    [[nodiscard]] DATAFLAG get_data_flag() const { return this->data_flag_; }
    [[nodiscard]] DATAAFFINITYTYPE get_data_affinity_type() const
    {
        if (this->p_data_affinity != nullptr) {
            return this->p_data_affinity->get_data_affinity();
        } else {
            return AFFIUNKNOWN;
        }
    }
    [[nodiscard]] DataAffinity* get_p_data_affinity() const { return this->p_data_affinity; }
    [[nodiscard]] CustomTokenType* get_p_custom_type() const { return this->p_custom_type; }
    [[nodiscard]] string get_str_val() const { return this->str_val_; }
    [[nodiscard]] IR* get_parent_node() const { return this->parent_; }
    [[nodiscard]] vector<IR*> get_children() const { return this->children_; }
    [[nodiscard]] vector<IR*>& get_children_ref() { return this->children_; }
    [[nodiscard]] bool get_node_fixed() const { return this->is_node_fixed; }
    [[nodiscard]] unsigned int get_mutated_times() const { return this->is_instantiated; }
    [[nodiscard]] bool get_is_instantiated() const { return this->is_instantiated; }
    [[nodiscard]] int get_lit_int_val() const { return this->int_val_; }
    [[nodiscard]] double get_lit_float_val() const { return this->float_val_; }
    [[nodiscard]] bool get_lit_bool_val() const { return this->float_val_; }
    [[nodiscard]] ProductionNode* get_mapped_prod_node() const { return this->mapped_prod_node; }
    [[nodiscard]] ExpressionNode* get_mapped_expr_node() const { return this->mapped_expr_node; }
    [[nodiscard]] TokenNode* get_mapped_token_node() const { return this->mapped_token_node; }

    // Transformation related.
    string to_string();

    // delete this IR and necessary clean up
    void drop();
    // delete the IR tree
    void deep_drop();
    // copy the IR tree
    IR* deep_copy();
    // find the root node of this node
    IR* get_root();
    // unlink the children node and free them.
    void free_children();
    // unlink the children node from this IR tree, not freed
    void detach_children();
    // unlink one specific child from the IR tree, not freed
    bool detach_one_child(IR* rov_node);
    // unlink the children node from this IR tree and freed
    bool swap_one_child(IR* old_node, IR* new_node, bool is_free = true);
    // append one child with idx
    bool add_one_child(IR*, int);

    void mutate_literal_random_affinity()
    {
        this->mutate_literal_with_type(get_random_affinity_type());
    };
    void mutate_literal_with_type(DATAAFFINITYTYPE data_affi)
    {
        this->set_data_affinity_type(data_affi);
        this->mutate_literal_helper();
    }
    void mutate_literal_with_type(DataAffinity data_affi)
    {
        this->set_data_affinity(data_affi);
        this->mutate_literal_helper();
    }
    void mutate_literal()
    {
        // mutate the IR with the original defined affinity.
        this->mutate_literal_helper();
    }

    bool is_empty();

    bool operator==(const IRTYPE& ir_type_in) const
    {
        if (this->get_ir_type() == ir_type_in) {
            return true;
        } else {
            return false;
        }
    }

    void debug(ostream& out, int depth = 0);

    void set_is_favor(IsFavor is_favor) { this->is_favor_ = is_favor; }
    [[nodiscard]] IsFavor get_is_favor() const { return this->is_favor_; }

    [[nodiscard]] uint64_t hash_tree() const;

    [[nodiscard]] bool get_is_empty() const;

private:
    // Main literal mutation function.
    void mutate_literal_helper();
    void to_string_helper(string&);

    unsigned int id_;
    SYMBOLTYPE symbol_type_;
    IRTYPE ir_type_;
    DATATYPE data_type_;
    DATAFLAG data_flag_;
    DataAffinity* p_data_affinity; // pointer. only necessary in literal type.
    CustomTokenType* p_custom_type; // pointer. only necessary in custom token type.

    IR* parent_;
    vector<IR*> children_;
    bool is_node_fixed = false; // Do not mutate this IR if this set to be true.
    unsigned int mutated_times_ = 0;
    bool is_instantiated = false;
    IsFavor is_favor_ = non_favor;

    ProductionNode* mapped_prod_node;

    ExpressionNode* mapped_expr_node;
    TokenNode* mapped_token_node;

    string str_val_;
    union {
        int int_val_;
        double float_val_;
        bool bool_val_;
    };

    void _hash_tree_helper(vector<IRTYPE>& v_types_in) const;
};

#endif // RSG_CPP_IR_H
