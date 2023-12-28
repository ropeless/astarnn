"""
Simple demonstration script showing the core functionality of AStarNN.
"""
__author__ = 'Barry Drake'

from astarnn import AStarNN
import numpy as np


DIM = 5
PACKING_RADIUS = 1.0
NUM_OF_SHELLS = 3


def main():
    hasher = AStarNN(DIM, PACKING_RADIUS, NUM_OF_SHELLS)

    hash_table = {}

    v1 = np.array([1.3, 4.2, 52.1, 5.2, 7.1])
    v1_data = 'My data associated with v1'

    v2 = np.array([2.7, 5.2, 49.1, 6.9, 7.0])
    v2_data = 'Some data associated with v2'

    hash_code = hasher.nearest_hash(v1)
    print(f'Insert v1: {v1} {hash_code} {v1_data!r}')
    hash_table[hash_code] = v1_data

    hash_code = hasher.nearest_hash(v2)
    print(f'Insert v2: {v2} {hash_code} {v2_data!r}')
    hash_table[hash_code] = v2_data

    q = np.array([1.3, 4.1, 52.3, 5.9, 7.0])
    query_hash_codes = hasher.extended_hash(q)

    print()
    print(f'Query: {q}')
    for hask_code in query_hash_codes:
        print(hask_code)
        query_data = hash_table.get(hask_code)
        if query_data is not None:
            print(f'Got it: {query_data!r}')

    print()
    print('Done.')


if __name__ == '__main__':
    main()
