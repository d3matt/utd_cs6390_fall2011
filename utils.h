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

uint32_t * string_to_int(const char * str, uint32_t &ret);

bool valid_AS(uint32_t AS);
bool valid_router(uint32_t router);

class MutexLocker
{
private:
    pthread_mutex_t *mutex;
public:
    MutexLocker(pthread_mutex_t *inMutex) : mutex(inMutex) {pthread_mutex_lock(mutex);}
    ~MutexLocker() {pthread_mutex_unlock(mutex);}
};

#endif //__UTILS_H__
