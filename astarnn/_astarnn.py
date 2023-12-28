"""
Python interface to the AStarNN native library.
"""
__author__ = 'Barry Drake'


import ctypes as ct
from typing import Any, Iterable, Optional, Tuple, Iterator
import numpy as np
import sys as _sys
import os as _os
from platform import architecture as _arch
from operator import mul as _mul
from functools import reduce as _reduce
import pathlib as _pathlib
from astarnn.utils.no_instance import NoInstance


# Pointer functor
_Ptr = ct.POINTER

# The standard ctypes
_bool_t = ct.c_bool
_size_t = ct.c_size_t
_uint16_t = ct.c_uint16
_uint32_t = ct.c_uint32
_uint64_t = ct.c_uint64
_int32_t = ct.c_int32
_int64_t = ct.c_int64
_double_t = ct.c_double
_str_t = ct.c_char_p


# The ctypes type of pointers to a native objects.
_AStarNN = ct.c_void_p
_AStarIndex = ct.c_void_p

# The ctypes type of Error code.
_Error_t = ct.c_uint

# The ctypes type of vector elements.
_VElem_t = np.double
_Vector_t = np.ctypeslib.ndpointer(dtype=_VElem_t)

# The ctypes type of lattice point cvector elements.
_CElem_t = np.int32
_CVector_t = np.ctypeslib.ndpointer(dtype=_CElem_t)

# The ctypes type of hash code.
_HashCode_t = _uint64_t
_HashVector_t = np.ctypeslib.ndpointer(dtype=_HashCode_t)

# The ctypes type of the number of dimensions.
_Dim_t = _uint16_t

# The ctypes type of lattice point 'k' value.
_K_t = _uint64_t

# The ctypes type for specifying and reporting a number of extended shells.
_NumShells_t = _uint32_t

# The ctypes type for reporting a number of probes.
_NumProbes_t = _size_t

# The ctypes type for specifying and reporting any packing radius, rho, scale or distance.
_Distance_t = _double_t

# The ctypes type for an AStarNN query callback function.
_AStarNN_Callback_t = ct.CFUNCTYPE(_Error_t, _HashCode_t, _K_t, _Ptr(_int32_t))

# The ctypes type for an AStarIndex get callback function.
_AStarIndex_size_t_Callback_t = ct.CFUNCTYPE(_Error_t, _HashCode_t, _size_t)

# The type of array of size_t elements.
_size_t_vector_t = np.ctypeslib.ndpointer(dtype=_size_t)


class Config(NoInstance):
    """
    Configuration variables for the astarnn module.

    A user can change the Config class attributes, but they must be set before using the DLL,
    i.e., any other class or function in this file.

    Class attributes:
        dll_loader:     The ctypes DLL loader to use.
        dll_file:       A path name to the preferred DLL file (or None).
        debug:          A Boolean flag to indicate preferring a debug DLL (changes the DLL search path).
        lib_source:     A Boolean flag to indicate preferring a lib_source DLL (changes the DLL search path).
        dll_dir:        A Path to the expected location of the DLL files.
        lib_source_dir: A Path to the expected location of the source for DLL files.

    Default values of the Config attributes are determined when astarnn.py
    is loaded. The attribute values will be read the first time the DLL is accessed
    via some method.
    """

    # Set debug to True to preferably use the DLL compiled with debug options.
    # By default, debug is True if sys.gettrace returns a trace.
    # This can be overridden using the OS environment variable ASTARNN_DEBUG (set to 0 or 1).
    _gettrace = getattr(_sys, 'gettrace', None)
    debug_request = _os.environ.get('ASTARNN_DEBUG', default='')
    debug = (
        (debug_request == '1') or
        (debug_request != '0' and (_gettrace is not None and _gettrace() is not None))
    )

    # Set lib_source to True to preferably use the DLL found in the lib_source directory.
    # By default, prefer_lib_source is False.
    # This can be overridden using the OS environment variable ASTARNN_LIB_SOURCE (set to 0 or 1).
    lib_source = _os.environ.get('ASTARNN_LIB_SOURCE', default='0') == '1'

    # A Path to the expected location of the DLL files.
    dll_dir = _pathlib.Path(__file__).parent

    # A Path to the expected location of the source for DLL files.
    lib_source_dir = dll_dir.parent / 'lib_source'

    # If dll_file is not None, then this will be the preferred DLL file to use.
    dll_file = None

    # Infer the dll_loader from _sys.platform
    if _sys.platform == "win32" and _arch()[0] == '64bit':
        dll_loader = ct.WinDLL
    else:
        dll_loader = ct.CDLL


