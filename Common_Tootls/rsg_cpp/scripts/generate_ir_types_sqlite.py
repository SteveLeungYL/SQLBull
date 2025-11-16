import sys
import os.path
import json
from loguru import logger
from typing import List
import re


ONETAB = " " * 4
ONESPACE = " "
default_ir_type = "kUnknown"

saved_ir_type = []

logger.remove()
logger.add(sys.stderr, level="INFO") # or sys.stdout or other file object

all_translated_types = []
all_rule_maps = dict()

total_edge_num = 0
total_block_num = 0

terminating_keyword_array = []
token_to_ir_type_map = dict()

class Token(object):
    def __init__(self, value, index):
        self.value = value 
        self.index = index

    @property
    def is_term_token(self):
        if self.value.startswith("'") or self.value.endswith("'"):
            return True
        if self.value[0].isupper():
            return True

        return False

    def __str__(self) -> str:
        if self.is_term_token:
            if self.value.startswith("'") and self.value.endswith("'"):
                return self.value.strip("'")

        return self.value

    def __repr__(self) -> str:
        return '{prefix}("{word}")'.format(
            prefix="Keyword" if self.is_term_token else "Token", word=self.value
        )

    def __gt__(self, other):
        other_index = -1
        if isinstance(other, Token):
            other_index = other.index

        return self.index > other_index


def snake_to_camel(word: str):
    return "".join(x.capitalize() or "_" for x in word.split("_"))


def camel_to_snake(word: str):
    return "".join(["_" + i.lower() if i.isupper() else i for i in word]).lstrip("_")

def is_terminating_keyword(word: str) -> bool:
    if word == "id" or word == "ids" or word == "number" or word == "idj":
        return True
    if word[0].isupper():
        return True
    else:
        return False

def search_next_keyword(token_seq, start_index):

    global terminating_keyword_array

    curr_token = None
    term_keywords = []

    if start_index >= len(token_seq):
        return curr_token, term_keywords

    for idx in range(start_index, len(token_seq)):
        if is_terminating_keyword(token_seq[idx]):
            curr_term_keyword = Token(token_seq[idx], idx)
            if token_seq[idx] not in terminating_keyword_array:
                terminating_keyword_array.append(token_seq[idx])
            term_keywords.append(curr_term_keyword)
        else:
            curr_token = Token(token_seq[idx], idx)
            break

    return curr_token, term_keywords


def ir_type_str_rewrite(cur_types) -> str:
    if cur_types == "":
        return "IRTypeUnknownType"

    cur_types_l = list(cur_types)
    cur_types_l[0] = cur_types_l[0].upper()

    is_upper = False
    for cur_char_idx in range(len(cur_types_l)):
        if cur_types_l[cur_char_idx] == "_":
            is_upper = True
            cur_types_l[cur_char_idx] = ""
            continue
        if is_upper == True:
            is_upper = False
            cur_types_l[cur_char_idx] = cur_types_l[cur_char_idx].upper()

    cur_types = "".join(cur_types_l)
    cur_types = "IRType" + cur_types
    return cur_types

def translate_single_rule(token_seq, parent):
    global total_block_num
    global all_rule_maps

    all_saved_str = parent  + "::= "

    tmp_idx = 0
    for cur_token in token_seq:
        all_saved_str += cur_token + " "
        tmp_idx += 1

    if parent not in all_rule_maps:
        all_rule_maps[parent] = [token_seq]
    else:
        all_rule_maps[parent].append(token_seq)

    total_block_num += 1
    return all_saved_str

