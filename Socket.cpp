

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
    connected(false), sockFD(0)
{
    if(host != "")
    {
        struct hostent      *h_server;
        union {
        struct sockaddr_in  saddr_in;
        struct sockaddr     saddr; };
        if( (h_server = gethostbyname(host.c_str())) == NULL) {
            throw( SocketException("Failed to find host") );
        }

        saddr_in.sin_family = AF_INET;
        saddr_in.sin_port   = htons(port);
        saddr_in.sin_addr   = *((struct in_addr *) h_server->h_addr);

        connectFD(&saddr);;
    }
}
Socket::Socket(struct sockaddr *saddr) {
    connectFD(saddr);
}

Socket::~Socket()
{
    if(connected) {
        close(sockFD);
        sockFD=0;
    }
}

void Socket::createFD(void)
{
    if(sockFD != 0)
        throw SocketException("socket already created");
    if( (sockFD = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        throw( SocketException("Failed to create a socket") );
    }
}

void Socket::connectFD(struct sockaddr * saddr)
{
    if(connected)
        throw SocketException("already connected");
    if(sockFD == 0) {
        createFD();
    }

    if( connect( sockFD, saddr, sizeof(sockaddr)) < 0) {
        throw( SocketException("Failed to connect to socket") );
    }

    connected = true;
}

int Socket::output()
{
    int length;
    char buffer[1024];

    length = myBuf.str().length();

    const char *str = myBuf.str().c_str();
    memcpy(buffer, str, length);

    int retVal = send(sockFD, buffer, length, 0);

    if(retVal == length) {
        myBuf.ignore();
        myBuf.seekg(0, std::ios::beg);
    }

    return retVal;
}

int Socket::input()
{
    char buffer[4096];

    int retVal = recv(sockFD, buffer, 4096, 0);

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

    if( (sockFD = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        throw( SocketException("Failed to create a socket") );
    }
    if( bind( sockFD, (const sockaddr *)&servaddr, sizeof(servaddr) ) ) {
        throw( SocketException("Failed to bind socket") );
    }
    if( listen(sockFD, 5) < 0 )
    {
        throw( SocketException("Failed to listen on socket") );
    }
}

ListenSocket::~ListenSocket()
{
    if(connected)
    {
        close(sockFD);
        connected=false;
    }
}

Socket* ListenSocket::acceptConnection()
{
    Socket *s;
    int newFD;
    struct sockaddr addr;
    socklen_t len = 0;
    newFD = accept(sockFD, &addr, &len);
    s = new Socket(newFD);
    return s;
}
