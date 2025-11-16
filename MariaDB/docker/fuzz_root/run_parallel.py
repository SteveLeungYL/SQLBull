import re
from socket import socket
import time
import os
import shutil
import subprocess
import atexit
import signal
import psutil
import MySQLdb
import getopt
import sys
import libtmux

from afl_config import *

server = libtmux.Server()
all_mariadb_server_tmux_window = []

session = server.new_session(session_name="mariadb_server_debug", kill_session=True, attach=False)

def exit_handler(signal, frame):
    print("########################\n\n\n\n\nRecevied terminate signal. Ignored!!!!!!! \n\n\n\n\n")
    pass

def check_pid_exist(pid: int):
    try:
        os.kill(pid, 0)
    except OSError:
        return False
    else:
        return True

def handle_background_pid_file_helper(pid_file_str: str) -> (int, bool):
    '''From pid file, get the current pid and if is running. '''
    '''If not running, return -1. '''
    if "mariadb" not in pid_file_str or "Running" not in pid_file_str:
        return -1, True

    pid_num = int(pid_file_str.split()[1])
    return pid_num, False


def handle_background_pid_file(cur_window: libtmux.Window) -> bool:
    '''Return is crash or not. '''

    cur_pane = cur_window.attached_pane

    cur_pane.send_keys("jobs -l &> mariadbd_background_pid")

    time.sleep(1)

    with open("./mariadbd_background_pid", "r") as pid_file:
        pid_file_str = pid_file.read()
        pid_num, is_crash = handle_background_pid_file_helper(pid_file_str)

    if pid_num != -1:
        if check_pid_exist(pid_num):
            with open("./pid_pass_to_fuzzer", "w") as pid_file:
                pid_file.write(str(pid_num))

    os.remove("./mariadbd_background_pid")

    return is_crash

# Parse the command line arguments:
output_dir_str = ""
feedback_str = ""
parallel_num = 1
starting_core_id = 0
shm_size = 65536

mariadb_root_dir = "/home/mariadb/mariadb-server/bld/"

try:
    opts, args = getopt.getopt(sys.argv[1:], "o:c:O:F:", ["odir=", "start-core=", "oracle=", "non-deter"])
except getopt.GetoptError:
    print("Arguments parsing error")
    exit(1)
for opt, arg in opts:
    if opt in ("-o", "--odir"):
        output_dir_str = arg
        print("Using output dir: %s" % (output_dir_str))
    elif opt in ("-c", "--start-core"):
        starting_core_id = int(arg)
        print("Using starting_core_id: %d" % (starting_core_id))
    # elif opt in ("-n", "--num-concurrent"):
    #     parallel_num = int(arg)
    #     print("Using num-concurrent: %d" % (parallel_num))
    else:
        print("Error. Input arguments not supported. \n")
        exit(1)

# signal.signal(signal.SIGTERM, exit_handler)
# signal.signal(signal.SIGINT, exit_handler)
# signal.signal(signal.SIGQUIT, exit_handler)
# signal.signal(signal.SIGHUP, exit_handler)

if os.path.isfile(os.path.join(os.getcwd(), "shm_env.txt")):
    os.remove(os.path.join(os.getcwd(), "shm_env.txt"))

mariadb_src_data_dir = os.path.join(mariadb_root_dir, "data_all/ori_data")