class AStarException(Exception):
    """
    An AStar exception.

    Attributes:
        ret     An instance of ReturnVal.
    """

    def __init__(self, ret):
        self.ret = ret

    def __int__(self):
        return int(self.ret)

    def __str__(self) -> str:
        return str(self.ret)

    def return_val_string(self) -> str:
        return self.ret.return_val_string()

    def return_val_code(self):
        return self.ret.return_val_code()


class ReturnVal(ct.Structure):
    """
    A AStarNN return value.

    Attributes:
        code     An unsigned integer which is a native return value code.
    """

    _fields_ = [('code', _Error_t)]

    def return_val_string(self) -> str:
        """String decode of the return value."""
        return _dll().AStar_error_string(self.code).decode()

    def return_val_code(self) -> int:
        """Integer decode of the return value."""
        return int(self.code)

    def ok(self) -> bool:
        """Return True iff the return value indicates no error."""
        return self.code == 0

    def check(self):
        """Will raise a AStarNNException if not ok."""
        if self.code != 0:
            raise AStarException(self)

    def __int__(self):
        return self.code

    def __str__(self) -> str:
        return f'ReturnVal({self.code}): {self.return_val_string()}'


_Error_invalid_dim = ReturnVal()
_Error_invalid_dim.code = 2

_loaded_dll = None


def _construct_dll_file_name(debug, lib_source):
    """
    Construct a path to a possible location for the DLL.
    """
    dll_dir = Config.dll_dir
    lib_source_dir = Config.lib_source_dir
    _platform = _sys.platform

    lib_base = 'AStarNN'

    if debug:
        _d = 'd'
        _dr = 'Debug'
    else:
        _d = ''
        _dr = 'Release'
    bits = '64' if _arch()[0] == '64bit' else '32'

    if _platform == 'win32':
        if lib_source:
            dll_dir = lib_source_dir / ('x' + bits) / _dr
        dll_file_name = lib_base + '_win' + bits + _d + '.dll'
    elif _platform == 'darwin':
        if lib_source:
            dll_dir = lib_source_dir / (_dr + bits)
            dll_file_name = lib_base + '_lib.so'
        else:
            dll_file_name = lib_base + '_mac64' + _d + '.so'
    elif _platform.startswith('linux'):
        if lib_source:
            dll_dir = lib_source_dir / (_dr + bits)
            dll_file_name = lib_base + '_lib.so'
        else:
            dll_file_name = lib_base + '_lin64' + _d + '.so'
    else:
        raise RuntimeError(f'cannot determine DLL file name for platform: {_platform}')

    return dll_dir / dll_file_name


def _register(name, *args, ret: Any = ReturnVal):
    """
    Register a single function prototype.
    Support for _dll().
    """
    global _loaded_dll
    f = getattr(_loaded_dll, name)
    f.restype = ret
    f.argtypes = args


