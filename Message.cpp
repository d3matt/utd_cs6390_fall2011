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

RREQ::RREQ(vector<string> &v)
{
    if(v.size() != 3)
        throw DeserializationException("too few parameters");
    type = v[0];
    source = boost::lexical_cast<uint32_t>(v[1]);
    dest = boost::lexical_cast<uint32_t>(v[2]);
}
string RREQ::serialize()
{
    ostringstream oss;
    oss << type << " " << source << " " << dest;
    return oss.str();
}

int RREQ::test(short port)
{
    Socket sock("localhost", port);
    {
        RREQ out(1, 2);
        sock.sendMessage(out);
    }
    Message *m = sock.getMessage();
    if(m != NULL) {
        try {
            auto_ptr<RREQ> in(dynamic_cast<RREQ *> (m));
            cout << *in;
        }
        catch (...) {
            cerr << "failed to dynamic cast" << endl;
            return -1;
        }
    }
    else {
        cerr << "getMessage() failed" << endl;
        return -1;
    }
    return 0;
}

ostream& operator<< (ostream& out, const RREQ& c)
{
    out << "RREQ  source: " << c.source << " dest: " << c.dest << endl;
    return out;
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

string BGP::serialize()
{
    ostringstream oss;
    oss << type    << " " 
        << AS      << " "
        << AS_hops;
    for(vector<uint32_t>::const_iterator it = nets.begin(); it != nets.end(); it++)
        oss << " " << *it;
    return oss.str();
}

ostream& operator<< (ostream& out, const BGP& c)
{
    out << "BGP  AS: " << c.AS << " AS_hops: " << c.AS_hops << endl;
    out << "network path: ";
    for(vector<uint32_t>::const_iterator it = c.nets.begin(); it != c.nets.end(); it++)
        out << " " << *it;
    out << endl;
    return out;
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
    Message *m = sock.getMessage();
    if(m != NULL) {
        try {
            auto_ptr<BGP> in(dynamic_cast<BGP *> (m));
            cout << *in;
        }
        catch (...) {
            cerr << "failed to dynamic cast" << endl;
            return -1;
        }
    }
    else {
        cerr << "getMessage() failed" << endl;
        return -1;
    }
    return 0;
}

} //cs6390
