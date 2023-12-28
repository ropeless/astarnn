"""
This is a simple use of AStarNN library as a Python implemented
locality sensitive hash (LSH).

In this script the hash table and marshalling code is implemented
in Python which is slower and has larger memory requirements
than a native implementations, AStarIndex or LSH in astarnn.
"""
__author__ = 'Barry Drake'


import numpy as np
from astarnn import AStarNN


class LSH(object):

    def __init__(self, dim, packing_radius, num_shells):
        self._nn = AStarNN(dim, packing_radius, num_shells)
        self._data = {}

    def insert(self, vector, data):
        """
        Insert the given data into the hash at the point determined by the given vector.
        """

        # Despite being non-pythonic, we test the validity of the input
        # vector now as is can be difficult to debug errors as the symptoms
        # may be rather delayed.
        if len(vector) != self._nn.dim:
            raise ValueError('input vector dimensionality should be {self._nn.dim()}')

        hash_code = self._nn.nearest_hash(vector)
        l = self._data.get(hash_code, None)
        if l is None:
            l = []
            self._data[hash_code] = l
        l.append((vector, data))

    def query(self, query_vector):
        """
        Return the nearest insertion point that is the one closest to
        the nearest query vector. If nothing is found, then (inf, None, None) is returned.
        :returns: (distance, vector, data)
        """
        best = (float('inf'), None, None)
        hash_codes = self._nn.extended_hash(query_vector)
        for hash_code in hash_codes:
            l = self._data.get(hash_code, None)
            if l is not None:
                for hash_vector, hash_data in l:
                    dist = np.linalg.norm(query_vector - hash_vector)
                    if dist < best[0]:
                        best = (dist, hash_vector, hash_data)
        return best

    def candidates(self, query_vector):
        """
        Return a list of candidates, each candidate is a tuple (vector, data) as inserted.
        The candidates are those reached by the lattice probes using the given query vector.
        """
        result = []
        hash_codes = self._nn.extended_hash(query_vector)
        for hash_code in hash_codes:
            l = self._data.get(hash_code, None)
            if l is not None:
                for insertion in l:
                    result.append(insertion)
        return result

    @property
    def dim(self):
        return self._nn.dim

    @property
    def num_shells(self):
        return self._nn.num_shells

    @property
    def num_probes(self):
        return self._nn.num_probes
