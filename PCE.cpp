#include <stdio.h>
#include <climits>
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

typedef pair<uint32_t, vector<uint32_t> > LocalNodeMapEntry;
typedef map<uint32_t, vector<uint32_t> > LocalNodeMap;

// value is ASno and hops
typedef pair<uint32_t, uint32_t> RemoteNodeMapValue;
typedef pair<uint32_t, RemoteNodeMapValue> RemoteNodeMapEntry;
typedef map<uint32_t, RemoteNodeMapValue> RemoteNodeMap;

//ASs our AS is directly connected to
//          myRouterList        remoteRouterList
typedef pair<vector<uint32_t>, vector<uint32_t> > ConnectedASMapValue;
//          ASno        Value
typedef map<uint32_t, ConnectedASMapValue > ConnectedASMap;

typedef struct 
{
    Socket *s;
} recvThreadParams_t;

typedef pair<pthread_t, recvThreadParams_t> RecvThreadId;

typedef boost::property<boost::edge_weight_t, int>  EdgeWeight;
typedef boost::adjacency_list < boost::listS, boost::vecS, boost::undirectedS, 
    boost::no_property, boost::property<boost::edge_weight_t, int> > graph_t;
typedef boost::graph_traits<graph_t>::vertex_descriptor vertex_descriptor;
typedef boost::graph_traits<graph_t>::edge_descriptor edge_descriptor;

uint32_t ASno=99;

pthread_mutex_t graphMutex;

EdgeMap edges;

LocalNodeMap nodes;
RemoteNodeMap remoteNodes;

ConnectedASMap connected_AS;

class MessageResponder
{
private:
    Message *in;
    Socket *s;

    RRES localDijkstra(uint32_t startNode, uint32_t endNode, bool router2router=false);

    void recvLSA();
    void recvBGP();
    void recvRREQ();
    void recvRRES();
    void recvIRRQ();
    void recvIRRS();

public:
    MessageResponder(Message *newIn, Socket *newS) 
        : in(newIn), s(newS) {}
    ~MessageResponder() {}

    void recv();

};

void pdie(const char * msg, int rc=1)
{
    perror(msg);
    _exit(rc);
}

PCEconfig *config;
void sendBGP(BGP &b, uint32_t from=0xffffffff)
{
    //implement sending the BGP message to all the PCEs
    //in class, someone brought up a good point (GASP!)
    //we shouldn't send BGP messages to ASs that we don't know if they're neighbors or not
    //we'll have to talk about how to implement that
    ConnectedASMap::const_iterator it;
    for(it = connected_AS.begin(); it != connected_AS.end(); ++it)
    {
        if(it->first != from) {
            AS a=config->getAS(it->first);
            try {
                Socket s( &a.saddr);
                s.sendMessage(b);
            } 
            catch(...) {
                cerr << "Failed to connect to the remote PCE" << endl;
            }

        }
    }
}
//Done --matt

IRRS sendIRRQ(uint32_t dest)
{
    IRRS resp;

    RemoteNodeMap::iterator it = remoteNodes.find(dest);
    if(it != remoteNodes.end())
    {
        IRRQ r;
        r.AS = ASno;
        r.dest_net = dest;

        AS a = config->getAS(it->second.first);
        Socket *s = NULL;
        try {
            s = new Socket( &a.saddr);
        }
        catch(...) {
            cerr << "Failed to connect to the remote PCE" << endl;
        }

        if(s->isConnected())
        {
            IRRS *m;
            try {
                s->sendMessage(r);
                m = (IRRS*)s->getMessage();

                resp = *m;
            }
            catch(...) {
                cerr << "Failed to get min route" << endl;
            }
        }
        if(s != NULL) {
            delete s;
        }
    }

    return resp;
}

RRES MessageResponder::localDijkstra(uint32_t startNode, uint32_t endNode, bool router2router)
{
    unsigned numEdges = 0;
    unsigned numNodes = 0;

    graph_t localGraph;
    vector<vertex_descriptor> localP;
    vector<int> localD;

    RRES resp;

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
    
            vertex_descriptor s = boost::vertex(startNode, localGraph);
    
            boost::dijkstra_shortest_paths(localGraph, s, boost::predecessor_map(&localP[0]).distance_map(&localD[0]));
    
        }
    }

    while(endNode != startNode)
    {
        if(localP[endNode] == endNode)
        {
            break;
        }

        if(router2router)
        {
            resp.routers.insert(resp.routers.begin(), endNode-((ASno+1)*100));
        }
        else
        {
            resp.routers.insert(resp.routers.begin(), (localP[endNode]-((ASno+1)*100)));
        }

        endNode = localP[localP[endNode]];
    }

    return resp;
}

