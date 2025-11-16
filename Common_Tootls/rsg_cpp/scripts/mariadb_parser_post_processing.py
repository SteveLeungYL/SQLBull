import os
import re


def main():
    res_lines = "\n"

    with open("assets/mariadb_grammar_modi.y", "r") as f:
        all_lines = f.read()

        if ("create_table:" in all_lines):
            return

        all_lines = all_lines.split("create:\n")[1]
        all_lines = all_lines.split(";")[0]

        index_str = "create_index:\n"
        is_first_index = True

        view_str = "create_view:\n"
        is_first_view = True

        for cur_sub_rule in all_lines.split("|"):
            if "TABLE_SYM" in cur_sub_rule:
                res_lines += "create_table:\n"
                res_lines += cur_sub_rule
                res_lines += ";\n"
            elif "INDEX_SYM" in cur_sub_rule:
                if not is_first_index:
                    index_str += "| "
                is_first_index = False
                index_str += cur_sub_rule
            elif "VIEW_SYM" in cur_sub_rule:
                if not is_first_view:
                    view_str += "| "
                is_first_view = False
                view_str += cur_sub_rule
            else:
                pass 

        index_str += ";\n"
        view_str += ";\n"

        res_lines += index_str
        res_lines += view_str

    with open("assets/mariadb_grammar_modi.y", "a") as f:
        f.write(res_lines)
        f.flush()

    # Also, remove the _empty keyword in the grammar. 
    with open("assets/mariadb_grammar_modi.y", "r") as f:
        all_lines = f.read()
    all_lines = all_lines.replace("_empty:\n;", "")
    res_lines = ""
    for cur_line in all_lines.splitlines():
        if cur_line.startswith("_empty"):
            res_lines += " /* EMPTY */ " + "\n"
        elif cur_line == "| analyze_stmt_command":
            res_lines += "| analyze_stmt_command\n"
            res_lines += "| create_table\n"
            res_lines += "| create_index\n"
            res_lines += "| create_view\n"
        else:
            res_lines += cur_line + "\n"

    with open("assets/mariadb_grammar_modi.y", "w") as f:
        f.write(res_lines)

if __name__ == "__main__":
    main()
