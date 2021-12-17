#ifndef _SENDER_H_
#define _SENDER_H_

#include <pthread.h>
#include "list.h"


// void Printer_init(pthread_mutex_t *psyncOkToPrintMutex, pthread_cond_t *psyncOkToPrintCondVar);
void Sender_init(int port1, char* port2, char* hN);
void typer_signalSender(List* lst, pthread_mutex_t* p_listMutex, char* dmsg);
void Sender_shutdown();


#endif