def _register_all():
    """
    Register all function prototypes.
    Support for _dll().
    """
    _register('info_string', ret=_str_t)
    _register('extended_info_string', ret=_str_t)

    _register('AStar_error_string', _Error_t, ret=_str_t)
    _register('AStar_max_num_shells', ret=_NumShells_t)

    _register('AStar_rho', _Dim_t, _Ptr(_Distance_t))
    _register('AStar_to_lattice_space', _Dim_t, _Distance_t, _Vector_t, _Vector_t)
    _register('AStar_from_lattice_space', _Dim_t, _Distance_t, _Vector_t, _Vector_t)
    _register('AStar_cvector_k_to_lattice_point_in_lattice_space', _Dim_t, _CVector_t, _K_t, _Vector_t)
    _register('AStar_cvector_k_to_lattice_point', _Dim_t, _Distance_t, _CVector_t, _K_t, _Vector_t)
    _register('AStar_cvector_to_lattice_point_in_lattice_space', _Dim_t, _CVector_t, _Vector_t)
    _register('AStar_cvector_to_lattice_point', _Dim_t, _Distance_t, _CVector_t, _Vector_t)

    _register('AStarNN_new', _Dim_t, _Distance_t, _NumShells_t, _Ptr(_AStarNN))
    _register('AStarNN_delete', _AStarNN)
    _register('AStarNN_dim', _AStarNN, _Ptr(_Dim_t))
    _register('AStarNN_packing_radius', _AStarNN, _Ptr(_Distance_t))
    _register('AStarNN_scale', _AStarNN, _Ptr(_Distance_t))
    _register('AStarNN_num_shells', _AStarNN, _Ptr(_NumShells_t))
    _register('AStarNN_num_probes', _AStarNN, _Ptr(_NumProbes_t))
    _register('AStarNN_nearest_callback', _AStarNN, _Vector_t, _AStarNN_Callback_t)
    _register('AStarNN_delaunay_callback', _AStarNN, _Vector_t, _AStarNN_Callback_t)
    _register('AStarNN_extended_callback', _AStarNN, _Vector_t, _AStarNN_Callback_t)
    _register('AStarNN_nearest_hash', _AStarNN, _Vector_t, _HashVector_t)
    _register('AStarNN_delaunay_hash', _AStarNN, _Vector_t, _HashVector_t)
    _register('AStarNN_extended_hash', _AStarNN, _Vector_t, _HashVector_t)
    _register('AStarNN_nearest_cvector', _AStarNN, _Vector_t, _CVector_t)
    _register('AStarNN_delaunay_cvector', _AStarNN, _Vector_t, _CVector_t)
    _register('AStarNN_extended_cvector', _AStarNN, _Vector_t, _CVector_t)

    _register('AStarIndex_size_t_new', _Dim_t, _Distance_t, _NumShells_t, _Ptr(_AStarIndex))
    _register('AStarIndex_size_t_delete', _AStarIndex)
    _register('AStarIndex_size_t_dim', _AStarIndex, _Ptr(_Dim_t))
    _register('AStarIndex_size_t_packing_radius', _AStarIndex, _Ptr(_Distance_t))
    _register('AStarIndex_size_t_scale', _AStarIndex, _Ptr(_Distance_t))
    _register('AStarIndex_size_t_num_shells', _AStarIndex, _Ptr(_NumShells_t))
    _register('AStarIndex_size_t_num_probes', _AStarIndex, _Ptr(_NumProbes_t))
    _register('AStarIndex_size_t_num_hashes', _AStarIndex, _Ptr(_size_t))
    _register('AStarIndex_size_t_num_elements', _AStarIndex, _Ptr(_size_t))
    _register('AStarIndex_size_t_put', _AStarIndex, _Vector_t, _size_t)
    _register('AStarIndex_size_t_clear', _AStarIndex)
    _register('AStarIndex_size_t_clear_by_vector', _AStarIndex, _Vector_t)
    _register('AStarIndex_size_t_put_all', _AStarIndex, _Vector_t, _size_t, _Ptr(_size_t))
    _register('AStarIndex_size_t_count', _AStarIndex, _Vector_t, _Ptr(_size_t))
    _register('AStarIndex_size_t_get_callback', _AStarIndex, _Vector_t, _AStarNN_Callback_t)
    _register('AStarIndex_size_t_get_elems', _AStarIndex, _Vector_t, _size_t, _Ptr(_size_t), _size_t_vector_t)

    # methods for testing purposes only
    _register('TESTING_round_up', _double_t, ret=_CElem_t)


