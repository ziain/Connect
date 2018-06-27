#ifndef LWTHREAD_H
#define LWTHREAD_H

#include <pthread.h>

class KThread
{
public:
    KThread();
    ~KThread();

    void Lock();
    void Unlock();
    void Wait(int timeout = 0);
    void Wake(bool boardcast);

private:
    pthread_mutex_t m_mutex;
    pthread_cond_t m_cond;
    pthread_attr_t m_attr;
};

#endif // LWTHREAD_H
