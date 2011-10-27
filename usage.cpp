#include <iostream>

/*
 *  Putting the router_usage (and possibly other usage() functions) in a shared
 * object allows them to be easily used in constructors that take argc/argv
 */

using namespace std;

void router_usage(const char * errmsg, bool exit, int rc)
{
    if(errmsg)
        cerr << "USAGE ERROR: " << errmsg << endl << endl;
    else
        cerr << "USAGE:" << endl << endl;
    cerr << "router <AS> <routerID> <configfile> <neighborAS> <neighborrouterID> <net1> [net2] [...]" << endl;
    cerr << "               AS: AS number of router’s domain" << endl;
    cerr << "         routerID: Router’s ID within its domain" << endl;
    cerr << "       configfile: file containing hostname and port number information for PCEs" << endl;
    cerr << "       neighborAS: if router is border router, this is the AS number of its neighboring AS (set to 99 if not border router)" << endl;
    cerr << " neighborrouterID: if router is a border router, this is the ID of the router in the neighboring domain (set to 99 if not border router) " << endl;
    cerr << " net1, net1, etc.: networks to which router’s interface are connected" << endl;
    if(exit)
        _exit(rc);
}
