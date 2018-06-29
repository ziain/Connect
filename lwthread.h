#ifndef LWTHREAD_H
#define LWTHREAD_H

#include <pthread.h>


#define THREAD_STOP    0x00
#define THREAD_RUNNING 0x01
#define THREAD_PAUSE   0x02

class KThread
{
public:
    KThread();
    ~KThread();

    void Lock();
    void Unlock();
    void Wait(int timeout = 0);
    void Wake(bool boardcast);

public:
    pthread_t m_thread_id;
    char m_thread_status;

private:
    pthread_mutex_t m_mutex;
    pthread_cond_t m_cond;
    pthread_attr_t m_attr;

};

#endif // LWTHREAD_H
