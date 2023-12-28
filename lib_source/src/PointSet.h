/*
 * Manage a set of lattice points.
 *
 * Author: Barry Drake
 */
#ifndef POINTSET_H
#define POINTSET_H

#include "common.h"
#include "AStarLattice.h"
#include "Hash.h"
#include <cstring>
#include <new>

///
/// A set of lattice points, each lattice point is represented by c-vector.
///
class PointSet
{
public:

    PointSet(size_t capacity)
        : m_size(0)
        , m_capacity(capacity)
    {
        m_mem_size       = (size_t)power_of_two(capacity << 1);
        m_mask           = m_mem_size - 1;
        m_entries_buffer = new PointSetEntry[capacity];
        m_hash_table     = new PointSetEntry*[m_mem_size];
        clear();
    }


    ~PointSet(void)
    {
        delete [] m_hash_table;
        delete [] m_entries_buffer;
    }

    ///
    /// Make the point set empty
    ///
    void clear(void)
    {
        memset(m_hash_table, 0, m_mem_size * sizeof (PointSetEntry*));
        m_size = 0;
    }


    ///
    /// Insert a lattice point c-vector into the set.
    ///
    /// \returns true if it was a new element added.
    ///
    bool insert(Dim_t dim, const CElem_t* c)
    {
        Hash_t hashCode = Hash::hash(dim, c);
        size_t idx      = hashCode & m_mask;

        //
        // See if it's in the hash already.
        //
        PointSetEntry* matching = m_hash_table[idx];
        while (matching != NULL)
        {
            if (memcmp(matching->m_c, c, (dim + 1) * sizeof(CElem_t)) == 0)
            {
                return false;
            }
            matching = matching->m_next;
        }

        // Add the new entry to the hash.
        if (m_size >= m_capacity)
        {
            throw Error_unknown; // unexpectedly out of capacity
        }

        // Placement new, uses the existing allocation
        PointSetEntry& new_entry = m_entries_buffer[m_size];
		new_entry.set(dim, c,  m_hash_table[idx]);
        ++m_size;
        m_hash_table[idx] = &new_entry;
        return true;
    }


private:
    struct PointSetEntry
    {
	public:
		PointSetEntry(void)
			: m_c(0)
			, m_next(0)
		{}

		~PointSetEntry(void)
		{
			delete [] m_c;
		}

		void set(Dim_t dim, const CElem_t* c, PointSetEntry* next)
        {
			if(m_c == 0)
			{
				m_c = new CElem_t[dim + 1];
			}
            memcpy(m_c, c, sizeof(CElem_t) * (dim + 1));
			m_next = next;
        }

        CElem_t*       m_c;
        PointSetEntry* m_next;
    };

    ///
    /// Find the smallest power of 2 greater than or equal to the input.
    /// Returns x, such that x/2 < val <= x and x = 2^i and i is
    /// an integer. (Note, val == 0 <=> x == 0.)
    ///
    static inline uint64_t power_of_two(uint64_t val)
    {
        // In 14 operations, this code computes the next highest
        // power of 2 for a 64-bit integer. 
        --val;
        val |= val >> 1;
        val |= val >> 2;
        val |= val >> 4;
        val |= val >> 8;
        val |= val >> 16;
        val |= val >> 32;
        ++val;
        return val;
    }

    PointSetEntry*          m_entries_buffer;
    PointSetEntry**         m_hash_table;
    size_t                  m_size;
    size_t                  m_capacity;
    size_t                  m_mem_size;
    size_t                  m_mask;
};


#endif // POINTSET_H
