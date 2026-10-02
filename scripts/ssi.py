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

import re
import sys
from datetime import datetime
from pathlib import Path

RECMD = re.compile(r'<!--#(?P<cmd>\w+)(?P<args>.*?)-->', re.DOTALL)
RETAG = re.compile(r'(\w+)\s*=\s*"([^"]*)"')

MAXREC = 5  # max recursion for include

def error(message: str):
    print(message, file=sys.stderr)

def mtime(path: Path) -> str:
    return datetime.fromtimestamp(path.stat().st_mtime).astimezone().isoformat(timespec="seconds")

def render(path: Path, root: Path, env: dict[str, str], level: int = 0) -> str:
    if level > MAXREC:
        return ""

    def replace(m: re.Match[str]) -> str:
        kwargs = dict(RETAG.findall(m["args"]))

        # "file" is relative to the current document, "virtual" to the root
        target = None
        if name := kwargs.get("file"):
            target = (path.parent / name).resolve()
        elif name := kwargs.get("virtual"):
            target = (root / name.lstrip("/")).resolve()
        if target and not target.is_relative_to(root):
            return ""  # refuse paths outside the root

        try:
            match m["cmd"]:
                case "include":
                    return render(target, root, env, level + 1) if target else ""
                case "echo":
                    return env.get(kwargs.get("var", ""), "")
                case "flastmod":
                    return mtime(target or path)
                case _:
                    return m[0]  # unknown directive: leave as is
        except (OSError, UnicodeDecodeError) as e:
            error(str(e))
            return ""
            
    try:
        return RECMD.sub(replace, path.read_text(encoding="utf-8"))
    except (OSError, UnicodeDecodeError) as e:
        error(f"error while reading '{path}'\n\t{e}")
        if level == 0:
            sys.exit(-1)
        return ""


def main(path: Path) -> str:
    path = path.resolve()
    env = {"LAST_MODIFIED": mtime(path), "DOCUMENT_NAME": path.name}
    return render(path, path.parent, env)


if __name__ == "__main__":
    if len(sys.argv) != 3:
        sys.exit(f"usage: {Path(sys.argv[0]).name} INPUT_FILE OUTPUT_FILE")
    rendered = main(Path(sys.argv[1]))
    Path(sys.argv[2]).write_text(rendered)
    