/*
 * Manage our own stack of working buffers.
 *
 * Author: Barry Drake
 */

#include "WorkBuff.h"

#include <stdlib.h>


BuffStack::BuffStack(Dim_t dim, size_t num_buffers)
	: m_buffers(0)
{
	if (num_buffers <= 0)
	{
		throw Error_unknown;
	}

	size_t size = WorkBuff::size(dim);
	char*  mem  = static_cast<char*>(malloc(size * num_buffers));
	if (!mem)
	{
		throw Error_mem_fail;
	}
	m_buffers = (WorkBuff*) mem;

	// initize the links
	WorkBuff* last = m_buffers;
	for (size_t i = 1; i < num_buffers; ++i)
	{
		last->m_next = reinterpret_cast<WorkBuff*>(mem + size * i);
		last = last->m_next;
	}
	last->m_next = 0;
}

BuffStack::~BuffStack(void)
{
	if (m_buffers)
	{
		free(m_buffers);
	}
}
