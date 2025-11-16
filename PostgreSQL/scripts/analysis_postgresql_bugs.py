import os
import sys
import collections.abc

bug_pattern_list = [
    "IsA(cte->ctequery, InsertStmt)",
    "cte->cterecursive",
    "Assert(\"false\")",
]

# local_user_name = os.getlogin()
local_user_name = "sly"

interesting_files_idx = 0

def get_str_assert_line(cur_str: str) -> str:
    # Search from the last line to the first line.
    for cur_line in reversed(cur_str.splitlines()):
        if "TRAP" in cur_line:
            return cur_line
    return ""

def is_bug_match(cur_str: str):
    global bug_pattern_list

    cur_str_assert_line = get_str_assert_line(cur_str)

    for cur_bug_pattern in bug_pattern_list:
        if isinstance(cur_bug_pattern, str):
            cur_bug_pattern = [cur_bug_pattern]
        for cur_pattern in cur_bug_pattern:
            if cur_pattern in cur_str_assert_line:
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

def copy_bug_folder_helper(bug_folder_idx: int):

    if os.path.exists(f"./bug_data_tmp"):
        os.system(f"sudo rm -rf ./bug_data_tmp")

    dest_container_name = f"postgresql_testing_{bug_folder_idx}"
    # print(f"Copying bug data from container {dest_container_name}")
    os.system(f"sudo docker cp {dest_container_name}:/home/postgresql/fuzzing/Bug_Analysis/detected_bugs ./bug_data_tmp")
    # print(f"Changing owner of bug data from container {dest_container_name} to user {local_user_name}. ")
    os.system(f"sudo chown -R {local_user_name}:{local_user_name} ./bug_data_tmp")
    if not os.path.exists("./bug_data_tmp"):
        # print(f"Error: Cannot find bug data in container {dest_container_name}")
        return
    return

def is_excluded_non_bug_queries(cur_str: str) -> bool:
    if "TRAP" not in cur_str and "Assert" not in cur_str:
        return True
    else:
        return False

def backup_interesting_files(cur_file: str, is_known_bug: bool):
    global interesting_files_idx

    target_name = f"interesting_file_{interesting_files_idx}"
    if is_known_bug:
        target_name = f"known_bug_file_{interesting_files_idx}"

    # print(f"Backing up interesting file {target_name}")
    if not os.path.exists("../interesting_files_dir"):
        os.system(f"mkdir -p ../interesting_files_dir")

    os.system(f"cp -r ./{cur_file} ../interesting_files_dir/{target_name}")
    interesting_files_idx += 1

    return target_name

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

start_core = 0
num_cores = 100

if len(sys.argv) == 3:
    start_core = sys.argv[1]
    num_cores = sys.argv[2]

output_log = open("./output_log", "w")

if os.path.exists("./interesting_files_dir"):
    os.system(f"rm -rf ./interesting_files_dir")

for idx in range(int(start_core), int(start_core) + int(num_cores)):
    copy_bug_folder_helper(idx)
    if not os.path.exists("./bug_data_tmp"):
        # print(f"Error: Cannot find bug data in container {idx}")
        continue

    with open("./bug_data_tmp/start_time", "r") as fd:
        start_time = int(fd.read())

    os.chdir("./bug_data_tmp")
    for cur_file in os.listdir("./"):
        if not os.path.isfile(cur_file) or "start_time" in cur_file or ".py" in cur_file:
            continue
        # print(f"Analyzing file {cur_file}")
        with open(cur_file, "r", errors="ignore") as cur_file_fd:

            cur_file_str = cur_file_fd.read()
            # bug_triggering_query_str = get_bug_triggering_query_line(cur_str=cur_file_str)
            bug_triggering_query_str = cur_file_str

            if (is_excluded_non_bug_queries(bug_triggering_query_str)):
                continue
            
            # print(f"Getting {bug_triggering_query_str}")
            match_pattern = is_bug_match(bug_triggering_query_str)

            if match_pattern is None:
                backup_file_name = backup_interesting_files(cur_file, is_known_bug=False)
                print(f"For file: {backup_file_name}, no match. \n")
                output_log.write(f"For file: {backup_file_name}, no match. \n")
            else:
                bug_found_time = os.stat(cur_file).st_ctime
                bug_found_time = (bug_found_time - start_time) / 3600.0
                if match_pattern not in detected_bug_dict:
                    bug_num += 1
                    backup_file_name = backup_interesting_files(cur_file, is_known_bug=True)
                    # print(f"For file: {backup_file_name}, match. \n")
                    detected_bug_dict[match_pattern] = BugStruct(
                        file_name=backup_file_name, detect_time=bug_found_time)

                else:
                    if detected_bug_dict[match_pattern].detect_time > bug_found_time:
                        detected_bug_dict[match_pattern] = BugStruct(
                            file_name=backup_file_name, detect_time=bug_found_time)

    os.chdir("..")

print(f"\n\n\nTotal bug number: {bug_num}\n")
output_log.write(f"\n\n\nTotal bug number: {bug_num}\n")

for cur_bug_pattern in detected_bug_dict.items():
    print(f"Getting bug pattern {cur_bug_pattern}\n")
    output_log.write(f"Getting bug pattern {cur_bug_pattern}\n")

output_log.close()
