#include <exception>
#include <string>

#ifndef __EXCEPTIONS_H__
#define __EXCEPTIONS_H__

class easyException : public std::exception
{
public:
    std::string s;
    virtual const char* what() const throw()
    {
        return s.c_str();
    }
    easyException(): s("") {}
    easyException(std::string s) : s(s) {}
    easyException(std::string msg, const char * f, uint32_t l)
    {
        std::stringstream ss;
        ss << msg << " (" << f << ":" << l << ")";
        s=ss.str();
    }
    ~easyException() throw() {}
};

class DeserializationException : public easyException {
public:
    DeserializationException(std::string s) : easyException(s) {}
};

#endif // __EXCEPTIONS_H__
