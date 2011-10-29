#include <stdio.h>
#include <iostream>
#include <boost/lexical_cast.hpp>

#include "usage.h"
#include "PCEconfig.h"


using namespace std;

void pdie(const char * msg, int rc=1)
{
    perror(msg);
    _exit(rc);
}

int main(int argc, char ** argv)
{
    if(argc == 1)
        PCE_usage(NULL, true, 0);
    else if(argc != 3)
        PCE_usage("invalid number of arguments");

    uint32_t ASno=99;

    try { ASno = boost::lexical_cast<uint32_t>(argv[1]); }
    catch (...) { PCE_usage("First argument must be integer"); }
    PCEconfig config(argv[2]);

    cout << "AS: " << ASno << endl;
    cout << config;

    AS me = config.getAS(ASno);
/*
    int fd;

    if ( (fd=socket(AF_INET, SOCK_DGRAM, 0) ) < 0 ) {
        pdie("socket()");
    }
    me.saddr_in.sin_addr.s_addr = htonl(INADDR_ANY);

    if( bind(fd, &me.saddr, sizeof(sockaddr) ) ) {
        pdie("bind()");
    }
*/
}
