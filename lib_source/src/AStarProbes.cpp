/*
 * Functions for A* lattice probing.
 *
 * Author: Barry Drake
 */

#include "AStarProbes.h"
#include "AStarLattice.h"
#include "Deleter.h"
#include "PointSet.h"
#include "CostSet.h"
#include "PriorityQueue.h"
#include <stdlib.h>
#include <math.h>

///
/// This is used to set the buffer size for keeping track of
/// seen probe points. This value puts a processing limit on the
/// number of remainder-zero probes in each shell. The present
/// value of MAX_ZERO_PROBES_PER_SHELL is extrordinary large and
/// no practical system is ever expected to reach this limit.
///
const size_t MAX_ZERO_PROBES_PER_SHELL = 16 * 1024;


/// What type to store the cost of shells when generating probes.
typedef int Cost_t;


///
/// PROBES_F caches precomputed values to determin the number of probes.
///
/// num_zero_probes(n, k) = PROBES_F[min(n, k)][k - min(n, k)].
/// Interestingly, this means that the number of remainder-0 probes per shell
/// is independent of dimensionality, n, for n > k.
///
/// n is the dimensionality.
/// k is the number of (extended) shells.
///
/// We have no proof that this formula is correct, however, it is emprically
/// validated for all k <= 10 and all n <= 512.
/// It is also checked and asserted every time a set of probes is generated.
///
/// The size of the array must be exactly MAX_NUM_SHELLS + 1.
///
/// Update this array with extreme caution.
///
static const size_t PROBES_F[AStarProbes::MAX_NUM_SHELLS + 1][AStarProbes::MAX_NUM_SHELLS + 1] =
    {
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31},
        {4,6,7,9,10,12,14,16,18,21,23,25,26,28,30,32,34,38,40,41,43,45,47,48,50,52,56,58,60},
        {7,8,11,14,17,21,25,27,29,36,39,44,50,52,56,63,66,70,77,82,90,95,99,103,111,116,122,129},
        {12,14,20,25,32,37,49,55,67,73,83,94,110,117,137,152,164,176,198,208,233,245,265,283,313,323,355},
        {19,24,33,43,55,67,81,101,121,142,165,189,213,245,274,309,345,389,436,474,521,570,622,677,735,794},
        {30,38,53,69,90,111,139,163,207,243,292,337,400,449,523,587,672,744,849,931,1064,1176,1296,1416,1581},
        {45,59,81,107,139,176,221,268,324,399,476,565,667,778,902,1044,1191,1358,1540,1736,1946,2188,2437,2725},
        {67,88,121,159,209,265,337,414,510,609,751,890,1067,1247,1475,1704,1992,2276,2633,2976,3406,3816,4335},
        {97,129,175,232,303,388,494,615,762,927,1117,1359,1626,1928,2278,2678,3121,3632,4197,4835,5550,6324},
        {139,184,250,329,431,552,706,882,1102,1350,1647,1977,2407,2859,3411,4016,4736,5513,6448,7438,8620},
        {195,260,349,460,600,771,984,1237,1547,1910,2342,2840,3423,4128,4928,5852,6912,8128,9507,11085},
        {272,360,482,632,824,1056,1350,1697,2129,2635,3247,3956,4803,5760,6948,8268,9828,11585,13653},
        {373,494,656,859,1114,1429,1821,2294,2876,3570,4405,5392,6566,7924,9520,11425,13603,16127},
        {508,669,885,1152,1492,1907,2429,3056,3833,4758,5883,7211,8807,10662,12865,15405,18459},
        {684,899,1180,1533,1975,2522,3202,4028,5043,6266,7744,9508,11622,14108,17057,20501},
        {915,1195,1563,2019,2595,3302,4185,5253,6573,8157,10083,12379,15145,18401,22288},
        {1212,1579,2051,2642,3380,4292,5421,6798,8486,10526,12996,15958,19515,23733},
        {1597,2068,2676,3430,4375,5535,6977,8726,10877,13469,16617,20384,24924},
        {2087,2694,3466,4428,5623,7098,8916,11132,13842,17120,21085,25849},
        {2714,3485,4466,5679,7191,9044,11333,14112,17515,21618,26592},
        {3506,4486,5719,7250,9142,11468,14324,17800,22035,27155},
        {4508,5740,7292,9204,11571,14466,18023,22335,27594},
        {5763,7314,9248,11636,14574,18172,22569,27909},
        {7338,9271,11682,14642,18285,22725,28154},
        {9296,11706,14690,18356,22843,28317},
        {11732,14715,18406,22917,28440},
        {14742,18432,22969,28517},
        {18460,22996,28571},
        {23025,28599},
        {28629},
    };


