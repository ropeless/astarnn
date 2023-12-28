/*
 * Functions for A* lattice hashing with multi-probe queries.
 *
 * The key class is AStarNN which provides query methods for
 *    (1) the nearest lattice point to a query vector,
 *    (2) the vertex lattice points of a Delaunay cell containing a query vector,
 *    (3) the lattice points in the extended shells around a lattice hole nearest to a query vector.
 *
 * Each query type can be given a query callback object. Different types of query callback
 * are available. Each type of query callback provides different kinds of information
 * on the matching lattice points.
 *
 * Author: Barry Drake
 */

#ifndef ASTARNN__H
#define ASTARNN__H

#include "common.h"
#include "version.h"


///
/// This is an interface to be called by query methods.
/// For each match, the callback is given the lattice point
/// hash code, c-vector and remainder value (k).
///
class QueryCallback
{
public:

	///
	/// This method is called once at the start of a query.
	/// The 'mapped' parameter will be kept valid only for the duration of the query.
	///
	/// \param dim		is the dimensionality of the query vector.
	/// \param mapped	is the dim + 1 dimensional vector that is the query
	///					vector mapped into the lattice representation space.
	///
	virtual void init(Dim_t dim, const VElem_t* mapped) = 0;

    ///
    /// This method is called once for each matching hash code during the query.
    ///
    /// \param hash_code    hash code of a matching lattice point.
    /// \param k            the remainder value of the lattice point.
    /// \param c            the (dims+1) c-vector representing the matching lattice point.
    ///
    virtual void match(Hash_t hash_code, K_t k, const CElem_t* c) = 0;
};


///
/// This is an interface to be called by query methods, when
/// only hash codes are needed.
///
class QueryCallback_Hash
{
public:

	///
	/// This method is called once at the start of a query.
	/// The 'mapped' parameter will be kept valid only for the duration of the query.
	///
	/// \param dim		is the dimensionality of the query vector.
	/// \param mapped	is the dim + 1 dimensional vector that is the query
	///					vector mapped into the lattice representation space.
	///
	virtual void init(Dim_t dim, const VElem_t* mapped) = 0;

	///
    /// This method is called once for each matching hash code during the query.
    ///
    /// \param hash_code    hash code of a matching lattice point.
    ///
    virtual void match(Hash_t hash_code) = 0;
};


///
/// This is an interface to be called by query methods, when
/// only c-vectors (and remainder values) are needed.
///
class QueryCallback_CVector
{
public:

	///
	/// This method is called once at the start of a query.
	/// The 'mapped' parameter will be kept valid only for the duration of the query.
	///
	/// \param dim		is the dimensionality of the query vector.
	/// \param mapped	is the dim + 1 dimensional vector that is the query
	///					vector mapped into the lattice representation space.
	///
	virtual void init(Dim_t dim, const VElem_t* mapped) = 0;

    ///
    /// This method is called once for each matching hash code during the query.
    ///
    /// \param k            the remainder value of the lattice point.
    /// \param c            the (dims+1) c-vector representing the matching lattice point.
    ///
    virtual void match(K_t k, const CElem_t* c) = 0;
};


///
/// This is an interface to be called by query methods, when
/// the lattice point coordinates are needed.
///
class QueryCallback_Point
{
public:

	///
	/// This method is called once at the start of a query.
	/// The 'mapped' parameter will be kept valid only for the duration of the query.
	///
	/// \param dim		is the dimensionality of the query vector.
	/// \param mapped	is the dim + 1 dimensional vector that is the query
	///					vector mapped into the lattice representation space.
	///
	virtual void init(Dim_t dim, const VElem_t* mapped) = 0;

	///
    /// This method is called once for each matching hash code during the query.
    ///
    /// \param lattice_point	is the dim + 1 dimensional vector of lattice point
	///							coordinates in the lattice representation space.
    ///
    virtual void match(const VElem_t* lattice_point) = 0;
};



/// A standard query callback - keep a store of the hash codes.
///
class KeepHashes : public QueryCallback_Hash
{
public:
	/// Create a callback where the matching hash codes are
	/// stored in the array, hashes.
	/// Assumes that 'match' will be called no more than max_size times.
    KeepHashes(size_t max_size, Hash_t* hashes)
		: m_start(hashes)
		, m_cur(hashes)
		, m_end(hashes + max_size)
    {}

	virtual void init(Dim_t dim, const VElem_t* mapped)
	{}

	virtual void match(Hash_t hash_code)
    {
		ASSERT(m_cur < m_end);
		*m_cur++ = hash_code;
    }

	inline size_t size(void) const
	{
		return m_cur - m_start;
	}

private:
    const Hash_t*	m_start;
    Hash_t*			m_cur;
    const Hash_t*	m_end;
};


/// A standard query callback - keep a store of the c-vectors.
///
class KeepCVectors : public QueryCallback_CVector
{
public:
	/// Create a callback where the matching c-vectors are
	/// stored in the array, cvectors.
	/// Assumes that 'match' will be called no more than max_size times.
    KeepCVectors(size_t max_size, Dim_t dimp, CElem_t* cvectors)
		: m_dimp(dimp)
		, m_start(cvectors)
		, m_cur(cvectors)
		, m_end(cvectors + (max_size * m_dimp))
    {}

	virtual void init(Dim_t dim, const VElem_t* mapped)
	{
		ASSERT(m_dimp == size_t(dim) + 1);
	}

	virtual void match(K_t k, const CElem_t* c)
    {
		ASSERT(m_cur < m_end);
		const CElem_t* end = m_cur + m_dimp;
		while (m_cur < end)
		{
			*m_cur++ = *c++;
		}
    }

