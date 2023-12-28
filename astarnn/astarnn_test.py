"""
Unit tests for 'astarnn'.
"""
__author__ = 'Barry Drake'

import unittest
import math
from dataclasses import dataclass
from astarnn import info_string, extended_info_string, ReturnVal, AStarException, AStarNN, max_num_shells, \
    rho, AStarIndex
from _astarnn import _round_up  # white box testing
import numpy as np


class Test_general(unittest.TestCase):

    def test_info_string(self):
        # Just ensure that it returns a string
        self.assertEqual(type(info_string()), str)
        # print info_string()

    def test_extended_info_string(self):
        # Just ensure that it returns a string
        self.assertEqual(type(extended_info_string()), str)
        # print extended_info_string()

    def test_exception_rendering(self):
        ret = ReturnVal()
        ret.code = 1  # Error_mem_fail

        self.assertFalse(ret.ok())

        self.assertEqual(1, ret.return_val_code())
        self.assertEqual('Error_mem_fail', ret.return_val_string())
        self.assertEqual(1, int(ret))
        self.assertEqual('ReturnVal(1): Error_mem_fail', str(ret))

        with self.assertRaises(AStarException) as context:
            ret.check()
        exception = context.exception
        self.assertEqual(1, exception.return_val_code())
        self.assertEqual('Error_mem_fail', exception.return_val_string())
        self.assertEqual(1, int(exception))
        self.assertEqual('ReturnVal(1): Error_mem_fail', str(exception))


