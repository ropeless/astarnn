/*
 * This is a C interface for the DLL.
 *
 * Author: Barry Drake
 */

#ifndef ASTARNN_C__H
#define ASTARNN_C__H

#include "common.h"

class AStarNN;
class AStarIndex_size_t;

#if _WIN32
#define DLL __declspec(dllexport)
#else
#define DLL
#endif


extern "C"
{
    /* type for AStarNN query callback function */
    typedef Error (*AStarNN_Callback_t) (Hash_t hash_code, K_t k, const CElem_t* c);

    /* type for AStarIndex_size_t query callback function */
    typedef Error (*AStarIndex_size_t_Callback_t) (Hash_t hash_code, size_t elem);

	/* static methods */

    DLL const char* info_string();
    DLL const char* extended_info_string();

    DLL const char* AStar_error_string(Error err);
    DLL NumShells_t AStar_max_num_shells(void);

    DLL Error AStar_rho(Dim_t dim, Distance_t* out_rho);
    DLL Error AStar_to_lattice_space(Dim_t dim, Distance_t scale, const VElem_t* in_v, VElem_t* out_v);
    DLL Error AStar_from_lattice_space(Dim_t dim, Distance_t scale, const VElem_t* in_v, VElem_t* out_v);
    DLL Error AStar_cvector_k_to_lattice_point_in_lattice_space(Dim_t dim, const CElem_t* c, K_t k, VElem_t* out_v);
    DLL Error AStar_cvector_k_to_lattice_point(Dim_t dim, Distance_t scale,const CElem_t* c,  K_t k, VElem_t* out_v);
    DLL Error AStar_cvector_to_lattice_point_in_lattice_space(Dim_t dim, const CElem_t* c, VElem_t* out_v);
    DLL Error AStar_cvector_to_lattice_point(Dim_t dim, Distance_t scale, const CElem_t* c, VElem_t* out_v);

    /* AStarNN object methods */

    DLL Error AStarNN_new(Dim_t dim, Distance_t packing_radius, NumShells_t num_shells, AStarNN** out_AStarNN);
    DLL Error AStarNN_delete(AStarNN* self);
    
	DLL Error AStarNN_nearest_callback(const AStarNN* self, const VElem_t* vector, AStarNN_Callback_t callback);
    DLL Error AStarNN_delaunay_callback(const AStarNN* self, const VElem_t* vector, AStarNN_Callback_t callback);
    DLL Error AStarNN_extended_callback(const AStarNN* self, const VElem_t* vector, AStarNN_Callback_t callback);

	DLL Error AStarNN_nearest_hash(const AStarNN* self, const VElem_t* vector, Hash_t* hashes);  // buff size >= 1
    DLL Error AStarNN_delaunay_hash(const AStarNN* self, const VElem_t* vector, Hash_t* hashes); // buff size >= dim + 1
    DLL Error AStarNN_extended_hash(const AStarNN* self, const VElem_t* vector, Hash_t* hashes); // buff size >= num_probes

    DLL Error AStarNN_nearest_cvector(const AStarNN* self, const VElem_t* vector, CElem_t* cvectors);  // buff size >= 1 x dim + 1
    DLL Error AStarNN_delaunay_cvector(const AStarNN* self, const VElem_t* vector, CElem_t* cvectors); // buff size >= (dim + 1) x (dim + 1)
    DLL Error AStarNN_extended_cvector(const AStarNN* self, const VElem_t* vector, CElem_t* cvectors); // buff size >= num_probes x (dim + 1)

    DLL Error AStarNN_nearest_probe(const AStarNN* self, const VElem_t* vector, Hash_t* hashes, CElem_t* cvectors);
    DLL Error AStarNN_delaunay_probe(const AStarNN* self, const VElem_t* vector, Hash_t* hashes, CElem_t* cvectors);
    DLL Error AStarNN_extended_probe(const AStarNN* self, const VElem_t* vector, Hash_t* hashes, CElem_t* cvectors);


    DLL Error AStarNN_dim(const AStarNN* self, Dim_t* out_dim);
    DLL Error AStarNN_packing_radius(const AStarNN* self, Distance_t* out_packing_radius);
    DLL Error AStarNN_scale(const AStarNN* self, Distance_t* out_scale);
    DLL Error AStarNN_num_shells(const AStarNN* self, NumShells_t* out_num_shells);
    DLL Error AStarNN_num_probes(const AStarNN* self, size_t* out_num_probes);

	/* AStarIndex_size_t object methods */

	DLL Error AStarIndex_size_t_new(Dim_t dim, Distance_t packing_radius, NumShells_t num_shells, AStarIndex_size_t** out_AStarIndex);
	DLL Error AStarIndex_size_t_delete(AStarIndex_size_t* self);

    DLL Error AStarIndex_size_t_dim(const AStarIndex_size_t* self, Dim_t* out_dim);
    DLL Error AStarIndex_size_t_packing_radius(const AStarIndex_size_t* self, Distance_t* out_packing_radius);
    DLL Error AStarIndex_size_t_scale(const AStarIndex_size_t* self, Distance_t* out_scale);
    DLL Error AStarIndex_size_t_num_shells(const AStarIndex_size_t* self, NumShells_t* out_num_shells);
    DLL Error AStarIndex_size_t_num_probes(const AStarIndex_size_t* self, size_t* out_num_probes);
	DLL Error AStarIndex_size_t_num_hashes(AStarIndex_size_t* self, size_t* out_size);
	DLL Error AStarIndex_size_t_num_elements(AStarIndex_size_t* self, size_t* out_size);

    DLL Error AStarIndex_size_t_clear(AStarIndex_size_t* self);
    DLL Error AStarIndex_size_t_clear_by_vector(AStarIndex_size_t* self, const VElem_t* vector);

	DLL Error AStarIndex_size_t_put(AStarIndex_size_t* self, const VElem_t* vector, size_t elem);
	DLL Error AStarIndex_size_t_put_all(AStarIndex_size_t* self, const VElem_t* vector, size_t count, const size_t* elems);

	DLL Error AStarIndex_size_t_count(const AStarIndex_size_t* self, const VElem_t* vector, size_t* out_count);

	DLL Error AStarIndex_size_t_get_callback(const AStarIndex_size_t* self, const VElem_t* vector, AStarIndex_size_t_Callback_t callback);
	DLL Error AStarIndex_size_t_get_elems(const AStarIndex_size_t* self, const VElem_t* vector, size_t max_size, size_t* out_count, size_t* out_elems);


	/* static testing methods - for whiltebox testing purposes only */
	DLL CElem_t TESTING_round_up(double x);

}

#endif // ASTARNN_C__H