def translate_single_action(token_seq, parent):

    i = 0
    tmp_num = 1
    body = ""
    need_more_ir = False

    if len(token_seq) == 0:
        logger.debug("Getting empty rule.")
        body += (
            f"""A = new IR({default_ir_type}, OP0());"""
            + "\n"
        )


    while i < len(token_seq):
        left_token, left_keywords = search_next_keyword(token_seq, i)
        logger.debug(f"Left tokens: '{left_token}', Left keywords: '{left_keywords}'")

        right_token = None
        mid_keywords = []
        if left_token is not None:
            right_token, mid_keywords = search_next_keyword(
                token_seq, left_token.index+1
            )
        right_keywords = []
        if right_token:
            _, right_keywords = search_next_keyword(
                token_seq, right_token.index + 1
            )

        left_keywords_str = " + ".join(
            ["string(" + chr(ord('B') + token.index) + ")" for token in left_keywords]
        )
        mid_keywords_str = " + ".join(
            ["string(" + chr(ord('B') + token.index) + ")" for token in mid_keywords]
        )
        right_keywords_str = " + ".join(
            ["string(" + chr(ord('B') + token.index) + ")" for token in right_keywords]
        )

        if len(left_keywords_str) == 0:
            left_keywords_str = "\"\""
        if len(mid_keywords_str) == 0:
            mid_keywords_str = "\"\""
        if len(right_keywords_str) == 0:
            right_keywords_str = "\"\""

        if need_more_ir:
            # Second or more loop
            # left node has been pre-defined as res.

            tmp_var = chr(ord('B') + left_token.index)
            body += (
                f"""A = new IR({default_ir_type}, OP3("", {left_keywords_str}, {mid_keywords_str}), (IR*)A, (IR*){tmp_var});"""
                + "\n"
            )
            tmp_num += 1

            if right_token is not None:
                tmp_var = chr(ord('B') + right_token.index)
                body += (
                    f"""A = new IR({default_ir_type}, OP3("", "", {right_keywords_str}), (IR*)A, (IR*){tmp_var});"""
                    + "\n"
                )
                tmp_num += 1

        elif right_token is not None:
            tmp_var = chr(ord('B') + left_token.index)
            tmp_var_2 = chr(ord('B') + right_token.index)
            body += (
                f"""A = new IR({default_ir_type}, OP3({left_keywords_str}, {mid_keywords_str}, {right_keywords_str}), (IR*){tmp_var}, (IR*){tmp_var_2});"""
                + "\n"
            )

            tmp_num += 2
            need_more_ir = True

        elif left_token is not None and (
                    left_token.index == len(token_seq) - 1 or
                    len(mid_keywords) > 0 and
                    mid_keywords[-1].index == len(token_seq) - 1
                ):
            # only single one token.
            logger.debug("Getting only single one non-term token. ")
            tmp_var = chr(ord('B') + left_token.index)
            body += (
                f"""A = new IR({default_ir_type}, OP3({left_keywords_str}, {mid_keywords_str}, ""), (IR*){tmp_var});"""
                + "\n"
            )

            tmp_num += 1
            need_more_ir = True
            break

        # Only zero or more keywords here.
        else:
            logger.debug("Getting Zero or more keywords only.")
            body += (
                f"""A = new IR({default_ir_type}, OP3({left_keywords_str}, "", ""));"""
                + "\n"
            )
            break

        compare_tokens = left_keywords + mid_keywords + right_keywords
        if left_token is not None:
            compare_tokens.append(left_token)
        if right_token is not None:
            compare_tokens.append(right_token)

        max_index_token = max(compare_tokens)
        i = max_index_token.index + 1

    if body:
        ir_type_str = ir_type_str_rewrite(parent)
        if parent not in token_to_ir_type_map:
            token_to_ir_type_map[parent] = ir_type_str
        # body = f"k{ir_type_str}".join(body.rsplit(default_ir_type, 1))
        # body += "*root_ir = (IR*)(A);\n"
        if f"{ir_type_str}" not in all_translated_types:
            all_translated_types.append(f"{ir_type_str}")


    logger.debug(f"Result: \n{body}")
    return body

