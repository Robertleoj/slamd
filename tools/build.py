#!/usr/bin/env python3

"""Build script for slamd: codegen + editable install + stubs."""

import subprocess
import sys
from pathlib import Path

REPO_DIR = Path(__file__).parent.parent.resolve()


def main():
    print("=== Syncing dev dependencies ===")
    subprocess.run(
        ["uv", "sync", "--group", "dev", "--no-install-project"],
        cwd=REPO_DIR,
        check=True,
    )

    from build_flatbuffers import compile_flatbuffers
    from embed_shaders import embed_shaders

    print("\n=== Compiling flatbuffers ===")
    compile_flatbuffers()

    print("\n=== Embedding shaders ===")
    embed_shaders()

    print("\n=== Installing package (editable) ===")
    subprocess.run(
        [
            "uv",
            "pip",
            "install",
            "--force-reinstall",
            "--no-build-isolation",
            "-e",
            ".",
        ],
        cwd=REPO_DIR,
        check=True,
    )

    print("\n=== Symlinking slamd_window into source tree ===")
    build_dirs = sorted(Path(REPO_DIR / "build").glob("*/slamd/slamd_window"))
    if build_dirs:
        exe = build_dirs[-1].resolve()
        link = REPO_DIR / "src" / "slamd" / "slamd_window"
        link.unlink(missing_ok=True)
        link.symlink_to(exe)
        print(f"  {link} -> {exe}")
    else:
        print("  WARNING: slamd_window not found in build directory")

    print("\n=== Generating type stubs ===")
    subprocess.run(
        [
            sys.executable,
            "-m",
            "pybind11_stubgen",
            "slamd.bindings",
            "--numpy-array-remove-parameters",
            "-o",
            "src",
        ],
        cwd=REPO_DIR,
        check=True,
    )

    stubs_dir = REPO_DIR / "src" / "slamd" / "bindings"
    stub_files = list(stubs_dir.glob("*.pyi"))
    if stub_files:
        print("\n=== Formatting and fixing stubs with ruff ===")
        stub_paths = [str(f) for f in stub_files]
        subprocess.run(["ruff", "check", "--fix"] + stub_paths, cwd=REPO_DIR, check=True)
        subprocess.run(["ruff", "format"] + stub_paths, cwd=REPO_DIR, check=True)

    print("\n=== Done ===")


if __name__ == "__main__":
    main()
