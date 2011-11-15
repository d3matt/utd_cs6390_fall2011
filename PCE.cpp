#include <stdio.h>
#include <iostream>

#include <vector>
#include <map>

#include <boost/lexical_cast.hpp>

#include <boost/config.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>

extern "C"
{
# include <pthread.h>
}

#include "usage.h"
#include "PCEconfig.h"
#include "Message.h"
#include "Socket.h"
#include "utils.h"

using namespace std;
using namespace cs6390;

typedef pair<uint32_t, uint32_t> Edge;

struct EdgeLess
{
    bool operator() (const Edge &left, const Edge &right)
    {
        if(left.first < right.first) return true;
        if(left.first == right.first)
        {
            if(left.second < right.second) return true;
        }
        return false;
    }
};

inline uint32_t max(uint32_t left, uint32_t right) {if(left >= right) return left; else return right;}

//Value is routerID, metric
typedef pair<uint32_t, uint32_t> EdgeMapValue;
typedef pair<Edge, EdgeMapValue> EdgeMapEntry;
typedef map<Edge, EdgeMapValue, EdgeLess> EdgeMap;

typedef pair<unsigned, unsigned> NodeMapEntry;
typedef map<unsigned, unsigned> NodeMap;

typedef struct 
{
    Socket *s;
    bool *update;
} recvThreadParams_t;

typedef std::pair<pthread_t, recvThreadParams_t> RecvThreadId;

typedef struct
{
    bool *update;
} workerThreadParams_t;

typedef boost::property<boost::edge_weight_t, int>  EdgeWeight;
typedef boost::adjacency_list < boost::listS, boost::vecS, boost::undirectedS, 
    boost::no_property, boost::property<boost::edge_weight_t, int> > graph_t;
typedef boost::graph_traits<graph_t>::vertex_descriptor vertex_descriptor;
typedef boost::graph_traits<graph_t>::edge_descriptor edge_descriptor;

uint32_t ASno=99;

pthread_mutex_t graphMutex;

EdgeMap edges;

/* For nodes:
 * second is the number of routers are connected to this net
 * For remoteNodes:
 * second is the current AS_hops to that net
 */
NodeMap nodes, remoteNodes;

void pdie(const char * msg, int rc=1)
{
    perror(msg);
    _exit(rc);
}

void sendBGP(auto_ptr<BGP> b)
{

}

void recvLSA(Message *in)
{
    auto_ptr<LSA> r(dynamic_cast<LSA *> (in));
    cout << r.get();
    uint32_t id = r->getRouterID();
    for(LinkMap::iterator it = r->getLinkMap()->begin();
        it != r->getLinkMap()->end(); ++it)
    {
        MutexLocker lock(&graphMutex);
        for(LinkMap::iterator it = r->getLinkMap()->begin();
            it != r->getLinkMap()->end(); ++it)
        {
            pair<NodeMap::iterator, bool> test = nodes.insert(NodeMapEntry(it->first, 1));
            if(test.second == false)
            {
                test.first->second++;
            }
        }

        for(LinkMap::iterator iter1 = r->getLinkMap()->begin();
            iter1 != r->getLinkMap()->end(); ++iter1)
        {
            for(LinkMap::iterator iter2 = iter1;
                iter2 != r->getLinkMap()->end(); ++iter2)
            {
                if(iter2 == iter1) continue;

                Edge newEdge(iter1->first, iter2->first);
                if(iter1->second == 99 || iter2->second == 99)
                {
                    edges.erase(newEdge);
                }
                else
                {
                    uint32_t metric = max(iter1->second, iter2->second);

                    pair<EdgeMap::iterator, bool> test = edges.insert(EdgeMapEntry(newEdge, EdgeMapValue(id, metric)));
                    if(test.second == false)
                    {
                        test.first->second.second = metric;
                    }
                }
            }
        }
    }

}

void recvBGP(Message *in)
{
    auto_ptr<BGP> b(dynamic_cast<BGP *> (in));

    for(vector<uint32_t>::iterator it = b->nets.begin(); it != b->nets.end(); ++it)
    {
        if(nodes.find(*it) != nodes.end())
        {
            return;
        }
    }

    b->AS_hops++;

    for(vector<uint32_t>::iterator it = b->nets.begin(); it != b->nets.end(); ++it)
    {
        NodeMap::iterator nodeIt = nodes.find(*it);
        if(nodeIt == nodes.end())
        {
            nodes.insert(NodeMapEntry(*it, b->AS_hops));
        }
        else
        {
            if(b->AS_hops < nodeIt->second)
            {
                nodeIt->second = b->AS_hops;
            }
        }
    }

    sendBGP(b);

}

void recvRREQ(Message *in)
{
    auto_ptr<RREQ> r(dynamic_cast<RREQ *> (in));
}

void recvRRES(Message *in)
{
    auto_ptr<RRES> r(dynamic_cast<RRES *> (in));
}

