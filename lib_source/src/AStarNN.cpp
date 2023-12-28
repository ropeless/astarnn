/*
 * Functions for A* lattice hashing with multi-probe queries.
 *
 * Author: Barry Drake
 */

#include "AStarNN.h"

#include "AStarLattice.h"
#include "AStarProbes.h"
#include "Hash.h"
#include "Deleter.h"
#include "WorkBuff.h"


///
/// The Switcher struct is templated on the Callback type.
/// This allows the compiler to optimise away unneeded code
/// for different callback kinds.
///
template<typename Callback> struct Switcher {static const int value;};
template<> const int Switcher<QueryCallback>::value         = 1;
template<> const int Switcher<QueryCallback_Hash>::value    = 2;
template<> const int Switcher<QueryCallback_CVector>::value = 3;
template<> const int Switcher<QueryCallback_Point>::value   = 4;

///
/// This macro gets the Switcher value for a given Callback type.
///
#define	TYPE(CallbackType) (Switcher<CallbackType>::value)

///
/// This macro is for use in the generic query methods.
/// It assumes 'Callback' type variable is in scope.
///
#define	IS(CallbackType)	(TYPE(CallbackType) == TYPE(Callback))

#define NEED_CVECTOR		(IS(QueryCallback) || IS(QueryCallback_CVector) || IS(QueryCallback_Point))
#define NEED_HASH			(IS(QueryCallback) || IS(QueryCallback_Hash))



///
/// This macro is for use in the generic query methods.
/// It assumes 'Callback' type variable and other variables are in scope.
/// A decent optimising compiler should optimize away the switch statement. 
///
#define MATCH(hash_code,k,c)			\
	switch (TYPE(Callback))				\
	{									\
	case TYPE(QueryCallback):			\
		reinterpret_cast<QueryCallback*>(callback)->match(hash_code, k, c);	\
		break;							\
	case TYPE(QueryCallback_Hash):		\
		reinterpret_cast<QueryCallback_Hash*>(callback)->match(hash_code);	\
		break;							\
	case TYPE(QueryCallback_CVector):	\
		reinterpret_cast<QueryCallback_CVector*>(callback)->match(k, c);	\
		break;							\
	case TYPE(QueryCallback_Point):		\
		AStarLattice::cvector_k_to_lattice_point_in_lattice_space(dim, c, k, lattice_point);\
		reinterpret_cast<QueryCallback_Point*>(callback)->match(lattice_point);				\
		break;							\
	default:							\
		throw Error_unknown;			\
	}



template<typename Callback>
static inline void _nearest_probe
(
	Dim_t			dim,
	Distance_t		scale,
	const VElem_t*	vector,
	Callback*		callback
)
{
	BuffStack		stack(dim, 6);
	WorkBuff*		buff = stack.buff();

	VElem_t*		lattice_point =
					IS(QueryCallback_Point) ?
					get_buff<VElem_t>(buff) :
					0;

    VElem_t*        mapped = get_buff<VElem_t>(buff);
    CElem_t*        c      = get_buff<CElem_t>(buff);
    K_t             k;

    //
    // Map the vector to the lattice representation space (including rescaling).
    //
    AStarLattice::to_lattice_space(dim, scale, vector, mapped);

	callback->init(dim, mapped);

    //
    // Find the closest lattice point (i.e. containing Voronoi cell).
    //
    AStarLattice::closest_point(dim, mapped, k, c, buff);

    Hash_t hash_code = 
		NEED_HASH ?
		Hash::hash(dim, c) :
		0;

	// Call callback->match(...) passing the nearest lattice point.
	MATCH(hash_code, k, c)
}



