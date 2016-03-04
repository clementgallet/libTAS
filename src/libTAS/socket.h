#ifndef SOCKET_H_INCL
#define SOCKET_H_INCL


#include <sys/socket.h>
#include <stdlib.h>
#include "../shared/lcf.h"
#include <unistd.h>
#include "logging.h"
#include <sys/un.h>

void initSocket(void);
void closeSocket(void);
void sendData(void* elem, size_t size);
void sendMessage(int message);
void receiveData(void* elem, size_t size);

#endif

