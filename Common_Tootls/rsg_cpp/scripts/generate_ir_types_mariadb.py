import os
import re

count = 0

all_saved_ir_types_mapping = dict()


def transform_snake_to_pascal_naming(cur_token: str) -> str:
    if len(cur_token) == 0:
        return ""
    if not cur_token[0].isalpha():
        return ""

    is_snake = False
    is_first = True
    res_str = ""
    for cur_char in cur_token:
        if is_first == True:
            cur_char = cur_char.upper()
            res_str += cur_char
            is_first = False
        elif is_snake == True:
            cur_char = cur_char.upper()
            res_str += cur_char
            is_snake = False
        elif cur_char == "_":
            is_snake = True
            continue
        else:
            res_str += cur_char
            is_snake = False

    return res_str


def is_token_str_or_literal(cur_token: str):
    if cur_token in {"IDENT", "IDENT_QUOTED", "SCONST", "BCONST", "BITCONST", "ICONST", "FCONST", "XCONST"}:
        return True
    else:
        return False


def gen_cov_logging_func_call(cur_keyword, token_seq: str, rule_idx: int) -> str:
    global count
    global all_saved_ir_types_mapping
    count += 1

    cur_keyword = cur_keyword.replace("|", "")
    cur_keyword = cur_keyword.replace("/", "")
    cur_keyword = cur_keyword.replace(" ", "")

    # if count >= 29:
    #     return ""

    res_str = ""
    if "%prec" in token_seq:
        token_seq = token_seq.split("%prec")[0]
    token_seq = token_seq.split()

    res_token_seq = []
    for cur_token in token_seq:
        if cur_token == "/*" or cur_token == "EMPTY" or cur_token == "*/":
            continue
        res_token_seq.append(cur_token)
    token_seq = res_token_seq

    token_idx = 0
    for cur_token in token_seq:
        cur_token = cur_token.replace("%ifdef", "")
        cur_token = cur_token.replace("%endif", "")
        
        token_idx += 1
        if is_token_str_or_literal(cur_token):
            res_str += f"fmt.Printf(\"LogGrammarStr({cur_token},-1,[%s])\\n\",${token_idx})\n"
        cur_token_type = transform_snake_to_pascal_naming(cur_token)
        cur_keyword_type = transform_snake_to_pascal_naming(cur_keyword)

        if cur_keyword not in all_saved_ir_types_mapping and len(cur_keyword_type) != 0:
            all_saved_ir_types_mapping[cur_keyword] = cur_keyword_type
        if cur_token not in all_saved_ir_types_mapping and len(cur_token_type) != 0:
            all_saved_ir_types_mapping[cur_token] = cur_token_type

    # res_str += f"fmt.Printf(\"LogGrammar(\\\"{cur_keyword}\\\",\\\"{token_seq}\\\")\")\n"
    res_str += (f"fmt.Printf(\"LogGrammar({cur_keyword},{rule_idx},[")
    idx = 1
    for cur_token in token_seq:
        if not cur_token.isspace():
            res_str += f'{cur_token}'
        if idx < len(token_seq):
            res_str += ","
        idx += 1
    res_str += (f"])\\n\")\n")

    return res_str

# def force_modify_keyword(cur_key: str)->str:
    # if cur_key == "FLOATP":
        # return "FLOAT"
    # elif cur_key == "BOOLEANP":
        # return "BOOLEAN"
    # else:
        # return cur_key

grammar_fd = open("assets/mariadb_grammar.y", "r")

grammar_str = grammar_fd.read()
tmp_res_str = ""

is_slash_blocked = False
is_star_comment_blocked = False

# Remove the EXTEND WITH HELP
# grammar_str = grammar_str.replace("EXTEND WITH HELP", " ")

# Remove comments first.
for idx in range(len(grammar_str)):
    # if grammar_str[idx] == "\n" and is_slash_blocked == True:
    #     is_slash_blocked = False
    #     tmp_res_str += "\n"
    #     continue
    if grammar_str[idx] == '/' and grammar_str[idx-1] == '*':
        if is_star_comment_blocked == False:
            tmp_res_str += grammar_str[idx]
        is_star_comment_blocked = False
        continue
    elif is_slash_blocked or is_star_comment_blocked:
        continue
    elif grammar_str[idx] == '/' and grammar_str[idx+1] == '*' and (grammar_str[idx+2] != "E" and grammar_str[idx+3] != "E"):
        is_star_comment_blocked = True
        continue
    # elif grammar_str[idx] == '/' and grammar_str[idx+1] == '/':
    #     is_slash_blocked = True
    #     continue
    tmp_res_str += grammar_str[idx]

tmp_str = ""
# tmp_res_str = grammar_str

