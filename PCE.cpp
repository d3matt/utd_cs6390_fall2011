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
    pthread_mutex_t *graphMutex;
    bool *update;
} recvThreadParams_t;

typedef std::pair<pthread_t, recvThreadParams_t> RecvThreadId;

typedef struct
{
    pthread_mutex_t *graphMutex;
    bool *update;
} workerThreadParams_t;

typedef boost::property<boost::edge_weight_t, int>  EdgeWeight;
typedef boost::adjacency_list < boost::listS, boost::vecS, boost::undirectedS, 
    boost::no_property, boost::property<boost::edge_weight_t, int> > graph_t;
typedef boost::graph_traits<graph_t>::vertex_descriptor vertex_descriptor;
typedef boost::graph_traits<graph_t>::edge_descriptor edge_descriptor;

EdgeMap edges;
NodeMap nodes;

void pdie(const char * msg, int rc=1)
{
    perror(msg);
    _exit(rc);
}

void * recvThread(void *params)
{
    Socket *s = ((recvThreadParams_t *)params)->s;
    pthread_mutex_t *graphMutex = ((recvThreadParams_t *)params)->graphMutex;
    bool *update = ((recvThreadParams_t *)params)->update;

    EdgeMap myEdges;
    NodeMap myNodes;

    while ( s->isConnected() )
    {
        Message *in;
        try {
            in=s->getMessage();
        }
        catch (Socket::NotConnectedException e) {
            continue;
        }
        auto_ptr<LSA> r(dynamic_cast<LSA *> (in));
        
        cout << "received router status: " << endl;
        cout << r.get();

        uint32_t id = r->getRouterID();
        
        for(LinkMap::iterator it = r->getLinkMap()->begin();
            it != r->getLinkMap()->end(); ++it)
        {
            MutexLocker lock(graphMutex);
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
            *update = true;
        }

    }

    return NULL;
}

void *workerThread(void *param)
{
    pthread_mutex_t *graphMutex = ((workerThreadParams_t*)param)->graphMutex;
    bool *update = ((workerThreadParams_t*)param)->update;

    unsigned numEdges = 0;
    unsigned numNodes = 0;

    graph_t localGraph;

    vector<vertex_descriptor> p;
    vector<int> d;

    while(1)
    {

        if(*update)
        {

            boost::graph_traits < graph_t >::vertex_iterator vt, vtend;
            {
                MutexLocker graphLock(graphMutex);

                numEdges = edges.size();
                numNodes = nodes.size();

                if(numNodes && numEdges)
                {
                    localGraph = graph_t(numNodes);
                    
                        for(EdgeMap::iterator it = edges.begin(); it != edges.end(); ++it)
                        {
                            boost::add_edge(it->first.first, it->first.second, EdgeWeight(it->second.second), localGraph);
                        }
    
                    p = vector<vertex_descriptor>(boost::num_vertices(localGraph));
                    d = vector<int>(boost::num_vertices(localGraph));
    
                    vertex_descriptor s = boost::vertex(nodes.begin()->first, localGraph);
    
                    boost::dijkstra_shortest_paths(localGraph, s, boost::predecessor_map(&p[0]).distance_map(&d[0]));
    
                    boost::tie(vt, vtend) = boost::vertices(localGraph);
                }
                *update = false;
            }
            
            if(numNodes && numEdges)
            {
                for( ; vt != vtend; ++vt)
                {
                    cout << "distance(" << *vt << ") = " << d[*vt] << ", ";
                    cout << "parent(" << *vt << ") = " << p[*vt] << endl;
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

    uint32_t ASno=99;

    try { ASno = boost::lexical_cast<uint32_t>(argv[1]); }
    catch (...) { PCE_usage("First argument must be integer"); }
    PCEconfig config(argv[2]);

    cout << "AS: " << ASno << endl;
    cout << config;

    AS me = config.getAS(ASno);

    ListenSocket sock(me.portno);

    pthread_mutex_t graphMutex;
    pthread_mutex_init(&graphMutex, NULL);
    
    // Will be used to signal the workerThread to run dijkstra again
    // Better ideas?
    bool update = false;  

    pthread_t workerId;
    workerThreadParams_t workerParams;
    workerParams.graphMutex = &graphMutex;
    workerParams.update = &update;
    pthread_create(&workerId, 0, workerThread, &workerParams);
    
    vector<RecvThreadId*> threadIds;
    
    while(1)
    {
        RecvThreadId *id = new RecvThreadId();    
        id->second.s = new Socket(sock.acceptConnection());
        id->second.graphMutex = &graphMutex;
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
