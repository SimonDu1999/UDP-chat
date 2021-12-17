#ifndef _PRINTER_H_
#define _PRINTER_H_

#include <pthread.h>
#include "list.h"


// void Printer_init(pthread_mutex_t *psyncOkToPrintMutex, pthread_cond_t *psyncOkToPrintCondVar);
void Printer_init();
void receiver_signalPrinter(List* lst, pthread_mutex_t* p_listMutex, char* dmsg);
void Printer_shutdown();


#endif
