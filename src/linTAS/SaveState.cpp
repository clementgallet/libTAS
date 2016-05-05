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

#include "SaveState.h"
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h> // shm_open
#include <dirent.h>
#include <fcntl.h>   // open
#include <unistd.h>  // read, write, close
#include <cstdio>    // BUFSIZ

static void attachToGame(pid_t game_pid)
{
    /* Try to attach to the game process */
    if (ptrace(PTRACE_ATTACH, game_pid, nullptr, nullptr) != 0)
    {
        /* if ptrace() gives EPERM, it might be because another process is already attached */
        if (errno == EPERM)
        {
            fprintf(stderr, "Process is currently attached\n");
        }
        return;
    }

    int status = 0;
    pid_t waitret = waitpid(game_pid, &status, 0);
    if (waitret != game_pid)
    {
        fprintf(stderr, "Function waitpid failed\n");
        return;
    }
    if (!WIFSTOPPED(status))
    {
        fprintf(stderr, "Unhandled status change: %d\n", status);
        return;
    }
    if (WSTOPSIG(status) != SIGSTOP)
    {
        fprintf(stderr, "Wrong stop signal: %d\n", WSTOPSIG(status));
        return;
    }
}

static void gettids(pid_t game_pid, std::vector<pid_t> &tids)
{
    std::ostringstream oss;
    oss << "/proc/" << game_pid << "/task";

    DIR *dir = opendir(oss.str().c_str());
    if (!dir) {
        fprintf(stderr, "Could not access to TIDs\n");
        return;
    }

    struct dirent *ent;
    while ((ent = readdir(dir)) != nullptr) {
        pid_t tid;
        try {
            tid = std::stoi(std::string(ent->d_name));
        }
        catch(std::invalid_argument& e) {
            continue;
        }
        tids.push_back(tid);
    }
    closedir(dir);
}


static void detachToGame(pid_t game_pid)
{
    ptrace(PTRACE_DETACH, game_pid, nullptr, nullptr);
}

/*
 * Access and save all memory regions of the game process that are writable.
 */
void SaveState::fillSections(pid_t game_pid)
{
    sections.clear();

    /* Compose the filename for the /proc memory map, and open it. */
    std::ostringstream oss;
    oss << "/proc/" << game_pid << "/maps";
    std::ifstream mapsfile(oss.str());
    if (!mapsfile) {
        std::cerr << "Could not open " << oss.str() << std::endl;
        detachToGame(game_pid);
        return;
    }

    /* Now iterate until end-of-file. */
    intptr_t total_size = 0;

    std::string line;
    while (std::getline(mapsfile, line)) {

        std::unique_ptr<StateSection> section = std::unique_ptr<StateSection>(new StateSection);
        section->readMap(line);

        /* Filter based on permissions */

        /* We must at least be able to read the section */
        if (!section->readflag)
            continue;

        /* There is no point saving if we cannot write it later */
        if (!section->writeflag)
            continue;

        /* The other rules are:
         * - copy the first rw segment (main application global memory)
         * - copy the heap (should be very small, because heap allocations
         *     are done with our memory manager
         * - copy the stack of each thread
         */
        if ( (total_size == 0) || 
             (section->filename.find("heap") != std::string::npos) ||
             (section->filename.find("stack") != std::string::npos) ) {

            std::cerr << "Save segment, " << section->size << " bytes";
            std::cerr << " at 0x" << std::hex << section->addr << std::dec << " (";
            std::cerr << (section->readflag ?'r':'-');
            std::cerr << (section->writeflag?'w':'-');
            std::cerr << (section->execflag ?'x':'-');
            std::cerr << (section->sharedflag ?'s':'p');
            std::cerr << ") " << section->filename << std::endl;

            /* Allocate actual memory section */
            section->mem.resize(section->size);

            total_size += section->size;

            /* Insert the section into the savestate */
            sections.push_back(std::move(section));
        }
    }
}

/*
 * Access and save all memory regions of the game process that are writable.
 */
