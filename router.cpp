#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>

#include <vector>

/*
#include <boost/version.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
*/
#include "PCEconfig.h"
#include "RouterStatus.h"
#include "MessageContainer.h"
#include "utils.h"
#include "usage.h"
#include "Socket.h"
#include "utils.h"

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

    MessageContainer m(&localStatus);

    AS myAS=pConfig.getAS(localStatus);
    Socket s(&myAS.saddr);
    while(1)
    {
        s.sendMessage(m);
        sleep(1);
    }

    return 0;
}
