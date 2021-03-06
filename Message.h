
#ifndef _MESSAGE_H
#define _MESSAGE_H

extern "C"
{
# include <stdint.h>
}

#include <iostream>
#include <list>
#include <map>
#include <string>
#include <utility>
#include <vector>

using std::string;
using std::vector;
using std::ostream;

namespace cs6390
{

class Socket;

//base message class
//  all other Message types derive from this one.
//  every derived class must implement:
//      serialize(bool readable=false) const
//          if readable is true, output is human readable
//          human readable cannot be de-serialized
//      contructor from a vector of strings
class Message
{

protected:
    string type;

public:
    Message(string type) : type(type) {}
    Message() : type("XXXX") {}
    virtual ~Message(void) {}

    virtual string serialize(bool readable=false) const = 0;

    string getType() const {return type;}

    static int test_catch(short port);
    static int test(Socket &sock);
};
ostream& operator<< (ostream& out, const Message *m);


typedef         std::map<uint32_t, uint32_t> LinkMap;

class LSA : public Message
{
private:
    //used a map to make lookups fast
    //access to complex data structure is through controlled methods:
    //  addLink
    //  setLinkMetric
    //
    //getLinkMap is used in PCE to iterate over entries
    LinkMap         linkStates;
public:
    uint32_t        AS;
    uint32_t        routerID;
    uint32_t        neighborAS;
    uint32_t        neighborRouterID;


    friend ostream& operator<< (ostream& out, const LSA& c);
                    LSA();
                    LSA(int argc, char ** argv);
                    ~LSA() { }
    int             addLink(uint32_t net, uint32_t metric=1);
    int             setLinkMetric(uint32_t net, uint32_t metric);
    int             setLinkMetric(uint32_t metric);

    LinkMap        *getLinkMap() {return &linkStates;}

    //serialization/deserialization
                    LSA(vector<string> &v);
    string          serialize(bool readable=false) const;

    //basisc LSA unit test
    static int test(short port);
};

class RREQ : public Message
{
public:
    uint32_t    source;
    uint32_t    dest;
                RREQ(int32_t source=0xff, int32_t dest=0xff) :
                    Message("RREQ"), source(source), dest(dest) {}

    //serialization/deserialization
                RREQ(vector<string> &v);
    string      serialize(bool readable=false) const;

    //basic RREQ unit test
    static int  test(short port);
};

class BGP : public Message
{
public:
    uint32_t AS;
    uint32_t AS_hops;
    vector<uint32_t> nets;

            BGP() : Message("BGP"), AS(0xff), AS_hops(0xff) {}

    //serialization/deserialization
            BGP(vector<string> &v);
    string  serialize(bool readable=false) const;

    //basic BPG unit test
    static int test(short port);
};

class RRES : public Message
{
public:
    vector<uint32_t> routers;
    
            RRES() : Message("RRES") {}

    //serialization/deserialization
            RRES(vector<string> &v);
    string  serialize(bool readable=false) const;

    //basic RRES unit test
    static int test(short port);
};

class IRRQ : public Message
{
public:
    uint32_t AS;
    uint32_t dest_net;

            IRRQ(int32_t AS=0xff, int32_t dest_net=0xff) :
                Message("IRRQ"), AS(AS), dest_net(dest_net) {}
            IRRQ(vector<string> &v);

    //serialization/deserialization
    string  serialize(bool readable=false) const;

    //basic RRES unit test
    static int test(short port);
};

typedef std::pair <uint32_t,vector<uint32_t> > ASroute;

class IRRS : public Message
{
public:
    std::list<ASroute>  ASlist;
    bool                blank;

            IRRS() : Message("IRRS") {}

    //serialization/deserialization
            IRRS(vector<string> &v);
    string  serialize(bool readable=false) const;

    //IRRS tests
    static int test_route(short port);  //tests a message with an actual route
    static int test_blk(short port);    //tests a message with only a BLK
    static int test(short port);
};




}   //namespace cs6390


#endif
