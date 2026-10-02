#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.14"
# dependencies = []
# ///
#
# Implement SSI macros include, echo, and flastmod
# 
# Original NCSA HTTPd docs: https://web.archive.org/web/19970303194503/http://hoohoo.ncsa.uiuc.edu/docs/tutorials/includes.html
# W3 SSI docs: https://www.w3.org/Jigsaw/Doc/User/SSI.html
##

import re
import sys
from pathlib import Path
from datetime import datetime

RECMD = re.compile(r'<!--#(?P<cmd>\w+)(?P<args>.*?)-->')
RETAG = re.compile(r'\s*(?P<tag>\w+)\s*=\s*"(?P<value>[^"]*)"\s*')

MAXREC = 5 # max recursion for include

LAST_MODIFIED = "LAST_MODIFIED"
ENV = {}

def readfile(filepath: Path) -> str:
    with open(filepath) as f:
        ans = f.read()        
    return ans

def include(level: int, **kwargs) -> str:
    # virtual tag is managed like file tag
    filename = kwargs.get("file", kwargs.get("virtual"))
    if filename is None:
        # print(f"WARNING: file/virtual tag not found")
        return ""
    
    try:
        with open(filename) as f:
            text = f.read()
        return render(text, level+1)
    except FileNotFoundError:
        # print(f"WARNING: file '{filename}' does not exist")
        return ""
              
def echo(**kwargs):
    return ENV.get(kwargs.get("var"), "")

def flastmod():
    return ENV[LAST_MODIFIED]

def render(text: str, level: int = 0) -> str:
    if level > MAXREC:
        return text
        
    parts = []
    for m in RECMD.finditer(text):
        # print(m)
        g = m.groupdict()
        # print(RETAG.groupdict(g["args"]))
        cmd = g["cmd"]
        args = [m.groupdict() for m in RETAG.finditer(g['args'])]
        kwargs = {d["tag"]: d["value"] for d in args}
    
        if cmd == "include":
            new = include(level=level, **kwargs)
        elif cmd == "echo":
            new = echo(**kwargs)
        elif cmd == "flastmod":
            new = flastmod()
        else:
            continue
        
        old = text[m.start(): m.end()]
        parts.append((old, new))

    # O(N^2), N is the number of commands... take a look if it's too slow
    for old, new in parts:
        text = text.replace(old, new, 1)

    return text

def main(filepath: Path):
    text = filepath.read_text()
    
    ENV[LAST_MODIFIED] = (datetime.fromtimestamp(filepath.stat().st_mtime)
                                   .astimezone().isoformat(timespec="seconds"))

    ans = render(text)
    return ans
    
    
if __name__ == "__main__":
    rendered = main(Path(sys.argv[1]))
    print(rendered)