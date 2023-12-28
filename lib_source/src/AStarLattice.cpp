/*
 * Functions for A* lattices.
 *
 * Author: Barry Drake
 */

#include "AStarLattice.h"
#include "WorkBuff.h"
#include <cstring>
#include <math.h>
#include <stdlib.h>


///
/// Swap two elements.
///
static inline void swap(Order_t& a, Order_t& b)
{
    Order_t t = a;
    a = b;
    b = t;
}

///
/// Swap two elements, as required, in array 'val' so that val[a] >= val[b].
///
static inline void swap_less(const VElem_t* val, Order_t& a, Order_t& b)
{
    if (val[a] < val[b])
    {
        swap(a, b);
    }
}


///
/// Insertion sort. Modifies the ordinal array to provide the sorting permutation.
///
/// \param[in]  vals    The values to sort.
///
static inline void insertion_sort_order(const VElem_t* vals, Order_t* ords, Order_t* ords_end)
{
    VElem_t  val;
    Order_t  ord;
    Order_t* ords_i;
    Order_t* ords_j;

    for (ords_i = ords + 1; ords_i < ords_end; ords_i++)
    {
        ord = *ords_i;
        val = vals[ord];

        ords_j = ords_i - 1;
        while (ords_j >= ords && val < vals[*ords_j])
        {
            *(ords_j + 1) = *ords_j;
            ords_j--;
        }

        *(ords_j + 1) = ord;
    }
}


///
/// Determins the sort order for the given array 'val' (while leaving val
/// unchanged). 'ord' is set to the permutation that would sort the array 'val'.
///
/// \param[in]  val     The array of values to sort. These are read but not
///                     modified.
/// \param      ord     The order, which will be sorted according to the
///                     the corresponding values in val. To sort the list, these
///                     should be initialised to the numbers 0, 1, 2, ... .
///                     This can equally be used to sort any subset of
///                     val in any initial order, as for instance part of a
///                     recursive call to this function.
/// \param[in]  ord_end The end of the order to be sorted. 
///                     The length of val on the other hand must be at
///                     least as large as the largest value in ords.
///
static void sort_order(const VElem_t* val, Order_t* ord, Order_t* ord_end)
{
    static const size_t INSERTION_SORT_THRESHOLD = 6;

    Order_t* l_adv;
    Order_t* r_adv;
    Order_t* med;
    Order_t* med_left;
    Order_t* med_right;
    VElem_t  temp_val;

    //
    // We call ourselves recursively for the
    // smaller part of the array. For the bigger
    // part, we iterate instead.
    //
    while (1)
    {
        size_t    num_left;
        size_t    num_right;
        size_t    size = ord_end - ord;
        Order_t*  last = ord_end - 1;

        //
        // Insertion sort small arrays.
        //
        if (size <= INSERTION_SORT_THRESHOLD)
        {
            insertion_sort_order(val, ord, ord_end);
            return;
        }

        //
        // Get the median. For larger arrays,
        // lookup several keys before deciding on a median.
        //
        med         = ord + size / 2;
        med_left    = ord;
        med_right   = last;

        swap_less(val, *med_right, *med);
        swap_less(val, *med_right, *med_left);
        swap_less(val, *med_left , *med);
        swap(*med, *(ord+1));

        //
        // We place the chosen median at the two
        // extremes of the array. We finally end up
        // with an array which looks like: (note s < m and b > m)
        //
        // m  s  [   < m    ]     [    >= m   ]  b
        // ^  ^             ^     ^              ^
        // 0  1          r_adv   l_adv          last
        //
        // where m is the median (med). The loop terminates
        // when r_adv and l_adv cross (i.e. when l_adv > r_adv).
        //
        l_adv   = ord + 1;
        r_adv   = last;

        //
        // for (l_adv < r_adv)
        //
        while (true)
        {
            // Left side: advance while smaller or equal to median.
            temp_val = val[*ord];
            do
            {
                l_adv++;
            }
            while (val[*l_adv] < temp_val);

            // Right side: advance while bigger or equal to median.
            do
            {
                r_adv--;
            }
            while (temp_val < val[*r_adv]);

            if (l_adv >= r_adv)
            {
                if (l_adv == r_adv)
                {
                    l_adv++;
                    r_adv--;
                }
                break;
            }

            //
            // Swap the left and right, and continue.
            //
            swap(*l_adv, *r_adv);
        }

        //
        // Put the median element back in the final place.
        //
        swap(*(l_adv - 1), *ord);

        //
        // Quick sort the left and the right.
        // Iterate on the bigger side, and recurse
        // on the smaller one.
        //
        num_left    = l_adv - (ord + 1);
        num_right   = last - r_adv;

        if (num_left == 0)
        {
            if (num_right == 0)
            {
                return;
            }
            else
            {
                ord = last + 1 - num_right;
                ord_end = ord + num_right;
            }
        }
        else if (num_right == 0)
        {
            ord_end = ord + num_left;
        }
        else
        {
            Order_t* right_ord;

            right_ord = last + 1 - num_right;

            if (num_left > num_right)
            {
                sort_order(val, right_ord, right_ord + num_right);
                ord_end = ord + num_left;
            }
            else
            {
                sort_order(val, ord, ord + num_left);
                ord = right_ord;
                ord_end = ord + num_right;
            }
        }
    }
}



