extern "C"
{
# include <unistd.h>
# include <stdint.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <ctype.h>
# include <pthread.h>
}

#ifndef __UTILS_H__
#define __UTILS_H__

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

#endif //__UTILS_H__
