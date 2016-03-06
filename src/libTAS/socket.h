#ifndef SOCKET_H_INCL
#define SOCKET_H_INCL

#include <stddef.h>

void initSocket(void);
void closeSocket(void);
void sendData(void* elem, size_t size);
void sendMessage(int message);
void receiveData(void* elem, size_t size);

#endif

