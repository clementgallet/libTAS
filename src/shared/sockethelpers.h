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

#ifndef LIBTAS_SOCKETHELPERS_H_INCL
#define LIBTAS_SOCKETHELPERS_H_INCL

#include <cstddef>
#include <string>

/* Remove the socker file */
void removeSocket();

/* Initiate a socket connection with libTAS */
bool initSocketProgram(void);

/* Initiate a socket connection with linTAS */
bool initSocketGame(void);

/* Close the socket connection */
void closeSocket(void);

/* Send data over the socket. Data is stored at the beginning of
 * pointer elem, and has the specified size in bytes.
 */
void sendData(const void* elem, size_t size);

/* Send a string object through the socket. It first sends the string lenght,
 * followed by the char array.
 */
void sendString(const std::string& str);

/* Helper function to send a message over the socket */
void sendMessage(int message);

/* Receive data from the socket. Same arguments as sendData() */
int receiveData(void* elem, size_t size);

/* Receive a message */
int receiveMessage();

/* Receive a string object from the socket. */
std::string receiveString();

/* Receive a char array from the socket. */
void receiveCString(char* str);


#endif
