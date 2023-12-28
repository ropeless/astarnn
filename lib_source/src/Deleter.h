/*
 * Automatic deletion of pointers.
 *
 * Author: Barry Drake
 */
#ifndef DELETER_H
#define DELETER_H

///
/// Calls delete on a pointer when the Deleter goes out of scope.
///
/// This is similar to std::auto_ptr, except that Deleter will work with
/// array types. It is also similar to std::unique_ptr, which is not available
/// in C++2003.
///
/// Usage:
/// <pre>
/// {
///     T*         myT = new T();
///     Deleter<T> myT_deleter(myT);
///
///     ...use myT...
///
/// }   // 'delete myT' is called
/// </pre>
///
/// Usage for arrays:
/// <pre>
/// {
///     T*           myT = new T[1234];
///     Deleter<T[]> myT_deleter(myT);
///
///     ...use myT...
///
/// }   // 'delete [] myT' is called
/// </pre>
///
template<class T>
class Deleter
{
public:
    Deleter(T*& ptr)
        : m_Ptr(ptr)
    {}

    ~Deleter(void)
    {
        delete m_Ptr;
        m_Ptr = 0;
    }
private:
    // Copy and assignment constructors not implemented
    Deleter (const Deleter &obj);
    Deleter& operator = (const Deleter &obj);

    T*& m_Ptr;
};


///
/// The array specialised version of Deleter.
///
template<class T>
class Deleter<T[]>
{
public:
    Deleter(T*& ptr)
        : m_Ptr(ptr)
    {}

    ~Deleter(void)
    {
        delete [] m_Ptr;
        m_Ptr = 0;
    }
private:
    // Copy and assignment constructors not implemented
    Deleter (const Deleter &obj);
    Deleter& operator = (const Deleter &obj);

    T*& m_Ptr;
};


#endif // DELETER_H