# Remove the prefix spacing.
for cur_line in tmp_res_str.splitlines():
    if cur_line.startswith(" |"):
        cur_line = cur_line[1:]
    if cur_line.startswith("  |"):
        cur_line = cur_line[2:]
    tmp_str += cur_line + "\n"
tmp_res_str = tmp_str

parser_prefix_str = tmp_res_str.split("%%")[0]
parser_rule_str = tmp_res_str.split("%%")[1]

cur_keyword_has_rules = True
res_has_cov = ""
cur_token_seq = ""
cur_keyword = ""
all_rule_maps = dict()

parent_level = 0

rule_idx = 0
for cur_line in parser_rule_str.splitlines():
    if ":" in cur_line and "':'" not in cur_line and parent_level == 0:

        is_mistake = False
        if "{" in cur_line:
            for idx in range(len(cur_line)):
                if cur_line[idx] == "{":
                    is_mistake = True
                    break
                if cur_line[idx] == ":":
                    is_mistake = False
                    break
        if "//" in cur_line and '"//"' not in cur_line:
            tmp_idx_0 = cur_line.find("//")
            tmp_idx_1 = cur_line.find(":")
            if tmp_idx_0 < tmp_idx_1:
                is_mistake = True
            else:
                is_mistake = False
        if is_mistake == False:
            # This is a new token line.
            if cur_keyword_has_rules == False and len(cur_keyword) != 0 and "_keyword" not in cur_keyword and "error" not in cur_token_seq:
                res_has_cov += "{\n" + gen_cov_logging_func_call(
                    cur_keyword=cur_keyword, token_seq=cur_token_seq, rule_idx=rule_idx) + "\n}\n"
            cur_token_seq = ""

            rule_idx = 0
            cur_keyword = cur_line.split(":")[0]
            cur_keyword_has_rules = False

            res_has_cov += cur_keyword + ":"
            cur_line = ":".join(cur_line.split(":")[1:])
            # always write to the result rules now. do not run 'continue'

    if "|" in cur_line and parent_level == 0:

        is_mistake = False
        if "{" in cur_line:
            for idx in range(len(cur_line)):
                if cur_line[idx] == "{":
                    is_mistake = True
                    break
                if cur_line[idx] == "|":
                    is_mistake = False
                    break
        if "//" in cur_line and "\"//\"" not in cur_line:
            tmp_idx_0 = cur_line.find("//")
            tmp_idx_1 = cur_line.find("|")
            if tmp_idx_0 < tmp_idx_1:
                is_mistake = True
            else:
                is_mistake = False
        if is_mistake == False:
            # only take care of the first symbol |
            cur_token_seq += cur_line.split("|")[0]
            if cur_keyword not in all_rule_maps:
                all_rule_maps[cur_keyword] = [cur_token_seq.split()]
            else:
                all_rule_maps[cur_keyword].append(cur_token_seq.split())

            if cur_keyword_has_rules == False and len(cur_keyword) != 0 and "_keyword" not in cur_keyword and "error" not in cur_token_seq:
                res_has_cov += "{\n" + gen_cov_logging_func_call(
                    cur_keyword=cur_keyword, token_seq=cur_token_seq, rule_idx=rule_idx) + "\n}\n"
                rule_idx += 1
            cur_token_seq = ""
            res_has_cov += cur_line.split("|")[0] + "|"

            cur_line = "|".join(cur_line.split("|")[1:])
            cur_keyword_has_rules = False

    saving_token = True
    is_add_gram_cov = False
    is_comment_text = False
    for cur_char_idx in range(len(cur_line)):
        cur_char = cur_line[cur_char_idx]

        if cur_char == "/" and (cur_char_idx + 1) < len(cur_line) and cur_line[cur_char_idx+1] == "/" and (cur_char_idx + 2) < len(cur_line) and cur_line[cur_char_idx+2] != "\"":
            is_comment_text = True

        if cur_char == '{' and cur_line[cur_char_idx-1] != "'" and is_comment_text == False:
            parent_level += 1
            saving_token = False
            # res_has_cov += f"parent_level: {parent_level}"

            if parent_level == 1:
                is_add_gram_cov = True
                # Trigger the ending of one single rule action.
                cur_keyword_has_rules = True

        elif cur_char == '}' and cur_line[cur_char_idx-1] != "'" and is_comment_text == False:
            parent_level -= 1
            # res_has_cov += f"parent_level: {parent_level}"

        # if parent_level < 0:
            # print(f"Error: parent: {parent_level}, {cur_char}, keyword: {cur_keyword}, token_seq: {cur_token_seq}\n")
            # exit(1)

        res_has_cov += cur_char

        if is_add_gram_cov == True:
            # Trigger the ending of one single rule action.
            cur_keyword_has_rules = True
            # res_has_cov += (f"Error: parent: {parent_level}, {cur_char}, keyword: {cur_keyword}, token_seq: {cur_token_seq}\n")
            if "error" not in cur_token_seq:
                res_has_cov += "\n" + gen_cov_logging_func_call(
                    cur_keyword=cur_keyword, token_seq=cur_token_seq, rule_idx=rule_idx) + "\n"
                rule_idx += 1
            cur_token_seq = ""
            is_add_gram_cov = False

        if parent_level == 0 and saving_token == True and cur_char != "}" and is_comment_text == False:
            cur_token_seq += cur_char

    res_has_cov += "\n"

