/*
 * Functions for hashing.
 *
 * Author: Barry Drake
 */

#include "Hash.h"
#include <stdlib.h>


Hash Hash::POW_RADIX;


///
/// Constructor
///
Hash::Hash(Dim_t initial_dim)
{
	m_dim = initial_dim;
    m_x   = (Hash_t*) malloc(sizeof(Hash_t) * (m_dim + 1));
	if (!m_x)
	{
		throw Error_mem_fail;
	}

	m_x[0] = 1;
    for (Dim_t i = 1; i <= m_dim; ++i)
    {
        m_x[i] = m_x[i-1] * RADIX;
    }
}


Hash::~Hash(void)
{
	if (m_x)
	{
		free(m_x);
	}
}


const Hash_t* Hash::_powers(Dim_t dim)
{
	if (dim <= m_dim)
	{
		return m_x;
	}

    m_x = (Hash_t*) realloc(m_x, sizeof(Hash_t) * (dim + 1));
	if (!m_x)
	{
		throw Error_mem_fail;
	}


    for (Dim_t i = m_dim + 1; i <= dim; ++i)
    {
        m_x[i] = m_x[i-1] * RADIX;
    }
	m_dim = dim;
	
	return m_x;
}