def _dll():
    """Internal: Safe way to access the AStarNN DLL."""
    global _loaded_dll
    if _loaded_dll is None:
        # Construct DLL search path based on Config settings
        dll_search_path = []
        if Config.dll_file is not None:
            dll_search_path.append(Config.dll_file)
        debug = Config.debug
        lib_source = Config.lib_source
        dll_search_path.append(_construct_dll_file_name(debug, lib_source))
        dll_search_path.append(_construct_dll_file_name(not debug, lib_source))
        dll_search_path.append(_construct_dll_file_name(debug, not lib_source))
        dll_search_path.append(_construct_dll_file_name(not debug, not lib_source))

        # Search for the DLL using the search path
        dll_file = None
        for check_file in dll_search_path:
            if _os.path.isfile(check_file):
                dll_file = check_file
                break
            print(f'DLL file missing: {check_file}', file=_sys.stderr)
        if dll_file is None:
            raise RuntimeError('No DLL file found')
        if Config.debug or dll_file != dll_search_path[0]:
            print(f'Using DLL file: {dll_file}', file=_sys.stderr)

        _loaded_dll = Config.dll_loader(str(dll_file))

        # ==========================================================
        # Register all DLL function prototypes
        # ==========================================================
        _register_all()

    return _loaded_dll


def info_string() -> str:
    """
    :return: a simple string (no newlines) to give information about this library.
    """
    return str(_dll().info_string().decode())


def extended_info_string() -> str:
    """
    :return: a multiline string giving extended information about this library.
    """
    return str(_dll().extended_info_string().decode())


def max_num_shells() -> int:
    """
    What is the maximum number of (extended) shells permitted by the library.
    """
    return int(_dll().AStar_max_num_shells())


def rho(dim: int) -> float:
    """
    The native packing radius of the A* lattice in the space it's
    represented in and with no scaling factor (scale = 1).
    """
    _rho = _Distance_t()
    ret = _dll().AStar_rho(dim, _rho)
    ret.check()
    return float(_rho.value)


def _round_up(x):
    """
    For testing purposes only.
    Used to test the library's internal rounding function.
    """
    return _dll().TESTING_round_up(x)


