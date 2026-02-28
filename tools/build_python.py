#!/usr/bin/env python3

"""A script to build the C++ backend and install the bindings into the source tree."""

import sys
import os
import shutil
import subprocess
from functools import partial
from pathlib import Path
from embed_shaders import embed_shaders
from build_flatbuffers import compile_flatbuffers

from fire import Fire

from build_config import CMAKE_FLAGS

BUILD_DIR = "build"

TARGET_NAME = "bindings.cpython-313-x86_64-linux-gnu.so"

LIB_PATH = Path("./build") / TARGET_NAME
DEST_PATH = Path("./src/slamd/") / TARGET_NAME


def _python_exe() -> str:
    return sys.executable


def build(debug: bool) -> None:
    """(Re)build the C++ backend."""
    embed_shaders()
    compile_flatbuffers()

    build_path = Path("build")
    build_path.mkdir(exist_ok=True)

    compile_cmd = [
        "cmake",
        "-B",
        str(build_path),
        "-G",
        "Ninja",
        *CMAKE_FLAGS,
        f"-DPython_EXECUTABLE={_python_exe()}",
        "-DPython_FIND_VIRTUALENV=ONLY",
    ]

    if debug:
        compile_cmd.append("-DCMAKE_BUILD_TYPE=Debug")

    subprocess.run(
        compile_cmd,
        check=True,
    )

    subprocess.run(["ninja", "-C", str(build_path)])

    DEST_PATH.unlink(missing_ok=True)
    DEST_PATH.symlink_to(LIB_PATH.resolve())

    os.chdir(Path("src/slamd"))

    env = os.environ.copy()
    env["PYTHONPATH"] = f".:{env.get('PYTHONPATH', '')}"

    subprocess.run(
        [
            "pybind11-stubgen",
            "bindings",
            "--numpy-array-remove-parameters",
            "-o",
            ".",
        ],
        env=env,
        check=True,
    )


def clean() -> None:
    """Clean the build folder and remove the symlink, if any."""
    shutil.rmtree(BUILD_DIR, ignore_errors=True)


def clean_build() -> None:
    """First clean and then build."""
    clean()
    build(False)


def debug_build():
    build(True)


def clean_debug_build():
    clean()
    build(True)


def check_in_repo() -> None:
    """Check that we are executing this from repo root."""
    assert Path(".git").exists(), "This command should run in repo root."


if __name__ == "__main__":
    check_in_repo()
    os.chdir(Path("./python_bindings"))

    Fire(
        {
            "build": partial(build, False),
            "clean": clean,
            "clean_build": clean_build,
            "debug_build": debug_build,
            "clean_debug_build": clean_debug_build,
        }
    )
