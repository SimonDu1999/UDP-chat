#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>			// for strncmp()
#include <unistd.h>			// for close()
#include <pthread.h>
#include <signal.h>
#include "receiver.h"
#include "printer.h"
#include "list.h"
#include "shutDownManager.h"

#define DYNAMIC_LEN 200
#define MSG_MAX_LEN 1024

static int myPort;
static pthread_mutex_t listMutex = PTHREAD_MUTEX_INITIALIZER;
static List* receiveList;
static char* dynamicMsg;
static pthread_t threadReceiver;
static int socketDescriptor;
static int myPort;

static void free_somthing(void* pItem){
    pItem = NULL;
    return;
}

void* receiveThread(void* unused)
{
	// Address set up
	struct sockaddr_in sin;
	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;                   
	sin.sin_addr.s_addr = htonl(INADDR_ANY);    
	sin.sin_port = htons(myPort);                 
	// Create the socket for UDP
	socketDescriptor = socket(PF_INET, SOCK_DGRAM, 0);

	// Bind the socket to the port (PORT) that we specify
	bind (socketDescriptor, (struct sockaddr*) &sin, sizeof(sin));
	
	while (1) {
        // malloc a dynamic message for receiving
        dynamicMsg = malloc(DYNAMIC_LEN);
		struct sockaddr_in sinRemote;
		unsigned int sin_len = sizeof(sinRemote);
        //handle the error case from receiving
		if(recvfrom(socketDescriptor,
			dynamicMsg, DYNAMIC_LEN, 0,
			(struct sockaddr *) &sinRemote, &sin_len)==-1){
                puts("Error: receiving fail!\n");
                signalToShutDown();
                return NULL;
            }
        // append the list with the receiving message(different list from sender/typer)
        pthread_mutex_lock(&listMutex);
        {
            List_append(receiveList,dynamicMsg);
        }
        pthread_mutex_unlock(&listMutex);
        // signal the printer to print the list
        receiver_signalPrinter(receiveList, &listMutex, dynamicMsg);
        // thread terminate if receive '!' and signal to the wait on the main thread
        if(dynamicMsg[0] == '!' && dynamicMsg[1] == '\0'){
            signalToShutDown();
            return NULL;
        }
    }
    // NOTE NEVER EXECUTES BECEAUSE THREAD IS CANCELLED
	return NULL;
}

// thread initializer
void Receiver_init(int port)
{
    receiveList = List_create();
    myPort = port;
    pthread_create(
        &threadReceiver,         // PID (by pointer)
        NULL,               // Attributes
        receiveThread,      // Function
        NULL);
}

// thread closer
void Receiver_shutdown(void)
{
    // Cancel thread
    pthread_cancel(threadReceiver);

    // Waits for thread to finish
    pthread_join(threadReceiver , NULL);

    //  Cleanup memory
    pthread_mutex_lock(&listMutex);
    {
       List_free(receiveList,free_somthing);
    }
    pthread_mutex_unlock(&listMutex);
    if(dynamicMsg){
        free(dynamicMsg);
        dynamicMsg = NULL;
    }    

    // close socket
    close(socketDescriptor);
}


