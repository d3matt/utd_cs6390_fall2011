#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>


using namespace std;
void usage(const char * errmsg=NULL, bool exit=false, int rc=1)
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
        usage("invalid number of arguments", true, 1);

    uint32_t AS;


    AS=strtoul(argv[1], NULL, 10);


    cout << "CONFIG: " << endl;
    cout << "               AS: " << AS << endl;
    cerr << "         routerID: " << endl;
    cerr << "       configfile: " << endl;
    cerr << "       neighborAS: " << endl;
    cerr << " neighborrouterID: " << endl;
    cerr << " net1, net1, etc.: " << endl;

    return 0;
}
