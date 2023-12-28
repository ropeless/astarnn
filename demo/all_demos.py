#!/usr/bin/env python
"""
Run all the demo scripts (except nominated exclusions).
This may be used as a 'smoke test'.
"""
__author__ = 'Barry Drake'

from itertools import chain
from pathlib import Path
from subprocess import call
import sys
from stop_watch import StopWatch


DEMO_TOP_DIR = '.'
DEMO_CHDIR = './_demo_output'
DEMO_EXCEPTIONS = [
    'stop_watch.py',       # support utility
]
LINE = "-" * 80
LINE_2 = "=" * 80


def find_demo_dirs(demos_top_dir_path: Path):
    return [
        d
        for d in chain([demos_top_dir_path], demos_top_dir_path.iterdir())
        if d.name.startswith('demo') and d.is_dir() and d.name not in DEMO_EXCEPTIONS
    ]


def main():
    python_exe = sys.executable
    print('Python executable:', python_exe)
    print()

    errors = []
    script_count = 0
    python_env = None
    total_time = StopWatch()

    # Start search of demo directories in the script directory.
    script_path = Path(__file__)
    script_name = script_path.stem
    demos_top_dir_path = (script_path.parent / DEMO_TOP_DIR).absolute().resolve()
    dirs = find_demo_dirs(demos_top_dir_path)

    # Always exclude self if we happen to find our own script.
    DEMO_EXCEPTIONS.append(Path(__file__).name)

    for demo_dir in dirs:
        for script in demo_dir.iterdir():
            if (
                script.is_file() and
                script.name.endswith('.py') and
                script.name not in DEMO_EXCEPTIONS
            ):
                print(LINE)
                print(script.name)
                print(LINE)
                script_count += 1

                return_code = call([python_exe, script.as_posix()], env=python_env)

                if return_code != 0:
                    errors.append(f'Error code {return_code}: {script.name}')
                print(LINE)

    print(LINE_2)
    print(f'Done running {script_name} ({script_count} scripts in {total_time}): number of errors = {len(errors)}')
    for error in errors:
        print(error)

    # Provide a useful exit code.
    if len(errors) > 0:
        return -1
    else:
        return 0


if __name__ == '__main__':
    ret_code = main()
    exit(ret_code)