class AStarNN:
    """
    Functions for A* lattice hashing with multi-probe queries.

    AStarNN which provides three kinds of query:
       (1) the nearest lattice point to a query vector,
       (2) the vertex lattice points of a Delaunay cell containing a query vector,
       (3) the lattice points in the extended shells around a lattice hole nearest to a query vector.

    There are methods for querying using callbacks (which are not too efficient in Python) and
    methods for querying returning just the hash codes or c-vectors.

    This class also has a number of functions for converting between vector representations,
    determining a lattice point remainder value, and getting important values.
    """

    def __init__(self, dim: int, packing_radius: float, num_shells: int):
        """
        :param dim: dimensionality of vectors that we process.
            This is a positive integer.
        :param packing_radius: a scaling value which is the radius of the largest sphere
            fitting within a voronoi cell. This is a positive floating point number.
        :param num_shells: how many extended shells to use in extended queries.
            This is a non-negative integer.
        """
        self._native_AStarNN = _AStarNN()
        self._dim = dim
        self._callback_cache = None
        self._callback_id = None

        ret = _dll().AStarNN_new(dim, packing_radius, num_shells, self._native_AStarNN)
        ret.check()

        scale = _Distance_t()
        ret = _dll().AStarNN_scale(self._native_AStarNN, scale)
        ret.check()
        self._scale = float(scale.value)

    def __del__(self):
        ret = _dll().AStarNN_delete(self._native_AStarNN)
        self._native_AStarNN = None
        ret.check()

    @property
    def dim(self) -> int:
        """
        :return: the dimensionality of vectors that we process.
        """
        # dim = _Dim_t()
        # ret = _dll().AStarNN_dim(self._native_AStarNN, dim)
        # ret.check()
        # return dim.value
        return self._dim

    @property
    def packing_radius(self) -> float:
        """
        :return: the lattice scaling value which is the radius of the largest sphere fitting within a voronoi cell.
        """
        packing_radius = _Distance_t()
        ret = _dll().AStarNN_packing_radius(self._native_AStarNN, packing_radius)
        ret.check()
        return float(packing_radius.value)

    @property
    def scale(self) -> float:
        """
        :return: the lattice scaling value which is a multiple of rho(self.dim)
        """
        # scale = _Distance_t()
        # ret = _dll().AStarNN_scale(self._native_AStarNN, scale)
        # ret.check()
        # return scale.value
        return self._scale

    @property
    def num_shells(self) -> int:
        """
        :return: the number of extended shells used in an extended query.
        """
        num_shells = _NumShells_t()
        ret = _dll().AStarNN_num_shells(self._native_AStarNN, num_shells)
        ret.check()
        return int(num_shells.value)

    @property
    def num_probes(self) -> int:
        """
        :return: the number of hash codes generated in an extended query.
        """
        num_probes = _NumProbes_t()
        ret = _dll().AStarNN_num_probes(self._native_AStarNN, num_probes)
        ret.check()
        return int(num_probes.value)

    def to_lattice_space(self, v) -> np.ndarray:
        """
        Return the vector when v is mapped from the quantisation space into the lattice representation space.
        """
        dim = self._dim
        v_array = _make_array(_VElem_t, v, dim)
        scale = self._scale
        v_out = np.empty(dim + 1, dtype=_VElem_t)
        ret = _dll().AStar_to_lattice_space(dim, scale, v_array, v_out)
        ret.check()
        return v_out

    def from_lattice_space(self, v) -> np.ndarray:
        """
        Return the vector when v is mapped from the lattice representation space into the quantisation space.
        """
        dim = self._dim
        v_array = _make_array(_VElem_t, v, dim + 1)
        scale = self._scale
        v_out = np.empty(dim, dtype=_VElem_t)
        ret = _dll().AStar_from_lattice_space(dim, scale, v_array, v_out)
        ret.check()
        return v_out

    def cvector_k_to_lattice_point_in_lattice_space(self, c, k) -> np.ndarray:
        dim = self._dim
        dimp = dim + 1
        c_array = _make_array(_CElem_t, c, dimp)
        v_out = np.empty(dimp, dtype=_VElem_t)
        ret = _dll().AStar_cvector_k_to_lattice_point_in_lattice_space(dim, c_array, k, v_out)
        ret.check()
        return v_out

    def cvector_to_lattice_point_in_lattice_space(self, c) -> np.ndarray:
        dim = self._dim
        dimp = dim + 1
        c_array = _make_array(_CElem_t, c, dimp)
        v_out = np.empty(dimp, dtype=_VElem_t)
        ret = _dll().AStar_cvector_to_lattice_point_in_lattice_space(dim, c_array, v_out)
        ret.check()
        return v_out

    def cvector_k_to_lattice_point(self, c, k) -> np.ndarray:
        dim = self._dim
        dimp = dim + 1
        c_array = _make_array(_CElem_t, c, dimp)
        scale = self._scale
        v_out = np.empty(dim, dtype=_VElem_t)
        ret = _dll().AStar_cvector_k_to_lattice_point(dim, scale, c_array, k, v_out)
        ret.check()
        return v_out

    def cvector_to_lattice_point(self, c) -> np.ndarray:
        dim = self._dim
        dimp = dim + 1
        c_array = _make_array(_CElem_t, c, dimp)
        scale = self._scale
        v_out = np.empty(dim, dtype=_VElem_t)
        ret = _dll().AStar_cvector_to_lattice_point(dim, scale, c_array, v_out)
        ret.check()
        return v_out

    def nearest_hash(self, vector) -> _HashCode_t:
        """
        Return the hash code of the lattice point nearest to the given vector.
        The vector is taken to be in the quantisation space.
        """
        self._check_dim(vector)
        hashes = np.empty(1, dtype=_HashCode_t)
        ret = _dll().AStarNN_nearest_hash(self._native_AStarNN, vector, hashes)
        ret.check()
        return hashes.item()

    def delaunay_hash(self, vector) -> np.ndarray:
        """
        Return the hash codes of the lattice point that are the vertices of the Delaunay cell containing
        the given vector. These lattice points are the same as those of the zeroth shell around the hole
        nearest to the given vector.
        This will return self.dim + 1 hash codes.
        The vector is taken to be in the quantisation space.
        """
        self._check_dim(vector)
        dimp = self._dim + 1
        hashes = np.empty(dimp, dtype=_HashCode_t)
        ret = _dll().AStarNN_delaunay_hash(self._native_AStarNN, vector, hashes)
        ret.check()
        return hashes

    def extended_hash(self, vector) -> np.ndarray:
        """
        Return the hash codes of the lattice point that are shells around the hole nearest to
        the given vector.
        This will return self.num_probes hash codes.
        The vector is taken to be in the quantisation space.
        """
        self._check_dim(vector)
        hashes = np.empty(self.num_probes, dtype=_HashCode_t)
        ret = _dll().AStarNN_extended_hash(self._native_AStarNN, vector, hashes)
        ret.check()
        return hashes

    def nearest_cvector(self, vector) -> np.ndarray:
        """
        Return the c-vector of the lattice point nearest to the given vector.
        The c-vector has self.dim + 1 coordinates, all integers.
        The vector is taken to be in the quantisation space.
        """
        self._check_dim(vector)
        dimp = self._dim + 1
        cvector = np.empty(dimp, dtype=_CElem_t)
        ret = _dll().AStarNN_nearest_cvector(self._native_AStarNN, vector, cvector)
        ret.check()
        return cvector

    def delaunay_cvector(self, vector) -> np.ndarray:
        """
        Return the c-vectors of the lattice point that are the vertices of the Delaunay cell containing
        the given vector. These lattice points are the same as those of the zeroth shell around the hole
        nearest to the given vector.
        Each c-vector has self.dim + 1 coordinates, all integers.
        This will return self.dim + 1 c-vectors.
        The vector is taken to be in the quantisation space.
        """
        self._check_dim(vector)
        dimp = self._dim + 1
        cvectors = np.empty((dimp, dimp), dtype=_CElem_t)
        ret = _dll().AStarNN_delaunay_cvector(self._native_AStarNN, vector, cvectors)
        ret.check()
        return cvectors

    def extended_cvector(self, vector) -> np.ndarray:
        """
        Return the c-vectors of the lattice point that are shells around the hole nearest to
        the given vector.
        Each c-vector has self.dim + 1 coordinates, all integers.
        This will return self.num_probes c-vectors.
        The vector is taken to be in the quantisation space.
        """
        self._check_dim(vector)
        dimp = self._dim + 1
        cvectors = np.empty((self.num_probes, dimp), dtype=_CElem_t)
        ret = _dll().AStarNN_extended_cvector(self._native_AStarNN, vector, cvectors)
        ret.check()
        return cvectors

    def nearest_callback(self, vector, callback):
        """
        This is the callback version of self.nearest_hash(vector)
        
        The argument 'callback' should be a Python query callback function, e.g.,
            def callback(hash_code, k, c):
                print('callback', hash_code, k, c)
                return 0
        """
        self._check_dim(vector)
        cb = self._wrap_callback(callback)
        ret = _dll().AStarNN_nearest_callback(self._native_AStarNN, vector, cb)
        ret.check()

    def delaunay_callback(self, vector, callback):
        """
        This is the callback version of self.delaunay_hash(vector)
        
        The argument 'callback' should be a python query callback function, e.g.,
            def callback(hash_code, k, c):
                print('callback', hash_code, k, c)
                return 0
        """
        self._check_dim(vector)
        cb = self._wrap_callback(callback)
        ret = _dll().AStarNN_delaunay_callback(self._native_AStarNN, vector, cb)
        ret.check()

    def extended_callback(self, vector, callback):
        """
        This is the callback version of self.extended_hash(vector)

        The argument 'callback' should be a python query callback function, e.g.,
            def callback(hash_code, k, c):
                print('callback', hash_code, k, c)
                return 0
        """
        self._check_dim(vector)
        cb = self._wrap_callback(callback)
        ret = _dll().AStarNN_extended_callback(self._native_AStarNN, vector, cb)
        ret.check()

    def _check_dim(self, vector):
        """
        Raise an exception if the given vector is not of the appropriate dimensionality.
        """
        if len(vector) != self._dim:
            raise AStarException(_Error_invalid_dim)

    def _check_dimp(self, vector):
        """
        Raise an exception if the given vector is not of the appropriate dimensionality.
        """
        if len(vector) != self._dim + 1:
            raise AStarException(_Error_invalid_dim)

    def _wrap_callback(self, callback):
        """
        Wrap the given callback function so that it may be passed to the
        AStarNN native library.
        """
        if id(callback) is not self._callback_id:
            def _adaptor(hash_code, k, c):
                c = np.ctypeslib.as_array(c, shape=(self._dim + 1,))
                # noinspection PyBroadException
                try:
                    ret = callback(hash_code, k, c)
                    if ret is None:
                        return 0
                    else:
                        return ret
                except Exception:
                    return -1
            self._callback_id = id(callback)
            self._callback_cache = _AStarNN_Callback_t(_adaptor)

        return self._callback_cache


