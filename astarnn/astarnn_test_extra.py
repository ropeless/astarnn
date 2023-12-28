"""
Additional tests for 'astarnn'.

These tests do not use the AStarNN library, rather they provide
extra testing of maths formula.
"""
__author__ = 'Barry Drake'


import numpy as np
import unittest
from math import sqrt


min_n = 1
max_n = 7
num_trials = 500
min_i = -999
max_i = 999
limit_perms = 1000


# ======================================================================
#  These are the functions under tests
# ======================================================================

def z_to_l(z):
    n = len(z)
    l = np.zeros(n + 1, dtype=np.int64)

    l[0] = -n * z[0] + (n + 1) * (z.sum() - z[0])
    for i in range(1, n):
        l[i] = z[0] - (n+1) * z[i]
    l[n] = z[0]
    return l


def l_to_z(l):
    n = len(l) - 1
    z = np.zeros(n, dtype=np.int64)

    z[0] = (n * l[n] - (l.sum() - l[n])) / (n + 1)
    for i in range(1, n):
        z[i] = (l[n] - l[i]) / (n + 1)
    return z


def z_to_c(z):
    n = len(z)
    c = np.zeros(n + 1, dtype=np.int64)
    k = k_z(z)

    c[0] = (-n * z[0] - k) / (n + 1) + z.sum() - z[0]
    c[n] = (z[0] - k) / (n + 1)
    for i in range(1, n):
        c[i] = c[n] - z[i]
    return c


def c_to_z(c):
    n = len(c) - 1
    z = np.zeros(n, dtype=np.int64)

    z[0] = (n+1) * c[n] - c.sum()
    for i in range(1, n):
        z[i] = c[n] - c[i]
    return z


def c_to_l(c):
    n = len(c) - 1
    l = np.zeros(n + 1, dtype=np.int64)
    k = k_c(c)

    for i in range(0, n+1):
        l[i] = (n+1) * c[i] + k
    return l


def l_to_c(l):
    n = len(l) - 1
    c = np.zeros(n + 1, dtype=np.int64)
    k = k_l(l)

    for i in range(0, n + 1):
        c[i] = (l[i] - k) / (n+1)
    return c


def k_z(z):
    n = len(z)
    return z[0] % (n + 1)


def k_l(l):
    n = len(l) - 1
    return l[0] % (n + 1)


def k_c(c):
    return -c.sum()


def cp_to_h(c, p):
    """
    Given the remainder-0 c-vector and permutation of a Delaunay cell,
    return the coordinates of the hole in the Delaunay cell, in the
    lattice representation space.
    """
    n = len(c) - 1
    h = np.zeros(n + 1, dtype=np.double)

    for i in range(0, n+1):
        j = p[i]
        h[j] = c[j] * (n+1) - n/2.0 + i
    return h


def quant_to_rep(x):
    """
    Convert a vector in R^n to an equivalent vector in the weight 0 subspace of R^(n+1).
    """
    n = len(x)
    v = np.zeros(n + 1, dtype=np.double)

    s = np.sum(x)
    v[n] = -s / sqrt(n + 1.0)
    t = (v[n] + s) / n
    for i in range(n):
        v[i] = x[i] - t
    return v


def rep_to_quant(v):
    """
    Convert a vector in the weight 0 subspace of R^(n+1) to an equivalent vector in R^n.
    """
    n = len(v) - 1
    x = np.zeros(n, dtype=np.double)

    norm = sqrt(n + 1.0)
    t = v[n] * (norm - n - 1) / n / norm
    for i in range(n):
        x[i] = v[i] + t
    return x


# ======================================================================
#  End of functions under tests
# ======================================================================


def all_perms(elements):
    if len(elements) <= 1:
        yield elements
    else:
        for perm in all_perms(elements[1:]):
            for i in range(len(elements)):
                yield perm[:i] + elements[0:1] + perm[i:]


def enumerate_Delaunay_c_vectors(c_0, perm):
    n = len(c_0) - 1
    c = np.copy(c_0)
    yield c
    for k in range(1, n + 1):
        c[perm[k-1]] -= 1
        yield c


class ExtraFixture(unittest.TestCase):
    """
    Randomly check A* space conversions
    """

    def assertEqualVectors(self, v1, v2):
        if len(v1) == len(v2):
            for i in range(len(v1)):
                if v1[i] != v2[i]:
                    self.fail(str(v1) + " != " + str(v2) + ", position " + str(i))
        else:
            self.fail(str(v1) + " != " + str(v2) + ", unequal lengths")

    def assertAlmostEqualVectors(self, v1, v2, places=None, delta=None):
        if len(v1) == len(v2):
            for i in range(len(v1)):
                msg = str(v1) + " != " + str(v2) + ", position " + str(i)
                self.assertAlmostEqual(v1[i], v2[i], places=places, msg=msg, delta=delta)
        else:
            self.fail(str(v1) + " != " + str(v2) + ", unequal lengths")


