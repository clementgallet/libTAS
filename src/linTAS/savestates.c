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
 * Code taken from GDB
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
 * Code taken partly from GDB.
 */

void saveState(pid_t game_pid, struct State* state)
{
    char memfilename[2048];
    char mapsfilename[2048];
    int memfd;
    FILE *mapsfile;
    unsigned long long int addr, endaddr, size, offset, inode;
    char permissions[8], device[8], filename[2048];
    int readflag, writeflag, execflag;

    /* Attach to the game process */
    attachToGame(game_pid);

    /* Open the process memory */
    sprintf(memfilename, "/proc/%d/mem", game_pid);
    memfd = open(memfilename, O_RDONLY);
    if (memfd == -1) {
        fprintf(stderr, "Cound not open file %s, error %d\n", memfilename, errno);
        return;
    }

    /* Compose the filename for the /proc memory map, and open it. */
    sprintf (mapsfilename, "/proc/%d/maps", game_pid);
    if ((mapsfile = fopen (mapsfilename, "r")) == NULL) {
        fprintf(stderr, "Could not open %s\n", mapsfilename);
        return;
    }

    /* Count how many lines in maps file to allocate the state */
    int n_sections = 0;
    for (int c = fgetc(mapsfile); c != EOF; c = fgetc(mapsfile)) {
        if (c == '\n') n_sections++;
    }
    rewind(mapsfile);

    /* Allocate the state */
    //fprintf(stderr, "Section size: %d.\n", sizeof(struct StateSection));
    state->sections = malloc(n_sections * sizeof(struct StateSection));

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
            return;
        }
        memcpy(state->sections[section_i].filename, filename, filename_size);
        state->sections[section_i].filename[filename_size] = '\0';

        /* Copy the actual memory section */
        state->sections[section_i].mem = malloc(size * sizeof(char));
        if (state->sections[section_i].mem == NULL) {
            fprintf(stderr, "Cound not alloc memory of size %lld", size);
            return;
        }

        /* Move pointer to offset. */
        lseek(memfd, (off_t)addr, SEEK_SET);

        /* Copy. TODO: should we split this in chunks? */

        /* Test if we must save more byte than can be stored in a 32-bit integer */
        if (size > (1L << 31)) {
            fprintf(stderr, "The size (%lld) of section does not hold in a 32-bit integer.\n", size);
            return;
        }

        size_t bytesread = read(memfd, state->sections[section_i].mem, (size_t)size);
        if (bytesread != (size_t)size) {
            fprintf(stderr, "Only read %zu bytes of memory of size %lld", bytesread, size);
            return;
        }

        total_size += size;

        section_i++;
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
        return;
    }

    struct StateSection* sections_realloc = realloc(state->sections, section_i * sizeof(struct StateSection));
    if (sections_realloc == NULL) {
        fprintf(stderr, "Realloc failed\n");
        free(state->sections);
        return;
    }
    else {
        state->sections = sections_realloc;
        state->n_sections = section_i;
    }

    fprintf(stderr, "Wow, we actually did not raise any error. Saving %lld bytes\n", total_size);

    fclose(mapsfile);
    close(memfd);

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
