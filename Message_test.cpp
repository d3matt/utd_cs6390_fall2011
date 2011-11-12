#include <iostream>
#include <sstream>
#include <string>
#include <memory>
#include <map>
#include <boost/lexical_cast.hpp>

#include "Message.h"

using namespace std;
using namespace cs6390;

#define DASH "--------------------------------------------------------------------------------"
#define UPAR "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^"
#define DNAR "vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv"

typedef map<string, int (*)(short)> funcMap;
int main(int argc, char ** argv)
{
    uint32_t port;
    if(argc == 2)
        port = boost::lexical_cast<uint32_t>(argv[1]);
    else
        port = 31337;
    int rc, retval=0;
    funcMap fmap;

    //unit tests are not necessarily run in this order...
    fmap["LSA"]  = &RouterStatus::test;
    fmap["RREQ"] = &RREQ::test;
    fmap["BGP"]  = &BGP::test;

    for(funcMap::const_iterator it = fmap.begin() ; it != fmap.end(); it++ )
    {
        cout << DASH << endl;
        cout << "    Begin " << it->first << " test" << endl;
        cout << DNAR << endl;
        rc=it->second(port);
        cout << UPAR << endl;

        if(rc == 0) {
            cout << "    "<< it->first << " test (PASS)" << endl;
        }
        else {
            retval=1;
            cout << "    " << it->first << " test (FAIL)" << endl;
        }
        cout << DASH << endl;
    }

    return retval;
}
