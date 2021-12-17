#include <stdio.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include "shutDownManager.h"

static pthread_cond_t s_syncOkToShutDownCondVar = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t s_syncOkToShutDownMutex = PTHREAD_MUTEX_INITIALIZER;
static bool shutDown = 0;

// wait until receiver or typer signal
void waitForShutDown(){
    pthread_mutex_lock(&s_syncOkToShutDownMutex);
    {
        pthread_cond_wait(&s_syncOkToShutDownCondVar, &s_syncOkToShutDownMutex);
    }
    pthread_mutex_unlock(&s_syncOkToShutDownMutex);
}

void signalToShutDown(){
    // let the shutting down become available
    shutDown = 1;
    pthread_mutex_lock(&s_syncOkToShutDownMutex);
    {
        pthread_cond_signal(&s_syncOkToShutDownCondVar);
    }
    pthread_mutex_unlock(&s_syncOkToShutDownMutex);
}
// use for terminating the sender and printer threads
bool isOkToShutDown(){
    return shutDown;
}