#!/usr/bin/env python3
"""Clang-tidy compile database matching this module's include paths."""

import json
import os
import shlex
import subprocess
import sys
from pathlib import Path

source = Path(__file__).resolve().parent.parent
build = Path(sys.argv[1]).resolve(strict=True)
offline = Path(os.environ["OFFLINE_MAIN"])
install = Path(os.environ["TPCQA_INSTALL"])
flags = shlex.split(subprocess.check_output(["root-config", "--cflags"], text=True))
flags += ["-I" + str(build), "-I" + str(source), "-I" + str(install / "include")]
flags += ["-isystem", str(offline / "include")]
entries = []
for file in sorted([*source.glob("*.cc"), *source.glob("tests/*.cc")]):
    entries.append(
        dict(directory=str(build), file=str(file), arguments=["g++", *flags, "-c", str(file)])
    )
(build / "compile_commands.json").write_text(json.dumps(entries, indent=2) + "\n")