void MessageResponder::recvLSA()
{
    auto_ptr<LSA> r(dynamic_cast<LSA *> (in));
    cout << r.get();
    uint32_t id = r->routerID;
    uint32_t neighbor = r->neighborAS;

    MutexLocker lock(&graphMutex);

    //update connected_AS map
    if(neighbor != 99) {
        if( connected_AS.find(neighbor) == connected_AS.end() )
            connected_AS[neighbor] = ConnectedASMapValue(vector<uint32_t>(), vector<uint32_t>());
        if( find( connected_AS[neighbor].first.begin(), connected_AS[neighbor].first.end(), id) == connected_AS[neighbor].first.end() )
        {
            connected_AS[neighbor].first.push_back(id);
            connected_AS[neighbor].second.push_back(r->neighborRouterID);
        }
    }

    vector<uint32_t> nets;
    for(LinkMap::iterator it = r->getLinkMap()->begin();
        it != r->getLinkMap()->end(); ++it)
    {
        nets.push_back(it->first);
        remoteNodes.insert(RemoteNodeMapEntry(it->first, RemoteNodeMapValue(ASno, 0)));
    }

    pair<LocalNodeMap::iterator, bool> test = nodes.insert(LocalNodeMapEntry(id, nets));
    if(test.second == false)
    {
        test.first->second = nets;
    }

    uint32_t routerAsNode = id+((ASno+1)*100);

        for(LinkMap::iterator iter1 = r->getLinkMap()->begin();
            iter1 != r->getLinkMap()->end(); ++iter1)
        {
            Edge newEdge(iter1->first, routerAsNode);
            if(iter1->second == 99)
            {
                edges.erase(newEdge);
            }
            else
            {
                uint32_t metric = iter1->second;

                pair<EdgeMap::iterator, bool> test = edges.insert(EdgeMapEntry(newEdge, EdgeMapValue(id, metric)));
                if(test.second == false)
                {
                    test.first->second.second = metric;
                }
            }
        }

    uint32_t nodesSent = 0, curHops = 0;
    while(nodesSent < remoteNodes.size())
    {
        //just use stack var for this...
        BGP b;
        b.AS = ASno;
        b.AS_hops = curHops;


        for(RemoteNodeMap::iterator it = remoteNodes.begin(); it != remoteNodes.end(); ++it)
        {
            if(it->second.second == curHops)
            {
                b.nets.push_back(it->first);
            }
        }

        nodesSent += b.nets.size();

        sendBGP(b);
        curHops++;
    }


}

void MessageResponder::recvBGP()
{
    auto_ptr<BGP> b(dynamic_cast<BGP *> (in));

    b->AS_hops++;
    bool changed=false;

    for(vector<uint32_t>::iterator it = b->nets.begin(); it != b->nets.end(); ++it)
    {
        pair<RemoteNodeMap::iterator, bool> test = remoteNodes.insert(RemoteNodeMapEntry(*it, RemoteNodeMapValue(b->AS, b->AS_hops)));
        if(test.second == false)
        {
            if(b->AS_hops < test.first->second.second)
            {
                test.first->second = RemoteNodeMapValue(b->AS, b->AS_hops);
                changed=true;
            }
        }
        else
        {
            changed = true;
        }
    }

    if(changed) {
        uint32_t from = b->AS;
        b->AS=ASno;
        b->AS_hops++;
        sendBGP( (*b.get()), from );
    }

}

void MessageResponder::recvRREQ()
{
    auto_ptr<RREQ> r(dynamic_cast<RREQ *> (in));

    uint32_t source = 0;
    source = r->source + ((ASno+1) * 100);

    RRES res;
    
    uint32_t distance = remoteNodes[r->dest].second;
    if(distance == 0)
    {
        res = localDijkstra(source, r->dest);
    }
    else
    {
        IRRS m;
        m = sendIRRQ(r->dest);
        cout << &m;
        if(!m.blank)
        {
            uint32_t sourceAS = m.ASlist.front().first;
            uint32_t destRouter = m.ASlist.front().second.front();
            uint32_t myRouter = UINT_MAX;
            for(uint32_t i = 0; i < connected_AS[sourceAS].second.size(); i++)
            {
                if(connected_AS[sourceAS].second[i] == destRouter)
                {
                    myRouter = connected_AS[sourceAS].first[i];
                    break;
                }

            }

            res = localDijkstra(source, (myRouter + ((ASno+1)*100)), true);
            cout << &res;
            for(list<ASroute>::iterator iter = m.ASlist.begin(); iter != m.ASlist.end(); ++iter) {
                res.routers.insert(res.routers.end(), iter->second.begin(), iter->second.end());
            }
        }
    }

    if(res.routers.empty())
    {
        res.routers.push_back(99);
    }


    s->sendMessage(res);
}

