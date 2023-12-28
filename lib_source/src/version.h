/*
 * Keep track of the development version.
 *
 * Author: Barry Drake
 */
#ifndef VERSION__H
#define VERSION__H

#include <string>


class Version
{
public:

    /// Return a simple string (no newlines) to give information about this library.
    static inline const char* info()
    {
        return VERSION.m_info.c_str();
    }

    /// Return a multiline string giving extended information about this library.
    static const char* extended_info()
    {
        return VERSION.m_extended_info.c_str();

    }
    
private:
    static Version VERSION;

    Version();

    const std::string m_name;
    const std::string m_compile_date;
    const std::string m_compile_time;
    const bool        m_debug;
    const std::string m_copyright;

    std::string m_info;
    std::string m_extended_info;
};



#endif // VERSION__H
