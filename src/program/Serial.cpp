/*
    Copyright 2015-2018 Clément Gallet <clement.gallet@ens-lyon.org>

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

#include "Serial.h"
#include <iostream>
#include <errno.h>
#include <fcntl.h>
//#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

/* Code taken from https://stackoverflow.com/a/38318768 */
int openSerial(std::string portname, int speed)
{
    int fd;

    if (portname.empty())
        return -1;

    fd = open(portname.c_str(), O_RDWR | O_NOCTTY | O_SYNC);
    if (fd < 0) {
        std::cerr << "!!! Error opening " << portname << ": " << strerror(errno) << std::endl;
        return -1;
    }

    struct termios tty;

    if (tcgetattr(fd, &tty) < 0) {
        std::cerr << "!!! Error from tcgetattr: " << strerror(errno) << std::endl;
        return -1;
    }

    cfsetospeed(&tty, (speed_t)speed);
    cfsetispeed(&tty, (speed_t)speed);

    tty.c_cflag |= (CLOCAL | CREAD);    /* ignore modem controls */
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;         /* 8-bit characters */
    tty.c_cflag &= ~PARENB;     /* no parity bit */
    tty.c_cflag &= ~CSTOPB;     /* only need 1 stop bit */
    tty.c_cflag &= ~CRTSCTS;    /* no hardware flowcontrol */

    /* setup for non-canonical mode */
    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
    tty.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
    tty.c_oflag &= ~OPOST;

    /* Set read timeout to 0.1 second */
    tty.c_cc[VMIN] = 0; // set 1 for blocking read
    tty.c_cc[VTIME] = 1;

    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        std::cerr << "!!! Error from tcsetattr: " << strerror(errno) << std::endl;
        return -1;
    }

    int wlen;

    /* Ping the device */
    char wcmd = 0xff;
    wlen = write(fd, &wcmd, 1);
    if (wlen != 1) {
        std::cerr << "!!! Could not write to device:" << strerror(errno) << std::endl;
        return -1;
    }

    unsigned char rbuf;
    int rdlen;

    rdlen = read(fd, &rbuf, 1);
    if ((rdlen != 1) || (rbuf != 0xff)) {
        std::cerr << "!!! Could not read device:" << strerror(errno) << std::endl;
        return -1;
    }

    std::cout << "--- Successfully connected to " << portname << std::endl;

    std::cout << "--- Sending reset command to device" << std::endl;

    /* Reset device */
    wcmd = 0x00;
    wlen = write(fd, &wcmd, 1);
    if (wlen != 1) {
        std::cerr << "!!! Could not write to device:" << strerror(errno) << std::endl;
        return -1;
    }

    usleep(100*1000); // 0.1 s

    std::cout << "--- Sending start command to device" << std::endl;

    /* command 1 (play), 16-bits, 1 port, 1 dataline, sync, no window 1, no window 2 */
    unsigned char start_cmd[7] = {0x01, 0x02, 0x01, 0x01, 0x00, 0x00, 0x00};
    wlen = write(fd, start_cmd, 7);
    if (wlen != 7) {
        std::cerr << "!!! Could not write to device:" << strerror(errno) << std::endl;
        return -1;
    }

    return fd;
}

enum class SerialButton {
    Right = 0x0001,
    Left = 0x0002,
    Down = 0x0004,
    Up = 0x0008,
    Start = 0x0010,
    Select = 0x0020,
    B1 = 0x0040,
    A1 = 0x0080,
    Y1 = 0x0100,
    X1 = 0x0200,
    B2 = 0x0400,
    A2 = 0x0800,
    RT = 0x1000,
    LT = 0x2000,
    Y2 = 0x4000,
    X2 = 0x8000
};

int sendInputsSerial(int fd, const AllInputs& ai, const AllInputs& prev_ai)
{
    if (fd < 0)
        return 0;

    char rbyte = 0;
    while (rbyte != 0x0f) {
        int rdlen = read(fd, &rbyte, 1);
        if (rdlen != 1) {
            std::cerr << "!!! Could not read device:" << strerror(errno) << std::endl;
            return -1;
        }

        if (rbyte == 0x0f) {
            int buttons = 0;
            for (const auto &key : ai.keyboard) {
                /* Using hardcoded values to be sure */
                switch (key) {
                    case 0x77: // w
                        buttons |= static_cast<int>(SerialButton::Up);
                        break;
                    case 0x73: // s
                        buttons |= static_cast<int>(SerialButton::Down);
                        break;
                    case 0x64: // d
                        buttons |= static_cast<int>(SerialButton::Right);
                        break;
                    case 0x61: // a
                        buttons |= static_cast<int>(SerialButton::Left);
                        break;
                    case 0xffe1: // left shift
                        buttons |= static_cast<int>(SerialButton::B1);
                        break;
                    case 0x20: // space
                        buttons |= static_cast<int>(SerialButton::A1);
                        break;
                    case 0xff0d: // return
                        buttons |= static_cast<int>(SerialButton::Start);
                        break;
                    case 0x65: // e
                        buttons |= static_cast<int>(SerialButton::RT);
                        break;
                    case 0x72: // r
                        buttons |= static_cast<int>(SerialButton::LT);
                        break;
                    default:
                        break;
                }
            }

            if (ai.pointer_mask & (1 << SingleInput::POINTER_B1)) {
                buttons |= static_cast<int>(SerialButton::X1);
            }
            if (ai.pointer_mask & (1 << SingleInput::POINTER_B3)) {
                buttons |= static_cast<int>(SerialButton::Y1);
            }

            if (ai.pointer_x > prev_ai.pointer_x) {
                buttons |= static_cast<int>(SerialButton::A2);
            }
            if (ai.pointer_x < prev_ai.pointer_x) {
                buttons |= static_cast<int>(SerialButton::Y2);
            }
            if (ai.pointer_y > prev_ai.pointer_y) {
                buttons |= static_cast<int>(SerialButton::B2);
            }
            if (ai.pointer_y < prev_ai.pointer_y) {
                buttons |= static_cast<int>(SerialButton::X2);
            }

            /* Send inputs */
            unsigned char wcmd[3] = {0x0f, static_cast<unsigned char>(buttons >> 8), static_cast<unsigned char>(buttons & 0xff)};
            std::cerr << "--- Sending input bytes " << std::hex << (unsigned int)wcmd[0] << " " << (unsigned int)wcmd[1] << " " << (unsigned int)wcmd[2] << std::endl;
            int wlen = write(fd, &wcmd, 3);
            if (wlen != 3) {
                std::cerr << "!!! Could not write inputs to device:" << strerror(errno) << std::endl;
                return -1;
            }
        }
    }
    return 0;
}

void closeSerial(int fd)
{
    if (fd < 0)
        return;
    close(fd);
}
