/*
    Copyright 2015-2016 Clément Gallet <clement.gallet@ens-lyon.org>

    This file is part of libTAS.

    libTAS is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    libTAS is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with libTAS.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "socket.h"
#include <sys/socket.h>
#include <sys/stat.h>
#include <stdlib.h>
#include "../shared/lcf.h"
#include <unistd.h>
#include "logging.h"
#include <sys/un.h>

#define SOCKET_FILENAME "/tmp/libTAS.socket"

/* Socket to communicate to the program */
static int socket_fd = 0;

bool initSocket(void)
{
    /* Check if socket file already exists. If so, it is probably because
     * the link is already done in another process of the game.
     * In this case, we just return immediately.
     */
    struct stat st;
    int result = stat(SOCKET_FILENAME, &st);
    if (result == 0)
        return false;

    /* Connect using a Unix socket */
    if (!unlink(SOCKET_FILENAME))
        debuglog(LCF_SOCKET, "Removed stall socket.");

    const struct sockaddr_un addr = { AF_UNIX, SOCKET_FILENAME };
    const int tmp_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (bind(tmp_fd, reinterpret_cast<const struct sockaddr*>(&addr), sizeof(struct sockaddr_un)))
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
    //unlink(SOCKET_FILENAME);

    return true;
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
