#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>

#include "PCEconfig.h"
#include "utils.h"


using namespace std;
void usage(const char * errmsg=NULL, bool exit=true, int rc=1)
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

int main(int argc, char ** argv)
{
    if(argc == 1)
        usage(NULL, true, 0);
    else if(argc < 7)
        usage("invalid number of arguments");

    uint32_t AS;
    uint32_t routerID;
    PCEconfig * pConfig;

    if(string_to_int(argv[1], AS) == NULL)
        usage("First argument must be an integer");
    if(!valid_AS(AS))
        usage("AS # must be between 0 and 9 and must match a configured AS");

    if(string_to_int(argv[2], routerID) == NULL)
        usage("Second argument must be an integer");
    if(!valid_router(routerID))
        usage("routerID # must be between 0 and 9");

    pConfig=new PCEconfig(argv[3]);

    cout << "CONFIG: " << endl;
    cout << "                  AS: " << AS << endl;
    cerr << "            routerID: " << routerID << endl;
    cerr << pConfig;
    cerr << "          neighborAS: " << endl;
    cerr << "    neighborrouterID: " << endl;
    cerr << "    net1, net1, etc.: " << endl;

    return 0;
}
