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

#define THROW_NC throw NotConnectedException(__FILE__,__LINE__)
#define THROW_SE(msg) throw SocketException(msg,__FILE__,__LINE__)

namespace cs6390
{

//simple wrapper around BSD tcp sockets
//  basic idea is to write the code to deal with opening/closing sockets once
//  uses children of Message for send/recv
//  as implemented will only work with ascii text
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

    //socket exceptions
    struct NotConnectedException : public easyException {
        NotConnectedException() : easyException("Not Connected") {}
        NotConnectedException(const char * f, uint32_t l) : easyException("Not Connected", f, l) {}
    } ;
    struct SocketException : public easyException {
        SocketException(std::string s) : easyException(s) {}
        SocketException(std::string msg, const char * f, uint32_t l) : easyException(msg, f, l) {}
    } ;

    //can be used to send arbitrary strings
    void sendToSocket(std::string str)
    {
        myBuf.str(str);
        output();
    }

    //get and recv a Message
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

