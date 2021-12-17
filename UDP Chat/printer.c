#include "printer.h"
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "list.h"
#include "shutDownManager.h"

static pthread_t threadPrinter;
static List* printList;
char* freeThisMsg;
static pthread_cond_t s_syncOkToPrintCondVar = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t s_syncOkToPrintMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t* p_printListMutex;

//static char* MESSAGE;
static char* msg;


void* printerThread(void* unused) 
{
    while(1){
        // Wait until receiver to signalled
        pthread_mutex_lock(&s_syncOkToPrintMutex);
        {
            pthread_cond_wait(&s_syncOkToPrintCondVar, &s_syncOkToPrintMutex);
        }
        pthread_mutex_unlock(&s_syncOkToPrintMutex);
        // remove the message from the list and print it out
        pthread_mutex_lock(p_printListMutex);
        {
            List_first(printList);
            msg = List_remove(printList); 
            char enter = '\n';
            while(msg != NULL){
                // hand the error case of writing and write each line
                if(write(0,msg,strlen(msg))==-1){
                    puts("Error: wring fail!\n");
                    signalToShutDown();
                    return NULL;
                }
                write(0,&enter,1);
                msg = List_remove(printList);
            }
            fflush(stdout);
        }
        pthread_mutex_unlock(p_printListMutex);
        // terminate case:
        if(isOkToShutDown()){
            return NULL;
        }
        // free the allocated memory from receiver
        free(freeThisMsg);
        freeThisMsg = NULL;
    }
return NULL;
}

// signer form receiver to printer
void receiver_signalPrinter(List* lst, pthread_mutex_t* p_listMutex, char* dmsg)
{
    //save the passing arguments
    printList = lst;
    p_printListMutex = p_listMutex;
    freeThisMsg = dmsg;
    // signal to the wait on the printer thread
    pthread_mutex_lock(&s_syncOkToPrintMutex);
    {
        pthread_cond_signal(&s_syncOkToPrintCondVar);
    }
    pthread_mutex_unlock(&s_syncOkToPrintMutex);
}

// thread initializer
void  Printer_init()
{
    pthread_create(&threadPrinter, NULL, printerThread, NULL );
}

// thread closer
void Printer_shutdown()
{
    pthread_cancel(threadPrinter);
    pthread_join(threadPrinter, NULL);
}
