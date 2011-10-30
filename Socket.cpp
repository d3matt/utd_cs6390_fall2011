

extern "C"
{
#   include <sys/types.h>
#   include <sys/socket.h>
#   include <netinet/in.h>
#   include <netdb.h>

#   include <errno.h>
#   include <string.h>
}


#include "Socket.h"

Socket::Socket(std::string host, uint16_t port) :
    connected(false), sockFd(-1)
{
    if(host != "")
    {
        struct hostent      *h_server;
        struct sockaddr_in  servaddr;
        if( (h_server = gethostbyname(host.c_str())) == NULL) {
            throw( SocketException("Failed to find host") );
        }

        servaddr.sin_family = AF_INET;
        servaddr.sin_port   = htons(port);
        servaddr.sin_addr   = *((struct in_addr *) h_server->h_addr);

        if( (sockFd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            throw( SocketException("Failed to create a socket") );
        }

        if( connect( sockFd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
            throw( SocketException("Failed to connect to socket") );
        }

        connected = true;
    }
}

Socket::~Socket()
{
    if(connected)
        close(sockFd);
}

int Socket::output()
{
    int length;
    char buffer[1024];

    length = myBuf.str().length();

    const char *str = myBuf.str().c_str();
    memcpy(buffer, str, length);

    int retVal = send(sockFd, buffer, length, 0);

    if(retVal == length) {
        myBuf.ignore();
        myBuf.seekg(0, std::ios::beg);
    }

    return retVal;
}

int Socket::input()
{
    char buffer[4096];

    int retVal = recv(sockFd, buffer, 4096, 0);

    if(retVal > 0) {
        myBuf.str(buffer);
    }
    
    return retVal;
}

ListenSocket::ListenSocket(uint16_t port)
{
    struct sockaddr_in  servaddr;
    servaddr.sin_family         = AF_INET;
    servaddr.sin_port           = htons(port);
    servaddr.sin_addr.s_addr    = htonl(INADDR_ANY);

    if( (sockFd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        throw( SocketException("Failed to create a socket") );
    }
    if( bind( sockFd, (const sockaddr *)&servaddr, sizeof(servaddr) ) ) {
        throw( SocketException("Failed to bind socket") );
    }
}
