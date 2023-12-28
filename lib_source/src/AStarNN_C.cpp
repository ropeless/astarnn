/*
 * This is a C interface for the DLL.
 *
 * Author: Barry Drake
 */

#include "AStarNN_C.h"
#include "AStarNN.h"
#include "AStarLattice.h"
#include "AStarProbes.h"
#include "AStarIndex.h"
#include "Deleter.h"
#include <new>

class AStarIndex_size_t : public AStarIndex<size_t>
{
public:
	AStarIndex_size_t(Dim_t dim, Distance_t packing_radius, NumShells_t num_shells)
		: AStarIndex<size_t>(dim, packing_radius, num_shells)
	{}
};


/// This macro catches C++ errors and converts them
/// into a returned enum Error code.
#define RETURN_ERROR(try_code)                              \
    try {{try_code} return Error_ok;}                       \
    catch(Error e) {return e;}                              \
    catch(const std::bad_alloc&) {return Error_mem_fail;}   \
    catch(...) {return Error_unknown;}



class AStarNN_CallUserFunction : public QueryCallback
{
public:
    AStarNN_CallUserFunction(AStarNN_Callback_t callback_function)
        : m_callback_function(callback_function)
    {}

	virtual void init(Dim_t dim, const VElem_t* mapped)
	{
		// ignored.
	}

	virtual void match(Hash_t hash_code, K_t k, const CElem_t* c)
    {
        Error err = m_callback_function(hash_code, k, c);
        if (err)
        {
			if (err > 0 && err <= Error_unknown)
			{
				throw err;
			}
			else
			{
				throw Error_in_callback;
			}
        }
    }

private:
    const AStarNN_Callback_t m_callback_function;
};



class AStarIndex_size_t_CallUserFunction : public IndexCallback<size_t>
{
public:
	AStarIndex_size_t_CallUserFunction(AStarIndex_size_t_Callback_t callback_function)
        : m_callback_function(callback_function)
    {}


	virtual void match(Hash_t hash_code, const size_t& elem)
	{
        Error err = m_callback_function(hash_code, elem);
        if (err)
        {
			if (err > 0 && err <= Error_unknown)
			{
				throw err;
			}
			else
			{
				throw Error_in_callback;
			}
        }
	}
private:
	const AStarIndex_size_t_Callback_t  m_callback_function;
};


const char* info_string()
{
     return Version::info();
}


const char* extended_info_string()
{
     return Version::extended_info();
}


const char* AStar_error_string(Error err)
{
    return error_to_string(err);
}


NumShells_t AStar_max_num_shells(void)
{
    return AStarProbes::MAX_NUM_SHELLS;
}


Error AStar_rho(Dim_t dim, Distance_t* out_rho)
{
    RETURN_ERROR({
        *out_rho = AStarLattice::rho(dim);
    })
}

Error AStar_to_lattice_space(Dim_t dim, Distance_t scale, const VElem_t* in_v, VElem_t* out_v)
{
    RETURN_ERROR({
         AStarLattice::to_lattice_space(dim, scale, in_v, out_v);
    })
}

Error AStar_from_lattice_space(Dim_t dim, Distance_t scale, const VElem_t* in_v, VElem_t* out_v)
{
    RETURN_ERROR({
         AStarLattice::from_lattice_space(dim, scale, in_v, out_v);
    })
}

Error AStar_cvector_k_to_lattice_point_in_lattice_space(Dim_t dim, const CElem_t* c, K_t k, VElem_t* out_v)
{
    RETURN_ERROR({
         AStarLattice::cvector_k_to_lattice_point_in_lattice_space(dim, c, k, out_v);
    })
}


Error AStar_cvector_k_to_lattice_point(Dim_t dim, Distance_t scale, const CElem_t* c, K_t k, VElem_t* out_v)
{
    RETURN_ERROR({
		VElem_t* tmp = new VElem_t[dim + 1];
		Deleter<VElem_t[]>	delete_tmp(tmp);

		AStarLattice::cvector_k_to_lattice_point_in_lattice_space(dim, c, k, tmp);
	    AStarLattice::from_lattice_space(dim, scale, tmp, out_v);
    })
}


Error AStar_cvector_to_lattice_point_in_lattice_space(Dim_t dim, const CElem_t* c, VElem_t* out_v)
{
    RETURN_ERROR({
         AStarLattice::cvector_to_lattice_point_in_lattice_space(dim, c, out_v);
    })
}