def handle_ori_comp_parser() -> str:
    # gather all the token information first. 

    file_fd = open("./assets/sqlite_ori_parser.y")

    all_lines = file_fd.readlines()
    all_saved_lines = ""

    is_fallback_multiline = False
    is_def_ignore = False
    rule_is_read = False
    macro_is_read = True 

    for cur_line in all_lines:

        # ignore the #ifdef line and all the contents between
        if cur_line.startswith("%ifdef "):
            is_def_ignore = True
            continue
        if "%endif" in cur_line:
            if is_def_ignore == True:
                is_def_ignore = False
            macro_is_read = True
            continue
        if is_def_ignore == True:
            continue

        # ignore all the `#ifndef` line, but still save all the things between.
        if cur_line.startswith("%ifndef ")  or \
                cur_line.startswith("%endif"):
                    continue

        # For the fallback grammar
        if cur_line.startswith("%fallback "):
            # all_saved_lines += cur_line
            is_fallback_multiline = True
            continue
        if cur_line.startswith("%token ") or cur_line.startswith("%token\n"):
            # all_saved_lines += cur_line
            if "." not in cur_line:
                is_fallback_multiline = True
            continue
        if is_fallback_multiline and "." in cur_line:
            # all_saved_lines += cur_line
            is_fallback_multiline = False
            continue
        if is_fallback_multiline == True:
            # all_saved_lines += cur_line
            continue


        # All other saved types.
        if cur_line.startswith("%left ") or \
                cur_line.startswith("%right ") or \
                cur_line.startswith("%nonassoc ") or \
                cur_line.startswith("%wildcard ") or \
                cur_line.startswith("%token_class "):
            # the line contains the new line symbol
            # all_saved_lines += cur_line
            continue

        if cur_line.startswith("%type "):
            # Change all the non-terminal types to IR*.
            cur_line = cur_line.split("{")[0]
            cur_line += "{IR*}\n"
            # all_saved_lines += cur_line
            continue


        if macro_is_read == False:
            continue

        if "%else" in cur_line:
            macro_is_read = False
            continue
        
        if rule_is_read == True and "." in cur_line:
            cur_line = cur_line.split("{")[0]
            cur_line = re.sub("\n", "", cur_line)
            # remove all the bracket and the contents within.
            cur_line = re.sub("[\(].*?[\)]", "", cur_line)
            all_saved_lines += cur_line+"\n"
            rule_is_read = False
            continue

        if rule_is_read == True and "." not in cur_line:
            cur_line = re.sub("[\(].*?[\)]", "", cur_line)
            cur_line = re.sub("\n", "", cur_line)
            all_saved_lines+=cur_line
            continue

        if "::=" in cur_line and "." not in cur_line:
            cur_line = re.sub("[\(].*?[\)]", "", cur_line)
            cur_line = re.sub("\n", "", cur_line)
            all_saved_lines+=cur_line
            rule_is_read = True
            continue

        if "::=" in cur_line and "." in cur_line:
            cur_line = cur_line.split("{")[0]
            cur_line = re.sub("\n", "", cur_line)
            # remove all the bracket and the contents within.
            cur_line = re.sub("[\(].*?[\)]", "", cur_line)
            all_saved_lines+=cur_line+"\n"
            continue

    all_saved_lines = all_saved_lines.replace("|", " | ")
    logger.debug("\n\n\nGetting all_saved_lines for token declaration : %s\n\n\n"%(all_saved_lines))

    file_fd.close()
    
    return all_saved_lines

def get_rules_text(all_saved_str: str) -> str:
    # gather all the token information first. 

    all_lines = all_saved_str.splitlines()
    all_saved_lines = ""

    for cur_line in all_lines:
        if cur_line.startswith("// "):
            continue

        if "::=" not in cur_line:
            all_saved_lines += cur_line + "\n"
            continue

        ori_line = cur_line
        # Remove the "\n" at the end.
        cur_line = cur_line[:-1]

        # Remove the . sign
        cur_line_split = cur_line.split(".")
        cur_line = cur_line_split[0]
        cur_line_after_dot = ""
        if len(cur_line_split) > 1:
            cur_line_after_dot = cur_line_split[1]

        token_list = cur_line.split()

        cur_keyword = token_list[0]

        if len(token_list) > 2:
            token_list = token_list[2:]
        else:
            token_list = []

        logger.debug(f"Translating single rule: {cur_keyword}")
        all_saved_lines += translate_single_rule(token_list, cur_keyword)
        all_saved_lines += ".\n\n"
        # all_saved_lines += ". " + cur_line_after_dot + "{\n"

        # Run by ignore the return body. 
        translate_single_action(token_list, cur_keyword)

        # all_saved_lines += "}\n\n"

    return all_saved_lines

