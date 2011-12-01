#include <exception>
#include <string>

#ifndef __EXCEPTIONS_H__
#define __EXCEPTIONS_H__

#define THROW_DES(msg) throw DeserializationException(msg,__FILE__,__LINE__)

//simple wrapper around std::exception to make it easy to throw various exceptions
class easyException : public std::exception
{
public:
    std::string s;
    virtual const char* what() const throw()
    {
        //if exception makes it to the user, s is printed
        return s.c_str();
    }
    easyException(): s("") {}

    //construct an exception from a string
    easyException(std::string s) : s(s) {}

    //construct an exception from a string, but include __FILE__ and __LINE__
    easyException(std::string msg, const char * f, uint32_t l)
    {
        std::stringstream ss;
        ss << msg << " (" << f << ":" << l << ")";
        s=ss.str();
    }
    ~easyException() throw() {}
};

//other exceptions are derived like this...
class DeserializationException : public easyException {
public:
    DeserializationException(std::string s) : easyException(s) {}
    DeserializationException(std::string s, const char * f, uint32_t l) : easyException(s,f,l) {}
};

#endif // __EXCEPTIONS_H__