Error AStar_cvector_to_lattice_point(Dim_t dim, Distance_t scale, const CElem_t* c, VElem_t* out_v)
{
    RETURN_ERROR({
		VElem_t* tmp = new VElem_t[dim + 1];
		Deleter<VElem_t[]>	delete_tmp(tmp);

		AStarLattice::cvector_to_lattice_point_in_lattice_space(dim, c, tmp);
	    AStarLattice::from_lattice_space(dim, scale, tmp, out_v);
    })
}


Error AStarNN_new(Dim_t dim, Distance_t packing_radius, NumShells_t num_shells, AStarNN** out_AStarNN)
{
    RETURN_ERROR({
        *out_AStarNN = 0;
        *out_AStarNN = new AStarNN(dim, packing_radius, num_shells);
    })
}


Error AStarNN_delete(AStarNN* self)
{
    RETURN_ERROR({
        delete self;
    })
}


Error AStarNN_nearest_hash(const AStarNN* self, const VElem_t* vector, Hash_t* hashes)
{
    RETURN_ERROR({
        KeepHashes collect(1, hashes);
        self->nearest_probe(vector, &collect);
    })
}

Error AStarNN_delaunay_hash(const AStarNN* self, const VElem_t* vector, Hash_t* hashes)
{
    RETURN_ERROR({
        KeepHashes collect(self->dim() + 1, hashes);
        self->delaunay_probes(vector, &collect);
    })
}

Error AStarNN_extended_hash(const AStarNN* self, const VElem_t* vector, Hash_t* hashes)
{
    RETURN_ERROR({
        KeepHashes collect(self->num_probes(), hashes);
        self->extended_probes(vector, &collect);
    })
}


Error AStarNN_nearest_cvector(const AStarNN* self, const VElem_t* vector, CElem_t* cvectors)
{
    RETURN_ERROR({
		Dim_t dimp = self->dim() + 1;
        KeepCVectors collect(1, dimp, cvectors);
        self->nearest_probe(vector, &collect);
    })
}

Error AStarNN_delaunay_cvector(const AStarNN* self, const VElem_t* vector, CElem_t* cvectors)
{
    RETURN_ERROR({
		Dim_t dimp = self->dim() + 1;
        KeepCVectors collect(dimp, dimp, cvectors);
        self->delaunay_probes(vector, &collect);
    })
}

Error AStarNN_extended_cvector(const AStarNN* self, const VElem_t* vector, CElem_t* cvectors)
{
    RETURN_ERROR({
		Dim_t dimp = self->dim() + 1;
        KeepCVectors collect(self->num_probes(), dimp, cvectors);
        self->extended_probes(vector, &collect);
    })
}


Error AStarNN_nearest_probe(const AStarNN* self, const VElem_t* vector, Hash_t* hashes, CElem_t* cvectors)
{
    RETURN_ERROR({
		Dim_t dimp = self->dim() + 1;
        KeepProbes collect(1, dimp, hashes, cvectors);
        self->nearest_probe(vector, &collect);
    })
}

Error AStarNN_delaunay_probe(const AStarNN* self, const VElem_t* vector, Hash_t* hashes, CElem_t* cvectors)
{
    RETURN_ERROR({
		Dim_t dimp = self->dim() + 1;
        KeepProbes collect(dimp, dimp, hashes, cvectors);
        self->delaunay_probes(vector, &collect);
    })
}

Error AStarNN_extended_probe(const AStarNN* self, const VElem_t* vector, Hash_t* hashes, CElem_t* cvectors)
{
    RETURN_ERROR({
		Dim_t dimp = self->dim() + 1;
        KeepProbes collect(self->num_probes(), dimp, hashes, cvectors);
        self->extended_probes(vector, &collect);
    })
}


	
Error AStarNN_nearest_callback(const AStarNN* self, const VElem_t* vector, AStarNN_Callback_t callback)
{
    RETURN_ERROR({
        AStarNN_CallUserFunction callback_object(callback);
        self->nearest_probe(vector, &callback_object);
    })
}


Error AStarNN_delaunay_callback(const AStarNN* self, const VElem_t* vector, AStarNN_Callback_t callback)
{
    RETURN_ERROR({
        AStarNN_CallUserFunction callback_object(callback);
        self->delaunay_probes(vector, &callback_object);
    })
}


Error AStarNN_extended_callback(const AStarNN* self, const VElem_t* vector, AStarNN_Callback_t callback)
{
    RETURN_ERROR({
        AStarNN_CallUserFunction callback_object(callback);
        self->extended_probes(vector, &callback_object);
    })
}


