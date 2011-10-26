#ifndef _SOCKET_H
#define _SOCKET_H

extern "C"
{
#   include<stdint.h>
}

#include <iostream>
#include <sstream>
#include <string>

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

    template <typename T>
    void sendToSocket(T *t)
    {
        myBuf.str(t->toString());
        output();
    }

    template <typename T>
    Socket & operator<< (T *t)
    {
        myBuf << t->toString();
        output();
        return (*this);
    }

    std::string receiveFromSocket()
    {
        input();
        return myBuf.str();
    }

};

#endif /* _SOCKET_H */

