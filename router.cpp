#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>

#include "PCEconfig.h"
#include "RouterStatus.h"
#include "utils.h"
#include "usage.h"

using namespace std;

int main(int argc, char ** argv)
{
    if(argc == 1)
        router_usage(NULL, true, 0);
    else if(argc < 7)
        router_usage("invalid number of arguments");

    RouterStatus localStatus(argc, argv);
    PCEconfig * pConfig=new PCEconfig(argv[3]);

    cout << pConfig;
    cout << "localStatus: " << endl;
    cout << &localStatus;

    return 0;
}
