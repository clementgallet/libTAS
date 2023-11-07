/*
    Copyright 2015-2023 Clément Gallet <clement.gallet@ens-lyon.org>

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

#ifdef LIBTAS_LIBRARY
#include "lcf.h"
#include "../library/logging.h"
#include "../library/GlobalState.h"
#else
#include <iostream>
#endif

#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <cstdlib>
#include <unistd.h>
#include <sys/un.h>
#include <iostream>
#include <vector>
#include <mutex>
#include <errno.h>


#define SOCKET_FILENAME "/tmp/libTAS.socket"

#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif



/* Socket to communicate between the program and the game */
static int socket_fd = 0;

static std::mutex mutex;

int removeSocket(void) {
    int ret = unlink(SOCKET_FILENAME);
    if ((ret == -1) && (errno != ENOENT))
        return errno;
    return 0;
}

bool initSocketProgram(pid_t fork_pid)
{
#ifdef __unix__
    const struct sockaddr_un addr = { AF_UNIX, SOCKET_FILENAME };
#elif defined(__APPLE__) && defined(__MACH__)
    const struct sockaddr_un addr = { sizeof(struct sockaddr_un), AF_UNIX, SOCKET_FILENAME };
#endif
    socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);

    struct timespec tim = {0, 500L*1000L*1000L};

    const int MAX_RETRIES = 10;
    int retry = 0;

    nanosleep(&tim, NULL);
    while (connect(socket_fd, reinterpret_cast<const struct sockaddr*>(&addr),
                sizeof(struct sockaddr_un))) {
        std::cout << "Attempt " << retry + 1 << ": Couldn't connect to socket." << std::endl;
        retry++;
        if (retry < MAX_RETRIES) {
            nanosleep(&tim, NULL);
        } else {
            return false;
        }
        
        /* Check if game is still running */
        int ret = waitpid(fork_pid, nullptr, WNOHANG);
        if (ret == fork_pid) {
            std::cout << "Game couldn't start properly." << std::endl;
            return false;
        }
        
        tim.tv_nsec *= 1.5;
        if (tim.tv_nsec >= 1000000000) {            
            tim.tv_sec++;
            tim.tv_nsec -= 1000000000;
        }
    }
    std::cout << "Attempt " << retry + 1 << ": Connected." << std::endl;

    return true;
}

#ifdef LIBTAS_LIBRARY

bool initSocketGame(void)
{
    libtas::GlobalNative gn;
    
    /* Check if socket file already exists. If so, it is probably because
     * the link is already done in another process of the game.
     * In this case, we just return immediately.
     */
    struct stat st;
    int result = stat(SOCKET_FILENAME, &st);
    if (result == 0)
        return false;

#ifdef __unix__
    const struct sockaddr_un addr = { AF_UNIX, SOCKET_FILENAME };
#elif defined(__APPLE__) && defined(__MACH__)
    const struct sockaddr_un addr = { sizeof(struct sockaddr_un), AF_UNIX, SOCKET_FILENAME };
#endif
    const int tmp_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (bind(tmp_fd, reinterpret_cast<const struct sockaddr*>(&addr), sizeof(struct sockaddr_un)))
    {
        libtas::debuglogstdio(LCF_SOCKET | LCF_ERROR, "Couldn't bind client socket %s", strerror(errno));
        exit(-1);
    }

    if (listen(tmp_fd, 1))
    {
        libtas::debuglogstdio(LCF_SOCKET | LCF_ERROR, "Couldn't listen on client socket %s", strerror(errno));
        exit(-1);
    }

    if ((socket_fd = accept(tmp_fd, NULL, NULL)) < 0)
    {
        libtas::debuglogstdio(LCF_SOCKET | LCF_ERROR, "Couldn't accept client connection %s", strerror(errno));
        exit(-1);
    }

    /* Don't generate SIGPIPE */
#if defined(__APPLE__) && defined(__MACH__)
    int option_value = 1;
    if (setsockopt(socket_fd, SOL_SOCKET, SO_NOSIGPIPE, &option_value, sizeof(option_value)))
    {
        libtas::debuglogstdio(LCF_SOCKET | LCF_ERROR, "Couldn't set SO_NOSIGPIPE option %s", strerror(errno));
        exit(-1);
    }
#endif
    
    close(tmp_fd);
    return true;
}

