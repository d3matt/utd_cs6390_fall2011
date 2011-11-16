#include <queue>
#include <pthread.h>

//most definitely not lock free...  we're going for correctness here...
template <class T>
class ThreadSafeQueue
{
    std::queue<T>   q;
    pthread_mutex_t M;
    pthread_cond_t  empty;
    pthread_cond_t  notempty;

public:
    ThreadSafeQueue()
    {
        pthread_mutex_init(&M, NULL);
        pthread_cond_init(&empty,NULL);
        pthread_cond_init(&notempty,NULL);
    }
    ~ThreadSafeQueue()
    {
        pthread_mutex_destroy(&M);
        pthread_cond_destroy(&empty);
        pthread_cond_destroy(&notempty);
    }
    void push(T in)
    {
        pthread_mutex_lock(&M);
        q.push(in);
        pthread_cond_signal(&notempty);
        pthread_mutex_unlock(&M);
    }
    //potentially waits forever...
    T pop()
    {
        pthread_mutex_lock(&M);
        if(q.size() == 0)
            pthread_cond_wait(&notempty, &M);
        T retval=q.front();
        q.pop();
        if(q.size() == 0)
            pthread_cond_signal(&empty);
        pthread_mutex_unlock(&M);
        return retval;
    }
    void waitOnEmpty(void)
    {
        pthread_mutex_lock(&M);
        pthread_cond_wait(&empty, &M);
        pthread_mutex_unlock(&M);
    }
};
