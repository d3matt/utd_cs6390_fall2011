#include <iostream>
#include <sstream>
#include <string>
#include <list>
#include <memory>
#include <map>
#include <utility>
#include <boost/lexical_cast.hpp>

#include "Message.h"

using namespace std;
using namespace cs6390;

#define DASH "--------------------------------------------------------------------------------"
#define UPAR "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^"
#define DNAR "vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv"

typedef list< pair <string, int (*)(short)> > funcList;
int main(int argc, char ** argv)
{
    uint32_t port;
    if(argc == 2)
        port = boost::lexical_cast<uint32_t>(argv[1]);
    else
        port = 31337;
    int rc, retval=0;
    funcList flist;

    //List of function pointers
    flist.push_back( make_pair("MESSAGE", &Message::test_catch) );
    flist.push_back( make_pair("LSA",     &LSA::test) );
    flist.push_back( make_pair("RREQ",    &RREQ::test) );
    flist.push_back( make_pair("BGP",     &BGP::test) );
    flist.push_back( make_pair("RRES",    &RRES::test) );
    flist.push_back( make_pair("IRRQ",    &IRRQ::test) );
    flist.push_back( make_pair("IRRS",    &IRRS::test) );

    for(funcList::const_iterator it = flist.begin() ; it != flist.end(); it++ )
    {
        cout << DASH << endl;
        cout << "    Begin " << it->first << " test" << endl;
        cout << DNAR << endl;
        //Call each pointer
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
