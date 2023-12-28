/*
 * Manage our own stack of working buffers.
 *
 * Author: Barry Drake
 */
#ifndef WORKBUFF__H
#define WORKBUFF__H


#include "common.h"



class WorkBuff
{
public:
	
	///
	/// Get the next working buffer.
	/// Returns 0 if no more buffers are available.
	///
	inline WorkBuff* next(void)
	{
		return m_next;
	}

	///
	/// Get the memory buffer, which is a array of (dim + 1) elements of type T.
	/// 'T' can only be one of the implemented types:
	///		VElem_t, CElem_t, Order_t, Hash_t.
	///
	/// The recommended idiom get the buffer is:
	///     T* buffer = get_buff(work_buff);
	///
	template<typename T>
	T* get(void);

private:
friend class BuffStack;

	///
	/// The buffer is suitable for any of these types of elements.
	///
	typedef union {VElem_t v; CElem_t c; Order_t o; Hash_t h;} Elem;

	///
	/// Return the sizeof a WorkBuff configures for the given dimensionality.
	/// I.e., the equivalent number of chars.
	///
	static size_t size(Dim_t dim)
	{
		// This is the total number of chars to store the overhead
		// field (m_next) and dim + 1 elements in the buffer.
		size_t size = sizeof(WorkBuff) + dim * sizeof (Elem);

		// Add padding as needed to allign memory with the word size.
		size_t w = sizeof(void*);
		size = ((size + w - 1) / w) * w;

		return size;
	}
	
	WorkBuff*	m_next;
	Elem		m_elems[1];
};


template<>
inline VElem_t* WorkBuff::get<VElem_t>(void)
{
	return &m_elems[0].v;
}

template<>
inline CElem_t* WorkBuff::get<CElem_t>(void)
{
	return &m_elems[0].c;
}

template<>
inline Order_t* WorkBuff::get<Order_t>(void)
{
	return &m_elems[0].o;
}

template<>
inline Hash_t* WorkBuff::get<Hash_t>(void)
{
	return &m_elems[0].h;
}




///
/// Get the buffer from the given work_buff
/// and advance work_buff to be work_buff->next().
/// This will throw if work_buff is 0.
///
template<typename T>
inline T* get_buff(WorkBuff*& work_buff)
{
	if (work_buff)
	{
		T* result = work_buff->get<T>();
		work_buff = work_buff->next();
		return result;
	}
	else
	{
		throw Error_insufficient_buffers;
	}
}


///
/// Allocate memory for a stack of WorkBuff objects.
///
class BuffStack
{
public:
	BuffStack(Dim_t dim, size_t num_buffers);

	~BuffStack(void);

	///
	/// Get the first WorkBuff in the stack.
	/// Use get_buff to access subsequent buffers in the stack.
	///
	inline WorkBuff* buff(void)
	{
		return m_buffers;
	}

private:
	WorkBuff* m_buffers;
};


#endif // WORKBUFF__H
