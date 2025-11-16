import os
import sys
import collections.abc

bug_pattern_list = [
    ["Attempted to dereference unique_ptr that is NULL", "bind_insert.cpp\" on line 501: stmt.select_statement", "stmt.select_statement"],
    "bind_select_node.cpp\" on line 313",
    "bind_insert.cpp\" on line 501",
    # this might need more testing.
    "INTERNAL Error: Failed to bind column reference",
    "Unexpected child of pivot source - not a ColumnRef",
    "INTERNAL Error: No binding with name",
    "logical_join.cpp\" on line 63: colref.depth == 0",
    "Logical operator type \"POSITIONAL_JOIN\" for dependent join",
    "Failed to cast expression to type - expression type mismatch",
    "Attempting to dereference an optional pointer that is not set",
    "Failed to bind column reference",
    "INTERNAL Error: Calling ExpressionExecutor::GetContext on an expression executor without a context",
    "bind_insert.cpp\" on line 151: insert.children[0]->type",
    "Attempting to initialize state of expression of unknown type",
    "types.cpp\" on line 1674",
    "Attempted to access index 0 within vector of size 0",
    "column_binding_resolver.cpp\" on line 151: expr.depth == 0",
    "children.size() == 1",
    "no more tables that refer to this using binding",
    "cannot remove primary binding from using binding",
    "!select_list.empty()",
    "Cannot ToString bound subquery node",
    "Expression with depth > 1 detected in non-lateral join",
    "physical_insert.cpp\" on line 150",
    "physical_insert.cpp\" on line 157",
    "Vector::Reference used on vector of different type",
    "std::find(other_alias.get().bindings.begin()",
    "entry.values.empty()",
    "left.GetType() == right.GetType() &&",
    "Value::LIST without providing a child-type requires a non-empty list of values",
    "new_prepared->properties.bound_all_parameters",
    "col_ref.column_names.size() <= 3",
    "node.type == QueryNodeType::SELECT_NODE",
    "types == input.GetTypes()",
    "!simplified_mark_join",
    "allow_unfoldable || expr.IsFoldable()",
    "source_p.GetType() == target.GetType()",
    "!values.empty()",
]


def is_bug_match(cur_str: str):
    global bug_pattern_list

    for cur_bug_pattern in bug_pattern_list:
        if isinstance(cur_bug_pattern, str):
            cur_bug_pattern = [cur_bug_pattern]
        for cur_pattern in cur_bug_pattern:
            if cur_pattern in cur_str:
                return cur_bug_pattern[0]

    return None


def get_bug_triggering_query_line(cur_str: str) -> str:
    prev_line = ""
    for cur_line in cur_str.splitlines():
        if len(cur_line) == 0:
            if prev_line == "":
                print(
                    "ERROR!!!! get_bug_triggering_query_line is not working properly. ")
                exit(1)
            return prev_line
        prev_line = cur_line


class BugStruct:
    file_name: str
    detect_time: int

    def __init__(self, file_name, detect_time):
        self.file_name = file_name
        self.detect_time = detect_time

    def __str__(self):
        return f"File: {self.file_name} Detection Time: {self.detect_time}"

    def __repr__(self):
        return f"File: {self.file_name} Detection Time: {self.detect_time}"


detected_bug_dict = dict()

bug_num = 0

start_time = 0

with open("./start_time", "r") as fd:
    start_time = int(fd.read())

for cur_file in os.listdir("./"):
    if not os.path.isfile(cur_file) or "start_time" in cur_file or ".py" in cur_file:
        continue
    with open(cur_file, "r", errors="ignore") as cur_file_fd:

        cur_file_str = cur_file_fd.read()
        # bug_triggering_query_str = get_bug_triggering_query_line(cur_str=cur_file_str)
        bug_triggering_query_str = cur_file_str
        # print(f"Getting {bug_triggering_query_str}")
        match_pattern = is_bug_match(bug_triggering_query_str)

        if match_pattern is None:
            print(f"For file: {cur_file}, no match. \n")
        else:
            bug_found_time = os.stat(cur_file).st_ctime
            bug_found_time = (bug_found_time - start_time) / 3600.0
            if match_pattern not in detected_bug_dict:
                bug_num += 1
                detected_bug_dict[match_pattern] = BugStruct(
                    file_name=cur_file, detect_time=bug_found_time)
            else:
                if detected_bug_dict[match_pattern].detect_time > bug_found_time:
                    detected_bug_dict[match_pattern] = BugStruct(
                        file_name=cur_file, detect_time=bug_found_time)

print(f"\n\n\nTotal bug number: {bug_num}\n")

for cur_bug_pattern in detected_bug_dict.items():
    print(f"Getting bug pattern {cur_bug_pattern}\n")