///
/// A precomputed identity permutation used for fast initialisation.
///
class StartSortOrd
{
public:
	StartSortOrd(Dim_t initial_dim = 16)
	{
		m_dim = initial_dim;
		m_ord = (Order_t*) malloc(sizeof(Order_t) * (m_dim + 1));
		if (!m_ord)
		{
			throw Error_mem_fail;
		}

        for (Dim_t i = 0; i <= m_dim; ++i)
        {
            m_ord[i] = i;
        }
	}

	~StartSortOrd(void)
	{
		if (m_ord)
		{
			free(m_ord); 
		}
	}

	Order_t* ord(Dim_t dim)
	{
		if (dim <= m_dim)
		{
			return m_ord;
		}

		// resize
		m_ord = (Order_t*) realloc(m_ord, sizeof(Order_t) * (dim + 1));
		if (!m_ord)
		{
			throw Error_mem_fail;
		}
        for (Dim_t i = m_dim + 1; i <= dim; ++i)
        {
            m_ord[i] = i;
        }
		m_dim = dim;

		return m_ord;
	}

private:
	Order_t*	m_ord;
	Dim_t 		m_dim;
};
static StartSortOrd START_SORT_ORD;


///
/// A sentinel value for block-sort sets (in AStar_closestPoint).
///
static const Order_t  END = -1;

///
/// A precomputed Array of END constants used for fast initialisation.
///
class EndFill
{
public:
	EndFill(Dim_t initial_dim = 16)
	{
		m_dim = initial_dim;
		m_fill = (Order_t*) malloc(sizeof(Order_t) * (m_dim + 1));
		if (!m_fill)
		{
			throw Error_mem_fail;
		}

        for (Dim_t i = 0; i <= m_dim; ++i)
        {
            m_fill[i] = END;
        }
	}

	~EndFill(void)
	{
		if (m_fill)
		{
			free(m_fill); 
		}
	}

	Order_t* fill(Dim_t dim)
	{
		if (dim <= m_dim)
		{
			return m_fill;
		}

		// resize
		m_fill = (Order_t*) realloc(m_fill, sizeof(Order_t) * (dim + 1));
		if (!m_fill)
		{
			throw Error_mem_fail;
		}
        for (Dim_t i = m_dim + 1; i <= dim; ++i)
        {
            m_fill[i] = END;
        }
		m_dim = dim;

		return m_fill;
	}

private:
	Order_t*	m_fill;
	Dim_t 		m_dim;
};
static EndFill END_FILL;


Distance_t AStarLattice::rho(Dim_t dim)
{
    return (Distance_t) sqrt(dim * (dim + 1.0)) / 2.0;
}


