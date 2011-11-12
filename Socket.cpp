
#include <iostream>
#include <sstream>

#include <vector>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::vector;

extern "C"
{
#   include <sys/types.h>
#   include <sys/socket.h>
#   include <netinet/in.h>
#   include <netdb.h>

#   include <errno.h>
#   include <string.h>
#   include <stdio.h>
}

#include "Socket.h"

namespace cs6390
{

Socket::Socket(const Socket &copy)
{
    this->connected=copy.connected;
    this->sockFD=copy.sockFD;
}

Socket::Socket(string host, uint16_t port) :
    connected(false), sockFD(-1)
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
Socket::Socket(struct sockaddr *saddr) : connected(false), sockFD(-1) {
    connectFD(saddr);
}

Socket::~Socket()
{
    if(connected) {
        close(sockFD);
        sockFD=-1;
    }
}

void Socket::createFD(void)
{
    if(sockFD != -1)
        throw SocketException("socket already created");
    if( (sockFD = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        throw( SocketException("Failed to create a socket") );
    }
}

void Socket::connectFD(struct sockaddr * saddr)
{
    if(connected)
        throw SocketException("already connected");
    if(sockFD == -1) {
        createFD();
    }

    if( connect( sockFD, saddr, sizeof(sockaddr)) < 0) {
        throw( SocketException("Failed to connect to socket") );
    }

    connected = true;
}

int Socket::output()
{
    //don't forget to add 1 for \0
    int length = myBuf.str().length()+1;
    cerr << "Sending: '" << myBuf.str() << "'" << endl;
    int retVal = send(sockFD, myBuf.str().c_str(), length , 0);

    if(retVal == length) {
        myBuf.ignore();
        myBuf.seekg(0, std::ios::beg);
    }
    else if (retVal < 0) {
        std::stringstream ss;
        ss << "error sending: " << errno;
        perror("send()");
        throw SocketException(ss.str());
    }
    else {
        cerr << "Odd send...  expected: " << length  << " sent: " << retVal << endl;
    }

    return retVal;
}

int Socket::input()
{
    char buffer[4096];

    if (!connected)
    {
        cerr << __FILE__"," << __LINE__ << endl;
        throw(NotConnectedException() );
    }

    int retVal = recv(sockFD, buffer, 4096, 0);

    if(retVal > 0) {
        myBuf.str(buffer);
    }
    else if(retVal < 0)
    {
        strerror_r(errno, buffer, 4096);
        throw SocketException( string("Error in recv()") + buffer);
    }
    if (retVal == 0) {
        connected=false;
        close(sockFD);
        sockFD=-1;
        cerr << __FILE__"," << __LINE__ << endl;
        throw NotConnectedException();
    }
    
    return retVal;
}

int Socket::sendMessage(Message &m)
{
    myBuf.str( m.serialize() );
    return output();
}

Message * Socket::getMessage()
{
    Message * m = NULL;


    if(input() <= 0)
        return NULL;

    vector<string> v;
    string save(myBuf.str());
    boost::split(v, save, boost::is_any_of(" \t\r\n"), boost::algorithm::token_compress_on );
    if(v[0] == "LSA")
        m = new RouterStatus(v);
    else if(v[0] == "RREQ")
        m = new RREQ(v);
    else if(v[0] == "BGP" )
        m = new BGP(v);
    else
        cerr << "Unknown message type: " << v[0] << endl;

    return m;
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

Socket ListenSocket::acceptConnection()
{
    int newFD;
    struct sockaddr addr;
    socklen_t len = 0;
    newFD = accept(sockFD, &addr, &len);
    Socket s(newFD);
    return s;
}

} //cs6390