///
/// A function to compute moves for probe generation.
/// A move is a pair of dimensions where one will be incremented and the
/// other will be decremented in order to propose a new remainder-zero
/// c-vector for the prioirity queue.
///
///  label  move_i(l)  move_j(l)
///  -----  ---------  ---------
///
///    0          0          0
///
///    1          1          0
///    2          0          1
///
///    3          2          0
///    4          1          1
///    5          0          2
///
///              ...
///
inline void move(size_t label, Order_t& i, Order_t& j)
{
	const double ETA = 10e-6; // protect against rounding errors

	size_t k = (size_t) ceil(sqrt(2.0 * label + 2.25) - 1.5 - ETA);
	size_t l = k * (k + 3) / 2;

	i = (Order_t) (l - label);
	j = (Order_t) (k - i);
}


///
/// A working data structure for calculating the probes for
/// method AStar_generateProbes.
///
class ProbePoint
{
public:
    ProbePoint(Dim_t dim)
    {
		m_code  = new CElem_t[dim+1];
        m_label = 0;
        memset(m_code, 0, (dim + 1) * sizeof(CElem_t));
    }

    ProbePoint(Dim_t dim, const CElem_t* code, Dim_t inc_i, Dim_t dec_i, size_t label)
    {
		m_code  = new CElem_t[dim+1];
        m_label = label;
        memcpy(m_code, code, (dim + 1) * sizeof(CElem_t));
        m_code[inc_i]++;
        m_code[dec_i]--;
    }

	~ProbePoint(void)
	{
		delete [] m_code;
	}


    inline const CElem_t* code(void) const
    {
        return m_code;
    }

    inline size_t label(void) const
    {
        return m_label;
    }


private:
    // Copy, assignment and equality not implemented
    ProbePoint(const ProbePoint& oth);
    ProbePoint& operator=(const ProbePoint& obj);
    bool operator==(ProbePoint const& oth) const;
    bool operator!=(ProbePoint const& oth) const;

    ///
    /// The c-vector associated with this point.
    ///
    CElem_t*   m_code;

    ///
    /// move label sequencing for generating probes.
    ///
    size_t    m_label;
};



///
/// An interface to process remainder-zero points by AStar_generateZeroProbes.
///
class ProbeProcessor
{
public:
    virtual void process_probe(int shell_distance, const CElem_t* probe) = 0;
};


///
/// An implementation of ProbeProcessor for generate_probes.
///
class ProbeCollector : public ProbeProcessor
{
public:
    ProbeCollector(Dim_t dim, size_t num_probes, CElem_t* probes)
        : cur(probes)
        , end(probes + num_probes * (dim + 1))
        , dim(dim)
        , dimp(dim + 1)
    {}


    ///
    /// A method for consistency checking.
    ///
    bool correct_probes_collected(void) const
    {
        return cur == end;
    }

    
    ///
    /// This is called to process a remainer-zero probe.
    /// It results in storing a whole orbit of probes in the probe memory.
    ///
    virtual void process_probe(int shell_distance, const CElem_t* probe)
    {
        // Consistency check - confirm the expected number of probes
        if (cur + dimp * dimp > end)
        {
            throw Error_unknown; // too many probes
        }

        // Copy the probe to the current probe (probe k = 0)
        for (Dim_t d = 0; d < dimp; ++d)
        {
            cur[d] = probe[d];
        }
        cur += dimp;

        // Add the other probes of the orbit,
        // remainder-k
        for (Dim_t k = 1; k < dimp; ++k)
        {
            // This does the following:
            //  set code_k to code_(k-1)
            //  rotate coordinates of code_k up by 1 dimension
            //  decrement code_k[0] by 1.
            const CElem_t* prev = cur - dimp;
            memcpy(cur + 1, prev, dim * sizeof(CElem_t));
            cur[0] = prev[dim] - 1;
            cur += dimp;
        }
    }

private:
    // Number of dimensions
    const Dim_t    dim;

    // Number of dimensions + 1
    const Dim_t    dimp;

    // The probe we are about to write
    CElem_t*       cur;

    // Where cur should end up after all probes are generated
    const CElem_t* end;
};




