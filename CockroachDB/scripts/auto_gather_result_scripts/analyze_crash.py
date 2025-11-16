import os
import sys
import collections.abc

bug_pattern_list = [
    "SHOW COMPLETIONS AT OFFSET",
    "START TRANSACTION AS OF SYSTEM TIME",
    "SHOW EXPERIMENTAL_FINGERPRINTS",
    ["SET LOCAL ROLE string_agg", "SET LOCAL ROLE concat_agg", "SET NAMES", "SET SESSION SCHEMA string_agg", "SET LOCAL SCHEMA", "SET LOCAL ROLE", "SET LOCAL SCHEMA", "SET LOCAL SCHEMA string_agg", "SET LOCAL SCHEMA concat_agg", "ALTER ROLE", "SET SCHEMA", "SET ROLE", "SET SESSION SCHEMA", "SET SESSION ROLE"],
    "AS OF SYSTEM TIME b'",
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
                print("ERROR!!!! get_bug_triggering_query_line is not working properly. ")
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
        bug_triggering_query_str = get_bug_triggering_query_line(cur_str=cur_file_str)
        # print(f"Getting {bug_triggering_query_str}")
        match_pattern = is_bug_match(bug_triggering_query_str)

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