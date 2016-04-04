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

#ifndef LIBTAS_SOCKET_H_INCL
#define LIBTAS_SOCKET_H_INCL

#include <stddef.h>

/* Initiate a socket connection with linTAS */
bool initSocket(void);

/* Close the socket connection */
void closeSocket(void);

/* Send data over the socket. Data is stored at the beginning of
 * pointer elem, and has the specified size in bytes.
 */
void sendData(void* elem, size_t size);

/* Helper function to send a message over the socket */
void sendMessage(int message);

/* Receive data from the socket. Same arguments as sendData() */
void receiveData(void* elem, size_t size);

#endif

