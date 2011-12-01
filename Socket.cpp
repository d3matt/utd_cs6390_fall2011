
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

//copy constructor
Socket::Socket(const Socket &copy)
{
    this->connected=copy.connected;
    this->sockFD=copy.sockFD;
}

//constructor from host:port
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
            THROW_SE("Failed to find host");
        }

        saddr_in.sin_family = AF_INET;
        saddr_in.sin_port   = htons(port);
        saddr_in.sin_addr   = *((struct in_addr *) h_server->h_addr);

        connectFD(&saddr);;
    }
}

//constructor with struct sockaddr
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

//open the socket
//  throw on failure
void Socket::createFD(void)
{
    if(sockFD != -1)
        THROW_SE("socket already created");
    if( (sockFD = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        THROW_SE("Failed to create a socket");
    }
}

//connect the socket
//  thrown on failure
void Socket::connectFD(struct sockaddr * saddr)
{
    if(connected)
        THROW_SE("already connected");
    if(sockFD == -1) {
        createFD();
    }

    if( connect( sockFD, saddr, sizeof(sockaddr)) < 0) {
        THROW_SE("Failed to connect to socket");
    }

    connected = true;
}

//send current contents of myBuf to peer
int Socket::output()
{
    //don't forget to add 1 for \0
    int length = myBuf.str().length()+1;
#ifdef SOCKET_DEBUG
    cerr << "Sending: '" << myBuf.str() << "'" << endl;
#endif
    int retVal = send(sockFD, myBuf.str().c_str(), length , 0);

    if(retVal == length) {
        myBuf.ignore();
        myBuf.seekg(0, std::ios::beg);
    }
    else if (retVal < 0) {
        std::stringstream ss;
        ss << "error sending: " << errno;
        perror("send()");
        THROW_SE(ss.str());
    }
    else {
        cerr << "Odd send...  expected: " << length  << " sent: " << retVal << endl;
    }

    return retVal;
}

//receive from peer into myBuf
int Socket::input()
{
    char buffer[4096];

    if (!connected)
    {
        cerr << __FILE__"," << __LINE__ << endl;
        THROW_NC;
    }

    int retVal = recv(sockFD, buffer, 4096, 0);

    if(retVal > 0) {
        myBuf.str(buffer);
    }
    else if(retVal < 0)
    {
        strerror_r(errno, buffer, 4096);
        THROW_SE( string("Error in recv()") + buffer );
    }
    if (retVal == 0) {
        connected=false;
        close(sockFD);
        sockFD=-1;
        THROW_NC;
    }
    
    return retVal;
}

//send a message
int Socket::sendMessage(Message &m)
{
    myBuf.str( m.serialize() );
    return output();
}

//receive and deserialize message
//  pointer must be freed
Message * Socket::getMessage()
{
    Message * m = NULL;

    if(input() <= 0)
        return NULL;

    vector<string> v;
    string save(myBuf.str());

    //much prettier than strtok, right?
    boost::split(v, save, boost::is_any_of(" \t\r\n"), boost::algorithm::token_compress_on );

    //not an easier way to do this...  (boost::serialization could do it implicitly)
    if(v[0] == "LSA")
        m = new LSA(v);
    else if(v[0] == "RREQ")
        m = new RREQ(v);
    else if(v[0] == "BGP" )
        m = new BGP(v);
    else if(v[0] == "RRES" )
        m = new RRES(v);
    else if(v[0] == "IRRQ" )
        m = new IRRQ(v);
    else if(v[0] == "IRRS" )
        m = new IRRS(v);
    else
        cerr << "Unknown message type: " << v[0] << endl;

    return m;
}

//open a listen socket on the specified port
ListenSocket::ListenSocket(uint16_t port)
{
    struct sockaddr_in  servaddr;
    servaddr.sin_family         = AF_INET;
    servaddr.sin_port           = htons(port);
    servaddr.sin_addr.s_addr    = htonl(INADDR_ANY);

    if( (sockFD = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        THROW_SE("Failed to create a socket");
    }

    int       reuse=1;
    /* set SO_REUSEADDR so echoserv will be able to be re-run instantly on shutdown */
    if(setsockopt(sockFD, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1) {
        close(sockFD);
        THROW_SE("Failed to set SO_REUSEADDR");
    }

    if( bind( sockFD, (const sockaddr *)&servaddr, sizeof(servaddr) ) ) {
        close(sockFD);
        THROW_SE("Failed to bind socket");
    }
    if( listen(sockFD, 5) < 0 )
    {
        close(sockFD);
        THROW_SE("Failed to listen on socket");
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

//accept an income TCP connection
//  blocking
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
