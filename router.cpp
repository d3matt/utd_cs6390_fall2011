#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>

#include "usage.h"
#include "RouterStatus.h"
#include "PCEconfig.h"
#include "Socket.h"
//#include "utils.h"

using namespace std;

int main(int argc, char ** argv)
{
    if(argc == 1)
        router_usage(NULL, true, 0);
    else if(argc < 7)
        router_usage("invalid number of arguments");

    RouterStatus localStatus(argc, argv);
    PCEconfig pConfig(argv[3]);

    localStatus.setLinkState(3, false);

    cout << pConfig;
    cout << "localStatus (before serialization): " << endl;

    cout << localStatus;

    stringstream buffer;
    {
        boost::archive::text_oarchive oa(buffer);
        oa << localStatus;
    }

    cout << "serialized data: '" << buffer.str() << "'" << endl;

    RouterStatus deserialized;
    {
        boost::archive::text_iarchive ia(buffer);
        ia >> deserialized;
    }
    cout << "localStatus (after serialization): " << endl;
    cout << deserialized;

    cout << pConfig.getAS(localStatus.getAS()).hostname << " " <<  pConfig.getAS(localStatus.getAS()).portno;

    Socket s(pConfig.getAS(localStatus.getAS()).hostname, pConfig.getAS(localStatus.getAS()).portno);

    s.sendToSocket(buffer.str());

    return 0;
}
