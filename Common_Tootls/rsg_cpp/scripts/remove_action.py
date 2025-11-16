import os
import re
import sys

if len(sys.argv) != 2:
    print("Usage: python3 remove_action.py <input_grammar_file_path>")
    exit(1)

input_file = sys.argv[1]

grammar_fd = open(input_file, "r")

grammar_str = grammar_fd.read()

parent_level = 0
res_str = ""

grammar_str = grammar_str.split("%%")[1]

# Dirty patch for MySQL grammar. 
res_str = ""
for cur_line in grammar_str.splitlines():
    if "not only the '{LEFT | RIGHT} [OUTER] JOIN' syntax" in cur_line:
        continue
    res_str += cur_line + "\n"

res_str = res_str.replace("%empty", "/* EMPTY */")

grammar_str = res_str
res_str = ""

previous_char = ""
for cur_char in grammar_str:
    if cur_char == '{' and previous_char != "'":
        parent_level += 1
        previous_char = cur_char
        continue
    elif cur_char == '}' and previous_char != "'":
        parent_level -= 1
        previous_char = cur_char
        continue
    elif parent_level != 0:
        previous_char = cur_char
        continue
    else:
        res_str += cur_char
        previous_char = cur_char

grammar_str = res_str
res_str = ""

for cur_line in grammar_str.splitlines():
    if cur_line.startswith("//"):
        continue
    if cur_line.startswith("%ifdef") or cur_line.startswith("%endif") or cur_line.startswith("%else"):
        continue
    if "//" in cur_line:
        cur_line = cur_line.split("//")[0]

    # repeatly delete all /* */ comments.
    cur_line = re.sub("\\/\\*.*?EMPTY.*?\\*\\/", "EMPTYMARKKKKK", cur_line)
    tmp_line = cur_line
    while True:
        cur_line = re.sub("\\/\\*.*?\\*\\/", " ", tmp_line)
        if tmp_line == cur_line:
            break
        tmp_line = cur_line

    cur_line = cur_line.replace("EMPTYMARKKKKK", "/* EMPTY */")

    res_str += cur_line + "\n"

grammar_str = res_str
res_str = ""

is_first_rule = True
for cur_line in grammar_str.splitlines():
    if cur_line.isspace() or len(cur_line) == 0:
        continue
    if ":" in cur_line and "':'" not in cur_line:
        if is_first_rule:
            is_first_rule = False
        else:
            res_str += ";\n"
    res_str += cur_line + "\n"
res_str += ";"

grammar_str = res_str
res_str = ""
is_skip = False
idx = 0
split_lines_tmp = grammar_str.splitlines()
for cur_line in split_lines_tmp:
    # print(f"Getting {idx}/{len(split_lines_tmp)}", end="\n")
    idx += 1
    cur_line = cur_line.strip()
    if cur_line.startswith("/*") and "*/" not in cur_line:
        is_skip = True
        continue
    elif cur_line.endswith("*/") and not cur_line.endswith("EMPTY */"):
        is_skip = False
        continue
    elif is_skip:
        continue
    elif cur_line.startswith("#"):
        continue
    else:
        res_str += cur_line.strip() + "\n"

res_str = res_str.replace(";\n;\n", ";\n")
grammar_str = res_str

if grammar_str.splitlines()[0].startswith(";"):
    grammar_str = "\n".join(grammar_str.splitlines()[1:])

if grammar_str[-1] != ";" and "mysql" in input_file:
    grammar_str += ";"

# output
input_file = input_file.split(".y")[0]
out_fd = open(f"{input_file}_modi.y", "w")
out_fd.write(grammar_str)