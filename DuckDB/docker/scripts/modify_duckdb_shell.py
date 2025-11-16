import os
import sys

def fix_shell_entry():
    shell_src_path_str = "/home/duckdb/duckdb/tools/shell/shell.c"

    if not os.path.exists(shell_src_path_str):
        shell_src_path_str = "/home/duckdb/duckdb/tools/shell/shell.cpp"

    fd = open(shell_src_path_str, "r")
    all_lines = fd.readlines()
    all_modified_lines = "#include <sys/inotify.h>\n"

    tmp_prev_line = ""
    ignore_lines = 0
    for i in range(len(all_lines)):
        cur_line = all_lines[i]
        if (
                "data.in = stdin;" in cur_line
                and i + 1 < len(all_lines)
                and "rc = process_input(&data);" in all_lines[i + 1]
        ):
            # old version
            ignore_lines = 2

            all_modified_lines += """
      // stdin is not interactive.
      while (__AFL_LOOP(10000)) {
          fseek(stdin, 0, SEEK_SET);
          fseek(stdout, 0, SEEK_SET);
          data.in = stdin;
          rc = process_input(&data);
          ftruncate(fileno(stdin), 0);
          fflush(stdout);
    }
"""

        elif (
                "data.in = stdin;" in cur_line
                and i + 1 < len(all_lines)
                and "rc = data.ProcessInput();" in all_lines[i + 1]
        ):
            # new version.
            ignore_lines = 2

            all_modified_lines += """
      // stdin is not interactive.
      while (__AFL_LOOP(10000)) {
          fseek(stdin, 0, SEEK_SET);
          fseek(stdout, 0, SEEK_SET);
          data.in = stdin;
          rc = data.ProcessInput();
          ftruncate(fileno(stdin), 0);
          fflush(stdout);
    }
"""

        if ignore_lines == 0:
            all_modified_lines += cur_line
        else:
            # Ignore the following "data.in = stdin;" and "rc = process_input(&data);" line
            ignore_lines -= 1

    fd.close()
    fd = open(shell_src_path_str, "wt")
    fd.write(all_modified_lines)
    fd.close()


if __name__ == "__main__":
    fix_shell_entry()