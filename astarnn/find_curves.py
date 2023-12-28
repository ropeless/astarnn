"""
This is a main program to estimate the probability
of matching as a function of distance.

Usage: python find_curves.py [param=value]*
    No spaces around the '=' character.
    'param' can be any of the following:
        minDims (minimum number of dimensions)
        maxDims (maximum number of dimensions)
        shells (number of extended shells in A* V:E)
        numBins (how many distances to simulate)
        maxR (maximum distance)
        match (matching method to use as per MatchType)
        numTrials (trials per bin, per dimensionality)
        seed (random seed)
        verbose ('true' or 'false')
        dims=value is shorthand for minDims=value maxDims=value.
"""
__author__ = 'Barry Drake'

from typing import Callable

import numpy as np
import random
from time import time
import sys
from enum import Enum
from astarnn import AStarNN


class MatchType(Enum):

    CALLBACK = 0
    HASH = 1
    CVECTOR = 2

    def function_value(self) -> Callable:
        return [
            self._match_callback,
            self._match_hash,
            self._match_cvector
        ][self.value]

    @staticmethod
    def _match_callback(prober, p1, p2):
        c1 = [np.zeros(0)]

        def callback_1(_hash_code, _k, c):
            c1[0] = np.array(c)

        prober.nearest_callback(p1, callback_1)
        c1 = c1[0]

        _match = [False]

        def callback_2(_hash_code, _k, c):
            _match[0] |= np.array_equal(c1, c)

        prober.extended_callback(p2, callback_2)
        result = _match[0]

        return result

    @staticmethod
    def _match_hash(prober, p1, p2):
        hash_code = prober.nearest_hash(p1)
        candidate_hash_codes = prober.extended_hash(p2)
        return hash_code in candidate_hash_codes

    @staticmethod
    def _match_cvector(prober, p1, p2):
        cvector = prober.nearest_cvector(p1)
        candidate_cvectors = prober.extended_cvector(p2)
        for candidate in candidate_cvectors:
            if np.array_equal(cvector, candidate):
                return True
        return False


default_minDims = 1
default_maxDims = 32
default_shells = 16
default_numBins = 200
default_maxR = 10
default_match = MatchType.HASH
default_numTrials = 10000
default_delim = '\t'
default_seed = None
default_verbose = False


def find_curves(
        minDims=default_minDims,
        maxDims=default_maxDims,
        shells=default_shells,
        numBins=default_numBins,
        maxR=default_maxR,
        match=default_match,
        numTrials=default_numTrials,
        delim=default_delim,
        seed=default_seed
    ):
    if seed is None:
        seed = time()
    seed = int(seed)
    random.seed(seed)

    # show parameters
    print("find_curves")
    print()
    print("minDims  ", minDims)
    print("maxDims  ", maxDims)
    print("shells   ", shells)
    print("numBins  ", numBins)
    print("maxR     ", maxR)
    print("numTrials", numTrials)
    print("seed     ", seed)
    print()

    # show header
    print(f"dim{delim}bin")
    print("---", end='')
    for bin_num in range(1, 1 + numBins):
        r = maxR / float(numBins) * bin_num
        print(f"{delim}{r:.2f}", end='')
        print(f"{delim}{r:.2f}", end='')
    print()

    # run the simulation - show data incrementally
    match_f = match.function_value()
    for dims in range(minDims, 1 + maxDims):
        prober = AStarNN(dims, 1.0, shells)

        print(dims, end='')
        sys.stdout.flush()

        for bin_num in range(1, 1 + numBins):
            radius = maxR / float(numBins) * bin_num
            matchCount = 0
            for trial in range(1, 1 + numTrials):
                p1 = np.random.uniform(-123456, 123456, size=dims)
                while True:
                    p2 = np.random.normal(size=dims)
                    s = np.linalg.norm(p2)
                    if s > 0.0001:
                        break
                p2 *= radius / s
                p2 += p1

                if match_f(prober, p1, p2):
                    matchCount += 1

            f = matchCount / float(numTrials)
            print(f"{delim}{f}", end='')
            sys.stdout.flush()
        print()


