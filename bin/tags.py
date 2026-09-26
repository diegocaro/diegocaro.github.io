#!/usr/bin/env python3
import sys, os, time

START, END = "<!--MODIFIED_AT_START-->", "<!--MODIFIED_AT_END-->"

def main():
    if len(sys.argv) <= 1:
        sys.exit("Error: add the name of the file")

    filename = sys.argv[1]
    st = os.stat(filename)
    text = time.ctime(st.st_mtime)  # e.g. "Fri Sep 25 12:34:56 2026"

    with open(filename, "r") as f:
        content = f.read()

    start = content.find(START)
    end = content.find(END)
    if start == -1 or end == -1 or start > end:
        sys.exit("file not modified: tags not found or malformed")

    start += len(START)
    new_content = content[:start] + text + content[end:]

    with open(filename, "w") as f:
        f.write(new_content)

    # restore original access/modified times so this edit doesn't change them
    os.utime(filename, (st.st_atime, st.st_mtime))

if __name__ == "__main__":
    main()