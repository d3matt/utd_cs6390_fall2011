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
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>

#include <boost/version.hpp>

#if BOOST_VERSION == 103301
BOOST_CLASS_TRACKING(TestClass, boost::serialization::track_never);
#endif /* BOOST_VERSION */

class Socket : public std::ostream
{
private:
    // Vars:
    std::stringstream myBuf;

    bool connected;
    int sockFd;
    
    //Functions:

    int send();
    int recv();


public:

    Socket();
    Socket(std::string host, uint16_t port);
    ~Socket();
    
    template <class T>
    Socket &operator<< (T &t)
    {
        boost::archive::binary_oarchive boa(myBuf);
        boa << t;

        return *this;
    }

    template <class T>
    Socket &operator>> (T &t)
    {
        boost::archive::binary_iarchive bia(myBuf);

        bia >> t;

        return *this;
    }

};

#endif /* _SOCKET_H */

