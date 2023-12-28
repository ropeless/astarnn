/*
 * A prioirity queue implementation.
 *
 * Author: Barry Drake
 */

#ifndef PRIORITYQUEUE__H
#define PRIORITYQUEUE__H

#include "common.h"
#include <stdlib.h>

///
/// A PriorityQueue<P, T> keeps a queue of T pointers, ordered by
/// prioirity of type P, which is provided at insertion time.
///
template<typename P, typename T>
class PriorityQueue
{
private:
    struct Elem
    {
        Elem(T* dataIn, P priorityIn)
        : data(dataIn)
        , priority(priorityIn)
        {}
        
        T* data;
        P  priority;
    };

    Elem*   m_data;
    size_t  m_size;
    size_t  m_alloc;

public:
    PriorityQueue(size_t allocation = 1024)
    {
        m_size  = 0;
        m_alloc = allocation;

        // we use 'malloc' instead of 'new' so that we can 'realloc'.
        m_data  = (Elem*) malloc (sizeof(Elem) * m_alloc);
        if (!m_data)
        {
            throw Error_mem_fail;
        }
    }
    

   ~PriorityQueue(void)
    {
        if (m_data)
        {
            free(m_data);
        }
    }


    inline void add(T* to_add, P priority)
    {
        Elem     key(to_add, priority);
        size_t   i;
        size_t   parent;
    
        m_size++;
    
        if (m_size > m_alloc)
        {
            m_alloc  = m_alloc * 2;
            Elem* new_data = (Elem*) realloc(m_data, sizeof(Elem) * m_alloc);
            if (!new_data)
            {
                throw Error_mem_fail;
            }
            m_data = new_data;
        }
    
        i = m_size - 1;
        m_data[i] = key;
        parent = (i - 1)/2;
    
        while (i > 0 && m_data[i].priority > m_data[parent].priority)
        {
            Elem  tmp = m_data[i];
            m_data[i] = m_data[parent];
            m_data[parent] = tmp;
        
            i = parent;
            parent  = (i - 1)/2;
        }
    }
    

    inline void poll(T** pObj, P* pPriority)
    {
        if (size() > 0)
        {
            // set the return value
            *pObj      = m_data[0].data;
            *pPriority = m_data[0].priority;
   
            // decrement the size
            m_size--;
    
            // swap in the tail
            m_data[0] = m_data[m_size];
    
            // reheapify
            size_t i = 0;
            while (i*2 + 1 < m_size)
            {
                const P  left_priority = m_data[i*2+1].priority;
                size_t   swap_to       = 0;
        
                if (i*2 + 2 < m_size)
                {
                    const P right_priority = m_data[i*2+2].priority;
                    if (left_priority >= right_priority && left_priority > m_data[i].priority)
                    {
                        swap_to  = i*2 + 1;
                    }
                    else if (right_priority >= left_priority && right_priority > m_data[i].priority)
                    {
                        swap_to  = i*2 + 2;
                    }
                    else
                    {
                        break;
                    }
                }
                else
                {
                    // right_priority = std::numeric_limits<P>::lowest();
                    if (left_priority > m_data[i].priority)
                    {
                        swap_to  = i*2 + 1;
                    }
                    else
                    {
                        break;
                    }
                }
        
                Elem tmp = m_data[i];
                m_data[i] = m_data[swap_to];
                m_data[swap_to] = tmp;
        
                i = swap_to;
            }
        }
        else
        {
            throw Error_unknown; // unexpected empty queue.
        }
    }
    

    inline size_t size(void) const
    {
        return m_size;
    }
    

    inline P head_priority(void) const
    {
        ASSERT(size() > 0);
        return m_data[0].priority;
    }


    inline T* head(void)
    {
        ASSERT(size() > 0);
        return m_data[0].data;
    }
    

    inline const T* head(void) const
    {
        ASSERT(size() > 0);
        return m_data[0].data;
    }
};


#endif // PRIORITYQUEUE__H
