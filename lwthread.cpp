#include "lwthread.h"

KThread::KThread()
{
    pthread_mutex_init(&m_mutex,NULL);
    pthread_cond_init(&m_cond,NULL);
    pthread_attr_init(&m_attr);

    m_thread_status = THREAD_STOP;
}

KThread::~KThread()
{
    pthread_attr_destroy(&m_attr);
    pthread_cond_destroy(&m_cond);
    pthread_mutex_destroy(&m_mutex);
}

void KThread::Lock()
{
    pthread_mutex_lock(&m_mutex);
}

void KThread::Unlock()
{
    pthread_mutex_unlock(&m_mutex);
}

void KThread::Wait(int timeout = 0)
{
    if(timeout)
    {
        timespec time;
        time.tv_sec = timeout / 1000;
        time.tv_nsec = (timeout - time.tv_sec * 1000)*1000;
        pthread_cond_timedwait(&m_cond,&m_mutex,&time);
    }
    pthread_cond_wait(&m_cond,&m_mutex);

}

void KThread::Wake(bool boardcast)
{
    if(boardcast)
        pthread_cond_broadcast(&m_cond);
    pthread_cond_signal(&m_cond);
}


