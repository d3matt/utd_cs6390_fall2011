#include <stdio.h>
#include <iostream>
#include <boost/lexical_cast.hpp>

#include <vector>
#include <map>

#include "usage.h"
#include "PCEconfig.h"
#include "MessageContainer.h"
#include "Socket.h"

typedef std::pair<uint32_t, uint32_t> Edge;

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

    ListenSocket sock(me.portno);
    
    while(1) {
        Socket s = sock.acceptConnection();
        MessageContainer out=s.getMessage();

        RouterStatus * r;
        r=(RouterStatus *) out.getMessage();
        cout << "received router status: " << endl;
        cout << *r;

        vector<Edge> edges;
    
        for(LinkMap::iterator iter1 = r->getLinkMap()->begin();
            iter1 != r->getLinkMap()->end(); ++iter1)
        {
            for(LinkMap::iterator iter2 = iter1;
                iter2 != r->getLinkMap()->end(); ++iter2)
            {
                if(iter2 == iter1) continue;
    
                edges.push_back(Edge(iter1->second.net, iter2->second.net));
            }
        }
    
        for(unsigned i = 0; i < edges.size(); i++)
        {
            cout << edges[i].first << " -- " << edges[i].second << endl;
        }
    }
    
}
