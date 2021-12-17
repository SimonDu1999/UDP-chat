#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>			// for strncmp()
#include <unistd.h>			// for close()
#include <pthread.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "sender.h"
#include "typer.h"
#include "list.h"
#include "shutDownManager.h"

#define DYNAMIC_LEN 200

static pthread_mutex_t* p_sendListMutex;
static pthread_cond_t s_syncOkToSendCondVar = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t s_syncOkToSendMutex = PTHREAD_MUTEX_INITIALIZER;
static List* typeList;
static char* dynamicMsg;
static pthread_t threadPID;
static int socketDescriptor;
static int myPort;
static char* chatPort;
static char* hostName;
static struct addrinfo* result=0;

void* senderThread(void* unused)
{
	// Get the remote Address from the arguments pass by thread initializer
    struct addrinfo hints;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = 0;
    if(getaddrinfo(hostName,chatPort,&hints,&result)!=0){
        printf("wrong host name and port!\n");
        return NULL;
    }
	socketDescriptor = socket(result->ai_family, result->ai_socktype,result->ai_protocol);
	while (1) {
        // wait until typer signal to send the message
        pthread_mutex_lock(&s_syncOkToSendMutex);
        {
            pthread_cond_wait(&s_syncOkToSendCondVar, &s_syncOkToSendMutex);
        }
        pthread_mutex_unlock(&s_syncOkToSendMutex);
        // get the message rid of the typeList and send it out
        pthread_mutex_lock(p_sendListMutex);
        {
            List_first(typeList);
            void* sendMsg = List_remove(typeList);
            while(sendMsg != NULL){
                // handle the error case
		        if(sendto(socketDescriptor,
			    sendMsg, strlen(sendMsg), 0,
			    result->ai_addr, result->ai_addrlen)==-1){
                    puts("Error: sending fail!\n");
                    signalToShutDown();
                    return NULL;
                }                    
                sendMsg = List_remove(typeList);
            }
        }
        pthread_mutex_unlock(p_sendListMutex);
        // end thread if satisfy the condition
        if(isOkToShutDown()){
            return NULL;
        }
        // free the dynamic message
        free(dynamicMsg);
        dynamicMsg = NULL;
    }

	return NULL;
}
// Signaler from typer to sender thread
void typer_signalSender(List* lst, pthread_mutex_t* p_listMutex, char* dmsg)
{
    // save the passing parameter
    typeList = lst;
    p_sendListMutex = p_listMutex;
    dynamicMsg = dmsg;
    // signal to the wait on the sender thread
    pthread_mutex_lock(&s_syncOkToSendMutex);
    {
        pthread_cond_signal(&s_syncOkToSendCondVar);
    }
    pthread_mutex_unlock(&s_syncOkToSendMutex);
}
// thread initializer
void Sender_init(int port1, char* port2, char* hN)
{
    myPort = port1;
    chatPort = port2;
    hostName = hN;
    pthread_create(
        &threadPID,         // PID (by pointer)
        NULL,               // Attributes
        senderThread,      // Function
        NULL);
}
// thread closer
void Sender_shutdown(void)
{
    // Cancel thread
    pthread_cancel(threadPID);

    // Waits for thread to finish
    pthread_join(threadPID , NULL);

    // free the addrinfo memory
    freeaddrinfo(result);

    // close socket
    close(socketDescriptor);
}

