/*
 * A simple vector index based on AStarNN hash codes with
 * STL unordered_map and vector.
 *
 * Author: Barry Drake
 */

#ifndef ASTARINDEX__H
#define ASTARINDEX__H

#include "common.h"
#include "AStarNN.h"
#include <unordered_map>
#include <vector>


/// A callback for index queries, AStarIndex::get... .
///
template <typename T>
class IndexCallback
{
public:
    /// Called for each element that matches the query.
    virtual void match(Hash_t hash_code, const T& elem) = 0;
};


/// A standard query callback - keep a store of the elements.
///
template <typename T>
class KeepElems : public IndexCallback<T>
{
public:
    /// Create a callback where the matching elements are
    /// stored in the array, elems.
    /// Assumes that 'match' will be called no more than max_size times.
    KeepElems(size_t max_size, T* elems)
        : m_start(elems)
        , m_cur(elems)
        , m_end(elems + max_size)
    {}

    virtual void match(Hash_t hash_code, const T& elem)
    {
        ASSERT(m_cur < m_end);
        *m_cur++ = elem;
    }

    /// How many elements are stored.
    inline size_t size(void) const
    {
        return m_cur - m_start;
    }

private:
    const T*    m_start;
    T*          m_cur;
    const T*    m_end;
};



template <typename T>
class AStarIndex
{
public:
    /// Create an AStarIndex.
    ///
    /// \param[in]  dim             number of dimensions in the lattice quantisation space, n.
    /// \param[in]  packing_radius  packing radius of the A* lattice.
    /// \param[in]  num_shells      number of extended shells for extended probes.
    ///
    AStarIndex(Dim_t dim, Distance_t packing_radius, NumShells_t num_shells);
    ~AStarIndex(void);

    /// Remove all elements (and hash codes) from the index.
    void clear(void);

    /// Put the given element into the index, indexed by the given vector.
    void put(const VElem_t* vector, const T& elem);

    /// Put the given elements into the index, indexed by the given vector.
    void put(const VElem_t* vector, size_t num_elements, const T* elems);

    /// Put the given elements into the index, indexed by the given vector.
    void put(const VElem_t* vector, const std::vector<T>& elems);

    /// Put the given element into the index, indexed by the given hash code.
    void put_hash(Hash_t hash_code, const T& elem);

    /// Put the given elements into the index, indexed by the given hash code.
    void put_hash(Hash_t hash_code, size_t num_elements, const T* elems);

    /// Put the given elements into the index, indexed by the given hash code.
    void put_hash(Hash_t hash_code, const std::vector<T>& elems);

    /// Call the given callback for each element found nearby to the
    /// given vector, using extended A* lattice probing.
    void get_extended(const VElem_t* vector, IndexCallback<T>* callback) const;

    /// How many elements are nearby to the
    /// given vector, using extended A* lattice probing.
    size_t count_extended(const VElem_t* vector) const;

    /// Call the given callback for each element stored with the
    /// given hash code.
    void get_hash(Hash_t hash_code, IndexCallback<T>* callback) const;

    /// How many elements stored with the given hash code.
    size_t count_hash(Hash_t hash_code) const;

    // Remove all element associated with the hash code of the given vector.
    void clear(const VElem_t* vector);

    // Remove all element associated with the given hash code.
    void clear_hash(Hash_t hash_code);

    /// Get the hash code for the given vector
    inline Hash_t hash(const VElem_t* vector) const
    {
        return m_hash.nearest_hash(vector);
    }

    /// Get the dimensionality of vectors processed by this index.
    inline Dim_t dim(void) const
    {
        return m_hash.dim();
    }

    /// Get the packing radius of the quantisation lattice.
    inline Distance_t packing_radius(void) const
    {
        return m_hash.packing_radius();
    }

    /// Get the packing internal scaling factor between the packing
    /// radius of the quantisation lattice and the native
    /// packing radius.
    inline Distance_t scale(void) const
    {
        return m_hash.scale();
    }

    /// Number of shells of lattice point beyond the Delaunay cell,
    /// that are used by 'get' queries.
    inline int num_shells(void) const
    {
        return m_hash.num_shells();
    }

    /// Number of probe points (hash codes) used by 'get' queries.
    inline size_t num_probes(void) const
    {
        return m_hash.num_probes();
    }

    /// Is the index empty.
    inline bool empty() const
    {
        return m_map.empty();
    }

    /// Get the number of distinct hash codes in the index.
    inline size_t num_hashes() const
    {
        return m_map.size();
    }