def calc_total_edge_num():
    global all_rule_maps
    global total_edge_num

    total_edge_num = 0

    for _, list_token_seq in all_rule_maps.items():
        # print(list_token_seq)
        for token_seq in list_token_seq:
            all_token_enum = 0
            pre_token_enum = 0
            idx = 0
            for cur_token in token_seq:
                if cur_token in all_rule_maps:
                    idx += 1
                    if pre_token_enum == 0:
                        pre_token_enum = len(all_rule_maps[cur_token])
                        continue
                    all_token_enum += len(all_rule_maps[cur_token]) * pre_token_enum
                    pre_token_enum = len(all_rule_maps[cur_token])

            if idx == 1:
                all_token_enum += pre_token_enum
            if pre_token_enum == 0:
                all_token_enum += 1

            # print("for %s, getting edge: %d" % (token_seq, all_token_enum))
            total_edge_num += all_token_enum

    # for _, list_token_seq in all_rule_maps.items():
        # print(list_token_seq)
        # is_prev_term = True
        # for token_seq in list_token_seq:
            # cur_rule_enum = 0
            # tmp = 0
            # for cur_token in token_seq:
                # if cur_token in all_rule_maps:
                    # if tmp == 0:
                        # tmp = 1
                    # # is non-term
                    # if not is_prev_term:
                        # # prev is NOT term
                        # # multiply
                        # tmp = tmp * len(all_rule_maps[cur_token])
                    # else:
                        # # prev is term
                        # # reset prev
                        # cur_rule_enum += len(all_rule_maps[cur_token])
                        # tmp = 0
                    # is_prev_term = True
                # else:
                    # is_prev_term = False
            # cur_rule_enum += tmp

            # print("for %s, getting edge: %d" % (token_seq, cur_rule_enum))
            # total_edge_num += cur_rule_enum

def write_to_ir_types_file(all_ir_type_fd):
    global all_translated_types
    global token_to_ir_type_map

    already_saved_types = set()
    already_saved_types.add("IRTypeBOOLEAN")
    already_saved_types.add("IRTypeUnknownType")
    already_saved_types.add("IRTypeIDENT")
    already_saved_types.add("IRTypeFLOAT")
    already_saved_types.add("IRTypeSelectStmt")
    already_saved_types.add("IRTypeFuncApplication")
    already_saved_types.add("IRTypeFuncExpr")
    already_saved_types.add("IRTypeFuncName")
    already_saved_types.add("IRTypeCreateViewStmt")
    already_saved_types.add("IRTypeCreateTableStmt")
    already_saved_types.add("IRTypeCreateIndexStmt")
    already_saved_types.add("IRTypeCreateTriggerStmt")
    already_saved_types.add("IRTypeAlterStmt")
    already_saved_types.add("IRTypeInsertStmt")

    prefix_str = """
#ifndef  IR_TYPES_CUSTOM_H
#define  IR_TYPES_CUSTOM_H

#define ALLIRTYPE(V) \\
V(IRTypeUnknownType) \\
V(IRTypeIDENT) \\
V(IRTypeBOOLEAN) \\
V(IRTypeFLOAT) \\
V(IRTypeSelectStmt) \\
V(IRTypeFuncApplication) \\
V(IRTypeFuncExpr) \\
V(IRTypeFuncName) \\
V(IRTypeCreateViewStmt) \\
V(IRTypeCreateTableStmt) \\
V(IRTypeCreateIndexStmt) \\
V(IRTypeCreateTriggerStmt) \\
V(IRTypeAlterStmt) \\
V(IRTypeInsertStmt) \\
"""

    token_to_ir_type_map["create_view_stmt"] = "IRTypeCreateViewStmt"
    token_to_ir_type_map["create_table_stmt"] = "IRTypeCreateTableStmt"
    token_to_ir_type_map["create_index_stmt"] = "IRTypeCreateIndexStmt"
    token_to_ir_type_map["create_trigger_stmt"] = "IRTypeCreateTriggerStmt"
    token_to_ir_type_map["insert_stmt"] = "IRTypeInsertStmt"
    token_to_ir_type_map["alter_stmt"] = "IRTypeAlterStmt"

    all_ir_type_fd.write(prefix_str)

    for cur_type in all_translated_types:
        if cur_type not in already_saved_types:
            all_ir_type_fd.write(f"V({cur_type}) \\\n")
            already_saved_types.add(cur_type)

    for cur_keyword in terminating_keyword_array:
        cur_true_keyword = f"IRType{cur_keyword}"
        if cur_true_keyword not in already_saved_types:
            all_ir_type_fd.write(f"V({cur_true_keyword}) \\\n")
            already_saved_types.add(cur_true_keyword)

    suffix_str = """

#endif // IR_TYPES_CUSTOM_H 
"""

    all_ir_type_fd.write(suffix_str)
    all_ir_type_fd.close()

    os.system("clang-format -i assets/ir_types_custom.h")