Error AStarNN_dim(const AStarNN* self, Dim_t* out_dim)
{
    RETURN_ERROR({
        *out_dim = self->dim();
    })
}


Error AStarNN_packing_radius(const AStarNN* self, Distance_t* out_packing_radius)
{
    RETURN_ERROR({
        *out_packing_radius = self->packing_radius();
    })
}


Error AStarNN_scale(const AStarNN* self, Distance_t* out_scale)
{
    RETURN_ERROR({
        *out_scale = self->scale();
    })
}


Error AStarNN_num_shells(const AStarNN* self, NumShells_t* out_num_shells)
{
    RETURN_ERROR({
        *out_num_shells = self->num_shells();
    })
}


Error AStarNN_num_probes(const AStarNN* self, size_t* out_num_probes)
{
    RETURN_ERROR({
        *out_num_probes = self->num_probes();
    })
}


Error AStarIndex_size_t_new(Dim_t dim, Distance_t packing_radius, NumShells_t num_shells, AStarIndex_size_t** out_AStarIndex)
{
	RETURN_ERROR({
        *out_AStarIndex = 0;
        *out_AStarIndex = new AStarIndex_size_t(dim, packing_radius, num_shells);
    })
}

Error AStarIndex_size_t_delete(AStarIndex_size_t* self)
{
	RETURN_ERROR({
		delete self;
	})
}


Error AStarIndex_size_t_dim(const AStarIndex_size_t* self, Dim_t* out_dim)
{
	RETURN_ERROR({
		*out_dim = self->dim();
	})
}


Error AStarIndex_size_t_packing_radius(const AStarIndex_size_t* self, Distance_t* out_packing_radius)
{
	RETURN_ERROR({
		*out_packing_radius = self->packing_radius();
	})
}


Error AStarIndex_size_t_scale(const AStarIndex_size_t* self, Distance_t* out_scale)
{
	RETURN_ERROR({
		*out_scale = self->scale();
	})
}


Error AStarIndex_size_t_num_shells(const AStarIndex_size_t* self, NumShells_t* out_num_shells)
{
	RETURN_ERROR({
		*out_num_shells = self->num_shells();
	})
}


Error AStarIndex_size_t_num_probes(const AStarIndex_size_t* self, size_t* out_num_probes)
{
	RETURN_ERROR({
		*out_num_probes = self->num_probes();
	})
}


Error AStarIndex_size_t_num_hashes(AStarIndex_size_t* self, size_t* out_size)
{
	RETURN_ERROR({
		*out_size = self->num_hashes();
	})
}


Error AStarIndex_size_t_num_elements(AStarIndex_size_t* self, size_t* out_size)
{
	RETURN_ERROR({
		*out_size = self->num_elements();
	})
}


Error AStarIndex_size_t_clear(AStarIndex_size_t* self)
{
	RETURN_ERROR({
		self->clear();
	})
}


Error AStarIndex_size_t_clear_by_vector(AStarIndex_size_t* self, const VElem_t* vector)
{
	RETURN_ERROR({
		self->clear(vector);
	})
}


Error AStarIndex_size_t_put(AStarIndex_size_t* self, const VElem_t* vector, size_t elem)
{
	RETURN_ERROR({
		self->put(vector, elem);
	})
}


Error AStarIndex_size_t_put_all(AStarIndex_size_t* self, const VElem_t* vector, size_t count, const size_t* elems)
{
	RETURN_ERROR({
		self->put(vector, count, elems);
	})
}



Error AStarIndex_size_t_count(const AStarIndex_size_t* self, const VElem_t* vector, size_t* out_count)
{
	RETURN_ERROR({
		*out_count = self->count_extended(vector);
	})
}


Error AStarIndex_size_t_get_callback(const AStarIndex_size_t* self, const VElem_t* vector, AStarIndex_size_t_Callback_t callback)
{
	RETURN_ERROR({
        AStarIndex_size_t_CallUserFunction callback_object(callback);
		self->get_extended(vector, &callback_object);
	})
}


Error AStarIndex_size_t_get_elems(const AStarIndex_size_t* self, const VElem_t* vector, size_t max_size, size_t* out_count, size_t* out_elems)
{
	RETURN_ERROR({
        KeepElems<size_t> callback_object(max_size, out_elems);
		self->get_extended(vector, &callback_object);
		*out_count = callback_object.size();
	})
}


CElem_t TESTING_round_up(double x)
{
	return round_up<CElem_t>(x);
}