void AStarLattice::to_lattice_space
(
    Dim_t           dim,
    Distance_t      scale,
    const VElem_t*  v_in,
    VElem_t*        v_out
)
{
    double              sum       = 0;
    const double*       pVecIn    = v_in;
    const double* const pVecInEnd = v_in + dim;
    do
    {
        sum += *pVecIn++;
    }
    while (pVecIn < pVecInEnd);

	// The norm of vector (1, ..., 1)
    const double norm = sqrt(dim + 1.0);

    const double v_n  = -sum / norm;
    const double t    = (v_n + sum) / dim;

    // Calculate the rotated and scaled vector.
    do
    {
        *v_out++ = scale * (*v_in++ - t);
    }
    while (v_in < pVecInEnd);
    *v_out = scale * v_n;
}


void AStarLattice::from_lattice_space
(
    Dim_t           dim,
    Distance_t      scale,
    const VElem_t*  v_in,
    VElem_t*        v_out
)
{
    // The rotation is simplified by decomposing each vector
    // into a sum of three orthogonal vectors
    // by projecting it onto the plane spanned by the vectors
    // (1, ..., 1, 0) and (0, ..., 0, 1).
    // Only the components in the plane are modified by the rotation.

    // Calculate the dot product of vector with (1, 1, ..., 1, 0).

	// The norm of vector (1, ..., 1)
    const double norm = sqrt(dim + 1.0);

    //
    // Calculate the rotated and scaled vector.
    //
    const double t = v_in[dim] * (norm - dim - 1) / dim / norm;

	const VElem_t* const pVecInEnd = v_in + dim;
    do
    {
        *v_out++ = (*v_in++ + t) / scale;
    }
    while (v_in < pVecInEnd);
}



void AStarLattice::cvector_k_to_lattice_point_in_lattice_space
(
    Dim_t          dim,
    const CElem_t* c,
    K_t            k,
    VElem_t*       v_out
)
{
    const CElem_t  dimp  = dim + 1;
    const CElem_t* pc    = c;
    const CElem_t* pcEnd = c + dimp;
    do 
    {
        *v_out++ = - (*pc++ * dimp + k);
    }
    while (pc < pcEnd);
}


void AStarLattice::cvector_to_lattice_point_in_lattice_space
(
    Dim_t          dim,
    const CElem_t* c,
    VElem_t*       v_out
)
{
    const CElem_t  dimp  = dim + 1;
    const CElem_t* pcEnd = c + dimp;

    K_t k = 0;
    {
        const CElem_t* pc = c;
        do 
        {
           k -= *pc++;
        }
        while (pc < pcEnd);
    }
    {
        const CElem_t* pc = c;
        do 
        {
            *v_out++ = - (*pc++ * dimp + k);
        }
        while (pc < pcEnd);
    }
}