class AStarIndex:
    """
    An AStarIndex is a simple but efficient hash table using AStarNN.
    An AStarIndex can only store objects of type size_t, and does not store the inserted vectors.
    """

    def __init__(self, dim: int, packing_radius: float, num_shells: int):
        """
        :param dim: dimensionality of vectors that we process.
            This is a positive integer.
        :param packing_radius: a scaling value which is the radius of the largest sphere
            fitting within a voronoi cell. This is a positive floating point number.
        :param num_shells: how many extended shells to use in extended queries.
            This is a non-negative integer.
        """
        self._native_AStarIndex = _AStarIndex()
        self._dim = dim
        self._packing_radius = packing_radius

        ret = _dll().AStarIndex_size_t_new(dim, packing_radius, num_shells, self._native_AStarIndex)
        ret.check()

    def __del__(self):
        ret = _dll().AStarIndex_size_t_delete(self._native_AStarIndex)
        self._native_AStarNN = None
        ret.check()

    @property
    def dim(self) -> int:
        """
        :return: the dimensionality of vectors that we process.
        """
        return self._dim

    @property
    def packing_radius(self) -> float:
        """
        :return: the lattice scaling value which is the radius of the largest sphere fitting within a voronoi cell.
        """
        return self._packing_radius

    @property
    def num_shells(self) -> int:
        """
        :return: the number of extended shells used in an extended query.
        """
        num_shells = _NumShells_t()
        ret = _dll().AStarIndex_size_t_num_shells(self._native_AStarIndex, num_shells)
        ret.check()
        return int(num_shells.value)

    @property
    def num_probes(self) -> int:
        """
        :return: the number of hash codes generated in an extended query.
        """
        num_probes = _NumProbes_t()
        ret = _dll().AStarIndex_size_t_num_probes(self._native_AStarIndex, num_probes)
        ret.check()
        return int(num_probes.value)

    def num_hashes(self) -> int:
        """
        :return: number of hash codes used in the index.
        """
        value = _size_t()
        ret = _dll().AStarIndex_size_t_num_hashes(self._native_AStarIndex, value)
        ret.check()
        return int(value.value)

    def num_elements(self) -> int:
        """
        :return: number of elements in the index.
        """
        value = _size_t()
        ret = _dll().AStarIndex_size_t_num_elements(self._native_AStarIndex, value)
        ret.check()
        return int(value.value)

    def clear(self):
        """
        Remove all elements from the index.
        """
        ret = _dll().AStarIndex_size_t_clear(self._native_AStarIndex)
        ret.check()

    def clear_by_vector(self, query_vector):
        """
        Remove elements from the index with hash code equal to that of the given vector.
        :param query_vector: a vector of the right dimensionality
        """
        query_array = _make_array(_VElem_t, query_vector, self._dim)
        ret = _dll().AStarIndex_size_t_clear_by_vector(self._native_AStarIndex, query_array)
        ret.check()

    def insert(self, vector, value):
        """
        :param vector: a vector of the right dimensionality
        :param value: an integer (size_t)
        """
        array = _make_array(_VElem_t, vector, self._dim)
        ret = _dll().AStarIndex_size_t_put(self._native_AStarIndex, array, value)
        ret.check()

    def candidates(self, query_vector) -> np.ndarray:
        """
        :param query_vector: a vector of the right dimensionality
        :return: an array of integer (size_t)
        """
        query_array = _make_array(_VElem_t, query_vector, self._dim)
        size = self._num_candidates(query_array)
        elems = np.empty(size, dtype=_size_t)
        out_count = _size_t()
        ret = _dll().AStarIndex_size_t_get_elems(self._native_AStarIndex, query_array, size, out_count, elems)
        ret.check()
        return elems

    def num_candidates(self, query_vector) -> int:
        """
        :param query_vector: a vector of the right dimensionality
        :return: number of items to be retrieved by the key
        """
        query_array = _make_array(_VElem_t, query_vector, self._dim)
        return self._num_candidates(query_array)

    def _num_candidates(self, query_array) -> int:
        value = _size_t()
        ret = _dll().AStarIndex_size_t_count(self._native_AStarIndex, query_array, value)
        ret.check()
        return int(value.value)


