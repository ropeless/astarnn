/*
 * A set of 'smallest costs'.
 *
 * Author: Barry Drake
 */

#ifndef COSTSET__H
#define COSTSET__H

#include "common.h"
#include <limits>

///
/// A set of smallest costs.
///
/// Inserting a cost into the set will push our a larger cost
/// if the size of the set is already 'num_to_keep'.
///
template<typename CostType>
class CostSet
{
public:
    inline CostSet(size_t num_to_keep)
        : m_size(num_to_keep)
        , m_heap(0)
    {
        if (num_to_keep < 1)
        {
            throw Error_unknown; // numToKeep must be positive.
        }

        m_heap = new CostType[num_to_keep + 1];

        // Pre-fill with largest possible number.
        const CostType upper = std::numeric_limits<CostType>::max();
        for (size_t i = 1; i <= num_to_keep; ++i)
        {
            m_heap[i] = upper;
        }
    }


    ~CostSet(void)
    {
        delete [] m_heap;
    }


    ///
    /// Record the new number as a seen number.
    /// Returns true if the given number is included in
    /// the set of kept smallest seen numbers.
    ///
    inline bool pushUniqueSmall(const CostType new_cost)
    {
        bool in_set = false;

        // New number smaller than biggest, so may be added if it's unique.
        const CostType largest = m_heap[1];
        if (new_cost < largest)     
        {
            // newNumber may be in the set already.
            for (size_t i = 1; i <= m_size; ++i)
            {
                if (new_cost == m_heap[i])
                {
                    // Already in there. Let them know.
                    in_set = true;
                    break;
                }
            }

            // Not in there. Replace largest with newNumber.
            if (!in_set)
            {
                replaceLargest(new_cost);
                in_set = true;
            }
        } 
        else
        {
            in_set = (new_cost == largest);
        }
        return in_set;
    }

private:

    inline void replaceLargest(const CostType new_cost)
    {
        // Imagine new_cost in newly vacant position 1 (a).
        size_t a = 1;
        size_t b = 2;
        size_t c = 3;

        while (b <= m_size)
        {
            if (new_cost < m_heap[b])
            {
                if (c <= m_size && m_heap[b] < m_heap[c])
                {
                    m_heap[a] = m_heap[c];
                    a = c;
                }
                else
                {
                    m_heap[a] = m_heap[b];
                    a = b;
                }
            }
            else if (c <= m_size && new_cost < m_heap[c])
            {
                m_heap[a] = m_heap[c];
                a = c;
            }
            else
            {
                break;
            }
            b = a << 1;
            c = b + 1;
        }
        m_heap[a] = new_cost;
    }

    // Heap of seen costs, based off element 1 for ease of computation.
    CostType*    m_heap;

    // Number of costs to keep.
    const size_t m_size;
};


#endif // COSTSET__H
