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

#include "URandom.h"

#include "FileHandleList.h"
#include "../logging.h"
#include <fcntl.h>
#include <unistd.h> // getpid()

namespace libtas {

static int readfd = -1;
static int writefd = -1;
static char* datestr = nullptr;
static FILE* stream = nullptr;

static void urandom_handler(int signum)
{
    /* TODO: write a simple PRNG which is seeded by shared_config.initial_time_sec
     * instead of outputting it directly as a string.
     */
    debuglogstdio(LCF_FILEIO | LCF_RANDOM, "Filling urandom fd");
    if (!datestr) {
        time_t tsec = static_cast<time_t>(shared_config.initial_time_sec);
        datestr = asctime(gmtime(&tsec));
    }

    /* Fill the pipe with data from initial time */
    int err = write(writefd, datestr, strlen(datestr));
    while (err != -1) {
        err = write(writefd, datestr, strlen(datestr));
    }
}

int urandom_create_fd()
{
    debuglogstdio(LCF_FILEIO | LCF_RANDOM, "Open /dev/urandom");

    if (readfd == -1) {
        std::pair<int, int> fds = FileHandleList::createPipe(O_NONBLOCK);
        readfd = fds.first;
        writefd = fds.second;

        /* Set the pipe size to some small value, because we won't need much.
         * It should be at least 2*page_size, because on Linux a pipe is
         * considered writeable if at least page_size can be written.
         */
        MYASSERT(fcntl(writefd, F_SETPIPE_SZ, 2*4096) != -1);

        /* Fill the pipe */
        urandom_handler(0);

        GlobalNative gn;

        /* Add async signal for when the pipe is writeable */
        MYASSERT(fcntl(writefd, F_SETOWN, getpid()) != -1);
        MYASSERT(fcntl(writefd, F_SETSIG, 0) != -1);
        MYASSERT(fcntl(writefd, F_SETFL, O_ASYNC | O_NONBLOCK) != -1);

        /* Unblock SIGIO signal */
        sigset_t mask;
        sigemptyset(&mask);
        sigaddset(&mask, SIGIO);
        MYASSERT(pthread_sigmask(SIG_UNBLOCK, &mask, nullptr) == 0);

        /* Add signal handler for SIGIO signal */
        struct sigaction sigio;
        sigfillset(&sigio.sa_mask);
        sigio.sa_handler = urandom_handler;
        MYASSERT(sigaction(SIGIO, &sigio, nullptr) == 0)
    }

    debuglog(LCF_FILEIO | LCF_RANDOM, "Return fd ", readfd);
    return readfd;
}

int urandom_get_fd() {
    return readfd;
}

FILE* urandom_create_file() {
    if (!stream) {
        int readfd = urandom_create_fd();
        stream = fdopen(readfd, "r");
        setvbuf(stream, nullptr, _IONBF, 0);
    }
    return stream;
}

FILE* urandom_get_file() {
    return stream;
}

}