template<typename Callback>
static inline void _delaunay_probes
(
	Dim_t			dim,
	Distance_t		scale,
	const VElem_t*	vector,
	Callback*		callback
)
{
	BuffStack		stack(dim, 5);
	WorkBuff*		buff = stack.buff();

	VElem_t*		lattice_point =
					IS(QueryCallback_Point) ?
					get_buff<VElem_t>(buff) :
					0;

	VElem_t*        mapped = get_buff<VElem_t>(buff);
    CElem_t*        c      = get_buff<CElem_t>(buff);
    VElem_t*        xmod   = get_buff<VElem_t>(buff);
    Order_t*        order  = get_buff<Order_t>(buff);

    //
    // Map the vector to the lattice representation space (including rescaling).
    //
    AStarLattice::to_lattice_space(dim, scale, vector, mapped);

	callback->init(dim, mapped);

	//
    // Find the containing Delaunay cell.
	// The first probe, where all elements of the canonical probe are zero.
    //
	AStarLattice::setK0(dim, mapped, xmod, c, order, buff);

    Hash_t hash_code = 
		NEED_HASH ?
		Hash::hash(dim, c) :
		0;

	// Call callback->match(...) on the first lattice point.
	MATCH(hash_code, 0, c)

	//
	// Determing the other Delauny cell verticies.
	//
    for (K_t k = 1; k <= ((K_t)dim); k++)
    {
        c[order[k - 1]]--;

		if (NEED_HASH)
			hash_code = Hash::hash(dim, c);

		// Call callback->match(...) on the next lattice point.
		MATCH(hash_code, k, c)
    }
}



template<typename Callback>
static inline void _extended_probes
(
	Dim_t			dim,
	Distance_t		scale,
	const Order_t*	probe_diff_stream,
	const Order_t*	end,
	const VElem_t*	vector,
	Callback*		callback
)
{
	BuffStack		stack(dim, 7);
	WorkBuff*		buff = stack.buff();

	VElem_t*		lattice_point =
					IS(QueryCallback_Point) ?
					get_buff<VElem_t>(buff) :
					0;

	VElem_t*        mapped         = get_buff<VElem_t>(buff);
    CElem_t*        c              = get_buff<CElem_t>(buff);
    VElem_t*        xmod           = get_buff<VElem_t>(buff);
    Order_t*        order          = get_buff<Order_t>(buff);
	Hash_t*         ordered_powers = get_buff<Hash_t>(buff);

    //
    // Map the vector to the lattice representation space (including rescaling).
    //
    AStarLattice::to_lattice_space(dim, scale, vector, mapped);

	callback->init(dim, mapped);

	//
    // Find the containing Delaunay cell.
    //
	AStarLattice::setK0(dim, mapped, xmod, c, order, buff);

    //
    // Precompute the ordered array of powers of RADIX for fast indexing in hash.
    //
	if (NEED_HASH)
	{
		Hash::makeOrdered(dim, order, ordered_powers);
	}

    //
    // The first probe, where all elements of the canonical probe are zero.
    //
    Hash_t hash_code =
		NEED_HASH ?
		Hash::hash(dim, c) :
		0;

	// Call callback->match(...) for the first lattice point.
	MATCH(hash_code, 0, c)

    //
    // Loop over each of the remaining probes.
    //
    do
    {
        // Extract k from the start of the stream segment for this probe
        const K_t k = *probe_diff_stream++;
    
        // Apply the decrement adjustments per column specified in the stream.
        Order_t diffCol = *probe_diff_stream++;
        while (diffCol != AStarProbes::STREAM_MARK)
        {
			if (NEED_CVECTOR)
				c[order[diffCol]]--;

			if (NEED_HASH)
				hash_code -= ordered_powers[diffCol];

			diffCol = *probe_diff_stream++;
        }

        // Apply the increment adjustments per column specified in the stream.
        diffCol = *probe_diff_stream++;
        while (diffCol != AStarProbes::STREAM_MARK)
        {
			if (NEED_CVECTOR)
				c[order[diffCol]]++;

			if (NEED_HASH)
				hash_code += ordered_powers[diffCol];

            diffCol = *probe_diff_stream++;
        }

		// Call callback->match(...) for the next lattice point.
		MATCH(hash_code, k, c)
    }
    while (probe_diff_stream < end);
}