	inline size_t size(void) const
	{
		return (m_cur - m_start) / m_dimp;
	}

private:
	const size_t	m_dimp;
    const CElem_t*	m_start;
    CElem_t*		m_cur;
    const CElem_t*	m_end;
};


/// A standard query callback - keep a store of the hash codes and c-vectors.
///
class KeepProbes : public QueryCallback
{
public:
	/// Create a callback where the matching hash codes are
	/// stored in the array, hashes, and the matching c-vectors
	/// are stored in the array, cvectors.
	/// Assumes that 'match' will be called no more than max_size times.
    KeepProbes(size_t max_size, Dim_t dimp, Hash_t* hashes, CElem_t* cvectors)
        : m_dimp(dimp)
		, m_start(hashes)
		, m_cur(hashes)
		, m_end(hashes + max_size)
		, m_cur_cvector(cvectors)
    {}

	virtual void init(Dim_t dim, const VElem_t* mapped)
	{
		ASSERT(m_dimp == size_t(dim) + 1);
	}

	virtual void match(Hash_t hash_code, K_t k, const CElem_t* c)
    {
		ASSERT(m_cur < m_end);
		*m_cur++ = hash_code;
		const CElem_t* end = m_cur_cvector + m_dimp;
		while (m_cur_cvector < end)
		{
			*m_cur_cvector++ = *c++;
		}
    }

	inline size_t size(void) const
	{
		return m_cur - m_start;
	}

private:
	const size_t	m_dimp;
    CElem_t*		m_cur_cvector;
    const Hash_t*	m_start;
    Hash_t*			m_cur;
    const Hash_t*	m_end;
};





class AStarNN
{
public:
	/// Create an AStarNN hash code generator.
	///
    /// \param[in]  dim				number of dimensions in the lattice quantisation space, n.
    /// \param[in]  packing_radius	packing radius of the A* lattice.
    /// \param[in]  num_shells		number of extended shells for extended probes.
	///
    AStarNN(Dim_t dim, Distance_t packing_radius, NumShells_t num_shells);
    ~AStarNN(void);


	/// Get the hash code of the lattice point nearest to the given vector.
	Hash_t nearest_hash(const VElem_t* vector) const;


	/// Call the given callback exactly once for the lattice point that is
	/// nearest to the given vector.
	///
	/// Callback can be one of either: QueryCallback, QueryCallback_Hash,
	/// QueryCallback_CVector, QueryCallback_Point.
	///
	template<typename Callback>
	inline void nearest_probe(const VElem_t* vector, Callback* callback) const
	{
		_nearest_probe(vector, callback);
	}


	/// Call the given callback for each of the lattice point that are the
	/// vertices of the Delaunay cell containing the given vector.
	/// The callback will be called exactly n+1 times, where n is the
	/// dimensionality of quantisation lattice.
	///
	/// Callback can be one of either: QueryCallback, QueryCallback_Hash,
	/// QueryCallback_CVector, QueryCallback_Point.
	///
	template<typename Callback>
	inline void delaunay_probes(const VElem_t* vector, Callback* callback) const
	{
		_delaunay_probes(vector, callback);
	}


	/// Call the given callback for each of the lattice point that form
	/// shells around the hole nearest to the given vector.
	/// The callback will be called exactly 'num_probes' times.
	///
	/// Callback can be one of either: QueryCallback, QueryCallback_Hash,
	/// QueryCallback_CVector, QueryCallback_Point.
	///
	template<typename Callback>
	inline void extended_probes(const VElem_t* vector, Callback* callback) const
	{
		_extended_probes(vector, callback);
	}


	/// Get the dimensionality of quantisation lattice.
    inline Dim_t dim(void) const
    {
        return m_dim;
    }

	/// Get the packing radius of the quantisation lattice.
    inline Distance_t packing_radius(void) const
    {
        return m_packing_radius;
    }

	/// Get the packing internal scaling factor between the packing
	/// radius of the quantisation lattice and the native
	/// packing radius.
    inline Distance_t scale(void) const
    {
        return m_scale;
    }

	/// Number of shells of lattice point beyond the Delaunay cell,
	/// that are used by 'extended_probes' queries.
    inline int num_shells(void) const
    {
        return m_num_shells;
    }

	/// Number of probe points used by 'extended_probes' queries.
    inline size_t num_probes(void) const
    {
        return m_num_probes;
    }

private:
    const Dim_t         m_dim;
    const NumShells_t   m_num_shells;
    const Distance_t    m_packing_radius;
    const Distance_t    m_scale;
    size_t              m_num_probes;
    Order_t*            m_probe_diff_stream;
    Order_t*            m_probe_diff_stream_end;


	// Concrete implementation for template methods delegations.

	void _nearest_probe(const VElem_t* vector, QueryCallback* callback) const;
    void _nearest_probe(const VElem_t* vector, QueryCallback_Hash* callback) const;
    void _nearest_probe(const VElem_t* vector, QueryCallback_CVector* callback) const;
    void _nearest_probe(const VElem_t* vector, QueryCallback_Point* callback) const;

	void _delaunay_probes(const VElem_t* vector, QueryCallback* callback) const;
    void _delaunay_probes(const VElem_t* vector, QueryCallback_Hash* callback) const;
    void _delaunay_probes(const VElem_t* vector, QueryCallback_CVector* callback) const;
    void _delaunay_probes(const VElem_t* vector, QueryCallback_Point* callback) const;

    void _extended_probes(const VElem_t* vector, QueryCallback* callback) const;
    void _extended_probes(const VElem_t* vector, QueryCallback_Hash* callback) const;
    void _extended_probes(const VElem_t* vector, QueryCallback_CVector* callback) const;
    void _extended_probes(const VElem_t* vector, QueryCallback_Point* callback) const;
};





#endif // ASTARNN__H

