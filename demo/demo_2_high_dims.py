"""
Demo 2: high dimensions.
"""
__author__ = 'Barry Drake'

from astarnn import LSH
import numpy as np


DIM = 256
NUM_OF_SHELLS = 6
NUM_OF_INSERTIONS = 1_000_000
NUM_OF_QUERIES = 100
PACKING_RADIUS = 0.49
RAND_SEED = 18491283


def main():
    print("dimensions           =", DIM)
    print("number of shells     =", NUM_OF_SHELLS)
    print("number of insertions =", NUM_OF_INSERTIONS)
    print("number of queries    =", NUM_OF_QUERIES)
    print("packing radius       =", PACKING_RADIUS)
    print("rand seed            =", RAND_SEED)

    np.set_printoptions(linewidth=5000)
    np.random.seed(RAND_SEED)
    lsh = LSH(DIM, PACKING_RADIUS, NUM_OF_SHELLS)

    print("number of probes     =", lsh.num_probes)

    print()
    print("Inserting")
    for insertion_num in range(NUM_OF_INSERTIONS):
        vector = np.random.rand(DIM)
        lsh.insert(vector, insertion_num)

    print()
    print("Querying")
    for query_num in range(NUM_OF_QUERIES):
        vector = np.random.rand(DIM)
        distance, insertion, data = lsh.query(vector)
        # print(query_num, distance, insertion, data)

    print()
    print("Measuring candidates set size")
    for query_num in range(NUM_OF_QUERIES):
        vector = np.random.rand(DIM)
        num_candidates = lsh.num_candidates(vector)
        print(query_num, num_candidates)

    print()
    print("Done.")


if __name__ == '__main__':
    main()