    /// Get the number of elements in the index.
    inline size_t num_elements() const
    {
        return m_num_elements;
    }

private:
    size_t                                      m_num_elements;
    AStarNN                                     m_hash;
    std::unordered_map<Hash_t, std::vector<T> > m_map;
};


//  Implementation

template <typename T>
AStarIndex<T>::AStarIndex(Dim_t dim, Distance_t packing_radius, NumShells_t num_shells)
    : m_hash(dim, packing_radius, num_shells)
    , m_num_elements(0)
{}


template <typename T>
AStarIndex<T>::~AStarIndex(void)
{}


template <typename T>
void AStarIndex<T>::clear(void)
{
    m_map.clear();
    m_num_elements = 0;
}

template <typename T>
void AStarIndex<T>::put(const VElem_t* vector, const T& elem)
{
    put_hash(hash(vector), elem);
}


template <typename T>
void AStarIndex<T>::put(const VElem_t* vector, size_t num_elements, const T* elems)
{
    put_hash(hash(vector), num_elements, elems);
}


template <typename T>
void AStarIndex<T>::put(const VElem_t* vector, const std::vector<T>& elems)
{
    put_hash(hash(vector), elems);
}


template <typename T>
void AStarIndex<T>::get_extended(const VElem_t* vector, IndexCallback<T>* callback) const
{
    class MyCallback : public QueryCallback_Hash
    {
    public:
        const AStarIndex<T>* m_self;
        IndexCallback<T>*    m_callback;

        MyCallback(const AStarIndex<T>* self, IndexCallback<T>* callback)
            : m_self(self)
            , m_callback(callback)
        {}

        void init(Dim_t dim, const VElem_t* mapped)
        {}

        void match(Hash_t hash_code)
        {
            m_self->get_hash(hash_code, m_callback);
        }
    }
    query_callback(this, callback);

    m_hash.extended_probes(vector, &query_callback);
}

template <typename T>
size_t AStarIndex<T>::count_extended(const VElem_t* vector) const
{
    class MyCallback : public QueryCallback_Hash
    {
    public:
        const AStarIndex<T>* m_self;
        size_t               m_count;;

        MyCallback(const AStarIndex<T>* self)
            : m_self(self)
            , m_count(0)
        {}

        void init(Dim_t dim, const VElem_t* mapped)
        {}

        void match(Hash_t hash_code)
        {
            m_count += m_self->count_hash(hash_code);
        }
    }
    query_callback(this);

    m_hash.extended_probes(vector, &query_callback);

    return query_callback.m_count;
}


template <typename T>
void AStarIndex<T>::clear(const VElem_t* vector)
{
    Hash_t hash_code = m_hash.nearest_hash(vector);
    clear_hash(hash_code);
}


template <typename T>
void AStarIndex<T>::put_hash(Hash_t hash_code, const T& elem)
{
    std::vector<T>& list(m_map[hash_code]);
    list.push_back(elem);
    m_num_elements += 1;
}


template <typename T>
void AStarIndex<T>::put_hash(Hash_t hash_code, size_t num_elements, const T* elems)
{
    if (num_elements > 0)
    {
        std::vector<T>& list(m_map[hash_code]);
        const T* end = elems + num_elements;
        for (; elems != end; ++elems)
        {
            list.push_back(*elems);
        }
        m_num_elements += num_elements;
    }
}



template <typename T>
void AStarIndex<T>::put_hash(Hash_t hash_code, const std::vector<T>& elems)
{
    auto it  = elems.begin();
    auto end = elems.end();

    if (it != end)
    {
        std::vector<T>& list(m_map[hash_code]);
        for (; it != end; ++it)
        {
            list.push_back(*it);
        }
        m_num_elements += elems.size();
    }
}


template <typename T>
void AStarIndex<T>::get_hash(Hash_t hash_code, IndexCallback<T>* callback) const
{
    auto found = m_map.find(hash_code);
    if (found != m_map.end())
    {
        const std::vector<T>& list(found->second);

        auto end = list.end();
        for (auto it(list.begin()); it != end; ++it)
        {
            callback->match(hash_code, *it);
        }
    }
}


template <typename T>
size_t AStarIndex<T>::count_hash(Hash_t hash_code) const
{
    auto found = m_map.find(hash_code);
    if (found != m_map.end())
    {
        const std::vector<T>& list(found->second);
        return list.size();
    }
    else
    {
        return 0;
    }
}


template <typename T>
void AStarIndex<T>::clear_hash(Hash_t hash_code)
{
    auto found = m_map.find(hash_code);
    if (found != m_map.end())
    {
        const std::vector<T>& list(found->second);
        m_num_elements -= list.size();
        m_map.erase(found);
    }
}



#endif // ASTARINDEX__H
