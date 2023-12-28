"""
Demo 1: Insert a large number of random points then perform random queries.
"""
__author__ = 'Barry Drake'

from astarnn import LSH
from stop_watch import StopWatch
import numpy as np


DIM = 16
NUM_OF_SHELLS = 4
NUM_OF_INSERTIONS = 1_000_000
NUM_OF_QUERIES = 10_000
PACKING_RADIUS = 0.25
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
    insert_time = StopWatch()
    vectors = []
    for insertion_num in range(NUM_OF_INSERTIONS):
        vector = np.random.rand(DIM)
        lsh.insert(vector, insertion_num)
        vectors.append(vector)
    insert_time.stop()

    print()
    print("Querying")
    query_time = StopWatch()
    for query_num in range(NUM_OF_QUERIES):
        vector = np.random.rand(DIM)
        distance, vector, data = lsh.query(vector)
        # print(query_num, distance, vector, data)
    query_time.stop()

    print()
    print("Measuring candidates set size")
    for query_num in range(NUM_OF_QUERIES):
        vector = np.random.rand(DIM)
        num_candidates = lsh.num_candidates(vector)
        print(query_num, num_candidates)

    print()
    print(f"insert time = {insert_time}")
    print(f"query time = {query_time}")
    print("Done.")


if __name__ == '__main__':
    main()
