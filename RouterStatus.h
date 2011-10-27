#include <stdint.h>

#include <map>
#include <iostream>

#ifndef __ROUTER_STATUS_H__
#define __ROUTER_STATUS_H__

using namespace std;
struct Link {
    uint32_t    net;
    bool        state;
};
ostream& operator<< (ostream& out, Link l);

class RouterStatus
{
private:
    uint32_t        AS;
    uint32_t        routerID;
    uint32_t        neighborAS;
    uint32_t        neighborrouterID;

    //used a map to make lookups fast
    map<int, Link>  linkStates;

public:
    friend ostream& operator<< (ostream& out, RouterStatus *c);

                    RouterStatus();
                    RouterStatus(int argc, char ** argv);
    int             addLink(uint32_t net);
    int             setLinkState(uint32_t net, bool state);
};

#endif //__ROUTER_STATUS_H__
