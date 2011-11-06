#include <stdio.h>
#include <iostream>
#include <boost/lexical_cast.hpp>

#include <vector>
#include <map>

extern "C"
{
# include <pthread.h>
}

#include "usage.h"
#include "PCEconfig.h"
#include "MessageContainer.h"
#include "Socket.h"


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
EdgeMap edges;

typedef struct 
{
    Socket *s;
    pthread_mutex_t *mutex;
} recvThreadParams_t;

typedef std::pair<pthread_t, recvThreadParams_t> RecvThreadId;

typedef struct
{
    pthread_mutex_t *mutex;
} workerThreadParams_t;

void pdie(const char * msg, int rc=1)
{
    perror(msg);
    _exit(rc);
}

void * recvThread(void *params)
{
    Socket *s = ((recvThreadParams_t *)params)->s;
    pthread_mutex_t *mutex = ((recvThreadParams_t *)params)->mutex;
    EdgeMap myEdges;

    while (1)
    {
        MessageContainer out;
        try
        {
            out=s->getMessage();
        } catch(...)
        {
            break;
        }

        RouterStatus * r;
        r=(RouterStatus *) out.getMessage();
        /*
        cout << "received router status: " << endl;
        cout << *r;
        */

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
    
                pthread_mutex_lock(mutex);
                if(state)
                {
                    edges.insert(EdgeMapEntry(newEdge, state));
                }
                else if(!state)
                {
                    edges.erase(newEdge);
                }
                pthread_mutex_unlock(mutex);
            }
        }
    }

    for(EdgeMap::iterator it = myEdges.begin(); it != myEdges.end(); ++it)
        edges.erase(it->first);

    return NULL;
}

void *workerThread(void *param)
{
    pthread_mutex_t *mutex = ((workerThreadParams_t*)param)->mutex;

    unsigned numEdges = 0;

    while(1)
    {
        pthread_mutex_lock(mutex);
        unsigned tmpNum = edges.size();
        if(numEdges != tmpNum)
        {
            numEdges = tmpNum;
            for(EdgeMap::iterator it = edges.begin(); it != edges.end(); ++it)
            {
                cout << it->first.first << " -- " << it->first.second << endl;
            }
        }
        pthread_mutex_unlock(mutex);

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
    
    pthread_t workerId;
    workerThreadParams_t workerParams;
    workerParams.mutex = &edgeMutex;
    pthread_create(&workerId, 0, workerThread, &workerParams);
    
    vector<RecvThreadId*> threadIds;
    
    while(1)
    {
        RecvThreadId *id = new RecvThreadId();    
        id->second.s = new Socket(sock.acceptConnection());
        id->second.mutex = &edgeMutex;

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