AStarNN::AStarNN(Dim_t dim, Distance_t packing_radius, NumShells_t num_shells)
    : m_dim(dim)
    , m_packing_radius(packing_radius)
    , m_num_shells(num_shells)
    , m_scale(AStarLattice::rho(dim) / packing_radius)
	, m_probe_diff_stream(0)
{ 
    if (dim <= 0)
    {
        throw Error_invalid_dim;
    }
    if (num_shells > AStarProbes::MAX_NUM_SHELLS)
    {
        throw Error_invalid_num_shells;
    }
    if (packing_radius <= 0.0)
    {
        throw Error_invalid_packing_radius;
    }

    m_num_probes = AStarProbes::num_probes(m_dim, m_num_shells);

    const size_t probes_size = m_num_probes * (m_dim + 1);

    CElem_t* probes = new CElem_t[probes_size];
    Deleter<CElem_t[]> delete_probes(probes);

    AStarProbes::generate_probes(m_dim, m_num_shells, probes);

    size_t size_diff_stream = AStarProbes::size_probe_stream(m_dim, m_num_probes, probes);
    m_probe_diff_stream = new Order_t[size_diff_stream];
    m_probe_diff_stream_end = AStarProbes::generate_probe_diffs(m_dim, m_num_probes, probes, m_probe_diff_stream);

    // consistency check
    if (m_probe_diff_stream_end != m_probe_diff_stream + size_diff_stream)
    {
        throw Error_unknown;
    }
}


AStarNN::~AStarNN(void)
{
    delete [] m_probe_diff_stream;
}


Hash_t AStarNN::nearest_hash(const VElem_t* vector) const
{
	Hash_t		hash_code;
	KeepHashes	query_callback(1, &hash_code);
	nearest_probe(vector, &query_callback);
	return hash_code;
}


void AStarNN::_nearest_probe(const VElem_t* vector, QueryCallback* callback) const
{
    ::_nearest_probe(m_dim, m_scale, vector, callback);
}

void AStarNN::_nearest_probe(const VElem_t* vector, QueryCallback_Hash* callback) const
{
    ::_nearest_probe(m_dim, m_scale, vector, callback);
}

void AStarNN::_nearest_probe(const VElem_t* vector, QueryCallback_CVector* callback) const
{
    ::_nearest_probe(m_dim, m_scale, vector, callback);
}

void AStarNN::_nearest_probe(const VElem_t* vector, QueryCallback_Point* callback) const
{
    ::_nearest_probe(m_dim, m_scale, vector, callback);
}


void AStarNN::_delaunay_probes(const VElem_t* vector, QueryCallback* callback) const
{
    ::_delaunay_probes(m_dim, m_scale, vector, callback);
}

void AStarNN::_delaunay_probes(const VElem_t* vector, QueryCallback_Hash* callback) const
{
    ::_delaunay_probes(m_dim, m_scale, vector, callback);
}

void AStarNN::_delaunay_probes(const VElem_t* vector, QueryCallback_CVector* callback) const
{
    ::_delaunay_probes(m_dim, m_scale, vector, callback);
}

void AStarNN::_delaunay_probes(const VElem_t* vector, QueryCallback_Point* callback) const
{
    ::_delaunay_probes(m_dim, m_scale, vector, callback);
}



void AStarNN::_extended_probes(const VElem_t* vector, QueryCallback* callback) const
{
    ::_extended_probes(m_dim, m_scale, m_probe_diff_stream, m_probe_diff_stream_end, vector, callback);
}

void AStarNN::_extended_probes(const VElem_t* vector, QueryCallback_Hash* callback) const
{
    ::_extended_probes(m_dim, m_scale, m_probe_diff_stream, m_probe_diff_stream_end, vector, callback);
}

void AStarNN::_extended_probes(const VElem_t* vector, QueryCallback_CVector* callback) const
{
    ::_extended_probes(m_dim, m_scale, m_probe_diff_stream, m_probe_diff_stream_end, vector, callback);
}

void AStarNN::_extended_probes(const VElem_t* vector, QueryCallback_Point* callback) const
{
    ::_extended_probes(m_dim, m_scale, m_probe_diff_stream, m_probe_diff_stream_end, vector, callback);
}

