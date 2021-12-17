#include "printer.h"
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "list.h"
#include "sender.h"
#include "string.h"
#include "shutDownManager.h"

#define DYNAMIC_LEN 200

static pthread_t threadTyper;
static List* sendList;
static char* dynamicMsg;
static pthread_mutex_t listMutex = PTHREAD_MUTEX_INITIALIZER;
static void free_somthing(void* pItem){
    pItem = NULL;
    return;
}

void* typerThread(void* unused) 
{
    while(1){
        // dynamic allocate message to type in
        dynamicMsg = malloc(DYNAMIC_LEN); 
        // get the input and handle the error case
        if(gets(dynamicMsg)==NULL){
            signalToShutDown();
            puts("Error: wrong input!\n");
            return NULL;
        }
        // append the list with the input(different list from receiver/typer)
        pthread_mutex_lock(&listMutex);
        {
            if(List_append(sendList,dynamicMsg) == -1){
                signalToShutDown();
                puts("Error: No more List to append!\n");
                return NULL;
            }
        }
        pthread_mutex_unlock(&listMutex);
        // signal the sender to send the message in the list
        typer_signalSender(sendList, &listMutex, dynamicMsg);
        // thread terminates if type '!',
        // and signal to the shutdown manager to continue the main thread
        if(dynamicMsg[0]=='!' && dynamicMsg[1]=='\0'){
            signalToShutDown();
            return NULL;
        }     
    }
    return NULL;
}

// typer thread initializer
void Typer_init()
{
    sendList = List_create();
    pthread_create(&threadTyper, NULL, typerThread, NULL );
}

// typer thread closer
void Typer_shutdown(void)
{   
    // Cancel thread
    pthread_cancel(threadTyper);
    // Waits for thread to finish
    pthread_join(threadTyper , NULL);

    //  Cleanup memory

    List_free(sendList,free_somthing);

    pthread_mutex_unlock(&listMutex);
    if(dynamicMsg){
        free(dynamicMsg);
        dynamicMsg = NULL;
    }    
}