#ifndef __SOCKET_H__
#define __SOCKET_H__

#include <stdint.h>
#include <iostream>
#include <sstream>
#include <string>
#include "Exceptions.h"

class Socket
{
protected:
    // Vars:
    std::stringstream myBuf;

    bool connected;
    int sockFD;
    
    //Functions:
    void createFD(void);
    void connectFD(struct sockaddr * psaddr);
    int output();
    int input();

public:
    Socket() {}
    Socket(std::string host, uint16_t port);
    Socket(int sockFD) : connected(true), sockFD(sockFD) {}
    Socket(struct sockaddr * psaddr);
    ~Socket();

    struct SocketException : public easyException {
        SocketException(std::string s) : easyException(s) {}
    } ;

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

class ListenSocket : public Socket
{
public:
    ListenSocket(uint16_t port);
    Socket accept();
};

#endif /* __SOCKET_H__ */