void MessageResponder::recvRRES()
{
    auto_ptr<RRES> r(dynamic_cast<RRES *> (in));
    cerr << "ERROR: EXTRANEOUS RRES MESSAGE!" << endl;
}

void MessageResponder::recvIRRQ()
{
    auto_ptr<IRRQ> r(dynamic_cast<IRRQ *> (in));

    IRRS m;

    if(remoteNodes[r->dest_net].second == 0)
    {
        vector<uint32_t>::iterator it;
        RRES localRoute;
        uint32_t min = UINT_MAX;

        for(it = connected_AS[r->AS].first.begin(); it != connected_AS[r->AS].first.end(); ++it)
        {
            RRES res = localDijkstra((*it + ((ASno+1)*100)), r->dest_net);
            cout << &res << endl;
            if(res.routers.size() < min)
            {
                localRoute = res;
                min = res.routers.size();
            }
        }
        m.ASlist.push_front(ASroute(ASno, localRoute.routers));
        if(m.ASlist.front().second.size() > 0)
        {
            m.blank = false;
        }
    }
    else
    {
        m = sendIRRQ(r->dest_net);

        if(m.blank)
        {
            m.ASlist.push_front(ASroute(ASno, vector<uint32_t>()));
        }
        else
        {
            vector<uint32_t>::iterator it;
            RRES localRoute;
            uint32_t min = UINT_MAX;
            for(it = connected_AS[r->AS].first.begin(); it != connected_AS[r->AS].first.end(); ++it)
            {
                uint32_t sourceAS = m.ASlist.front().first;
                uint32_t destRouter = m.ASlist.front().second.front();
                uint32_t myRouter = UINT_MAX;
                for(uint32_t i = 0; i < connected_AS[sourceAS].second.size(); i++)
                {
                    if(connected_AS[sourceAS].second[i] == destRouter)
                    {
                        myRouter = connected_AS[sourceAS].first[i];
                        break;
                    }
                }

                RRES res = localDijkstra((*it + ((ASno+1)*100)), (myRouter + ((ASno+1)*100)));
                if(res.routers.size() < min)
                {
                    localRoute = res;
                    min = res.routers.size();
                }
            }
            m.ASlist.push_front(ASroute(ASno, localRoute.routers));
    
        }
    }

    s->sendMessage(m);
}

void MessageResponder::recvIRRS()
{
    auto_ptr<IRRS> r(dynamic_cast<IRRS *> (in));
    cerr << "ERROR: EXTRANEOUS IRRS MESSAGE!" << endl;
}

void MessageResponder::recv()
{
    string type = in->getType();

    if(type == "LSA")
    {
        recvLSA();
    }
    else if(type == "BGP")
    {
        recvBGP();
    }
    else if(type == "RREQ")
    {
        recvRREQ();
    }
    else if(type == "RRES")
    {
        recvRRES();
    }
    else if(type == "IRRQ")
    {
        recvIRRQ();
    }
    else if(type == "IRRS")
    {
        recvIRRS();
    }
    else
    {
        cerr << "Invalid Message Type" << endl;
    }

    return;
}

void * recvThread(void *params)
{
    Socket *s = ((recvThreadParams_t *)params)->s;

    while ( s->isConnected() )
    {
        Message *in;
        try {
            in=s->getMessage();
        }
        catch (Socket::NotConnectedException e) {
            continue;
        }

        MessageResponder responder(in, s);
        responder.recv();

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
    config = new PCEconfig(argv[2]);

    cout << "AS: " << ASno << endl;
    cout << *config;

    AS me = config->getAS(ASno);

    ListenSocket sock(me.portno);

    pthread_mutex_init(&graphMutex, NULL);
    
    vector<RecvThreadId*> threadIds;

    while(1)
    {
        RecvThreadId *id = new RecvThreadId();    
        id->second.s = new Socket(sock.acceptConnection());

        pthread_create(&(id->first), 0, recvThread, &(id->second));

        threadIds.push_back(id);

#ifndef __CYGWIN__    
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
#endif //__CYGWIN
    }

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
