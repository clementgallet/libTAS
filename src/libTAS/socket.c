#include "socket.h"

#define SOCKET_FILENAME "/tmp/libTAS.socket"

/* Socket to communicate to the program */
static int socket_fd = 0;

void initSocket(void)
{
    /* Connect using a Unix socket */

    if (!unlink(SOCKET_FILENAME))
        debuglog(LCF_SOCKET, "Removed stall socket.");

    const struct sockaddr_un addr = { AF_UNIX, SOCKET_FILENAME };
    const int tmp_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (bind(tmp_fd, (const struct sockaddr*)&addr, sizeof(struct sockaddr_un)))
    {
        debuglog(LCF_ERROR | LCF_SOCKET, "Couldn't bind client socket.");
        exit(-1);
    }

    if (listen(tmp_fd, 1))
    {
        debuglog(LCF_ERROR | LCF_SOCKET, "Couldn't listen on client socket.");
        exit(1);
    }

    debuglog(LCF_SOCKET, "Loading complete, awaiting client connection...");

    if ((socket_fd = accept(tmp_fd, NULL, NULL)) < 0)
    {
        debuglog(LCF_ERROR | LCF_SOCKET, "Couldn't accept client connection.");
        exit(1);
    }

    debuglog(LCF_SOCKET, "Client connected.");

    close(tmp_fd);
    unlink(SOCKET_FILENAME);

}

void closeSocket(void)
{
    close(socket_fd);

}

void sendData(void* elem, size_t size)
{
    send(socket_fd, elem, size, 0);
}

void sendMessage(int message)
{
    sendData(&message, sizeof(int));
}

void receiveData(void* elem, size_t size)
{
    recv(socket_fd, elem, size, 0);
}