class Test_AStarNN(unittest.TestCase):

    def test_create_small(self):
        dim = 1
        packing_radius = 1
        num_shells = 0

        nn = AStarNN(dim, packing_radius, num_shells)

        self.assertEqual(dim,            nn.dim)
        self.assertEqual(packing_radius, nn.packing_radius)
        self.assertEqual(num_shells,     nn.num_shells)

    def test_create_big_dim(self):
        dim = 100
        packing_radius = 1
        num_shells = 0

        nn = AStarNN(dim, packing_radius, num_shells)

        self.assertEqual(dim,            nn.dim)
        self.assertEqual(packing_radius, nn.packing_radius)
        self.assertEqual(num_shells,     nn.num_shells)

    def test_create_big_shell(self):
        dim = 1
        packing_radius = 1
        num_shells = 7

        nn = AStarNN(dim, packing_radius, num_shells)

        self.assertEqual(dim,            nn.dim)
        self.assertEqual(packing_radius, nn.packing_radius)
        self.assertEqual(num_shells,     nn.num_shells)

    def test_create_big_dim_and_shell(self):
        dim = 100
        packing_radius = 1
        num_shells = 7

        nn = AStarNN(dim, packing_radius, num_shells)

        self.assertEqual(dim,            nn.dim)
        self.assertEqual(packing_radius, nn.packing_radius)
        self.assertEqual(num_shells,     nn.num_shells)

    def test_nearest_callback(self):
        dim = 2
        packing_radius = 1
        num_shells = 1

        nn = AStarNN(dim, packing_radius, num_shells)

        v = np.array([6.1, -0.2], dtype=np.double)

        def callback(hash_code, k, c):
            self.assertEqual(18446744073709549664, hash_code)
            self.assertEqual(2, k)
            self.assertEqual(3, len(c))
            self.assertEqual(1, c[0])
            self.assertEqual(-1, c[1])
            self.assertEqual(-2, c[2])

        nn.nearest_callback(v, callback)

    def test_nearest_hash(self):
        dim = 2
        packing_radius = 1
        num_shells = 1

        nn = AStarNN(dim, packing_radius, num_shells)

        v = np.array([6.1, -0.2], dtype=np.double)
        hash_code = nn.nearest_hash(v)
        self.assertEqual(18446744073709549664, hash_code)

    def test_nearest_cvector(self):
        dim = 2
        packing_radius = 1
        num_shells = 1

        nn = AStarNN(dim, packing_radius, num_shells)

        v = np.array([6.1, -0.2], dtype=np.double)

        expect = np.array([1, -1, -2])

        cvector = nn.nearest_cvector(v)

        self.assertTrue(np.array_equal(expect, cvector))

    def test_delaunay_callback(self):
        dim = 2
        packing_radius = 1
        num_shells = 1

        nn = AStarNN(dim, packing_radius, num_shells)

        v = np.array([10.1, -0.2], dtype=np.double)

        @dataclass
        class MyCounter:
            test_case: unittest.TestCase
            count: int = 0

            def assertEqual(self, *args, **kwargs):
                self.test_case.assertEqual(*args, **kwargs)

            def fail(self, *args, **kwargs):
                self.test_case.fail(*args, **kwargs)

            def __call__(self, hash_code, k, c):
                self.count += 1
                if self.count == 1:
                    self.assertEqual(18446744073709549666, hash_code)
                    self.assertEqual(0, k)
                    self.assertEqual(3, len(c))
                    self.assertEqual(3, c[0])
                    self.assertEqual(-1, c[1])
                    self.assertEqual(-2, c[2])
                elif self.count == 2:
                    self.assertEqual(18446744073709548705, hash_code)
                    self.assertEqual(1, k)
                    self.assertEqual(3, len(c))
                    self.assertEqual(3, c[0])
                    self.assertEqual(-1, c[1])
                    self.assertEqual(-3, c[2])
                elif self.count == 3:
                    self.assertEqual(18446744073709548674, hash_code)
                    self.assertEqual(2, k)
                    self.assertEqual(3, len(c))
                    self.assertEqual(3, c[0])
                    self.assertEqual(-2, c[1])
                    self.assertEqual(-3, c[2])
                else:
                    self.fail("callback called too many times")

        callback = MyCounter(self)
        nn.delaunay_callback(v, callback)

    def test_delaunay_hash(self):
        dim = 2
        packing_radius = 1
        num_shells = 1

        nn = AStarNN(dim, packing_radius, num_shells)

        v = np.array([10.1, -0.2], dtype=np.double)

        expect = [18446744073709549666, 18446744073709548705, 18446744073709548674]

        hash_codes = nn.delaunay_hash(v)

        self.assertEqual(len(expect), len(hash_codes))
        self.assertEqual(expect[0], hash_codes[0])
        self.assertEqual(expect[1], hash_codes[1])
        self.assertEqual(expect[2], hash_codes[2])

    def test_delaunay_cvector(self):
        dim = 2
        packing_radius = 1
        num_shells = 1

        nn = AStarNN(dim, packing_radius, num_shells)

        v = np.array([10.1, -0.2], dtype=np.double)

        expect = np.array([[3, -1, -2], [3, -1, -3], [3, -2, -3]])

        cvectors = nn.delaunay_cvector(v)

        self.assertTrue(np.array_equal(expect, cvectors))

    def test_extended_callback(self):
        dim = 2
        packing_radius = 1
        num_shells = 1

        nn = AStarNN(dim, packing_radius, num_shells)

        self.assertEqual(6, nn.num_probes)

        v = np.array([10.1, -0.2], dtype=np.double)

        @dataclass
        class MyCounter:
            test_case: unittest.TestCase
            count: int = 0

            def assertEqual(self, *args, **kwargs):
                self.test_case.assertEqual(*args, **kwargs)

            def fail(self, *args, **kwargs):
                self.test_case.fail(*args, **kwargs)

            def __call__(self, hash_code, k, c):
                self.count += 1
                if self.count == 1:
                    self.assertEqual(18446744073709549666, hash_code)
                    self.assertEqual(0, k)
                    self.assertEqual(3, len(c))
                    self.assertEqual(3, c[0])
                    self.assertEqual(-1, c[1])
                    self.assertEqual(-2, c[2])
                elif self.count == 2:
                    self.assertEqual(18446744073709548705, hash_code)
                    self.assertEqual(1, k)
                    self.assertEqual(3, len(c))
                    self.assertEqual(3, c[0])
                    self.assertEqual(-1, c[1])
                    self.assertEqual(-3, c[2])
                elif self.count == 3:
                    self.assertEqual(18446744073709548674, hash_code)
                    self.assertEqual(2, k)
                    self.assertEqual(3, len(c))
                    self.assertEqual(3, c[0])
                    self.assertEqual(-2, c[1])
                    self.assertEqual(-3, c[2])
                elif self.count == 4:
                    self.assertEqual(18446744073709548704, hash_code)
                    self.assertEqual(2, k)
                    self.assertEqual(3, len(c))
                    self.assertEqual(2, c[0])
                    self.assertEqual(-1, c[1])
                    self.assertEqual(-3, c[2])
                elif self.count == 5:
                    self.assertEqual(18446744073709549635, hash_code)
                    self.assertEqual(1, k)
                    self.assertEqual(3, len(c))
                    self.assertEqual(3, c[0])
                    self.assertEqual(-2, c[1])
                    self.assertEqual(-2, c[2])
                elif self.count == 6:
                    self.assertEqual(18446744073709548706, hash_code)
                    self.assertEqual(0, k)
                    self.assertEqual(3, len(c))
                    self.assertEqual(4, c[0])
                    self.assertEqual(-1, c[1])
                    self.assertEqual(-3, c[2])
                else:
                    self.fail("callback called too many times")

        callback = MyCounter(self)
        nn.extended_callback(v, callback)

        self.assertEqual(6, callback.count)

    def test_extended_hash(self):
        dim = 2
        packing_radius = 1
        num_shells = 1

        nn = AStarNN(dim, packing_radius, num_shells)

        self.assertEqual(6, nn.num_probes)

        v = np.array([10.1, -0.2], dtype=np.double)

        expect = [
            18446744073709549666, 18446744073709548705, 18446744073709548674,
            18446744073709548704, 18446744073709549635, 18446744073709548706
        ]

        hash_codes = nn.extended_hash(v)

        self.assertEqual(len(expect), len(hash_codes))
        self.assertEqual(expect[0], hash_codes[0])
        self.assertEqual(expect[1], hash_codes[1])
        self.assertEqual(expect[2], hash_codes[2])
        self.assertEqual(expect[3], hash_codes[3])
        self.assertEqual(expect[4], hash_codes[4])
        self.assertEqual(expect[5], hash_codes[5])

    def test_extended_cvector(self):
        dim = 2
        packing_radius = 1
        num_shells = 1

        nn = AStarNN(dim, packing_radius, num_shells)

        self.assertEqual(6, nn.num_probes)

        v = np.array([10.1, -0.2], dtype=np.double)

        expect = [
            [3, -1, -2], [3, -1, -3], [3, -2, -3],
            [2, -1, -3], [3, -2, -2], [4, -1, -3]
        ]

        cvectors = nn.extended_cvector(v)

        self.assertTrue(np.array_equal(np.array(expect), cvectors))

    def test_extended_callback_high_dim(self):
        dim = 32
        packing_radius = 1
        num_shells = 4

        expect_probes = 396

        nn = AStarNN(dim, packing_radius, num_shells)

        self.assertEqual(expect_probes, nn.num_probes)

        v = np.random.normal(0.0, 10.0, dim)

        @dataclass
        class MyCounter:
            count: int = 0

            def __call__(self, hash_code, k, c):
                self.count += 1

        callback = MyCounter()
        nn.extended_callback(v, callback)

        self.assertEqual(expect_probes, callback.count)

    def test_zero_dim(self):
        packing_radius = 1
        num_shells = 1
        dim = 0

        with self.assertRaises(AStarException) as context:
            AStarNN(dim, packing_radius, num_shells)
        self.assertEqual("Error_invalid_dim", context.exception.return_val_string())

    def test_max_num_shells(self):
        value = max_num_shells()
        self.assertEqual(30, value)   # we know the limit for the library

        dim = 1
        packing_radius = 1

        # should be ok
        nn = AStarNN(dim, packing_radius, value)
        self.assertEqual(value, nn.num_shells)

        with self.assertRaises(AStarException) as context:
            AStarNN(dim, packing_radius, value + 1)
        self.assertEqual("Error_invalid_num_shells", context.exception.return_val_string())

    def test_bad_packing_radius(self):
        dim = 1
        num_shells = 1

        with self.assertRaises(AStarException) as context:
            AStarNN(dim, 0.0, num_shells)
        self.assertEqual("Error_invalid_packing_radius", context.exception.return_val_string())

        with self.assertRaises(AStarException) as context:
            AStarNN(dim, -1.0, num_shells)
        self.assertEqual("Error_invalid_packing_radius", context.exception.return_val_string())

    def test_rho(self):

        def expect(dim):
            return (dim + 1.0) * math.sqrt(dim / (dim + 1.0)) / 2.0

        self.assertAlmostEqual(expect(1), rho(1), 6)
        self.assertAlmostEqual(expect(2), rho(2), 6)
        self.assertAlmostEqual(expect(13), rho(13), 6)
        self.assertAlmostEqual(expect(378), rho(378), 6)

    def test_cvector_k_to_lattice_point_in_lattice_space(self):
        nn = AStarNN(dim=2, packing_radius=1, num_shells=1)
        c = [1, -1, 2]
        k = -2
        lat_point_mapped = nn.cvector_k_to_lattice_point_in_lattice_space(c, k)
        self.assertEqual(list(lat_point_mapped), [-1, 5, -4])

    def test_cvector_to_lattice_point_in_lattice_space(self):
        nn = AStarNN(dim=2, packing_radius=1, num_shells=1)
        c = [1, -1, 2]
        lat_point_mapped = nn.cvector_to_lattice_point_in_lattice_space(c)
        self.assertEqual(list(lat_point_mapped), [-1, 5, -4])

    def cvector_k_to_lattice_point(self):
        nn = AStarNN(dim=2, packing_radius=1, num_shells=1)
        c = [1, -1, 2]
        k = -2
        lat_point_mapped = nn.cvector_k_to_lattice_point(c, k)
        self.assertEqual(list(lat_point_mapped), [-5.27791687, -0.3789378])

    def cvector_to_lattice_point(self):
        nn = AStarNN(dim=2, packing_radius=1, num_shells=1)
        c = [1, -1, 2]
        lat_point_mapped = nn.cvector_to_lattice_point(c)
        self.assertEqual(list(lat_point_mapped), [-5.27791687, -0.3789378])

    def test_lattice_point(self):
        dim = 2
        packing_radius = 1
        num_shells = 1

        nn = AStarNN(dim, packing_radius, num_shells)

        v = np.array([6.1, -0.2], dtype=np.double)

        v_mapped = nn.to_lattice_space(v)

        def callback(_, k, c):

            lat_point_mapped = nn.cvector_k_to_lattice_point_in_lattice_space(c, k)
            lat_point_mapped_2 = nn.cvector_to_lattice_point_in_lattice_space(c)

            self.assertEqual(dim + 1, len(lat_point_mapped))
            self.assertEqual(dim + 1, len(lat_point_mapped_2))
            self.assertEqual(lat_point_mapped[0], lat_point_mapped_2[0])
            self.assertEqual(lat_point_mapped[1], lat_point_mapped_2[1])
            self.assertEqual(lat_point_mapped[2], lat_point_mapped_2[2])

            lat_point = nn.cvector_k_to_lattice_point(c, k)
            lat_point_2 = nn.cvector_to_lattice_point(c)

            self.assertEqual(dim, len(lat_point))
            self.assertEqual(dim, len(lat_point_2))
            self.assertEqual(lat_point[0], lat_point_2[0])
            self.assertEqual(lat_point[1], lat_point_2[1])

            dist_mapped = np.linalg.norm(lat_point_mapped - v_mapped) / nn.scale
            dist = np.linalg.norm(lat_point - v)

            self.assertAlmostEqual(dist, dist_mapped, 6)

        nn.nearest_callback(v, callback)

    def test_invalid_vector_dimensionality_0(self):
        test_rho = 0.2
        dim = 2
        num_shells = 0

        data = np.array([], dtype=np.float64)
        hasher = AStarNN(dim, test_rho, num_shells)

        with self.assertRaises(AStarException) as context:
            hasher.extended_hash(data)
        self.assertEqual("Error_invalid_dim", context.exception.return_val_string())

    def test_invalid_vector_dimensionality_1(self):
        test_rho = 0.2
        dim = 2
        num_shells = 0

        data = np.array([1], dtype=np.float64)
        hasher = AStarNN(dim, test_rho, num_shells)

        with self.assertRaises(AStarException) as context:
            hasher.extended_hash(data)
        self.assertEqual("Error_invalid_dim", context.exception.return_val_string())

    def test_invalid_vector_dimensionality_3(self):
        test_rho = 0.2
        dim = 2
        num_shells = 0

        data = np.array([1, 2, 3], dtype=np.float64)
        hasher = AStarNN(dim, test_rho, num_shells)

        with self.assertRaises(AStarException) as context:
            hasher.extended_hash(data)
        self.assertEqual("Error_invalid_dim", context.exception.return_val_string())


