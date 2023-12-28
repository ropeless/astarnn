"""
This is a main program to report the number of probes.

Usage: python num_of_probes.py [param=value]*
    No spaces around the '=' character.
    'param' can be any of the following:
        minDims (minimum number of dimensions)
        maxDims (maximum number of dimensions)
        minShells = (minimum number of extended shells in A* V:E)
        minShells = (minimum number of extended shells in A* V:E)
        verbose = ('true' or 'false')
        dims=value is shorthand for minDims=value maxDims=value
        shells=value is shorthand for minShells=value maxShells=value.
"""
__author__ = 'Barry Drake'

import sys
from astarnn import AStarNN

default_minDims = 1
default_maxDims = 32
default_minShells = 0
default_maxShells = 14
default_delim = '\t'
default_verbose = False


def num_of_probes(
        minDims=default_minDims,
        maxDims=default_maxDims,
        minShells=default_minShells,
        maxShells=default_maxShells,
        delim=default_delim
    ):
    # show parameters
    print("number_of_probes")
    print()
    print("minDims  ", minDims)
    print("maxDims  ", maxDims)
    print("minShells", minShells)
    print("maxShells", maxShells)
    print()

    # show header
    print("dim", end='')
    for shells in range(minShells, 1 + maxShells):
        print(f"{delim}{shells}", end='')
    print()

    # run the simulation - show data incrementally
    for dims in range(minDims, 1 + maxDims):
        print(dims, end='')
        sys.stdout.flush()
        for shells in range(minShells, 1 + maxShells):
            prober = AStarNN(dims, 1.0, shells)
            print(f"{delim}{prober.num_probes}", end='')
            sys.stdout.flush()
        print()


def main():
    minDims = default_minDims
    maxDims = default_maxDims
    minShells = default_minShells
    maxShells = default_maxShells
    delim = default_delim
    verbose = default_verbose

    # parse the command line
    arg_dims = "dims="
    arg_minDims = "minDims="
    arg_maxDims = "maxDims="
    arg_shells = "shells="
    arg_minShells = "minShells="
    arg_maxShells = "maxShells="
    arg_verbose = "verbose="

    for arg in sys.argv[1:]:
        if arg.startswith(arg_dims):
            minDims = maxDims = _value_of(int, arg_dims, arg)
        elif arg.startswith(arg_shells):
            minShells = maxShells = _value_of(int, arg_shells, arg)
        elif arg.startswith(arg_minDims):
            minDims = _value_of(int, arg_minDims, arg)
        elif arg.startswith(arg_maxDims):
            maxDims = _value_of(int, arg_maxDims, arg)
        elif arg.startswith(arg_minShells):
            minShells = _value_of(int, arg_minShells, arg)
        elif arg.startswith(arg_maxShells):
            maxShells = _value_of(int, arg_maxShells, arg)
        elif arg.startswith(arg_verbose):
            verbose = _value_of(bool, arg_verbose, arg)
        else:
            print(f"Unknown command line argument: {arg!r}", file=sys.stderr)
            print(
                f"""Usage: python num_of_probes.py [param=value]*
                No spaces around the '=' character.
                'param' can be any of the following, showing default values:
                minDims = {default_minDims} (minimum number of dimensions)
                maxDims = {default_maxDims} (maximum number of dimensions)
                minShells = {default_minShells} (minimum number of extended shells in A* V:E)
                minShells = {default_maxShells} (minimum number of extended shells in A* V:E)
                verbose = {default_verbose} ('true' or 'false')
                dims=value is shorthand for minDims=value maxDims=value.
                shells=value is shorthand for minShells=value maxShells=value.
                """,
                file=sys.stderr
            )
            exit(1)

    # Check parameter values are valid
    if minDims < 1:
        print(f"dims must be >= 1, but got: {minDims}", file=sys.stderr)
        exit(1)
    if maxDims < 1:
        print(f"dims must be >= 1, but got: {maxDims}", file=sys.stderr)
        exit(1)
    if minDims > maxDims:
        print("minDims must not be greater than maxDims", file=sys.stderr)
        exit(1)
    if minShells < 0:
        print("shells cannot be negative", file=sys.stderr)
        exit(1)
    if minShells > maxShells:
        print("minShells must not be greater than maxShells", file=sys.stderr)
        exit(1)

    if verbose:
        print(
            f"""minDims={minDims}
            maxDims={maxDims}
            minShells={minShells}
            maxShells={maxShells}
            """
        )

    num_of_probes(
        minDims,
        maxDims,
        minShells,
        maxShells,
        delim
    )


def _value_of(value_type, prefix, arg):
    val = arg[len(prefix):]
    if value_type is bool:
        val = val.lower()
        if val == "true" or val == "yes" or val == "1":
            return True
        elif val == "false" or val == "no" or val == "0":
            return False
    return value_type(val)


if __name__ == '__main__':
    main()
    