/*
 * Functions for A* lattices.
 *
 * Author: Barry Drake
 */

#ifndef ASTARLATTICE__H
#define ASTARLATTICE__H

#include "common.h"

class WorkBuff;

///
/// This is just a name space for A* lattice functions.
///
class AStarLattice
{
public:

    ///
    /// The native packing radius of the A* lattice in the space that the lattice is
    /// represented, and with no scaling factor (scale = 1).
    ///
    static Distance_t rho(Dim_t dim);

    ///
    /// Convert a vector to the representation space of the lattice. This is a
    /// scaled version (n+1) so that the coordinates are integers.
    /// 
    /// Refs:
    /// Conway & Sloan, 1998, p 115.
    /// Baek & Adams, 2009.
    ///
    /// \param[in]  dim         number of dimensions, n.
    /// \param[in]  scale       scaling factor (rho(dim)/packing radius).
    /// \param[in]  v_in        n dimensional input vector.
    /// \param[out] v_out       n+1 dimensional output vector.
    ///
    static void to_lattice_space
    (
        Dim_t           dim,
        Distance_t      scale,
        const VElem_t*  v_in,
        VElem_t*        v_out
    );


    ///
    /// Convert a vector from the representation space of the lattice back to
    /// the ordinary working space.
    ///
    /// \param[in]  dim       number of dimensions, n.
    /// \param[in]  scale     scaling factor (rho(dim)/packing radius).
    /// \param[in]  v_in      n+1 dimensional input vector.
    /// \param[out] v_out     n dimensional output vector.
    ///
    static void from_lattice_space
    (
        Dim_t           dim,
        Distance_t      scale,
        const VElem_t*  v_in,
        VElem_t*        v_out
    );


    ///
    /// Convert from a c-vector to its lattice point in the
    /// lattice representation space.
    ///
    /// \param[in]  dim      n.
    /// \param[in]  c        n+1 dimensional c-vector representation of a lattice point.
    /// \param[out] k        k value of the lattice point, k = -sum(c).
    /// \param[out] v_out    n+1 dimensional output vector.
    ///
    static void cvector_k_to_lattice_point_in_lattice_space
    (
        Dim_t          dim,
        const CElem_t* c,
        K_t            k,
        VElem_t*       v_out
    );


    ///
    /// Convert from a c-vector to its lattice point in the
    /// lattice representation space.
    ///
    /// \param[in]  dim      n.
    /// \param[in]  c        n+1 dimensional c-vector representation of a lattice point.
    /// \param[out] v_out    n+1 dimensional output vector.
    ///
    static void cvector_to_lattice_point_in_lattice_space
    (
        Dim_t          dim,
        const CElem_t* c,
        VElem_t*       v_out
    );


    ///
    /// Find the closest A* lattice point to v.
    ///
    /// \param[in]  dim         number of dimensions, n.
    /// \param[in]  v           n+1 dimensional query vector (in the lattice representation space).
    /// \param[out] k           k value of the lattice point, k = -sum(c).
    /// \param[out] c           n+1 dimensional c-vector for the lattice point.
    ///
    static void closest_point
    (
        Dim_t               dim,
        const VElem_t*      v,
        K_t&                k,
        CElem_t*            c,
		WorkBuff*			buff
    );


    ///
    /// Find the closest k=0 A* lattice point to v.
    ///
    /// \param[in]  dim   number of dimensions, n.
    /// \param[in]  v     n+1 dimensional query vector (in the lattice representation space).
    /// \param[out] xmod  v translated so that the closest k=0 point is the origin.
    /// \param[out] c     n+1 dimensional c-vector of the closest k=0 lattice point.
    /// \param[out] order permutation order.
    ///
    static void setK0
    (
        Dim_t           dim,
        const VElem_t*  v,
        VElem_t*        xmod,
        CElem_t*        c,
        Order_t*        order,
		WorkBuff*		buff
    );


private:
    // constructor not implemented
    AStarLattice(void);
    ~AStarLattice(void);

};




#endif // ASTARLATTICE__H
