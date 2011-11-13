#ifndef __SOCKET_H__
#define __SOCKET_H__

#include <stdint.h>
#include <iostream>
#include <sstream>
#include <string>
#include "Exceptions.h"
#include "Message.h"

#ifdef PROJ_DEBUG
#define SOCKET_DEBUG
#endif

namespace cs6390
{

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
    Socket(const Socket &copy);
    Socket(std::string host, uint16_t port);
    Socket(int sockFD) : connected(true), sockFD(sockFD) {}
    Socket(struct sockaddr * psaddr);
    ~Socket();
    bool isConnected() { return connected; }

    struct NotConnectedException : public easyException {
        NotConnectedException() : easyException("Not Connected") {}
    } ;
    struct SocketException : public easyException {
        SocketException(std::string s) : easyException(s) {}
    } ;

    void sendToSocket(std::string str)
    {
        myBuf.str(str);
        output();
    }

    int                 sendMessage(Message &m);
    Message *           getMessage();

    friend std::ostream & operator<< (std::ostream &ostr, Socket s)
    {
        if(s.connected)
            ostr << "Connected with FD: " << s.sockFD;
        else
            ostr << "Not Connected";
        return ostr;
    }
};

class ListenSocket : public Socket
{
public:
    ~ListenSocket();
    ListenSocket(uint16_t port);
    Socket acceptConnection();
};

} //cs6390

#endif /* __SOCKET_H__ */