///
/// Generate remainder-zero probes and pass
/// them, in order, to the given processor.
///
/// In this algorithm, each remainder-zero probe is given a
/// cost. The cost of a probe is the negative of the shell
/// distance number, where the shell distance number is an
/// integer that is proportional to the squared distance from
/// the zeroth shell.
///
/// For a remainder-zero point with c-vector 'c':
/// cost = - ( sum {i = 0 to n} (n+1)/2 * c[i]^2 - i * c[i] )
///
static void generate_zero_probes
(
    Dim_t           dim,
    NumShells_t     num_shells,
    ProbeProcessor* processor
)
{
    PointSet points(MAX_ZERO_PROBES_PER_SHELL);

    PriorityQueue<Cost_t, ProbePoint> queue;
    CostSet<Cost_t>                   seenCosts(num_shells + 1);
    int                               shells_to_go = num_shells; // must be signed integer.

    ProbePoint*  probe_point_zero = new ProbePoint(dim);

    // Register probe point zero
    seenCosts.pushUniqueSmall(0);
    queue.add(probe_point_zero, 0);

    // The cost of the last candidate removed from the queue (none yet).
    // Actually this is negative of cost, which save an operation.
    // So this initialisation is like -1, so the first probe, cost = 0, is
    // recognised as a new shell.
    Cost_t cost = 1;

    while (queue.size() > 0)
    {
        ProbePoint* probe_point;
        Cost_t      probe_cost;
        
        queue.poll(&probe_point, &probe_cost);

        // When we leave this block, we will delete the probe_point
        Deleter<ProbePoint> delete_probe_point(probe_point);

        // Are we seeing a new shell?
        if (probe_cost < cost)
        {
            // We have just recieved a probe point for a new shell.

            // Clear the point set
            points.clear();

            // Record the cost for the new shell
            cost = probe_cost;

            // If we have processed enough shells, then return
            shells_to_go--;
            if (shells_to_go < -1)
            {
                break;
            }
        }

        // Try to insert probe_point into set of points
        const CElem_t* code = probe_point->code();
        bool is_new = points.insert(dim, code);

        // Check if insert succeeded (probe_point was not already in points)
        if (is_new)
        {
            // Process the newly found remainder-zero probe point
            processor->process_probe(-cost, code);

            // Spawn new points to search.
            // Add them to the priority queue.
            const size_t  l_max = (dim + 1) * dim;
            const size_t  l_swp = l_max / 2;
            for
            (
                size_t l = probe_point->label();
                l < l_max;
                ++l
            )
            {
                // Work out the dimension to increment (i) and decrement (j).
                int i, j;
                if (l < l_swp)
                {
					Order_t li, lj;
					move(l, li, lj);

                    i = dim -  li;
                    j = lj;
                }
                else
                {
                    const size_t ll = l_max - 1 - l;
					Order_t lli, llj;
					move(ll, lli, llj);

                    i = lli;
                    j = dim - llj;
                }

                const CElem_t old_code_i(code[i]);
                if (old_code_i < 0)
                    continue; // shortcut

                const CElem_t old_code_j(code[j]);
                if (old_code_j > 0)
                    continue; // shortcut

                // Calculate the cost after incrementing dimension i
                // and decrementing dimension j.
                const Cost_t new_cost = cost
                                - (dim + 1) * (old_code_i - old_code_j + 1)
                                - j + i;

                if (seenCosts.pushUniqueSmall(-new_cost))
                {
                    ProbePoint* new_point = new ProbePoint
                    (
                        dim, code, i, j, l
                    );

                    // Add the new candidate to the queue.
                    queue.add(new_point, new_cost);
                }
            }
        }
    }

    // Delete any points left in the queue to release memory
    const size_t n = queue.size();
    for (size_t i = 0; i < n; i++)
    {
        ProbePoint*  probe_point;
        Cost_t       probe_cost;
        queue.poll(&probe_point, &probe_cost);
        delete probe_point;
    }
}



size_t AStarProbes::num_probes(Dim_t dim, NumShells_t num_shells)
{
    size_t num_zero_probes = AStarProbes::num_zero_probes(dim, num_shells);
    return (dim + 1) * num_zero_probes;
}


