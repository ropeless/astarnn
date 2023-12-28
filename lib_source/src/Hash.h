/*
 * Functions for hashing.
 *
 * Author: Barry Drake
 */
#ifndef HASH_H
#define HASH_H

#include "common.h"




///
/// Hash function based on powers of RADIX.
/// This is intended to hash the c-vectors of lattice points.
///
struct Hash
{
    ///
    /// What powers will be used in the hash function.
    ///
    static const Hash_t RADIX = 31;

    ///
    /// Compute a hash of the given c-vector.
    ///
    inline static Hash_t hash(Dim_t dim, const CElem_t* to_hash)
    {
        Hash_t               hash_code = 0;
		Hash_t				 mul       = 1;
        const CElem_t* const end       = to_hash + dim;
        do
        {
            hash_code += (Hash_t)(*to_hash) * mul;
			mul *= RADIX;
        }
        while (++to_hash <= end);
        return hash_code;
    }


    ///
    /// Get the powers of RADIX in the standard
    /// order (identity permutation).
    ///
    inline static const Hash_t* powers(Dim_t dim)
    {
		return POW_RADIX._powers(dim);
    }


    ///
    /// Precompute ordered powers of RADIX.
	///
	/// \param dim				is the dimensionality of the lattice.
	/// \param order			is a dim + 1 permutation vector defining an ordering.
	/// \param ordered_powers	is a dim + 1 buffer to receive orderd RADIX powers.
    ///
    inline static void makeOrdered
    (
        Dim_t           dim,
        const Order_t*  order,
		Hash_t*         ordered_powers
		)
    {
        const Hash_t*  p_powers  = POW_RADIX._powers(dim);
        const Order_t* order_end = order + dim;
        do
        {
            *ordered_powers++ = *(p_powers + *order++);
        }
        while (order <= order_end);
    }


private:
    Hash(Dim_t initial_dim = 16);
	~Hash(void);

	const Hash_t* _powers(Dim_t dim);

    Hash_t*       m_x;
	Dim_t         m_dim;
    static Hash   POW_RADIX;
};



#endif // HASH_H