void recvIRRQ(Message *in)
{
    auto_ptr<IRRQ> r(dynamic_cast<IRRQ *> (in));
}

void recvIRRS(Message *in)
{
    auto_ptr<IRRS> r(dynamic_cast<IRRS *> (in));
}

void * recvThread(void *params)
{
    Socket *s = ((recvThreadParams_t *)params)->s;
    bool *update = ((recvThreadParams_t *)params)->update;

    while ( s->isConnected() )
    {
        Message *in;
        try {
            in=s->getMessage();
        }
        catch (Socket::NotConnectedException e) {
            continue;
        }

        string type = in->getType();

        if(type == "LSA")
        {
            recvLSA(in);
        }
        else if(type == "BGP")
        {
            recvBGP(in);
        }
        else if(type == "RREQ")
        {
            recvRREQ(in);
        }
        else if(type == "RRES")
        {
            recvRRES(in);
        }
        else if(type == "IRRQ")
        {
            recvIRRQ(in);
        }
        else if(type == "IRRS")
        {
            recvIRRS(in);
        }
        else
        {
            cerr << "Invalid Message Type" << endl;
            break;
        }

        *update = true;

    }

    return NULL;
}

void *workerThread(void *param)
{
    bool *update = ((workerThreadParams_t*)param)->update;

    unsigned numEdges = 0;
    unsigned numNodes = 0;

    graph_t localGraph, remoteGraph;
    vector<vertex_descriptor> localP, remoteP;
    vector<int> localD, remoteD;

    while(1)
    {

        if(*update)
        {

            boost::graph_traits < graph_t >::vertex_iterator vt, vtend;
            {
                MutexLocker graphLock(&graphMutex);

                numEdges = edges.size();
                numNodes = nodes.size();

                if(numNodes && numEdges)
                {
                    localGraph = graph_t(numNodes);
                    
                        for(EdgeMap::iterator it = edges.begin(); it != edges.end(); ++it)
                        {
                            boost::add_edge(it->first.first, it->first.second, EdgeWeight(it->second.second), localGraph);
                        }
    
                    localP = vector<vertex_descriptor>(boost::num_vertices(localGraph));
                    localD = vector<int>(boost::num_vertices(localGraph));
    
                    vertex_descriptor s = boost::vertex(nodes.begin()->first, localGraph);
    
                    boost::dijkstra_shortest_paths(localGraph, s, boost::predecessor_map(&localP[0]).distance_map(&localD[0]));
    
                    boost::tie(vt, vtend) = boost::vertices(localGraph);
                }
                *update = false;
            }
            
            if(numNodes && numEdges)
            {
                for( ; vt != vtend; ++vt)
                {
                    cout << "distance(" << *vt << ") = " << localD[*vt] << ", ";
                    cout << "parent(" << *vt << ") = " << localP[*vt] << endl;
                }
            }
        }


        sleep(1);
    }

    return NULL;

}

int main(int argc, char ** argv)
{
    if(argc == 1)
        PCE_usage(NULL, true, 0);
    else if(argc != 3)
        PCE_usage("invalid number of arguments");

    try { ASno = boost::lexical_cast<uint32_t>(argv[1]); }
    catch (...) { PCE_usage("First argument must be integer"); }
    PCEconfig config(argv[2]);

    cout << "AS: " << ASno << endl;
    cout << config;

    AS me = config.getAS(ASno);

    ListenSocket sock(me.portno);

    pthread_mutex_init(&graphMutex, NULL);
    
    // Will be used to signal the workerThread to run dijkstra again
    // Better ideas?
    bool update = false;  

    pthread_t workerId;
    workerThreadParams_t workerParams;
    workerParams.update = &update;
    pthread_create(&workerId, 0, workerThread, &workerParams);
    
    vector<RecvThreadId*> threadIds;
    
    while(1)
    {
        RecvThreadId *id = new RecvThreadId();    
        id->second.s = new Socket(sock.acceptConnection());
        id->second.update = &update;

        pthread_create(&(id->first), 0, recvThread, &(id->second));

        threadIds.push_back(id);

        for(vector<RecvThreadId*>::iterator it = threadIds.begin();
            it != threadIds.end(); ++it)
        {
            int ret = pthread_tryjoin_np((*it)->first, NULL);
            if(ret == 0)
            {
                delete (*it)->second.s;
                (*it)->second.s = NULL;

                delete (*it);
                (*it) = NULL;

                threadIds.erase(it);
            }
        }
    }

    pthread_tryjoin_np(workerId, NULL);

/*
    for(vector<recvThreadParams_t*>::iterator it = threadParams.begin();
        it != threadParams.end(); ++it)
    {
        delete *it;
    }

    for(vector<pthread_t*>::iterator it = threadIds.begin();
        it != threadIds.end(); ++it)
    {
        delete *it;
    }
*/

    return 0;

}
