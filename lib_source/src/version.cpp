/*
 * Keep track of the development version.
 *
 * Author: Barry Drake
 */

#include "version.h"
#include <string>

using namespace std;

#define COPYRIGHT_YEAR  (__DATE__ + 7)
#define COPYRIGHT_OWNER "Barry Drake"
#define LIBRARY_NAME    "AStarNN"

#if NDEBUG
static bool ndebug(true);
#else
static bool ndebug(false);
#endif


static inline const char* to_string(bool value)
{
    return value ? "true" : "false";
}


Version::Version()
    : m_name(LIBRARY_NAME)
    , m_compile_date(__DATE__)
    , m_compile_time(__TIME__)
    , m_debug(!ndebug)
    , m_copyright(string("copyright (c) ") + COPYRIGHT_YEAR + " " + COPYRIGHT_OWNER + ", all rights reserved")
{
    m_info = m_name + ", compiled " + m_compile_date + ", " + m_compile_time;
    if (m_debug)
    {
        m_info += " (debug)";
    }
    m_info += ", (c)";
    m_info += COPYRIGHT_YEAR;

    m_extended_info =
        m_info + "\n" +
        m_copyright + "\n"
    ;
}


Version Version::VERSION;
