#include <sstream>
#include <iostream>
#include <string>
#include <memory>
#include <string.h>

#include <boost/lexical_cast.hpp>

#include "Socket.h"
#include "Exceptions.h"
#include "Message.h"
#include "utils.h"
#include "usage.h"

using namespace std;

namespace cs6390
{

int Message::test_catch(short port)
{
    Socket sock("localhost", port);
    sock.sendToSocket("XXXX"); //can't instantiate Message...
    Message *m = sock.getMessage();
    if(m != NULL) {
        cerr << "getMessage() should have failed" << endl;
        return -1;
    }
    return 0;
}

int Message::test(Socket &sock)
{
    Message *m = sock.getMessage();
    if(m != NULL) {
        cout << m; //shoul throw() if not a child class...
    }
    else {
        cerr << "getMessage() failed" << endl;
        return -1;
    }
    return 0;
}

ostream& operator<< (ostream& out, const Message *m)
{
    out << m->serialize(true);
    return out;
}

LSA::LSA()
    : Message("LSA"), AS(99), routerID(99), neighborAS(99), neighborRouterID(99) { }

//LSA contructor, only to be used by the main() for router.cpp
//most of the command line parsing for router.cpp happens in this function
LSA::LSA(int argc, char ** argv)
    : Message("LSA")
{
    if(string_to_int(argv[1], AS) == NULL)
        router_usage("First argument must be an integer");

    if(!valid_AS(AS))
        router_usage("AS # must be between 0 and 9 and must match a configured AS");

    if(string_to_int(argv[2], routerID) == NULL)
        router_usage("Second argument must be an integer");
    if(!valid_router(routerID))
        router_usage("routerID # must be between 0 and 9");

    if(string_to_int(argv[4], neighborAS) == NULL)
        router_usage("Fourth argument must be an integer");

    if(string_to_int(argv[5], neighborRouterID) == NULL)
        router_usage("Fifth argument must be an integer");

    for(int32_t i=6; i < argc; i ++)
    {
        uint32_t tmp;
        if(string_to_int(argv[i], tmp) == NULL)
            router_usage("net argmenst must be integers");
        if(tmp > 99)
            router_usage("networks must be less than 99");
        if( linkStates.find(tmp) != linkStates.end() )
            router_usage("Don't specify duplicate networks");
        linkStates[tmp]=1;
    }
}

int LSA::addLink(uint32_t net, uint32_t metric)
{
    if ( linkStates.find(net) != linkStates.end() )
        return 1;
    linkStates[net]=metric;
    return 0;
}

//set link state of a single link
int LSA::setLinkMetric(uint32_t net, uint32_t metric)
{
    LinkMap::iterator it = linkStates.find(net);
    if ( it == linkStates.end())
        return 1;
    it->second=metric;
    return 0;
}

//set link states of all links
int LSA::setLinkMetric(uint32_t metric)
{
    for(LinkMap::iterator it = linkStates.begin(); it != linkStates.end(); it++)
        it->second=metric;
    return 0;
}

LSA::LSA(vector<string> &v)
{
    if(v.size() < 4) 
        throw DeserializationException("too few parameters");
    else if(v.size() % 2 != 0)
        throw DeserializationException("odd number of parameters");
    type = v[0];
    if(type != "LSA")
        throw DeserializationException("wrong message type");
    routerID = boost::lexical_cast<uint32_t>(v[1]);
    neighborAS = boost::lexical_cast<uint32_t>(v[2]);
    neighborRouterID = boost::lexical_cast<uint32_t>(v[3]);

    for(uint32_t i=4; i < v.size(); i+=2)
    {
        linkStates[boost::lexical_cast<uint32_t>(v[i])] = boost::lexical_cast<uint32_t>(v[i+1]);
    }
}

string LSA::serialize(bool readable) const
{
    stringstream ss;

    ss  << type << " ";
    if(readable) ss << endl << " routerID: ";
    ss  << routerID << " ";
    if(readable) ss << endl << " neighborAS: ";
    ss  << neighborAS << " ";
    if(readable) ss << endl << " neighborRouterID: ";
    ss  << neighborRouterID;
    if(readable) ss << endl << " linkStates: " << endl;
    for(LinkMap::const_iterator it = linkStates.begin(); it != linkStates.end(); it++) {
        if(readable) ss << "net: ";
        ss << " " << it->first << " ";
        if(readable) ss << "(";
        ss << it-> second;
        if(readable) ss << ")" << endl;
    }
    return ss.str();
}

int LSA::test(short port)
{
    Socket sock("localhost", port);
    {
        LSA out;
        out.addLink(1,1);
        out.addLink(5,1);
        out.addLink(9,99);
        sock.sendMessage(out);
    }
    return Message::test(sock);
}

RREQ::RREQ(vector<string> &v)
{
    if(v.size() != 3)
        throw DeserializationException("too few parameters");
    type = v[0];
    source = boost::lexical_cast<uint32_t>(v[1]);
    dest = boost::lexical_cast<uint32_t>(v[2]);
}
string RREQ::serialize(bool readable) const
{
    ostringstream oss;
    oss << type << " ";
    if(readable) oss << "source: ";
    oss << source << " ";
    if(readable) oss << "dest: ";
    oss << dest;
    if(readable) oss << endl;
    return oss.str();
}

int RREQ::test(short port)
{
    Socket sock("localhost", port);
    {
        RREQ out(1, 2);
        sock.sendMessage(out);
    }
    return Message::test(sock);
}

BGP::BGP(vector<string> &v)
{
    if(v.size() < 4)
        throw DeserializationException("too few parameters");
    type    = v[0];
    AS      = boost::lexical_cast<uint32_t>(v[1]);
    AS_hops = boost::lexical_cast<uint32_t>(v[2]);
    for(uint32_t i=3; i < v.size(); i++)
        nets.push_back( boost::lexical_cast<uint32_t>(v[i]) );
}

string BGP::serialize(bool readable) const
{
    ostringstream oss;
    oss << type    << " ";
    if(readable) oss << "AS: ";
    oss << AS      << " ";
    if(readable) oss << "AS_hops: ";
    oss << AS_hops;
    if(readable) oss << endl << "networks: ";
    for(vector<uint32_t>::const_iterator it = nets.begin(); it != nets.end(); it++)
        oss << " " << *it;
    if(readable) oss <<endl;
    return oss.str();
}

int BGP::test(short port)
{
    Socket sock("localhost", port);
    {
        BGP out;
        out.nets.push_back(1);
        out.nets.push_back(2);
        out.nets.push_back(3);
        sock.sendMessage(out);
    }
    return Message::test(sock);
}

RRES::RRES(vector<string> &v)
{
    if(v.size() < 2)
        throw DeserializationException("too few parameters");
    type    = v[0];
    for(uint32_t i=1; i < v.size(); i++)
        routers.push_back( boost::lexical_cast<uint32_t>(v[i]) );
}

string RRES::serialize(bool readable) const
{
    ostringstream oss;
    oss << type;
    if(readable) oss << " routers:";
    for(vector<uint32_t>::const_iterator it = routers.begin(); it != routers.end(); it++)
        oss << " " << *it;
    if(readable) oss << endl;
    return oss.str();
}

int RRES::test(short port)
{
    Socket sock("localhost", port);
    {
        RRES out;
        out.routers.push_back(1);
        out.routers.push_back(2);
        out.routers.push_back(3);
        sock.sendMessage(out);
    }
    return Message::test(sock);
}

IRRQ::IRRQ(vector<string> &v)
{
    if(v.size() != 3)
        throw DeserializationException("too few parameters");
    type = v[0];
    AS = boost::lexical_cast<uint32_t>(v[1]);
    dest_net = boost::lexical_cast<uint32_t>(v[2]);
}
string IRRQ::serialize(bool readable) const
{
    ostringstream oss;
    oss << type << " ";
    if(readable)
        oss << "AS: ";
    oss << AS << " ";
    if(readable)
        oss << "dest_net: ";
    oss << dest_net;
    if(readable)
        oss << endl;
    return oss.str();
}

int IRRQ::test(short port)
{
    Socket sock("localhost", port);
    {
        IRRQ out(1, 77);
        sock.sendMessage(out);
    }
    return Message::test(sock);
}

IRRS::IRRS(vector<string> &v)
{
    if( v.size() < 3)
        throw DeserializationException("too few parameters");
    type = v[0];
    if(v[v.size() -1] == "BLK") {
        blank=true;
        for(vector<string>::const_iterator it = v.begin() +1 ; *it != "BLK" ; it++) {
            if(it->substr(0,2) != "AS")
                throw DeserializationException(string("WTF") + *it);
            ASlist.push_back( 
                make_pair(
                    boost::lexical_cast<uint32_t>(it->substr(2, string::npos)), 
                    vector<uint32_t>()
                    ) );
        }
    }
    else {
        blank=false;
        vector<string>::const_iterator it1,it2;
        for(it1=v.begin() + 1; it1 != v.end(); it1++)
        {
            if(it1->substr(0,2) != "AS")
                throw DeserializationException(string("WTF") + *it1);
            uint32_t as=boost::lexical_cast<uint32_t>(it1->substr(2, string::npos));
            vector<uint32_t> intv;

            for(it2=it1 + 1; it2 != v.end(); it2++) {
                intv.push_back( boost::lexical_cast<uint32_t>(*it2) );
                if( (it2 + 1) == v.end() || (it2+1)->substr(0,2) == "AS")
                    break;
            }
            ASlist.push_back( make_pair(as, intv) );
            it1=it2;
        }
    }
}

string IRRS::serialize(bool readable) const
{
    ostringstream oss;
    oss << type;
    if(readable)
        oss << endl;
    for(list<ASroute>::const_iterator it = ASlist.begin(); it != ASlist.end(); it++ )
    {
        oss << " " << "AS" << it->first;
        if(!blank) {
            if(readable)
                oss << " routers:";
            for(vector<uint32_t>::const_iterator vit = it->second.begin(); vit != it->second.end(); vit++)
                oss << " " << *vit;
        }
        if(readable)
            oss << endl;
    }
    if(blank) oss << " BLK";
    if(readable) oss << endl;
    return oss.str();
}

int IRRS::test_route(short port)
{
    Socket sock("localhost", port);
    {
        IRRS out;
        vector<uint32_t> v;
        v.push_back(1); v.push_back(2); v.push_back(3);
        out.ASlist.push_back( make_pair( 1, v) );
        v.clear(); v.push_back(4); v.push_back(5); v.push_back(6);
        out.ASlist.push_back( make_pair( 5, v) );
        v.clear(); v.push_back(7); v.push_back(8); v.push_back(9);
        out.ASlist.push_back( make_pair( 9, v) );
        out.blank=false;
        sock.sendMessage(out);
    }
    return Message::test(sock);
}

int IRRS::test_blk(short port)
{
    Socket sock("localhost", port);
    {
        IRRS out;
        out.ASlist.push_back( make_pair( 1, vector<uint32_t>() ) );
        out.ASlist.push_back( make_pair( 5, vector<uint32_t>() ) );
        out.ASlist.push_back( make_pair( 9, vector<uint32_t>() ) );
        out.blank=true;
        sock.sendMessage(out);
    }
    return Message::test(sock);
}

int IRRS::test(short port)
{
    int retval=0;
    if(test_route(port) != 0) {
        cerr << "route test failed" << endl;
        retval=1;
    }
    if(test_blk(port) != 0) {
        cerr << "BLK test failed" << endl;
        retval=1;
    }
    return retval;
}

} //cs6390