size_t AStarProbes::num_zero_probes(Dim_t dim, NumShells_t num_shells)
{
    if (num_shells > AStarProbes::MAX_NUM_SHELLS)
    {
        throw Error_invalid_num_shells;
    }

    if ((int)dim > (int)num_shells)
    {
        dim = num_shells;
    }

    return PROBES_F[dim][num_shells - dim];
}


void AStarProbes::generate_probes
(
    Dim_t       dim,
    NumShells_t num_shells,
    CElem_t*    probes
)
{
    size_t num_probes = AStarProbes::num_probes(dim, num_shells);

    ProbeCollector collector (dim, num_probes, probes);

    generate_zero_probes(dim, num_shells, &collector);

    // Consistency check - confirm the expected number of probes
    if (!collector.correct_probes_collected())
    {
        throw Error_unknown; // unexpected number of probes
    }
}


///
/// Helper for generate_probe_diffs.
/// This returns the probe index for a probe from generate_probes
/// for a corresponding index, i, from generate_probe_diffs.
/// This effectively flips the order of every second orbit.
///
static inline size_t flipIdx(const size_t i, const size_t dimp, const size_t dimp2)
{
    size_t j = i % dimp2;
    return (j < dimp) ? i : i - j - j + dimp2 + dimp - 1;
}


size_t AStarProbes::size_probe_stream(Dim_t dim, size_t num_probes, const CElem_t* probes)
{
    // This algorithm is just a dry run through generate_probe_diffs.

    const size_t  dimp  = dim + 1;
    const size_t  dimp2 = dimp * 2;
    size_t        size  = 3 * (num_probes - 1); // initial account for STREAM_MARK and k entries

    for (size_t i = 1; i < num_probes; i++)
    {
        const size_t s = flipIdx(i - 1, dimp, dimp2);
        const size_t t = flipIdx(i, dimp, dimp2);

        const CElem_t* probeC_s = probes + s * dimp;
        const CElem_t* probeC_t = probes + t * dimp;

        for (Dim_t d = 0; d < dimp; ++d)
        {
            CElem_t diff(probeC_t[d] - probeC_s[d]);
            size += abs(diff);
        }
    }
    return size;
}


Order_t* AStarProbes::generate_probe_diffs
(
    Dim_t           dim,
    size_t          num_probes,
    const CElem_t*  probes,
    Order_t*        probe_diff_stream
)
{
    const size_t   dimp  = dim + 1;
    const size_t   dimp2 = dimp * 2;
    Order_t*       p_probe_diff_stream = probe_diff_stream;

    // A buffer used in the following loop.
    // A place to temporarily stack up positive increment column numbers.
    Order_t*            temp_cols = new Order_t[dimp + MAX_NUM_SHELLS];
	Deleter<Order_t[]>	delete_temp_cols(temp_cols);

    // Loop over probes, generating a stream of instructions for differences
    // per probe.
    for (size_t i = 1; i < num_probes; i++)
    {
        // i is our difference entry
        // s = the 1st source probe
        // t = the 2nd source probe
        const size_t s = flipIdx(i - 1, dimp, dimp2);
        const size_t t = flipIdx(i, dimp, dimp2);

        const CElem_t* probeC_s = probes + s * dimp;
        const CElem_t* probeC_t = probes + t * dimp;

        // Put the probe remainder value, k, into the stream.
        // This k calculation computes the remainder value of the
        // current probe point.
        *p_probe_diff_stream++ = (Order_t) (i % dimp2 < dimp ? i % dimp : dim - i % dimp);

        Order_t* pTempCols = temp_cols;
        for (Dim_t d = 0; d < dimp; ++d)
        {
            CElem_t diff = probeC_t[d] - probeC_s[d];
            if (diff < 0)
            {
                // Put negative columns straight into the stream.
                do
                {
                    *p_probe_diff_stream++ = d;
                    diff++;
                }
                while (diff < 0);
            }
            else if (diff > 0)
            {
                // Stack the positive columns up to add later.
                do
                {
                    *pTempCols++ = d;
                    diff--;
                }
                while (diff > 0);
            }
        }
        // Append the 'negative' terminator.
        *p_probe_diff_stream++ = STREAM_MARK;

        // Put the stacked up positive columns into the stream.
        for (Order_t* pPosCols = temp_cols; pPosCols < pTempCols; pPosCols++)
        {
            *p_probe_diff_stream++ = *pPosCols;
        }
        // Append the 'positive' terminator.
        *p_probe_diff_stream++ = STREAM_MARK;
    }

    return p_probe_diff_stream;
}
