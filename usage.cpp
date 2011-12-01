#include <iostream>
#include <sstream>

/*
 *  Putting the router_usage (and possibly other usage() functions) in a shared
 * object allows them to be easily used in constructors that take argc/argv
 */

using namespace std;

//Print out a common usage message
static void usage_common(const char * errmsg, string usage, bool exit, int rc)
{
    if(errmsg)
        cerr << "USAGE ERROR: " << errmsg << endl << endl;
    else
        cerr << "USAGE: " << endl << endl;
    cerr << usage;
    if(exit)
        _exit(rc);
}

//Print out a message on how to call router
void router_usage(const char * errmsg, bool exit, int rc)
{
    stringstream s;
    s << "router <AS> <routerID> <configfile> <neighborAS> <neighborrouterID> <net1> [net2] [...]" << endl;
    s << "               AS: AS number of router’s domain" << endl;
    s << "         routerID: Router’s ID within its domain" << endl;
    s << "       configfile: file containing hostname and port number information for PCEs" << endl;
    s << "       neighborAS: if router is border router, this is the AS number of its neighboring AS (set to 99 if not border router)" << endl;
    s << " neighborrouterID: if router is a border router, this is the ID of the router in the neighboring domain (set to 99 if not border router) " << endl;
    s << " net1, net1, etc.: networks to which router’s interface are connected" << endl;
    usage_common(errmsg, s.str(), exit, rc);
}

//Print out a message on how to call the PCE
void PCE_usage(const char * errmsg, bool exit, int rc)
{
    stringstream s;
    s << "PCE <AS> <configfile>" << endl;
    s << "        AS: AS number of PCE's domain" << endl;
    s << "configfile: file containing hostname and port number information for PCEs" << endl;
    usage_common(errmsg, s.str(), exit, rc);
}