class Test_AStarIndex(unittest.TestCase):

    def test_create(self):
        dim = 10
        packing_radius = 1
        num_shells = 7

        index = AStarIndex(dim, packing_radius, num_shells)

        self.assertEqual(dim,            index.dim)
        self.assertEqual(packing_radius, index.packing_radius)
        self.assertEqual(num_shells,     index.num_shells)
        self.assertEqual(0,              index.num_hashes())

    def test_get_empty(self):
        dim = 3
        packing_radius = 1
        num_shells = 7

        index = AStarIndex(dim, packing_radius, num_shells)

        v = np.array([6.1, -0.2, 0.8], dtype=np.double)

        count = index.num_candidates(v)
        self.assertEqual(0, count)

        result = index.candidates(v)
        self.assertEqual(0, len(result))

    def test_get_singleton(self):
        dim = 3
        packing_radius = 1
        num_shells = 7

        index = AStarIndex(dim, packing_radius, num_shells)

        v = np.array([6.1, -0.2, 0.8], dtype=np.double)

        index.insert(v, 123)

        count = index.num_candidates(v)
        self.assertEqual(1, count)

        result = index.candidates(v)
        self.assertEqual(1, len(result))

        self.assertEqual(123, result[0])

    def test_get_multiple(self):
        dim = 3
        packing_radius = 1
        num_shells = 7

        index = AStarIndex(dim, packing_radius, num_shells)

        v = np.array([6.1, -0.2, 0.8], dtype=np.double)

        index.insert(v, 123)
        index.insert(v, 456)  # this will go to the same hash code

        self.assertEqual(1, index.num_hashes())
        self.assertEqual(2, index.num_elements())

        count = index.num_candidates(v)
        self.assertEqual(2, count)

        result = index.candidates(v)
        self.assertEqual(2, len(result))

        self.assertEqual(123, result[0])
        self.assertEqual(456, result[1])

    def test_get_different_hashes(self):
        dim = 3
        packing_radius = 1
        num_shells = 7

        v1 = np.array([6.1, -0.3, 0.8], dtype=np.double)
        v2 = np.array([6.1,  0.3, 0.8], dtype=np.double)

        # confirm that the vectors map to different hashes
        nn = AStarNN(dim, packing_radius, num_shells)
        h1 = nn.nearest_hash(v1)
        h2 = nn.nearest_hash(v2)
        self.assertNotEqual(h1, h2)

        index = AStarIndex(dim, packing_radius, num_shells)

        index.insert(v1, 123)
        index.insert(v2, 456)  # this will go to a different, nearby hash code

        self.assertEqual(2, index.num_hashes())
        self.assertEqual(2, index.num_elements())

        count = index.num_candidates(v1)
        self.assertEqual(2, count)

        result = index.candidates(v1)
        self.assertEqual(2, len(result))

        self.assertEqual(123, result[0])
        self.assertEqual(456, result[1])

    def test_clear(self):
        dim = 3
        packing_radius = 1
        num_shells = 7

        v = np.array([6.1, -0.3, 0.8], dtype=np.double)

        index = AStarIndex(dim, packing_radius, num_shells)
        self.assertEqual(0, index.num_hashes())
        self.assertEqual(0, index.num_elements())

        index.insert(v, 123)
        self.assertEqual(1, index.num_hashes())
        self.assertEqual(1, index.num_elements())

        index.insert(v, 456)
        self.assertEqual(1, index.num_hashes())
        self.assertEqual(2, index.num_elements())

        index.clear()
        self.assertEqual(0, index.num_hashes())
        self.assertEqual(0, index.num_elements())

    def test_clear_by_vector(self):
        dim = 3
        packing_radius = 1
        num_shells = 7

        v1 = np.array([6.1, -0.3, 0.8], dtype=np.double)
        v2 = np.array([9.1, -9.3, 9.8], dtype=np.double)

        index = AStarIndex(dim, packing_radius, num_shells)
        self.assertEqual(0, index.num_hashes())
        self.assertEqual(0, index.num_elements())

        index.insert(v1, 123)
        self.assertEqual(1, index.num_hashes())
        self.assertEqual(1, index.num_elements())

        index.insert(v2, 456)
        self.assertEqual(2, index.num_hashes())
        self.assertEqual(2, index.num_elements())

        index.clear_by_vector(v1)
        self.assertEqual(1, index.num_hashes())
        self.assertEqual(1, index.num_elements())