void SaveState::fillRegisters(pid_t game_pid)
{
    threads.clear();

    std::vector<pid_t> tids;
    gettids(game_pid, tids);
 
    for (auto tid: tids) {
        ThreadInfo ti;
        ti.tid = tid;
        ti.needattach = tid != game_pid;
        ti.saveRegisters();
        threads.push_back(ti);
    }
}

bool SaveState::save(pid_t game_pid)
{
    /* Attach to the game process */
    attachToGame(game_pid);

    /* Save heap memory */
    int heap_fd = shm_open("/libtas", O_RDONLY, 0666);
    int state_fd = open("savestate.bin", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    char buf[BUFSIZ];
    size_t size;

    while ((size = read(heap_fd, buf, BUFSIZ)) > 0) {
        write(state_fd, buf, size);
    }

    close(heap_fd);
    close(state_fd);

    fillRegisters(game_pid);

    fillSections(game_pid);

    for (auto& section : sections) {
        struct iovec local, remote;
        section->toIovec(local, remote);

        /* Read call */
        std::cout << "Reading section of size " << section->size << " starting " << std::hex << section->addr << std::dec << std::endl;
        ssize_t nread = process_vm_readv(game_pid, &local, 1, &remote, 1, 0);

        /* Checking for errors */
        if (nread == -1) {
            switch (errno) {
                case EINVAL:
                    std::cerr << "The amount of bytes to read is too big!" << std::endl;
                    break;
                case EFAULT:
                    std::cerr << "Bad address space of the game process or own process!" << std::endl;
                    break;
                case ENOMEM:
                    std::cerr << "Could not allocate memory for internal copies of the iovec structures." << std::endl;
                    break;
                case EPERM:
                    std::cerr << "Do not have permission to read the game process memory." << std::endl;
                    break;
                case ESRCH:
                    std::cerr << "The game PID does not exist." << std::endl;
                    break;
            }
        }

        if (nread != (ssize_t)section->size) {
            std::cerr << "Not all memory was read! Only " << nread << std::endl;
            detachToGame(game_pid);
            return false;
        }
    }
    detachToGame(game_pid);
    return true;
}

bool SaveState::load(pid_t game_pid)
{
    /* Attach to the game process */
    /* 
     * Actually, we don't need this, just the signal to freeze the game, I guess.
     * TODO: leaving it for now.
     */
    attachToGame(game_pid);

    int heap_fd = shm_open("/libtas", O_WRONLY, 0666);
    int state_fd = open("savestate.bin", O_RDONLY, 0644);
    char buf[BUFSIZ];
    size_t size;

    while ((size = read(state_fd, buf, BUFSIZ)) > 0) {
        write(heap_fd, buf, size);
    }

    close(heap_fd);
    close(state_fd);

    for (auto& ti : threads) {
        ti.loadRegisters();
    }

    for (auto& section : sections) {
        struct iovec local, remote;
        section->toIovec(local, remote);

        /* Read call */
        std::cout << "Writing section of size " << section->size << std::endl;
        ssize_t nwrite = process_vm_writev(game_pid, &local, 1, &remote, 1, 0);

        /* Checking for errors */
        if (nwrite == -1) {
            switch (errno) {
                case EINVAL:
                    std::cerr << "The amount of bytes to write is too big!" << std::endl;
                    break;
                case EFAULT:
                    std::cerr << "Bad address space of the game process or own process!" << std::endl;
                    break;
                case ENOMEM:
                    std::cerr << "Could not allocate memory for internal copies of the iovec structures." << std::endl;
                    break;
                case EPERM:
                    std::cerr << "Do not have permission to write the game process memory." << std::endl;
                    break;
                case ESRCH:
                    std::cerr << "The game PID does not exist." << std::endl;
                    break;
            }
        }

        if (nwrite != (ssize_t)section->size) {
            std::cerr << "Not all memory was written! Only " << nwrite << std::endl;
            detachToGame(game_pid);
            return false;
        }
    }
    detachToGame(game_pid);
    return true;
}

