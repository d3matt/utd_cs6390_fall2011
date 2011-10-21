#ifndef _SOCKET_H
#define _SOCKET_H

extern "C"
{
#   include<stdint.h>
}

#include <iostream>
#include <sstream>
#include <string>

/* Boost headers */
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

class Socket
{
private:
    // Vars:
    std::stringstream myBuf;

    bool connected;
    int sockFd;
    
    //Functions:

    int output();
    int input();


public:

    Socket();
    Socket(std::string host, uint16_t port);
    ~Socket();
    
    template <class T>
    Socket &operator<< (T *t)
    {
        myBuf << t;
        output();
        return *this;
    }

    template <class T>
    Socket &operator>> (T *t)
    {
        input();

        myBuf.str() >> t;
        return *this;
    }

};

#endif /* _SOCKET_H */

