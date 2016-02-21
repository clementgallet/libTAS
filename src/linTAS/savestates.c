#include "savestates.h"

void attachToGame(pid_t game_pid)
{
    /* Try to attach to the game process */
    if (ptrace(PTRACE_ATTACH, game_pid, NULL, NULL) != 0)
    {
        int errattch = errno;
        /* if ptrace() gives EPERM, it might be because another process is already attached */
        if (errattch == EPERM)
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

void detachToGame(pid_t game_pid)
{
    ptrace(PTRACE_DETACH, game_pid, NULL, NULL);
}


/* 
 * Parse a single line from the /proc/pid/maps file.
 * Code originally taken from GDB
 */
int 
read_mapping (FILE *mapfile, 
          unsigned long long int *addr, 
          unsigned long long int *endaddr, 
          char *permissions, 
          unsigned long long int *offset, 
          char *device, 
          unsigned long long int *inode, 
          char *filename)
{
    /* 
     * fscanf is not working properly when dealing with 64 bit hex numbers
     * So I'm using strtoull instead.
     */

    char* linestring = malloc(sizeof(char) * 1024);
    if (linestring == NULL){
        fprintf(stderr, "malloc failed\n");
        return 0;
    }
    char* initls = linestring;
        
    char* ret = fgets(linestring, 1024, mapfile);

    if (ret == NULL) {
        free(initls);
        return 0;
    }

    *addr = strtoull(linestring, &linestring, 16);
    if (addr == 0) { // Adress 0 is never used
        free(initls);
        return 0;
    }

    linestring++; // Skip the '-'

    *endaddr = strtoull(linestring, &linestring, 16);

    sscanf (linestring," %s", permissions);
    linestring += 6; // Skip the permissions

    *offset = strtoull(linestring, &linestring, 16);

    sscanf (linestring," %s", device);
    linestring += 6; // Skip the device

    *inode = strtoull(linestring, &linestring, 16);

    if (*inode != 0)
    {
        sscanf (linestring, "%s\n", filename);
    }
    else
    {
        filename[0] = '\0';   /* no filename */
        sscanf (linestring, "\n");
    }

    free(initls);
    return 1;
}

/*
 * Access and save all memory regions of the game process that are writable.
 * Code originally taken from GDB
 */

void saveState(pid_t game_pid, struct State* state)
{
    char mapsfilename[2048];
    FILE *mapsfile;
    unsigned long long int addr, endaddr, size, offset, inode;
    char permissions[8], device[8], filename[2048];
    int readflag, writeflag, execflag;
    int haserror = 0;

    /* Attach to the game process */
    /* 
     * Actually, we don't need this, just the signal to freeze the game, I guess.
     * TODO: leaving it for now.
     */
    attachToGame(game_pid);

    /* Compose the filename for the /proc memory map, and open it. */
    sprintf (mapsfilename, "/proc/%d/maps", game_pid);
    if ((mapsfile = fopen (mapsfilename, "r")) == NULL) {
        fprintf(stderr, "Could not open %s\n", mapsfilename);
        detachToGame(game_pid);
        return;
    }

    /* Count how many lines in maps file to allocate the state */
    int n_sections = 0;
    for (int c = fgetc(mapsfile); c != EOF; c = fgetc(mapsfile)) {
        if (c == '\n') n_sections++;
    }
    rewind(mapsfile);

    /* Allocate the state */
    state->sections = malloc(n_sections * sizeof(struct StateSection));

    /* Allocate the structures to prepare the memory read */
    struct iovec* local = malloc(n_sections * sizeof(struct iovec));
    struct iovec* remote = malloc(n_sections * sizeof(struct iovec));

    /* Now iterate until end-of-file. */
    int section_i = 0;
    unsigned long long int total_size = 0;

    while (read_mapping (mapsfile, &addr, &endaddr, &permissions[0], 
                &offset, &device[0], &inode, &filename[0]))
    {
        /* Get the segment's permissions.  */
        readflag  = (strchr (permissions, 'r') != 0);
        writeflag = (strchr (permissions, 'w') != 0);
        execflag  = (strchr (permissions, 'x') != 0);

        /*
        fprintf(stderr, 
                "Save segment, %lld bytes at 0x%llx (%c%c%c)", 
                size, addr, 
                readflag  ? 'r' : '-', 
                writeflag ? 'w' : '-',
                execflag  ? 'x' : '-');
        if (filename[0])
            fprintf(stderr, " for %s", filename);
        fprintf(stderr, "\n");
        */

        /* Filter based on permissions */

        /* We must at least be able to read the section */
        if (!readflag)
            continue;

        /* There is no point saving if we cannot write it later */
        if (!writeflag)
            continue;

        size = endaddr - addr;

        /* Fill the information on the section */
        state->sections[section_i].addr = addr;
        state->sections[section_i].endaddr = endaddr;
        state->sections[section_i].readflag = readflag;
        state->sections[section_i].writeflag = writeflag;
        state->sections[section_i].execflag = execflag;
        state->sections[section_i].offset = offset;
        strncpy(state->sections[section_i].device, device, 8);
        state->sections[section_i].inode = inode;
        size_t filename_size = strnlen(filename, 2048);
        state->sections[section_i].filename = malloc((filename_size + 1) * sizeof(char));
        if (state->sections[section_i].filename == NULL) {
            fprintf(stderr, "Cound not alloc memory for filename\n");
            haserror = 1;
            break;
        }
        memcpy(state->sections[section_i].filename, filename, filename_size);
        state->sections[section_i].filename[filename_size] = '\0';

        /* Allocate actual memory section */
        state->sections[section_i].mem = malloc(size * sizeof(char));
        if (state->sections[section_i].mem == NULL) {
            fprintf(stderr, "Cound not alloc memory of size %lld for mem\n", size);
            haserror = 1;
            break;
        }

        /* Prepare the structures for the read call */
        local[section_i].iov_base = state->sections[section_i].mem;
        local[section_i].iov_len = size;
        remote[section_i].iov_base = (void *) addr;
        remote[section_i].iov_len = size;

        total_size += size;

        section_i++;
    }

    /* 
     * TODO: deallocating each resource for each error does not seem optimal
     * How to do it better?
     */
    if (haserror) {
        free(local);
        free(remote);
        fclose(mapsfile);
        detachToGame(game_pid);
        return;
    }

    /* Now, making all the reads in one call */
    fprintf(stderr, "Saving the actual memory, %lld bytes\n", total_size);
    ssize_t nread = process_vm_readv(game_pid, local, section_i, remote, section_i, 0);

    /* Checking for errors */
    if (nread != (ssize_t)total_size) {
        fprintf(stderr, "Not all memory was read!\n");
        free(local);
        free(remote);
        fclose(mapsfile);
        detachToGame(game_pid);
        return;
    }

    if (nread == -1) {
        switch (errno) {
            case EINVAL:
                fprintf(stderr, "The amount of bytes read is too big!\n");
                break;
            case EFAULT:
                fprintf(stderr, "Bad address space of the game process or own process!\n");
                break;
            case ENOMEM:
                fprintf(stderr, "Could not allocate memory for internal copies of the iovec structures.\n");
                break;
            case EPERM:
                fprintf(stderr, "Do not have permission to read the game process memory.\n");
                break;
            case ESRCH:
                fprintf(stderr, "The game PID does not exist.\n");
                break;
        }
        free(local);
        free(remote);
        fclose(mapsfile);
        detachToGame(game_pid);
        return;
    }

    /*
     * We probably have fewer sections than inited, because of non-writable sections
     * So we truncate the sections array.
     * We only realloc if the destination size is not 0, otherwise the behavior of realloc 
     * depends on the implementation
     */
    if (section_i == 0) {
        fprintf(stderr, "After filtering, no section are saved!\n");
        free(state->sections);
        free(local);
        free(remote);
        fclose(mapsfile);
        detachToGame(game_pid);
        return;
    }

    struct StateSection* sections_realloc = realloc(state->sections, section_i * sizeof(struct StateSection));
    if (sections_realloc == NULL) {
        fprintf(stderr, "Realloc failed\n");
        free(state->sections);
        free(local);
        free(remote);
        fclose(mapsfile);
        detachToGame(game_pid);
        return;
    }
    else {
        state->sections = sections_realloc;
        state->n_sections = section_i;
    }

    fprintf(stderr, "Wow, we actually did not raise any error. Saved %lld bytes\n", total_size);
    state->total_size = total_size;

    free(local);
    free(remote);
    fclose(mapsfile);

    /* Detach from the game process */
    detachToGame(game_pid);
}

void loadState(pid_t game_pid, struct State* state)
{
    /* Some duplicate code of saveState, factorize it? */

    char mapsfilename[2048];
    FILE *mapsfile;
    unsigned long long int addr, endaddr, size, offset, inode;
    char permissions[8], device[8], filename[2048];
    int readflag, writeflag, execflag;

    /* Attach to the game process */
    attachToGame(game_pid);

    /* Compose the filename for the /proc memory map, and open it. */
    sprintf (mapsfilename, "/proc/%d/maps", game_pid);
    if ((mapsfile = fopen (mapsfilename, "r")) == NULL) {
        fprintf(stderr, "Could not open %s\n", mapsfilename);
        detachToGame(game_pid);
        return;
    }

    /* Count how many lines in maps file to allocate the state */
    int n_sections = 0;
    for (int c = fgetc(mapsfile); c != EOF; c = fgetc(mapsfile)) {
        if (c == '\n') n_sections++;
    }
    rewind(mapsfile);

    /* 
     * Allocate the structures to prepare the memory read
     * As opposed to memory read, we are doing writes one at a time,
     * so that we can give more information on the first write error
     */
    struct iovec* local = malloc(sizeof(struct iovec));
    struct iovec* remote = malloc(sizeof(struct iovec));

    /* Now iterate until end-of-file. */
    unsigned long long int total_size_loaded = 0;
    int section_i = 0;

    while (read_mapping (mapsfile, &addr, &endaddr, &permissions[0], 
                &offset, &device[0], &inode, &filename[0]))
    {
        /* Get the segment's permissions.  */
        readflag  = (strchr (permissions, 'r') != 0);
        writeflag = (strchr (permissions, 'w') != 0);
        execflag  = (strchr (permissions, 'x') != 0);

        /*
           fprintf(stderr, 
           "Save segment, %lld bytes at 0x%llx (%c%c%c)", 
           size, addr, 
           readflag  ? 'r' : '-', 
           writeflag ? 'w' : '-',
           execflag  ? 'x' : '-');
           if (filename[0])
           fprintf(stderr, " for %s", filename);
           fprintf(stderr, "\n");
           */

        /* If we cannot write to the section, skip it */
        if (!writeflag)
            continue;

        /* Find a match section in the savestate */
        int si;
        for (si = 0; si<state->n_sections; si++) {
            if (state->sections[si].addr == addr)
                break;
        }
        if (si == state->n_sections) {
            fprintf(stderr, "Did not find a match section to write to.\n");
            continue;
        }

        /* We may have found a match */
        if (state->sections[si].endaddr != endaddr) {
            fprintf(stderr, "Matching section has a different size.\n");
            continue;
        }

        /* Match found, preparing the write */
        size = endaddr - addr;

        local[0].iov_base = state->sections[si].mem;
        local[0].iov_len = size;
        remote[0].iov_base = (void *) addr;
        remote[0].iov_len = size;

        /* Now, writing the section to the game process */
        ssize_t nread = process_vm_writev(game_pid, local, 1, remote, 1, 0);

        /* Checking for errors */
        if (nread != (ssize_t)size) {
            fprintf(stderr, "Not all memory was written! Only %zd\n", nread);
            fprintf(stderr, 
                    "Current segment has %lld bytes at 0x%llx (%c%c%c)", 
                    size, addr, 
                    readflag  ? 'r' : '-', 
                    writeflag ? 'w' : '-',
                    execflag  ? 'x' : '-');
            if (filename[0])
                fprintf(stderr, " for %s", filename);
            fprintf(stderr, "\n");
            fprintf(stderr, "Continue loading the state...\n");
        }

        if (nread == -1) {
            switch (errno) {
                case EINVAL:
                    fprintf(stderr, "The amount of bytes read is too big!\n");
                    break;
                case EFAULT:
                    fprintf(stderr, "Bad address space of the game process or own process!\n");
                    break;
                case ENOMEM:
                    fprintf(stderr, "Could not allocate memory for internal copies of the iovec structures.\n");
                    break;
                case EPERM:
                    fprintf(stderr, "Do not have permission to write the game process memory.\n");
                    break;
                case ESRCH:
                    fprintf(stderr, "The game PID does not exist.\n");
                    break;
            }
        }

        if (nread >= 0)
            total_size_loaded += size;

        section_i++;
    }

    /* Test the number of sections */
    if (section_i != state->n_sections) {
        fprintf(stderr, "The number of rw sections was changed from %d to %d!\n", state->n_sections, section_i);
    }

    fprintf(stderr, "This is the end, loaded %lld bytes.\n", total_size_loaded);

    free(local);
    free(remote);
    fclose(mapsfile);

    /* Detach from the game process */
    detachToGame(game_pid);

}

void deallocState(struct State* state)
{
    int si = 0;
    for (si=0; si<state->n_sections; si++) {
        free(state->sections[si].filename);
        free(state->sections[si].mem);
    }
    free(state->sections);
}
