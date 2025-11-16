import os
import sys

bug_pattern_list = [
    "invalid memory address or nil pointer dereference",
    "invalid union",
    "column statistics cannot be determined for empty column set",
    "expected *DInt, found tree.dNull",
    ["unexpected error from the vectorized engine", "AS MATERIALIZED ( SHOW STATISTICS"],
    ["REVOKE CREATE ON SEQUENCE","internal error: user public must not have"],
    "*tree.SetTracing",
    "decoding unset EncDatum",
    "generator functions cannot be evaluated as scalars",
    "comparison overload not found (eq, unknown, unknown)",
    "func(context.Context, *eval.Context, tree.Datums) (tree.Datum, error)"
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
    if not os.path.isfile(cur_file) or "start_time" in cur_file:
        continue
    with open(cur_file, "r", errors="ignore") as cur_file_fd:

        cur_file_str = cur_file_fd.read()
        match_pattern = is_bug_match(cur_file_str)

        if match_pattern is None:
            print(f"For file: {cur_file}, no match. \n")
        else:
            bug_found_time = os.stat(cur_file).st_ctime
            bug_found_time = (bug_found_time - start_time) / 3600.0
            if match_pattern not in detected_bug_dict:
                bug_num += 1
                detected_bug_dict[match_pattern] = BugStruct(file_name=cur_file, detect_time=bug_found_time)
            else:
                if detected_bug_dict[match_pattern].detect_time > bug_found_time:
                    detected_bug_dict[match_pattern] = BugStruct(file_name=cur_file, detect_time=bug_found_time)
    
print(f"\n\n\nTotal bug number: {bug_num}\n")

for cur_bug_pattern in detected_bug_dict.items():
    print(f"Getting bug pattern {cur_bug_pattern}\n")