class Test_WhiteBox(unittest.TestCase):

    def test_round_up(self):
        tests = [
            (0.0, 0),
            (-0.0, 0),

            (1.0, 1),
            (2.0, 2),
            (3.0, 3),
            (4.0, 4),
            (5.0, 5),
            (6.0, 6),
            (7.0, 7),
            (8.0, 8),
            (9.0, 9),
            (10.0, 10),
            (11.0, 11),
            (12.0, 12),
            (13.0, 13),
            (14.0, 14),
            (15.0, 15),
            (16.0, 16),

            (0.5, 1),
            (1.5, 2),
            (2.5, 3),
            (3.5, 4),
            (4.5, 5),
            (5.5, 6),
            (6.5, 7),
            (7.5, 8),
            (8.5, 9),
            (9.5, 10),
            (10.5, 11),
            (11.5, 12),
            (12.5, 13),
            (13.5, 14),
            (14.5, 15),
            (15.5, 16),

            (-1.0, -1),
            (-2.0, -2),
            (-3.0, -3),
            (-4.0, -4),
            (-5.0, -5),
            (-6.0, -6),
            (-7.0, -7),
            (-8.0, -8),
            (-9.0, -9),
            (-10.0, -10),
            (-11.0, -11),
            (-12.0, -12),
            (-13.0, -13),
            (-14.0, -14),
            (-15.0, -15),
            (-16.0, -16),

            (-0.5, 0),
            (-1.5, -1),
            (-2.5, -2),
            (-3.5, -3),
            (-4.5, -4),
            (-5.5, -5),
            (-6.5, -6),
            (-7.5, -7),
            (-8.5, -8),
            (-9.5, -9),
            (-10.5, -10),
            (-11.5, -11),
            (-12.5, -12),
            (-13.5, -13),
            (-14.5, -14),
            (-15.5, -15),
            (-16.5, -16),
        ]
        for x, expect in tests:
            result = _round_up(x)
            self.assertEqual(result, expect)


if __name__ == '__main__':
    unittest.main()
