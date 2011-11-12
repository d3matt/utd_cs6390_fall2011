#include <vector>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

#include "Exceptions.h"
#include "Message.h"
#include "Socket.h"

#include "utils.h"
#include "usage.h"

using namespace std;

namespace cs6390
{
ostream& operator<< (ostream& out, const RouterStatus& c)
{
    out << "              AS: " << c.AS << endl
        << "        routerID: " << c.routerID << endl
        << "      neighborAS: " << c.neighborAS << endl
        << "neighborrouterID: " << c.neighborrouterID << endl
        << "      linkStates:"  << endl;
    for(LinkMap::const_iterator it=c.linkStates.begin(); it != c.linkStates.end(); it++)
        out << "net: " << it->first << "(" << it->second << ")" << endl;
    return out;
}

RouterStatus::RouterStatus()
    : Message("LSA"), AS(99), routerID(99), neighborAS(99), neighborrouterID(99) { }

//RouterStatus contructor, only to be used by the main() for router.cpp
//most of the command line parsing for router.cpp happens in this function
RouterStatus::RouterStatus(int argc, char ** argv)
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

    if(string_to_int(argv[5], neighborrouterID) == NULL)
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

int RouterStatus::addLink(uint32_t net, uint32_t metric)
{
    if ( linkStates.find(net) != linkStates.end() )
        return 1;
    linkStates[net]=metric;
    return 0;
}

//set link state of a single link
int RouterStatus::setLinkMetric(uint32_t net, uint32_t metric)
{
    LinkMap::iterator it = linkStates.find(net);
    if ( it == linkStates.end())
        return 1;
    it->second=metric;
    return 0;
}

//set link states of all links
int RouterStatus::setLinkMetric(uint32_t metric)
{
    for(LinkMap::iterator it = linkStates.begin(); it != linkStates.end(); it++)
        it->second=metric;
    return 0;
}

RouterStatus::RouterStatus(vector<string> &v)
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
    neighborrouterID = boost::lexical_cast<uint32_t>(v[3]);

    for(uint32_t i=4; i < v.size(); i+=2)
    {
        linkStates[boost::lexical_cast<uint32_t>(v[i])] = boost::lexical_cast<uint32_t>(v[i+1]);
    }
}


string RouterStatus::serialize(void)
{
    stringstream ss;

    ss  << type << " "
        << routerID << " "
        << neighborAS << " "
        << neighborrouterID;
    for(LinkMap::iterator it = linkStates.begin(); it != linkStates.end(); it++)
        ss << " " << it->first << " " << it-> second;
    return ss.str();
}

int RouterStatus::test(short port)
{
    Socket sock("localhost", port);
    {
        RouterStatus out;
        out.addLink(1,1);
        sock.sendMessage(out);
    }

    Message *m = sock.getMessage();
    if(m != NULL) {
        try {
            auto_ptr<RouterStatus> in(dynamic_cast<RouterStatus *> (m));
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
