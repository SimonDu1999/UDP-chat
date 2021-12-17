#ifndef _SHUTDOWNMANAGER_H_
#define _SHUTDOWNMANAGER_H_

void waitForShutDown();

void signalToShutDown();

bool isOkToShutDown();
#endif