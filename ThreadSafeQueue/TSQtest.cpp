#include <stdint.h>
#include <iostream>
#include "TSQ.h"

using namespace std;

typedef ThreadSafeQueue<uint32_t> intq;

bool stopping=false;
void *consumer(void *param)
{
    intq *q = (intq *)param;
    while(!stopping)
        cout << "Popped: " << q->pop() << endl;
    cout << "Worker Stopping" << endl;
    return NULL;
}

int main(int argc, char ** argv)
{
    pthread_t workerId;
    intq * q = new intq();

    pthread_create(&workerId, NULL, &consumer, q);
    for(uint32_t i=0; i < 100000 ; i++)
        q->push(i);
    q->waitOnEmpty();
    stopping=true;
    delete q;
    pthread_join(workerId);
}