#endif

void closeSocket(void)
{
#ifdef LIBTAS_LIBRARY
    libtas::GlobalNative gn;
#endif
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
#ifdef LIBTAS_LIBRARY
    libtas::debuglogstdio(LCF_SOCKET, "Send socket data of size %u", size);
#endif

    ssize_t ret = 0;
    do {
        ret = send(socket_fd, elem, size, MSG_NOSIGNAL);
    } while ((ret == -1) && (errno == EINTR));

    if (ret == -1) {
#ifdef LIBTAS_LIBRARY
        libtas::debuglogstdio(LCF_SOCKET | LCF_ERROR, "send() returns -1 with error %s", strerror(errno));
#else
        std::cerr << "send() returns -1 with error " << strerror(errno) << std::endl;
#endif
    }
    else if (ret != static_cast<ssize_t>(size)) {
#ifdef LIBTAS_LIBRARY
        libtas::debuglogstdio(LCF_SOCKET | LCF_ERROR, "send() %u bytes instead of %u", ret, size);
#else
        std::cerr << "send() " << ret << " bytes instead of " << size << std::endl;
#endif
    }
    
    return ret;
}

int sendMessage(int message)
{
#ifdef LIBTAS_LIBRARY
    libtas::debuglogstdio(LCF_SOCKET, "Send socket message %d", message);
#endif
    return sendData(&message, sizeof(int));
}

void sendString(const std::string& str)
{
#ifdef LIBTAS_LIBRARY
    libtas::debuglogstdio(LCF_SOCKET, "Send socket string %s", str.c_str());
#endif
    unsigned int str_size = str.size();
    sendData(&str_size, sizeof(unsigned int));
    if (str_size > 0)
        sendData(str.c_str(), str_size);
}

int receiveData(void* elem, unsigned int size)
{
#ifdef LIBTAS_LIBRARY
    libtas::debuglogstdio(LCF_SOCKET, "Receive socket data of size %u", size);
#endif

    ssize_t ret = 0;
    do {
        ret = recv(socket_fd, elem, size, MSG_WAITALL);
    } while ((ret == -1) && (errno == EINTR));

    if (ret == -1) {
#ifdef LIBTAS_LIBRARY
        libtas::debuglogstdio(LCF_SOCKET | LCF_ERROR, "recv() returns -1 with error %s", strerror(errno));
#else
        std::cerr << "recv() returns -1 with error " << strerror(errno) << std::endl;
#endif
    }
    else if (ret == 0 && size > 0) { // socket has been closed
#ifdef LIBTAS_LIBRARY
        libtas::debuglogstdio(LCF_SOCKET | LCF_WARNING, "recv() returns 0 -> socket closed");
#else
        std::cerr << "recv() returns 0 -> socket closed" << std::endl;
#endif
    }
    else if (ret != static_cast<ssize_t>(size)) {
#ifdef LIBTAS_LIBRARY
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
#ifdef LIBTAS_LIBRARY
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
#ifdef LIBTAS_LIBRARY
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

    if (str_size == 0)
        return "";

    /* TODO: There might be a better way to do this...? */
    std::vector<char> buf(str_size, 0x00);
    receiveData(buf.data(), str_size);

    std::string str(buf.data(), str_size);
#ifdef LIBTAS_LIBRARY
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
#ifdef LIBTAS_LIBRARY
    libtas::debuglogstdio(LCF_SOCKET, "Receive socket C string %s", str);
#endif
}
