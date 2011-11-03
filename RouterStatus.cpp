#include "RouterStatus.h"

#include "utils.h"
#include "usage.h"

using namespace std;

ostream& operator<< (ostream& out, const Link l)
{
    out << "net: " << l.net;
    if(l.state)
        out << " (UP)";
    else
        out << " (DOWN)";
    out << endl;
    return out;
}

ostream& operator<< (ostream& out, const RouterStatus& c)
{
    out << "              AS: " << c.AS << endl
        << "        routerID: " << c.routerID << endl
        << "      neighborAS: " << c.neighborAS << endl
        << "neighborrouterID: " << c.neighborrouterID << endl
        << "      linkStates:"  << endl;
    for(map<int, Link>::const_iterator it=c.linkStates.begin(); it != c.linkStates.end(); it++)
        out << it->second;
    return out;
}

RouterStatus::RouterStatus()
    : Message("RTST"), AS(99), routerID(99), neighborAS(99), neighborrouterID(99) { }

//RouterStatus contructor, only to be used by the main() for router.cpp
//most of the command line parsing for router.cpp happens in this function
RouterStatus::RouterStatus(int argc, char ** argv)
    : Message("RTST")
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

    Link l;
    l.state=true;
    for(int32_t i=6; i < argc; i ++)
    {
        uint32_t tmp;
        if(string_to_int(argv[i], tmp) == NULL)
            router_usage("net argmenst must be integers");
        if(tmp > 99)
            router_usage("networks must be less than 99");
        if( linkStates.find(tmp) != linkStates.end() )
            router_usage("Don't specify duplicate networks");
        l.net=tmp;
        linkStates[tmp]=l;
    }
}

int RouterStatus::addLink(uint32_t net)
{
    if ( linkStates.find(net) != linkStates.end() )
        return 1;
    Link l;
    l.state=true;
    l.net=net;
    linkStates[net]=l;

    return 0;
}

int RouterStatus::setLinkState(uint32_t net, bool state)
{
    map<int, Link>::iterator it = linkStates.find(net);
    if ( it == linkStates.end())
        return 1;
    it->second.state=state;
    return 0;
}
