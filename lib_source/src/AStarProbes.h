/*
 * Functions for A* lattice probing.
 *
 * Author: Barry Drake
 */

#ifndef ASTARPROBES__H
#define ASTARPROBES__H

#include "common.h"


///
/// This is just a name space for A* lattice probing functions.
///
class AStarProbes
{
public:

    ///
    /// The maximum number of (extended) shells.
    /// This is limited by the number of precalculated
    /// the count of remainder-zero probes.
    ///
    static const NumShells_t MAX_NUM_SHELLS = 30;

    ///
    /// A sentinel value used in a probe diff stream.
    /// See generate_probe_diffs.
    ///
    static const Order_t    STREAM_MARK = -1;

    ///
    /// num_probes(dim, num_shells) is the number of probes for num_shells
    /// extended shells and for dim dimensions.
    ///
    /// num_probes(dim, num_shells) = num_zero_probes(dim, num_shells) * (dim + 1).
    ///
    static size_t num_probes(Dim_t dim, NumShells_t num_shells);

    ///
    /// num_zero_probes(dim, num_shells) is the number of remainder-zero probes
    /// for num_shells extended shells and for dim dimensions.
    ///
    /// num_probes(dim, num_shells) = num_zero_probes(dim, num_shells) * (dim + 1).
    ///
    /// This is the same as the number of "orbits".
    ///
    static size_t num_zero_probes(Dim_t dim, NumShells_t num_shells);


    ///
    /// Generate the probes used by the extended Delaunay probing method.
    ///
    /// The generated probes will be placed in memory pointed to by probes.
    /// Each probe is a (dim + 1) dimensional vector, with each element of
    /// integer type (CElem_t). Each probes is a c-vector for an A* lattice
    /// point.
    ///
    /// The probes are arranged in blocks of dim + 1 probes.
    /// The probes of each block are sorted so that the first probe
    /// of the block is remainder-0, the second is remainder-1 and so on
    /// up to remainder-dim. Each block represents an "orbit" and thus
    /// the probes of a block all belong to the same shell. Thus the
    /// remainder value, k, for a probe at index i is given by:
    /// k = i % (dim + 1).
    ///
    /// The blocks are sorted in shell order. Thus the first block is
    /// shell zero.
    ///
    /// As a consequence the first probe is the remainder-0 lattice point
    /// of the zeroth shell, hence the first probe is always at the origin
    /// (i.e. all coordinates are zero).
    ///
    /// \param[in]  dim         number of dimensions, n.
    /// \param[in]  num_shells  number of extended shells.
    /// \param[out] probes      pointer to store c-vectors of probes.
    ///
    static void generate_probes
    (
        Dim_t       dim,
        NumShells_t num_shells,
        CElem_t*    probes
    );


    ///
    /// Caclulate the size needed for a diff probe stream.
    /// See method generate_probe_diffs.
    ///
    static size_t size_probe_stream
    (
        Dim_t          dim,
        size_t         num_probes,
        const CElem_t* probes
    );


    ///
    /// Generate a 'diff' representation of the given probes.
    ///
    /// The probes are represented by a differences between pair of adjacent
    /// probes. A stream of instructions describe how to change one probe into
    /// the next.
    ///
    /// probe_diff_stream format:
    ///      The following pattern repeats for every probe, except for probe 0
    ///      which has no C+/C- entries and so is not represented. 
    ///      |k|C-|...|F|C+|...|F|
    ///      where:
    ///          k  is the remainder value for the probe lattice point.
    ///          C- is a dimension requiring to be decremented by one unit.
    ///          C+ is a dimension requiring to be incremented by one unit.
    ///          F  is a sentinel marker (STREAM_MARK) for end of each C+
    ///             or C- repeating sections.
    ///
    /// probe_diff_stream should have allocated least size_probe_stream(...) number
    /// of element.
    ///
    /// In this representation, every second block is reverse order, which
    /// makes small the number of differences between any two adgacent probes.
    ///
    /// \param[in]  dim               number of dimensions.
    /// \param[in]  num_probes        number of probes in probes.
    /// \param[in]  probes            precomputed probes, an array of size numProbes * (dim + 1).
    /// \param[out] probe_diff_stream pointer to store difference instructions.
    /// \returns a pointer to one element beyond the end of the of the probe_diff_stream.
    ///
    static Order_t* generate_probe_diffs
    (
        Dim_t           dim,
        size_t          num_probes,
        const CElem_t*  probes,
        Order_t*        probe_diff_stream
    );

private:
    // constructor and destructor not implemented
    AStarProbes(void);
    ~AStarProbes(void);

};


#endif // ASTARPROBES__H