void AStarLattice::closest_point(Dim_t dim, const VElem_t* v, K_t& k, CElem_t* c, WorkBuff* buff)
{
    /// This is a variation on Algorithm 2 from:
    /// McKilliam, Clarkson, Smith and Quinn, 2008, ISTA

    K_t             s_k;
    double          D;
	const int       dimi   = dim;
    const int       dimp   = dim + 1;
    const double    dimpd  = dimp;
    int             sum    = 0;
    double          alpha  = 0;
    double          beta   = 0;

    VElem_t*        z      = get_buff<VElem_t>(buff);
    Order_t*        link   = get_buff<Order_t>(buff);
    Order_t*        bucket = get_buff<Order_t>(buff);

    // initialise the block sets to be empty
    std::memcpy(bucket, END_FILL.fill(dim), sizeof(Order_t) * dimp);

    for (int i = 0; i < dimp; ++i)
    {
        const double     y_i       = v[i] / dimpd;
		const CElem_t    y_round_i = round_up<CElem_t>(y_i); // y_round_i = floor(y_i + 0.5)
        const double     z_i       = y_i - y_round_i;        // -0.5 <= z_i < 0.5

        sum     += y_round_i; 
        c[i]    =  y_round_i;
        z[i]    =  z_i;
        alpha   += z_i;
        beta    += z_i * z_i;

        // These lines perform a block sort on z.
        // The cast to (int) effectively performs floor as -0.5 <= z_i < 0.5.
        const Order_t ii = dimi - (int)(dimpd * (z_i + 0.5));
        link[i]    = bucket[ii];
        bucket[ii] = i;
    }

    D = beta * dimpd - alpha * alpha;

    Order_t* m            = 0;
    Order_t* p_bucket     = bucket;
    Order_t* p_bucket_end = bucket + dimp;
    do
    {
        Order_t t = *p_bucket;
        if (t != END)
        {
            do
            {
                alpha = alpha - 1.0;
                beta  = beta - 2.0 * z[t] + 1.0;
                t = link[t];
            }
            while (t != END);

            const double d(beta * dimpd - alpha * alpha);
            if (d < D)
            {
                D = d;
                m = p_bucket;
            }
        }
    }
    while (++p_bucket < p_bucket_end);

    p_bucket = bucket;
    while (p_bucket <= m)
    {
        Order_t t = *p_bucket++;
        while (t != END)
        {
            c[t]++;
            sum++;
            t = link[t];
        }
    }

    k   = ((-sum % dimp) + dimp) % dimp;  // k = -sum mod dimp
    s_k = (sum + k) / dimp;

    // Convert McKilliam's k-vector into a c-vector in place.
    const CElem_t* c_end(c + dimp);
    while (c < c_end)
    {
        *c++ -= s_k;
    }

    // We could return the distance.
    // The multipication by dimpd compensates for the different scales.
    // return D * dimpd;
}


void AStarLattice::setK0
(
    Dim_t           dim,
    const VElem_t*  v,
    VElem_t*        xmod,
    CElem_t*        c,
    Order_t*        order,
	WorkBuff*		buff
)
{
    int          h      = 0;
    const int    dimp   = dim + 1;
	const double dimpd  = dimp;
    VElem_t*     p_xmod = xmod;

    CElem_t* p_c     = c;
    CElem_t* p_c_end = c + dimp;
    while (p_c < p_c_end)
    {
        const CElem_t cx = round_up<CElem_t>(*v / dimpd);
        *p_c++ = cx;
        *p_xmod++ = *v++ - cx * dimpd;
        h += cx;
    }

    // c is our first guess at the c-vector for the nearest
    // remainder-0 (k=0) lattice point.
    //
    // h is the sum of the c vector elements.
    // 
    // For a remainder-0 lattice point h should be zero.
    // The following code finds the sort order of the
    // residuals and adjusts the c vector so that h = 0
    // making a minimal increase to the sum of absolute
    // residuals.
    //

    if (h == 0)
    {
        // simple case
        memcpy(order, START_SORT_ORD.ord(dim), dimp * sizeof(Order_t));
        sort_order(xmod, order, &order[dimp]);
        return;
    }

    Order_t* sortord = get_buff<Order_t>(buff);
    memcpy(sortord, START_SORT_ORD.ord(dim), dimp * sizeof(Order_t));
    sort_order(xmod, sortord, &sortord[dimp]);
    
    if (h > 0)
    {
        Order_t* p_sortord     = sortord;
        Order_t* p_sortord_end = sortord + h;
        while (p_sortord < p_sortord_end)
        {
            const Order_t idx = *p_sortord++;
            c[idx]--;
            xmod[idx] += dimpd;
        }
        const int part(dimp - h);
        memcpy(order, sortord + h, part * sizeof (Order_t));
        memcpy(order + part, sortord, h * sizeof (Order_t));
    }
    else
    {
        const size_t part          = dimp + h;
        Order_t*     p_sortord     = sortord + part;
        Order_t*     p_sortord_end = sortord + dimp;
        while (p_sortord < p_sortord_end)
        {
            const Order_t idx = *p_sortord++;
            c[idx]++;
            xmod[idx] -= dimpd;
        }
        memcpy(order - h, sortord, part * sizeof (Order_t));
        memcpy(order, sortord + part, -h * sizeof (Order_t));
    }
}



