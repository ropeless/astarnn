/*
 * A common header for all functional source files.
 *
 * Author: Barry Drake
 */

#ifndef COMMON__H
#define COMMON__H


// ===========================================================================
// Manage different compilers
// ===========================================================================

#pragma warning(disable : 26812) // disabling Visual Studio warning about using traditional enums.


#ifdef VS2008
//
// Some integer types from stdint.h, which VS2008 can't find.
//
typedef unsigned __int8  uint8_t;
typedef unsigned __int16 uint16_t;
typedef unsigned __int32 uint32_t;
typedef unsigned __int64 uint64_t;
typedef __int8           int8_t;
typedef __int16          int16_t;
typedef __int32          int32_t;
typedef __int64          int64_t;

#else // ndef VS2008
#include <stdint.h>
#endif // VS2008

#include <stddef.h>

/*
// for simple DEBUG

#include <iostream>

template<typename T>
inline void DEBUG_ARRAY(const char* msg, int size, T* vec)
{
	std::cout << "DEBUG " << msg << ":";
	for (int i = 0; i < size; i++)
		std::cout << " " << vec[i];
	std::cout << std::endl;
}
*/



// ===========================================================================
// Constants and Types
// ===========================================================================



/// The type used to represent dimensionality.
typedef uint32_t Dim_t;


/// Order_t is the type for representing indexes into dimensions, for
/// example, the elements of a permutation of dimensions. It is required
/// to represent values from 0 to dim + 1 inclusive. The range is needed
/// so that dimensions 0 to dim (inclusive) can be indexed, as well as
/// having a sentinel value of all bits on, i.e. (Order_t)(-1).
///
/// It is given a minimal size as an optimisation to improve speed and
/// memory usage.
///
typedef uint16_t Order_t;


/// The type of elements for general vectors.
typedef double  VElem_t;

/// The type of elements used for the c-vector representation of lattice points.
typedef int32_t CElem_t;

/// The type used for the remainder value, k, of a lattice point.
typedef int32_t K_t;

/// The type used for the number of extended lattice shells.
typedef uint32_t NumShells_t;

/// The type used for any packing radius, rho, scale or distance.
typedef double Distance_t;

///
/// Hash_t is the type used for hash codes of lattice points.
///
/// This type must be an unsigned integer type as hash codes are calculated
/// modulo 2^b where b is the number of bit of the type. The C specification
/// guarantees modulo arithmetic for unsigned integer types (but not for
/// signed types).
///
typedef uint64_t Hash_t;


/// Enum of error codes.
/// Error code 0 (Error_ok) indicates no error.
/// Error_unknown represents the highest error code.
enum Error
{
    Error_ok = 0,
    Error_mem_fail,
    Error_invalid_dim,
    Error_invalid_num_shells,
    Error_invalid_packing_radius,
	Error_in_callback,
	Error_insufficient_buffers,
    Error_unknown
};


/// Convert a error code into a human readable string.
inline const char* error_to_string(Error err)
{
    switch(err)
    {
        case Error_ok: return "Error_ok";
        case Error_mem_fail: return "Error_mem_fail";
        case Error_invalid_dim: return "Error_invalid_dim";
        case Error_invalid_num_shells: return "Error_invalid_num_shells";
        case Error_invalid_packing_radius: return "Error_invalid_packing_radius";
        case Error_in_callback: return "Error_in_callback";
		case Error_insufficient_buffers: return "Error_insufficient_buffers";
        case Error_unknown: return "Error_unknown";
        default: return "<unknown error code>";
    }
}


/// Hook for assertions
inline void ASSERT(bool expression)
{
    if (!expression)
    {
        throw Error_unknown;
    }
}


/// Round a double, x, to a T such that round_up(x) = (T)floor(x + 0.5).
/// Assumes T is an integer type.
///
template<typename T>
inline T round_up(double x)
{
	// No calls to stdlib or branches
	//
	// Here is what the Microsoft compiler produced...
	//
	// 00  addsd       xmm0,mmword ptr [__real@3fe0000000000000 (07FFBCD0FB278h)]  
	// 08  xor         ecx,ecx  
	// 0A  cvttsd2si   eax,xmm0  
	// 0E  movd        xmm1,eax  
	// 12  cvtdq2pd    xmm1,xmm1  
	// 16  comisd      xmm1,xmm0  
	// 1A  seta        cl  
	// 1D  sub         eax,ecx
	//
	x += 0.5;
	T i = T(x);
	i -= (x < double(i));
	return i;
}





#endif // COMMON__H