tmp_res_str = res_has_cov
res_has_cov = ""

tmp_res_str = "\n%%\n".join([parser_prefix_str, tmp_res_str])

# for cur_line in tmp_res_str.splitlines():
#     if cur_line.isspace() or len(cur_line) == 0:
#         continue
#     res_has_cov += cur_line + "\n"

# mutual-exclusive with previous lines.
res_has_cov = tmp_res_str

res_has_cov += "\n%%\n"

# out_fd = open("assets/duckdb_grammar_modi.y", "w")
# out_fd.write(res_has_cov)
# out_fd.close()

# IR Types header generation related.

ir_type_prefix = """
#ifndef  IR_TYPES_CUSTOM_H
#define  IR_TYPES_CUSTOM_H

#define ALLIRTYPE(V) \\
V(IRTypeUnknownType) \\
V(IRTypeBOOLEAN) \\
V(IRTypeFLOAT) \\
V(IRTypeINTEGER) \\
V(IRTypeSTRING) \\
V(IRTypeOptOnEmptyOrErrorJsonTable) \\
V(IRTypeCreateTable) \\
V(IRTypeCreateView) \\
V(IRTypeCreateIndex) \\
V(IRTypeOptIndexHintsList) \\
V(IRTypeReservedKeywordUdtParamType) \\
"""

ir_type_suffix = """


#endif // IR_TYPES_CUSTOM_H 
"""

out_fd = open("assets/ir_types_custom.h", "w")
out_fd.write(ir_type_prefix)

ir_types_dict_dedup = dict()
for cur_key, cur_type in all_saved_ir_types_mapping.items():
    cur_type = cur_type.replace(";", "")
    cur_key = cur_key.replace(";", "")
    if "Standard." in cur_type:
        continue
    if cur_type == "ident" or cur_type == "Ident":
        continue
    if cur_type not in ir_types_dict_dedup:
        ir_types_dict_dedup[cur_type] = 1
    else:
        continue
    if "*/" in cur_type:
        continue
    elif "')'" in cur_type:
        continue
    cur_type = "IRType" + cur_type
    out_fd.write(f"V({cur_type}) \\\n")
    # modi_key = force_modify_keyword(cur_key)
    # if modi_key is not None:
    #     cur_key = "IRType" + modi_key
    #     out_fd.write(f"V({modi_key}) \\\n")

out_fd.write(ir_type_suffix)
out_fd.close()

os.system("clang-format -i assets/ir_types_custom.h")

out_mapping_fd = open("assets/ir_types_mapping.txt", "w")
out_mapping_fd.write(f"FLOAT,IRTypeFLOAT\n")
out_mapping_fd.write(f"BOOLEAN,IRTypeBOOLEAN\n")
out_mapping_fd.write(f"TEXT_STRING,IRTypeSTRING\n")
out_mapping_fd.write(f"ident,IRTypeIDENT\n")
out_mapping_fd.write(f"Ident,IRTypeIDENT\n")
out_mapping_fd.write(f"opt_on_empty_or_error_json_table,IRTypeOptOnEmptyOrErrorJsonTable\n")
out_mapping_fd.write(f"create_table,IRTypeCreateTable\n")
out_mapping_fd.write(f"create_view,IRTypeCreateView\n")
out_mapping_fd.write(f"create_index,IRTypeCreateIndex\n")
out_mapping_fd.write(f"opt_index_hints_list,IRTypeOptIndexHintsList\n")
out_mapping_fd.write(f"reserved_keyword_udt_param_type,IRTypeReservedKeywordUdtParamType\n")

for cur_value, cur_type in all_saved_ir_types_mapping.items():
    if cur_value == "ident":
        continue
    elif "standard" in cur_value:
        continue
    elif "." in cur_value:
        cur_value = cur_value.replace(".", "")
        cur_type = cur_type.replace(".", "")
    elif "*" in cur_value:
        continue
    elif ";" in cur_value:
        cur_value = cur_value.replace(";", "")
        cur_type = cur_type.replace(";", "")
    out_mapping_fd.write(f"{cur_value},IRType{cur_type}\n")
out_mapping_fd.close()