def write_to_ir_types_mapping_file(all_ir_type_fd):
    global token_to_ir_type_map

    for cur_keyword, cur_ir_type in token_to_ir_type_map.items():
        if cur_ir_type == "IRTypeSelect":
            cur_ir_type = "IRTypeSelectStmt"
        all_ir_type_fd.write(f"{cur_keyword},{cur_ir_type}\n")
    
    for cur_keyword in terminating_keyword_array:
        all_ir_type_fd.write(f"{cur_keyword},IRType{cur_keyword}\n")

def append_custom_rules(rules_str):
    # for create view:
    rules_str += "\ncreate_view_stmt::= createkw temp VIEW ifnotexists nm dbnm eidlist_opt AS select . \n" 
    rules_str += "\ncmd::= create_view_stmt . \n" 

    # for create table:
    rules_str += "\ncreate_table_stmt::= create_table create_table_args .\n" 
    rules_str += "\ncmd::= create_table_stmt . \n" 

    # for create index:
    rules_str += "\ncreate_index_stmt::= createkw uniqueflag INDEX ifnotexists nm dbnm ON nm LP sortlist RP where_opt .\n" 
    rules_str += "\ncmd::= create_index_stmt . \n" 

    # for create trigger:
    rules_str += "\ncreate_trigger_stmt::= createkw trigger_decl BEGIN trigger_cmd_list END .\n" 
    rules_str += "\ncmd::= create_trigger_stmt . \n" 

    # for insert statement:
    rules_str += "\ninsert_stmt::= with insert_cmd INTO xfullname idlist_opt select upsert . \n\ninsert_stmt::= with insert_cmd INTO xfullname idlist_opt DEFAULT VALUES returning .\n"
    rules_str += "\ncmd::= insert_stmt . \n"

    # for the modified from expression:
    rules_str += "\nfrom::= FROM nm COMMA seltablist .\n"

    # for alter statements:
    rules_str += """
\nalter_stmt::= ALTER TABLE fullname RENAME TO nm .

alter_stmt::= ALTER TABLE add_column_fullname ADD kwcolumn_opt columnname carglist .

alter_stmt::= ALTER TABLE fullname DROP kwcolumn_opt nm .

alter_stmt::= ALTER TABLE fullname RENAME kwcolumn_opt nm TO nm .

cmd::= alter_stmt .
"""

    return rules_str


def run(output_fd, all_ir_type_fd):
    global total_block_num
    global total_edge_num

    token_str = handle_ori_comp_parser()
    rules_str = get_rules_text(token_str)

    rules_str = append_custom_rules(rules_str)

    # output_fd.write(predef_str)
    # output_fd.write(token_str)
    output_fd.write(rules_str)

    write_to_ir_types_file(all_ir_type_fd)
    with open("assets/ir_types_mapping.txt", "w") as fd:
        write_to_ir_types_mapping_file(fd)

    # Summarize the total block number and total edge number
    calc_total_edge_num()
    # logger.info("Total block num: %d, total edge num: %d.\n"% (total_block_num, total_edge_num))

    return

if __name__ == "__main__":

    output_file_str = "assets/sqlite_parser.y"
    all_ir_type_file_str = "assets/ir_types_custom.h"

    if os.path.exists(output_file_str):
        os.remove(output_file_str)

    if not os.path.exists("./assets"):
        os.error("Error: The assets folder does not exists in the current working \
                directory: %s. \n", os.getcwd())
    if not os.path.isfile("./assets/sqlite_ori_parser.y"):
        os.error("Error: sqlite_ori_parser.y not found. \n")

    with open(output_file_str, "w+") as fd, open(all_ir_type_file_str, "w+") as fd2:
        run(fd, fd2)
