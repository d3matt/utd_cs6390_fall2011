#include <sstream>
#include <iostream>
#include <string>
#include <memory>
#include <string.h>

#include <boost/lexical_cast.hpp>

#include "Socket.h"
#include "Exceptions.h"
#include "Message.h"

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
