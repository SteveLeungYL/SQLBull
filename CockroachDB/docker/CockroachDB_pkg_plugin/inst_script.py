import os
import sys
import getopt
from loguru import logger
import subprocess

db_dir = "./cockroach"
opts = []

logger.remove()
logger.add(sys.stdout, level="INFO")

try:
    opts, args = getopt.getopt(sys.argv[1:], "i:", ["db-dir="])
except getopt.GetoptError:
    logger.error("Input flag error. Expected flag -i or -db-dir only. ")

for opt, arg in opts:
    if opt in ("-i", "--db-dir"):
        db_dir = arg
        logger.debug("Using CockroachDB source dir: %s" % (db_dir))
    else:
        logger.error("Error: getting unexpected flag: %s" % (opt))
        exit(1)

if not os.path.isdir(db_dir):
    logger.error("The requested dir: %s is not existed. Error" % (db_dir))
    exit(1)

os.chdir(db_dir)

global_idx = 0

# Iterate all files in the CockroachDB GoLang source files.
for subdir, _, files in os.walk("./"):
    for cur_file in files:
        # only instrument .go files.
        if cur_file[-3:] != ".go" or "doc.go" in cur_file or "test" in cur_file:
            if "cov_unit_test" not in cur_file:
                logger.debug("Ignore file: %s %s" % (subdir, cur_file))
                continue

        ## TODO::FIXME, re-enable and try again.
        if "_tmpl" in cur_file:
            # Do not instrument the tmpl files.
            # They are used to generated the original Go files,
            # but their injection logic has some problems (very fragile) and
            # we should not touch the code snippet that used for code generation.
            continue

        if "pkg/util/interval/generic" in subdir:
            # this folder is only used for generating new code.
            # DO NOT INSTRUMENT because it will break the fragile
            # original code generation logic.
            continue

        if "pkg/util/ctxutil" in subdir:
            continue

        cur_file_dir = os.path.join("./", subdir, cur_file)

        tmp_contents = ""

        is_instr = False

        with open(cur_file_dir, "r") as fd:

            whole_file_str = fd.read()

            if "globalcov" in whole_file_str:
                is_instr = True

        if not is_instr:

            instr_command_str = "./goInstr --file=%s --idx=%d" % (cur_file_dir, global_idx)
            global_idx += 1

            logger.debug("Running with command: %s\n" % (instr_command_str))

            process = subprocess.Popen(instr_command_str, shell=True)
            process.wait()

            logger.debug("Finished running goInstr on file: %s" % (cur_file_dir))

# direclty modify the WORKSPACE file for CockroachDB

res_str = ""
is_first = True
with open("./WORKSPACE", "r") as fd:
    is_omit = False
    for cur_line in fd.read().splitlines():
        if "go_download_sdk(" in cur_line and is_first == True:
            is_omit = True
            is_first = False
            res_str += "#"
        if cur_line.startswith(")") and is_omit == True:
            is_omit = False
            res_str += "#"
        if is_omit == True:
            res_str += "#"

        if "go_local_sdk(" in cur_line:
            res_str += """go_local_sdk(
    name = "go_sdk",
    path = "/usr/local/go",
)
"""

        res_str += cur_line + "\n"

with open("./WORKSPACE", "w") as fd:
    fd.write(res_str)