class SpaceConversionTest(ExtraFixture):
    """
    Randomly check A* space conversions
    """

    def test_from_z(self):
        for n in range(min_n, min_n + max_n - 1):
            for trial in range(num_trials):

                z = np.random.randint(min_i, max_i, n, dtype=np.int64)

                l = z_to_l(z)
                c = z_to_c(z)

                self.assertEqualVectors(z, l_to_z(l))
                self.assertEqualVectors(z, c_to_z(c))
                self.assertEqual(k_z(z), k_l(l))
                self.assertEqual(k_z(z), k_c(c))

    def test_from_l(self):
        for n in range(min_n, min_n + max_n - 1):
            for trial in range(num_trials):

                # assumes z_to_l is okay
                l = z_to_l(np.random.randint(min_i, max_i, n, dtype=np.int64))

                c = l_to_c(l)
                z = l_to_z(l)

                self.assertEqualVectors(l, c_to_l(c))
                self.assertEqualVectors(l, z_to_l(z))
                self.assertEqual(k_l(l), k_c(c))
                self.assertEqual(k_l(l), k_z(z))

    def test_from_c(self):
        for n in range(min_n, min_n + max_n - 1):
            for trial in range(num_trials):

                k = np.random.randint(0, n+1)
                c = np.random.randint(min_i, max_i, n+1, dtype=np.int64)
                c[np.random.randint(0, n+1)] -= c.sum() + k
                self.assertEqual(k, k_c(c))

                l = c_to_l(c)
                z = c_to_z(c)

                self.assertEqualVectors(c, l_to_c(l))
                self.assertEqualVectors(c, z_to_c(z))
                self.assertEqual(k, k_l(l))
                self.assertEqual(k, k_z(z))


class HoleTest(ExtraFixture):
    """
    Randomly check that the fast way to compute a hole of a Delaunay cell works.
    """

    def test_hole(self):
        for n in range(min_n, min_n + max_n - 1):
            for trial in range(num_trials):

                # make a random remainder-0 c vector
                c = np.random.randint(min_i, max_i, n+1, dtype=np.int64)
                c[np.random.randint(0, n+1)] -= c.sum()
                self.assertEqual(0, k_c(c))

                # make a random permutation
                p = np.random.permutation(n+1)

                # compute the hole the long way...

                # 1st generate the Delaunay cell vertices, a c-vectors
                ck = np.zeros((n+1, n+1), dtype=np.int64)
                for i in range(n+1):
                    ck[0, i] = c[i]
                for k in range(1, n+1):
                    for i in range(n + 1):
                        ck[k, i] = ck[k-1, i]
                    ck[k, p[k-1]] -= 1

                # 2nd, convert the c-vectors, ck, to lattice points, lk
                lk = np.zeros((n + 1, n + 1), dtype=np.int64)
                for k in range(0, n+1):
                    for i in range(n + 1):
                        lk[k, i] = ck[k, i] * (n + 1) + k

                # 3rd compute the average (centroid)
                h = np.zeros(n + 1, dtype=np.double)
                for k in range(0, n+1):
                    for i in range(n + 1):
                        h[i] += lk[k, i]
                for i in range(n + 1):
                    h[i] /= (n+1)

                # compare the long way to the fact way
                self.assertEqualVectors(h, cp_to_h(c, p))

    def test_hole_is_centroid(self):
        for n in range(min_n, 1+max_n):

            # reference Conway and Sloane, 1999, 3rd ed, page 474.
            expect = np.array(range(-n, n+1, 2), dtype=np.double)

            # enumerate over all Delaunay cells that pass through the origin
            # i.e. enumerate all permutations of length (n+1), of which there are (n+1)! of them
            perm_num = limit_perms
            for perm in all_perms(tuple(range(n+1))):
                perm_num -= 1
                if perm_num < 0:
                    break

                # for the current Delaunay cell, enumerate the vertices

                ls = []
                sum_value = np.zeros(n + 1, dtype=np.double)
                for c in enumerate_Delaunay_c_vectors([0] * (n+1), perm):
                    l = c_to_l(c)
                    ls.append(l)
                    sum_value += l
                centroid_2 = sum_value * (2.0 / (n+1))
                sorted_centroid_2 = np.sort(centroid_2)
                self.assertEqualVectors(sorted_centroid_2, expect)

                # print('----------------------')
                for i in range(n+1):
                    for j in range(i+1, n+1):
                        li = ls[i]
                        lj = ls[j]

                        dist_2 = 0
                        for x in (li - lj):
                            dist_2 += x * x

                        # TODO
                        # print(dist_2)


class SpaceTest(ExtraFixture):
    """
    Randomly check that mapping between the quantisation space and
    representation space works correctly.
    """

    def test_quant_rep_quant_round_trip(self):
        for n in range(min_n, min_n + max_n - 1):
            for trial in range(num_trials):
                x = np.random.uniform(low=-100.0, high=100.0, size=n)

                v = quant_to_rep(x)

                self.assertEqual(len(v), n + 1)
                self.assertAlmostEqual(sum(v), 0, places=6)

                x_back = rep_to_quant(v)

                self.assertEqual(len(x_back), n)
                self.assertAlmostEqualVectors(x, x_back, places=6)

    def test_rep_quant_rep_round_trip(self):
        for n in range(min_n, min_n + max_n - 1):
            for trial in range(num_trials):
                v = np.random.uniform(low=-min_i, high=max_i, size=n+1)
                v[np.random.randint(0, n+1)] -= v.sum()

                x = rep_to_quant(v)

                self.assertEqual(len(x), n)

                v_back = quant_to_rep(x)

                self.assertEqual(len(v_back), n+1)
                self.assertAlmostEqualVectors(v, v_back, places=6)


if __name__ == "__main__":
    unittest.main()
