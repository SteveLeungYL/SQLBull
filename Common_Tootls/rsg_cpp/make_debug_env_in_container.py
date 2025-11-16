import subprocess
import shutil
import getpass
import os

def main():
    username = getpass.getuser()
    os.chdir(f"/home/{username}/rsg_cpp/")

    parserfuzz_main_str = open("./parserfuzz.cpp", "r").read()

    if "#define DEBUG" not in parserfuzz_main_str:
        """Instrument the main file"""

        res_str = ""
        for cur_line in parserfuzz_main_str.splitlines():
            res_str += cur_line + "\n"
            if " } // stmt_idx loop" in cur_line:
                res_str += "sample_current_execution(query_seq_gen);\n"

        with open("./parserfuzz.cpp", "w") as fd:
            fd.write(res_str)

    subprocess.run("make clean && make $(whoami)_debug -j $(nproc)", shell=True)
    shutil.copy2(f"/home/{username}/rsg_cpp/parserfuzz", f"/home/{username}/fuzzing/fuzz_root")

if __name__ == "__main__":
    main()