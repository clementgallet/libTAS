/*
    Copyright 2015-2020 Clément Gallet <clement.gallet@ens-lyon.org>

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

#include "sockethelpers.h"
#include <sys/socket.h>
#include <cstdlib>
#include <unistd.h>
#include <sys/un.h>
#include <iostream>
#include <vector>
#include <mutex>
#include <errno.h>
#include <fcntl.h>

#ifdef SOCKET_LOG
#include "lcf.h"
#include "../library/logging.h"
#else
#include <iostream>
#endif

#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif



/* Socket to communicate between the program and the game */
static int socket_fd = 0;

static std::mutex mutex;

void setSocket(int sock) {
    socket_fd = sock;
}

void initSocketGame(void)
{
    socket_fd = std::stoi(getenv("LIBTAS_SOCKET_FD"));

    if (fcntl(socket_fd, F_GETFD) == -1) {
        if (errno == EBADF) {
            perror("LIBTAS_SOCKET_FD has been closed");
        } else {
            perror("Unable to use game socket");
        }
        exit(-1);
    }

    /* Don't generate SIGPIPE */
#if defined(__APPLE__) && defined(__MACH__)
    int option_value = 1;
    if (setsockopt(socket_fd, SOL_SOCKET, SO_NOSIGPIPE, &option_value, sizeof(option_value)))
    {
        std::cerr << "Couldn't set SO_NOSIGPIPE option." << std::endl;
        exit(-1);
    }
#endif
}

void closeSocket(void)
{
    close(socket_fd);
}

void lockSocket(void)
{
    mutex.lock();
}

void unlockSocket(void)
{
    mutex.unlock();
}

int sendData(const void* elem, unsigned int size)
{
#ifdef SOCKET_LOG
    libtas::debuglogstdio(LCF_SOCKET, "Send socket data of size %u", size);
#endif

    ssize_t ret = 0;
    do {
        ret = send(socket_fd, elem, size, MSG_NOSIGNAL);
    } while ((ret == -1) && (errno == EINTR));

    if (ret == -1) {
#ifdef SOCKET_LOG
        libtas::debuglogstdio(LCF_SOCKET | LCF_ERROR, "send() returns -1 with error %s", strerror(errno));
#else
        std::cerr << "send() returns -1 with error " << strerror(errno) << std::endl;
#endif
    }
    else if (ret != static_cast<ssize_t>(size)) {
#ifdef SOCKET_LOG
        libtas::debuglogstdio(LCF_SOCKET | LCF_ERROR, "send() %u bytes instead of %u", ret, size);
#else
        std::cerr << "send() " << ret << " bytes instead of " << size << std::endl;
#endif
    }
    
    return ret;
}

int sendMessage(int message)
{
#ifdef SOCKET_LOG
    libtas::debuglogstdio(LCF_SOCKET, "Send socket message %d", message);
#endif
    return sendData(&message, sizeof(int));
}

void sendString(const std::string& str)
{
#ifdef SOCKET_LOG
    libtas::debuglogstdio(LCF_SOCKET, "Send socket string %s", str.c_str());
#endif
    unsigned int str_size = str.size();
    sendData(&str_size, sizeof(unsigned int));
    sendData(str.c_str(), str_size);
}

int receiveData(void* elem, unsigned int size)
{
#ifdef SOCKET_LOG
    libtas::debuglogstdio(LCF_SOCKET, "Receive socket data of size %u", size);
#endif

    ssize_t ret = 0;
    do {
        ret = recv(socket_fd, elem, size, MSG_WAITALL);
    } while ((ret == -1) && (errno == EINTR));

    if (ret == -1) {
#ifdef SOCKET_LOG
        libtas::debuglogstdio(LCF_SOCKET | LCF_ERROR, "recv() returns -1 with error %s", strerror(errno));
#else
        std::cerr << "recv() returns -1 with error " << strerror(errno) << std::endl;
#endif
    }
    else if (ret == 0) { // socket has been closed
#ifdef SOCKET_LOG
        libtas::debuglogstdio(LCF_SOCKET | LCF_WARNING, "recv() returns 0 -> socket closed");
#else
        std::cerr << "recv() returns 0 -> socket closed" << std::endl;
#endif
    }
    else if (ret != static_cast<ssize_t>(size)) {
#ifdef SOCKET_LOG
        libtas::debuglogstdio(LCF_SOCKET | LCF_ERROR, "recv() %u bytes instead of %u", ret, size);
#else
        std::cerr << "recv() " << ret << " bytes instead of " << size << std::endl;
#endif
    }
    return ret;
}

int receiveMessage()
{
    int msg;
    int ret = receiveData(&msg, sizeof(int));
#ifdef SOCKET_LOG
    libtas::debuglogstdio(LCF_SOCKET, "Receive socket message %d", msg);
#endif
    /* Handle special case for closed socket */
    if (ret == 0)
        return -2;
        
    if (ret < 0)
        return ret;
    return msg;
}

int receiveMessageNonBlocking()
{
    int msg;
    int ret = recv(socket_fd, &msg, sizeof(int), MSG_WAITALL | MSG_DONTWAIT);
    if (ret < 0)
        return ret;
#ifdef SOCKET_LOG
    libtas::debuglogstdio(LCF_SOCKET, "Receive non-blocking socket message %d", msg);
#endif

    /* Handle special case for closed socket */
    if (ret == 0)
        return -2;

    return msg;
}

std::string receiveString()
{
    unsigned int str_size;
    receiveData(&str_size, sizeof(unsigned int));

    /* TODO: There might be a better way to do this...? */
    std::vector<char> buf(str_size, 0x00);
    receiveData(buf.data(), str_size);

    std::string str(buf.data(), str_size);
#ifdef SOCKET_LOG
    libtas::debuglogstdio(LCF_SOCKET, "Receive socket string %s", str.c_str());
#endif
    return str;
}

void receiveCString(char* str)
{
    unsigned int str_size;
    receiveData(&str_size, sizeof(unsigned int));
    receiveData(str, str_size);
    str[str_size] = '\0';
#ifdef SOCKET_LOG
    libtas::debuglogstdio(LCF_SOCKET, "Receive socket C string %s", str);
#endif
}
