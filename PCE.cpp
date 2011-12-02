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

using namespace std;
using namespace cs6390;

//simple class meant to be used as a stack variable.
//  constructor locks the mutex
//  destructor unlocks the mutex
class MutexLocker
{
private:
    pthread_mutex_t *mutex;
public:
    MutexLocker(pthread_mutex_t *inMutex) : mutex(inMutex) {pthread_mutex_lock(mutex);}
    ~MutexLocker() {pthread_mutex_unlock(mutex);}
};

typedef pair<uint32_t, uint32_t> Edge;

// Structure that is used to compare edges (useful for map ordering)
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

// A c++ way of getting a max
inline uint32_t max(uint32_t left, uint32_t right) {if(left >= right) return left; else return right;}

// The EdgeMap is a map of the edges, with an Edge as a key
// The EdgeMapvValue is the routerId this edge belongs to, and the metric of the edge
// Value is routerID, metric
typedef pair<uint32_t, uint32_t> EdgeMapValue;
typedef pair<Edge, EdgeMapValue> EdgeMapEntry;
typedef map<Edge, EdgeMapValue, EdgeLess> EdgeMap;

// LocalNodeMap is just a map of the networks that are connected to a router
typedef pair<uint32_t, vector<uint32_t> > LocalNodeMapEntry;
typedef map<uint32_t, vector<uint32_t> > LocalNodeMap;

// RemoteNodeMap is the same as LocalNodeMap, except it tells what AS I need to go through to get to a specific network
// value is ASno and hops
typedef pair<uint32_t, uint32_t> RemoteNodeMapValue;
typedef pair<uint32_t, RemoteNodeMapValue> RemoteNodeMapEntry;
typedef map<uint32_t, RemoteNodeMapValue> RemoteNodeMap;

// ASs our AS is directly connected to
//          myRouterList        remoteRouterList
typedef pair<vector<uint32_t>, vector<uint32_t> > ConnectedASMapValue;
//          ASno        Value
typedef map<uint32_t, ConnectedASMapValue > ConnectedASMap;

// Structure to hold the parameters of the receive thread
typedef struct 
{
    Socket *s;
} recvThreadParams_t;

// A single object to hold the id and params of a thread
typedef pair<pthread_t, recvThreadParams_t> RecvThreadId;

// Typedefs for the boost graph
typedef boost::property<boost::edge_weight_t, int>  EdgeWeight;
typedef boost::adjacency_list < boost::listS, boost::vecS, boost::undirectedS, 
    boost::no_property, EdgeWeight > graph_t;
typedef boost::graph_traits<graph_t>::vertex_descriptor vertex_descriptor;
typedef boost::graph_traits<graph_t>::edge_descriptor edge_descriptor;

// Global my AS number
uint32_t ASno=99;

pthread_mutex_t graphMutex;

EdgeMap edges;

LocalNodeMap nodes;
RemoteNodeMap remoteNodes;

ConnectedASMap connected_AS;

//A simple class to respond to all messages that come in.
//The constructor takes in the new message, and it dispatches the correct function
//It also includes a function to run Dijkstra on the local nodes
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

//prints a message and quits
void pdie(const char * msg, int rc=1)
{
    perror(msg);
    _exit(rc);
}

PCEconfig *config;
//sends a BGP message to all connected neighbors
void sendBGP(BGP &b, uint32_t from=0xffffffff)
{
    //implement sending the BGP message to all the PCEs
    //in class, someone brought up a good point (GASP!)
    //we shouldn't send BGP messages to ASs that we don't know if they're neighbors or not
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

//Sends an IRRQ message to a specific AS looking for a destination network
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

//Runs dijkstra on the nodes within the AS
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

		//If there are nodes AND edges in the network
        if(numNodes && numEdges)
        {
			//Build a boost graph
            localGraph = graph_t(numNodes);
            
            for(EdgeMap::iterator it = edges.begin(); it != edges.end(); ++it)
            {
                boost::add_edge(it->first.first, it->first.second, EdgeWeight(it->second.second), localGraph);
            }
    
            localP = vector<vertex_descriptor>(boost::num_vertices(localGraph));
            localD = vector<int>(boost::num_vertices(localGraph));
    
			//Get a start node in boost terms
            vertex_descriptor s = boost::vertex(startNode, localGraph);
    
			//Run dijkstra
            boost::dijkstra_shortest_paths(localGraph, s, boost::predecessor_map(&localP[0]).distance_map(&localD[0]));
    
        }
    }

	//We move the end node back until it equals the start node.
	//Then each node the endNode is equal to along the way is part of the path.
	//If the endNode ever equals its parent, there is no path
    while(endNode != startNode)
    {
        if(localP[endNode] == endNode)
        {
			//No path
            break;
        }

		// We have to handle paths differently if they're from a router to a router, or from a router to a network
        if(router2router)
        {
			//Insert the router in to the path
            resp.routers.insert(resp.routers.begin(), endNode-((ASno+1)*100)); //See comment at line 306
        }
        else
        {
			//Insert the router between two networks in to the path
            resp.routers.insert(resp.routers.begin(), (localP[endNode]-((ASno+1)*100))); //See comment at line 306
        }

		//Make the endNode equal to it's parent's parent (The next router in the path)
        endNode = localP[localP[endNode]];
    }

    return resp;
}

