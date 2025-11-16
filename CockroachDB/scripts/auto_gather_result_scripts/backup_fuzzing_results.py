import os
import subprocess
import shutil
from pathlib import Path

dest_result_saving_loc = os.getcwd()
container_home_directory = "/home/cockroach"
# cur_plotting_script_file = ""

def detect_docker_container_name():
    p = subprocess.Popen(
        ["docker ps"],
        shell=True,
        stdout=subprocess.PIPE,
        stdin=subprocess.PIPE,
        stderr=subprocess.STDOUT
    )

    out = ""
    try:
        out, _ = p.communicate(timeout=8)
        out = out.decode('utf-8', errors='ignore')
    except subprocess.TimeoutExpired:
        print("ERROR: Timeout. ")

    container_lines = out.splitlines()[1:]

    res_container_name = []
    for cur_line in container_lines:
        res_container_name.append(cur_line.split()[-1])

    return res_container_name

def detect_new_bugs(bug_source_dir: str, container_name: str):
    global dest_result_saving_loc

    shutil.copy2("./analyze_bugs.py", bug_source_dir)
    if os.path.exists(bug_source_dir + "/crashes"):
        shutil.copy2("./analyze_crash.py", bug_source_dir + "/crashes")

    command = f"python3 ./analyze_bugs.py"

    print(f"Running {command}\n")
    p = subprocess.run([command], shell=True, stdout=subprocess.PIPE,
                   cwd = bug_source_dir)
    out = p.stdout

    new_bug_str = ""

    for cur_line in out.decode("utf-8", errors="ignore").splitlines():
        if not cur_line:
            continue
        if "For file:" in cur_line and "no match" in cur_line and "Getting bug pattern" not in cur_line and "Total bug number" not in cur_line:
            new_bug_str += cur_line + "\n"

    if os.path.exists(bug_source_dir + "/crashes"):
        command = f"python3 analyze_crash.py"

        print(f"Running {command}\n")
        p = subprocess.run([command], shell=True, stdout=subprocess.PIPE,
                       cwd = bug_source_dir+ "/crashes")
        out = p.stdout

        for cur_line in out.decode("utf-8", errors="ignore").splitlines():
            if not cur_line:
                continue
            if "For file:" in cur_line and "no match" in cur_line and "Getting bug pattern" not in cur_line and "Total bug number" not in cur_line:
                new_bug_str += "crash: " + cur_line + "\n"

    print(f"Getting new_bug_str: {new_bug_str}")
    if new_bug_str:
        file_name = f"{dest_result_saving_loc}/{container_name}_bug_summary.txt"
        with open(f"{file_name}", "w") as fd:
            print(f"Outputting bug summary on file: {file_name}\n")
            fd.write(new_bug_str)


def pull_docker_results_to_local_fs(container_name: str):
    global dest_result_saving_loc

    # clean up the original file first.
    dest_loc = f"{dest_result_saving_loc}/{container_name}"
    if os.path.exists(dest_loc):
        shutil.rmtree(dest_loc)
    Path(f"{dest_loc}").mkdir(parents=True, exist_ok=False)
    print(f"Working on {dest_loc}\n")

    command = f"docker cp {container_name}:{container_home_directory}/fuzzing/Bug_Analysis/bug_samples {dest_loc}/bug_samples"

    print(f"Running {command}\n")
    subprocess.run([command], shell=True,
                   stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)

    command = f"docker cp {container_name}:{container_home_directory}/fuzzing/fuzz_root/outputs {dest_loc}/outputs"

    print(f"Running {command}\n")
    subprocess.run([command], shell=True,
                   stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)

    detect_new_bugs(dest_loc + "/bug_samples", container_name)

    command = f"zip -r {dest_loc}.zip {dest_loc}"

    print(f"Running {command}\n")
    subprocess.run([command], shell=True,
                   stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)


if __name__ == "__main__":
    all_container_names = detect_docker_container_name()

    for cur_name in all_container_names:
        print(f"Working with container: {cur_name}\n")
        pull_docker_results_to_local_fs(cur_name)
        print(f"End: {cur_name}\n")