def main():
    minDims = default_minDims
    maxDims = default_maxDims
    shells = default_shells
    numBins = default_numBins
    maxR = default_maxR
    match = default_match
    numTrials = default_numTrials
    delim = default_delim
    seed = default_seed
    verbose = default_verbose

    # parse the command line
    arg_dims = "dims="
    arg_minDims = "minDims="
    arg_maxDims = "maxDims="
    arg_shells = "shells="
    arg_numBins = "numBins="
    arg_maxR = "maxR="
    arg_match = "match="
    arg_numTrials = "numTrials="
    arg_seed = "seed="
    arg_verbose = "verbose="

    for arg in sys.argv[1:]:
        if arg.startswith(arg_dims):
            minDims = maxDims = _value_of(int, arg_dims, arg)
        elif arg.startswith(arg_minDims):
            minDims = _value_of(int, arg_minDims, arg)
        elif arg.startswith(arg_maxDims):
            maxDims = _value_of(int, arg_maxDims, arg)
        elif arg.startswith(arg_shells):
            shells = _value_of(int, arg_shells, arg)
        elif arg.startswith(arg_numBins):
            numBins = _value_of(int, arg_numBins, arg)
        elif arg.startswith(arg_maxR):
            maxR = _value_of(float, arg_maxR, arg)
        elif arg.startswith(arg_match):
            maxR = _value_of(MatchType, arg_match, arg)
        elif arg.startswith(arg_numTrials):
            numTrials = _value_of(int, arg_numTrials, arg)
        elif arg.startswith(arg_seed):
            seed = _value_of(int, arg_seed, arg)
        elif arg.startswith(arg_verbose):
            verbose = _value_of(bool, arg_verbose, arg)
        else:
            print(f"Unknown command line argument: {arg!r}", file=sys.stderr)
            print(
                f"""Usage: python find_curves.py [param=value]*
                No spaces around the '=' character.
                'param' can be any of the following, showing default values:
                minDims = {default_minDims} (minimum number of dimensions)
                maxDims = {default_maxDims} (maximum number of dimensions)
                shells = {default_shells} (number of extended shells in A* V:E)
                numBins = {default_numBins} (how many distances to simulate)
                maxR = {default_maxR} (maximum distance)
                match = {default_match.name} (matching method to use as per MatchType)
                numTrials = {default_numTrials} (trials per bin, per dimensionality)
                seed = current time (random seed)
                verbose = {default_verbose} ('true' or 'false')
                dims=value is shorthand for minDims=value maxDims=value.
                """,
                file=sys.stderr)
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
    if shells < 0:
        print("shells cannot be negative", file=sys.stderr)
        exit(1)
    if numBins <= 0:
        print("numBins must be positive", file=sys.stderr)
        exit(1)
    if maxR <= 0:
        print("maxR must be positive", file=sys.stderr)
        exit(1)
    if numTrials <= 0:
        print("numTrials must be positive", file=sys.stderr)
        exit(1)

    if verbose:
        print(
            f"""minDims={minDims}
            maxDims={maxDims}
            shells={shells}
            numBins={numBins}
            maxR={maxR}
            match={match.value}
            numTrials={numTrials}
            seed={seed}
            """
        )

    find_curves(
        minDims,
        maxDims,
        shells,
        numBins,
        maxR,
        match,
        numTrials,
        delim,
        seed
    )


def _value_of(value_type, prefix, arg):
    val = arg[len(prefix):]

    if value_type is bool:
        val = val.lower()
        if val == "true" or val == "yes" or val == "1":
            return True
        elif val == "false" or val == "no" or val == "0":
            return False
    elif value_type is MatchType:
        val = val.upper()
        return MatchType[val]

    return value_type(val)


if __name__ == '__main__':
    main()