//Process the LSA message from routers
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

	//I can reach all the nodes in this LSA in 0 AS hops
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

	//Here, we store the router as a node in the graph.
	//Since there are only 99 networks, all ids above number 100 are free to use.
	//Since there are only 10 routers in a given AS, each AS can have 100 ids for its routers
	//The id of a router is generated by adding (AS+1)*100 to the routerId.
	//This way, router 0 in AS 0 has ID 100
	//router 0 in AS 1 has ID 200, and so on...
    uint32_t routerAsNode = id+((ASno+1)*100);

	for(LinkMap::iterator iter1 = r->getLinkMap()->begin();
		iter1 != r->getLinkMap()->end(); ++iter1)
	{
		Edge newEdge(iter1->first, routerAsNode);
		//Don't even save networks with metric 99, they are unreachable
		if(iter1->second == 99)
		{
			edges.erase(newEdge);
		}
		else
		{
			uint32_t metric = iter1->second;

			//Add each network in the LSA to the edge map
			pair<EdgeMap::iterator, bool> test = edges.insert(EdgeMapEntry(newEdge, EdgeMapValue(id, metric)));
			if(test.second == false)
			{
				//If it already exists, update the metric
				test.first->second.second = metric;
			}
		}
	}

    uint32_t nodesSent = 0, curHops = 0;
	//Send out new BGP messages
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

//Process BGP messages
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

//Process RREQ messages
void MessageResponder::recvRREQ()
{
    auto_ptr<RREQ> r(dynamic_cast<RREQ *> (in));

    uint32_t source = 0;
    source = r->source + ((ASno+1) * 100);

    RRES res;
    
	//Is it in my AS?
    uint32_t distance = remoteNodes[r->dest].second;
    if(distance == 0)
    {
		//Yes, run dijksstra
        res = localDijkstra(source, r->dest);
    }
    else
    {
		//No, send IRRQ
        IRRS m;
        m = sendIRRQ(r->dest);
        if(!m.blank)
        {
			//process IRRS message
			//Figure out which router i should route to
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

			//Shortest path from current router to dest router in AS
            res = localDijkstra(source, (myRouter + ((ASno+1)*100)), true);
            if(!res.routers.empty())
            {
                for(list<ASroute>::iterator iter = m.ASlist.begin(); iter != m.ASlist.end(); ++iter) {
                    res.routers.insert(res.routers.end(), iter->second.begin(), iter->second.end());
                }
            }
        }
    }

	//There was no route, report
    if(res.routers.empty())
    {
        res.routers.push_back(99);
    }


    s->sendMessage(res);
}

//I should never receive an RRES message out of the blue
void MessageResponder::recvRRES()
{
    auto_ptr<RRES> r(dynamic_cast<RRES *> (in));
    cerr << "ERROR: EXTRANEOUS RRES MESSAGE!" << endl;
}

//Process IRRQ message
void MessageResponder::recvIRRQ()
{
    auto_ptr<IRRQ> r(dynamic_cast<IRRQ *> (in));

    IRRS m;

	//Is the destination in my local AS?
    if(remoteNodes[r->dest_net].second == 0)
    {
		//Yes, find shortest path from source AS connected router to destination network
        vector<uint32_t>::iterator it;
        RRES localRoute;
        uint32_t min = UINT_MAX;

        for(it = connected_AS[r->AS].first.begin(); it != connected_AS[r->AS].first.end(); ++it)
        {
            RRES res = localDijkstra((*it + ((ASno+1)*100)), r->dest_net);
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
		//No, forward IRRQ message to next AS
        m = sendIRRQ(r->dest_net);

		//Did i get a path back?
        if(m.blank)
        {
			//No, add my AS number and return
            m.ASlist.push_front(ASroute(ASno, vector<uint32_t>()));
        }
        else
        {
			//Yes, find shortest path from source AS connected router to destination AS connected router
            vector<uint32_t>::iterator it;
            RRES localRoute;
            uint32_t min = UINT_MAX, min_source = 0;
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

                RRES res = localDijkstra((*it + ((ASno+1)*100)), (myRouter + ((ASno+1)*100)), true);
                if(res.routers.size() < min)
                {
                    localRoute = res;
                    min = res.routers.size();
                    min_source = *it;
                }
            }
			//Is there a route in my AS?
			if(localRoute.routers.empty())
			{
				//No
				m.blank = true;
			}
			else
			{
				//Yes, create path
	            localRoute.routers.insert(localRoute.routers.begin(), min_source);
	            m.ASlist.push_front(ASroute(ASno, localRoute.routers));	
			}
        }
    }

    s->sendMessage(m);
}

//Should not receive an IRRS message out of the blue
void MessageResponder::recvIRRS()
{
    auto_ptr<IRRS> r(dynamic_cast<IRRS *> (in));
    cerr << "ERROR: EXTRANEOUS IRRS MESSAGE!" << endl;
}

//Receive any message and call processor
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

//A thread to receive a message and call MessageResponder
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
	//Process arguments
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

	//Receive messages, for every received message spawn a thread that processes the message
	//We do this because we may have to wait a long time for a response from other ASs
    while(1)
    {
        RecvThreadId *id = new RecvThreadId();    
        id->second.s = new Socket(sock.acceptConnection());

		//create thread
        pthread_create(&(id->first), 0, recvThread, &(id->second));

        threadIds.push_back(id);

//If running in cygwin, pthread_tryjoin_np fails to link
#ifndef __CYGWIN__    
        for(vector<RecvThreadId*>::iterator it = threadIds.begin();
            it != threadIds.end(); ++it)
        {
			//Nonblocking join call, to clean up after threads
            int ret = pthread_tryjoin_np((*it)->first, NULL);
            if(ret == 0)
            {
				//No memory leaks!
                delete (*it)->second.s;
                (*it)->second.s = NULL;

                delete (*it);
                (*it) = NULL;

                threadIds.erase(it);
            }
        }
#endif //__CYGWIN
    }

    return 0;

}
