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

typedef pair<Edge, bool> EdgeMapEntry;
typedef map<Edge, bool, EdgeLess> EdgeMap;

typedef pair<unsigned, unsigned> NodeMapEntry;
typedef map<unsigned, unsigned> NodeMap;

typedef struct 
{
    Socket *s;
    pthread_mutex_t *edgeMutex;
    pthread_mutex_t *nodeMutex;
    pthread_mutex_t *graphMutex;
} recvThreadParams_t;

typedef std::pair<pthread_t, recvThreadParams_t> RecvThreadId;

typedef struct
{
    pthread_mutex_t *edgeMutex;
    pthread_mutex_t *nodeMutex;
    pthread_mutex_t *graphMutex;
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
    pthread_mutex_t *edgeMutex = ((recvThreadParams_t *)params)->edgeMutex;
    pthread_mutex_t *nodeMutex = ((recvThreadParams_t *)params)->nodeMutex;
    pthread_mutex_t *graphMutex = ((recvThreadParams_t *)params)->graphMutex;

    EdgeMap myEdges;
    NodeMap myNodes;

    while ( s->isConnected() )
    {
        MessageContainer out;
        try {
            out=s->getMessage();
        }
        catch (Socket::NotConnectedException e) {
            continue;
        }
        auto_ptr<RouterStatus> r(dynamic_cast<RouterStatus *> (out.getMessage()));
        
        cout << "received router status: " << endl;
        cout << *r;
        

        for(LinkMap::iterator it = r->getLinkMap()->begin();
            it != r->getLinkMap()->end(); ++it)
        {
            if(it->second.state)
            {
                pair<NodeMap::iterator, bool> myTest = myNodes.insert(NodeMapEntry(it->second.net, 1));
                if(myTest.second)
                {
                    MutexLocker nodeLock(nodeMutex);
                    {
                        pair<NodeMap::iterator, bool> test = nodes.insert(NodeMapEntry(it->second.net, 1));
                        if(test.second == false)
                        {
                            unsigned count = test.first->second + 1;
                            nodes.erase(it->second.net);
                            nodes.insert(NodeMapEntry(it->second.net, count));
                        }
                    }
                }
            }
            else
            {
                if(myNodes.erase(it->second.net) > 0)
                {
                    MutexLocker nodeLock(nodeMutex);
                    {
                        NodeMap::iterator test = nodes.find(it->second.net);
                        if(test->second == 1)
                        {
                            nodes.erase(it->second.net);
                        }   
                        else
                        {
                            unsigned count = test->second-1;
                            nodes.erase(it->second.net);
                            nodes.insert(NodeMapEntry(it->second.net, count));
                        }
                    }
                }
            }

        }

        for(LinkMap::iterator iter1 = r->getLinkMap()->begin();
            iter1 != r->getLinkMap()->end(); ++iter1)
        {
            for(LinkMap::iterator iter2 = iter1;
                iter2 != r->getLinkMap()->end(); ++iter2)
            {
                if(iter2 == iter1) continue;

                Edge newEdge(iter1->second.net, iter2->second.net);
                bool state = iter1->second.state && iter2->second.state;

                myEdges.insert(EdgeMapEntry(newEdge, state));
    
                if(state)
                {
                    MutexLocker graphLock(graphMutex);
                    {
                        MutexLocker edgeLock(edgeMutex);
                        edges.insert(EdgeMapEntry(newEdge, state));
                    }
                }
                else
                {
                    MutexLocker graphLock(graphMutex);
                    {
                        MutexLocker edgeLock(edgeMutex);
                        edges.erase(newEdge);
                    }
                }
            }
        }
    }

    {
        MutexLocker graphLock(graphMutex);
        for(NodeMap::iterator it = myNodes.begin(); it != myNodes.end(); ++it)
        {
            MutexLocker nodeLock(nodeMutex);
            {
                NodeMap::iterator test = nodes.find(it->first);
                if(test->second == 1)
                {
                    nodes.erase(it->first);
                }
                else
                {
                    unsigned count = test->second-1;
                    nodes.erase(it->first);
                    nodes.insert(NodeMapEntry(it->first, count));
                }
            }
        }

        for(EdgeMap::iterator it = myEdges.begin(); it != myEdges.end(); ++it)
        {
            MutexLocker edgeLock(edgeMutex);
            edges.erase(it->first);
        }

    }
    cerr << "thread exit" << endl;
    return NULL;
}

void *workerThread(void *param)
{
    pthread_mutex_t *edgeMutex = ((workerThreadParams_t*)param)->edgeMutex;
    pthread_mutex_t *nodeMutex = ((workerThreadParams_t*)param)->nodeMutex;
    pthread_mutex_t *graphMutex = ((workerThreadParams_t*)param)->graphMutex;

    unsigned numEdges = 0;
    unsigned numNodes = 0;

    graph_t g;

    vector<vertex_descriptor> p;
    vector<int> d;

    while(1)
    {
        unsigned tmpNum;

        {
            MutexLocker graphLock(graphMutex);
            MutexLocker edgeLock(edgeMutex);
            tmpNum = edges.size();
        }

        if(numEdges != tmpNum)
        {
            numEdges = tmpNum;

            boost::graph_traits < graph_t >::vertex_iterator vt, vtend;
            {
                MutexLocker graphLock(graphMutex);
                {
                    MutexLocker nodeLock(nodeMutex);
                    numNodes = nodes.size();
                }

                if(numNodes && numEdges)
                {
                    g = graph_t(numNodes);
                    
                    {
                        MutexLocker lock(edgeMutex);
                        for(EdgeMap::iterator it = edges.begin(); it != edges.end(); ++it)
                        {
                            cout << it->first.first << " -- " << it->first.second << endl;
                            boost::add_edge(it->first.first, it->first.second, EdgeWeight(1), g);
                        }
                    }
    
                    p = vector<vertex_descriptor>(boost::num_vertices(g));
                    d = vector<int>(boost::num_vertices(g));
    
                    vertex_descriptor s = boost::vertex(nodes.begin()->first, g);
    
                    boost::dijkstra_shortest_paths(g, s, boost::predecessor_map(&p[0]).distance_map(&d[0]));
    
                    boost::tie(vt, vtend) = boost::vertices(g);
                }
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

    pthread_mutex_t edgeMutex;
    pthread_mutex_init(&edgeMutex, NULL);

    pthread_mutex_t nodeMutex;
    pthread_mutex_init(&nodeMutex, NULL);

    pthread_mutex_t graphMutex;
    pthread_mutex_init(&graphMutex, NULL);
    
    pthread_t workerId;
    workerThreadParams_t workerParams;
    workerParams.edgeMutex = &edgeMutex;
    workerParams.nodeMutex = &nodeMutex;
    workerParams.graphMutex = &graphMutex;
    pthread_create(&workerId, 0, workerThread, &workerParams);
    
    vector<RecvThreadId*> threadIds;
    
    while(1)
    {
        RecvThreadId *id = new RecvThreadId();    
        id->second.s = new Socket(sock.acceptConnection());
        id->second.edgeMutex = &nodeMutex;
        id->second.nodeMutex = &nodeMutex;
        id->second.graphMutex = &graphMutex;

        pthread_create(&(id->first), 0, recvThread, &(id->second));

        threadIds.push_back(id);

        for(vector<RecvThreadId*>::iterator it = threadIds.begin();
            it != threadIds.end(); ++it)
        {
            int ret = pthread_tryjoin_np((*it)->first, NULL);
            if(ret == 0)
            {
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

}