class LSH:
    """
    An LSH is a more general form of AStartIndex.
    An LSH can store objects of any type and DOES store the inserted vectors.
    """

    def __init__(self, dim: int, packing_radius: float, num_shells: int):
        """
        :param dim: dimensionality of vectors that we process.
            This is a positive integer.
        :param packing_radius: a scaling value which is the radius of the largest sphere
            fitting within a voronoi cell. This is a positive floating point number.
        :param num_shells: how many extended shells to use in extended queries.
            This is a non-negative integer.
        """
        self._index = AStarIndex(dim, packing_radius, num_shells)
        self._vecs = []
        self._objs = []

    @property
    def dim(self) -> int:
        """
        :return: the dimensionality of vectors that we process.
        """
        return self._index.dim

    @property
    def packing_radius(self) -> float:
        """
        :return: the lattice scaling value which is the radius of the largest sphere fitting within a voronoi cell.
        """
        return self._index.packing_radius

    @property
    def num_shells(self) -> int:
        """
        :return: the number of extended shells used in an extended query.
        """
        return self._index.num_shells

    @property
    def num_probes(self) -> int:
        """
        :return: the number of hash codes generated in an extended query.
        """
        return self._index.num_probes

    def count(self, query_vector) -> int:
        """
        :param query_vector: a vector of the right dimensionality
        :return: number of items to be retrieved by the key
        """
        return self._index.num_candidates(query_vector)

    def num_hashes(self) -> int:
        """
        :return: number of hash codes used in the index.
        """
        return self._index.num_hashes()

    def num_elements(self) -> int:
        """
        :return: number of elements in the index.
        """
        return self._index.num_elements()

    def clear(self):
        """
        Remove all elements from the index.
        """
        self._index.clear()
        self._vecs = []
        self._objs = []

    def insert(self, vector, data: Any):
        """
        Insert the given data into the LSH object, using the given vector as the key.
        The vector and data can be retrieved using the candidates(...) or query(...) method
        """
        idx = len(self._vecs)
        self._vecs.append(vector)
        self._objs.append(data)
        self._index.insert(vector, idx)

    def num_inserts(self) -> int:
        """
        :return: the number of things inserted into this LSH object.
        """
        return len(self._objs)

    def candidates(self, query_vector) -> Iterator[Tuple]:
        """
        Return the candidates that are close to query vector.
        :returns: an iterator over (vector, data) tuples.
        """
        for idx in self._index.candidates(query_vector):
            yield self._vecs[idx], self._objs[idx]

    def num_candidates(self, query_vector) -> int:
        return self._index.num_candidates(query_vector)

    def query(self, query_vector) -> Tuple[float, np.ndarray, Any]:
        """
        Return the one nearest insertion that is closest to
        the nearest query vector. If nothing is found, then (inf, None, None) is returned.
        :returns: (distance, vector, data)
        """
        best = (float('inf'), None, None)

        for idx in self._index.candidates(query_vector):
            hash_vector = self._vecs[idx]
            dist = np.linalg.norm(query_vector - hash_vector)
            if dist < best[0]:
                best = (dist, hash_vector, self._objs[idx])
        return best


def _make_array(dtype, data, dim: Optional[int] = None) -> np.ndarray:
    """
    Support function from converting the given data (list) into
    a numpy array.

    This method will attempt to minimise copying data. No copy
    will be made if data is a numpy.ndarray of contiguous memory
    with the given dtype.

    :return: ctypes array with element type of dtype, shape
    """
    if isinstance(data, np.ndarray) and data.dtype == dtype:
        np_array = data
    else:
        np_array = np.array(data, dtype=dtype)

    np_array = np.ascontiguousarray(np_array)

    if dim is not None:
        if len(np_array.shape) != 1 or np_array.shape[0] != dim:
            raise AStarException(f'array is not {dim}d')

    return np_array


def _multiply(items: Iterable, initial=1):
    """
    Return the product of the given items.
    """
    return _reduce(_mul, items, initial)