for cur_inst_id in range(starting_core_id, starting_core_id + parallel_num, 1):
    print("#############\nSetting up core_id: " + str(cur_inst_id))

    # Set up the mariadb data folder first. 
    cur_mariadb_data_dir_str = os.path.join(mariadb_root_dir, "data_all/data_" + str(cur_inst_id))
    if os.path.isdir(cur_mariadb_data_dir_str):
        shutil.rmtree(cur_mariadb_data_dir_str)
    shutil.copytree(mariadb_src_data_dir, cur_mariadb_data_dir_str)

    # Set up SQLRight output folder
    cur_output_dir_str = ""
    if output_dir_str != "":
        if not os.path.isdir(output_dir_str):
            os.mkdir(output_dir_str)
        cur_output_dir_str = output_dir_str + "/outputs_"  + str(cur_inst_id - starting_core_id)
    else:
        if not os.path.isdir("./outputs"):
            os.mkdir("./outputs")
        cur_output_dir_str = "./outputs/outputs_" + str(cur_inst_id - starting_core_id)
    if not os.path.isdir(cur_output_dir_str):
        os.mkdir(cur_output_dir_str)

    cur_output_file = os.path.join(cur_output_dir_str, "output.txt")

    cur_output_file_2 = os.path.join(cur_output_dir_str, "output_AFL.txt")
    cur_output_file_2 = open(cur_output_file_2, "w")
    
    # Prepare for env shared by the fuzzer and mariadb. 
    cur_port_num = port_starting_num + cur_inst_id
    socket_path = "/tmp/mariadb_" + str(cur_inst_id) + ".sock"

    # Deprecated. Not used for AFL. 
    if os.path.isfile("./shm_size"):
        shm_size_str = open("./shm_size", "r").read()
        shm_size = int(shm_size_str)
    else:
        shm_size = 262144

    # modi_env = dict()
    # modi_env["AFL_I_DONT_CARE_ABOUT_MISSING_CRASHES"] = "1"
    # modi_env["AFL_SKIP_CPUFREQ"] = "1"

    fuzzing_command = [
        # "strace -s 2000 -o afl-fuzz-strace_output_" + str(cur_inst_id - starting_core_id),
        # "gdb --ex=run --args",
        "./parserfuzz",
        "-t", "2000",
        "-m", "none",
        "-P", str(cur_port_num), 
        "-K", socket_path,
        "-i", "./inputs",
        "-o", cur_output_dir_str,
        "-c", str(cur_inst_id),
        # "-s", str(shm_size), # not used for AFL, only used for AFLplusplus. 
        ]

    fuzzing_command.append("aaa")
    # fuzzing_command.append(" &> output.txt ")

    fuzzing_command = " ".join(fuzzing_command)
    print("Running fuzzing command: " + fuzzing_command)
    # p = subprocess.Popen(
                        # fuzzing_command,
                        # cwd=os.getcwd(),
                        # shell=True,
                        # stderr=cur_output_file_2,
                        # stdout=cur_output_file_2,
                        # stdin=subprocess.DEVNULL,
                        # env=modi_env
                        # )

    cur_window = session.new_window(attach=True, window_name="fuzzing_test_"+str(cur_inst_id - starting_core_id))
    cur_pane = cur_window.attached_pane
    cur_pane.send_keys(fuzzing_command) 

    # Read the current generated shm_mem_id
    while not (os.path.isfile(os.path.join(os.getcwd(), "shm_env.txt"))):
        time.sleep(1)
    shm_env_fd = open(os.path.join(os.getcwd(), "shm_env.txt"))
    cur_shm_str = shm_env_fd.read()
    shm_env_fd.close()

    os.remove(os.path.join(os.getcwd(), "shm_env.txt"))

    mariadb_bin_dir = os.path.join(mariadb_root_dir, "bin/mariadbd")

    # mariadb_command = "__AFL_SHM_ID=" + cur_shm_str + " " + mariadb_bin_dir + " --basedir=" + mariadb_root_dir + " --datadir=" + cur_mariadb_data_dir_str + " --port=" + str(cur_port_num) + " --socket=" + socket_path + " & "

    mariadbd_output_file = os.path.join(cur_output_dir_str, "mariadbd_output.txt")

    mariadb_command = [
        #"gdb --ex=run --args",
        # "strace -s 2000 -o mariadbd_strace_output_" + str(cur_inst_id - starting_core_id),
        "__AFL_SHM_ID=" + cur_shm_str,
        # "AFL_MAP_SIZE=" + shm_size_str,
        # "AFL_IGNORE_PROBLEMS=1",
        # "AFL_IGNORE_PROBLEMS_COVERAGE=1",
        mariadb_bin_dir,
        "--basedir=" + mariadb_root_dir,
        "--datadir=" + cur_mariadb_data_dir_str,
        "--port=" + str(cur_port_num),
        "--socket=" + socket_path,
        "--performance_schema=OFF",
        " &> " + mariadbd_output_file,
        " & " # run in the background
     ]

    # mariadb_modi_env = dict()
    # mariadb_modi_env["__AFL_SHM_ID"] = cur_shm_str

    mariadb_command = " ".join(mariadb_command)

    print("Running mariadb command: " + mariadb_command)

    cur_window = session.new_window(attach=True, window_name="mariadb_test_"+str(cur_inst_id - starting_core_id))
    cur_pane = cur_window.attached_pane
    cur_pane.send_keys(mariadb_command) 

    handle_background_pid_file(cur_window)

    all_mariadb_server_tmux_window.append([cur_window, mariadb_command, cur_mariadb_data_dir_str, mariadbd_output_file])
    
    time.sleep(1)


print("Finished launching the fuzzing. ")

# Avoid script exist
while True:
    time.sleep(1)

    # Go through all the created window, check whether the mariadbd process is still active. 
    for cur_window, mariadb_command, cur_mariadb_data_dir_str, mariadbd_output_file in all_mariadb_server_tmux_window:
        is_crash = False

        is_crash = handle_background_pid_file(cur_window)
        
        if is_crash:
            '''recover the backup mariadb data. '''
            if os.path.isdir(cur_mariadb_data_dir_str):
                shutil.rmtree(cur_mariadb_data_dir_str)
            shutil.copytree(mariadb_src_data_dir, cur_mariadb_data_dir_str)

            
            '''Gather the crash information. '''
            with open(mariadbd_output_file, "r") as mariadbd_output_file_fd, open("./mysqld_output_to_fuzzer.txt", "w") as cur_output_file_fd:
                mariadbd_output_file_str = mariadbd_output_file_fd.read()
                cur_output_file_fd.write(mariadbd_output_file_str)
            
            os.remove(mariadbd_output_file)

            cur_pane.send_keys(mariadb_command)
        
