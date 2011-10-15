

extern "C"
{
#   include <sys/types.h>
#   include <sys/socket.h>
#   include <netinet/in.h>
#   include <netdb.h>

#   include <errno.h>
//#   include <stdio.h>
}


#include "Socket.h"

Socket::Socket()
{
    Socket("", 12543);
}

Socket::Socket(std::string host, uint16_t port)
{
    connected = false;
    sockFd = -1;

    if(host != "")
    {
        struct hostent      *h_server;
        struct sockaddr_in  servaddr;
        if( (h_server = gethostbyname(host.c_str())) == NULL)
        {
            std::cerr << "Failed to find host" << std::endl;
            return;
        }

        servaddr.sin_family = AF_INET;
        servaddr.sin_port   = htons(port);
        servaddr.sin_addr   = *((struct in_addr *) h_server->h_addr);

        if( (sockFd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
            std::cerr << "Failed to create a socket" << std::endl;
            return;
        }

        if( connect( sockFd, &servaddr, sizeof(servaddr)) < 0)
        {
            std::cerr << "Failed to connect to socket" << std::endl;
            close(sockFd);
            return;
        }

        connected = true;
    }
}

Socket::~Socket()
{
    if(connected)
    {
        close(sockFd);
    }
}

int Socket::send()
{

    return -1;
}

int Socket::recv()
{

    return -1;
}

