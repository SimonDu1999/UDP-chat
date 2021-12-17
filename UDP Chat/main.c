#include <stdio.h>
#include <stdlib.h>
#include "receiver.h"
#include "printer.h"
#include "sender.h"
#include "typer.h"
#include "shutDownManager.h"

int main(int argc, char** args)
{
    int myPort = atoi(args[1]);
    printf("Start chatting!!\n");

    // Startup my modules
    Printer_init();
    Receiver_init(myPort);
    Typer_init();
    Sender_init(myPort,args[3],args[2]);
    // Wait for shutting down
    waitForShutDown();
    // Shutdown my modules
    Printer_shutdown();
    Receiver_shutdown();
    Typer_shutdown();
    Sender_shutdown();
    return 0;
}