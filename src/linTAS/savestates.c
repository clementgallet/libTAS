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
    char* ret = fgets(linestring, 1024, mapfile);

    if (ret == NULL)
        return 0;

    *addr = strtoull(linestring, &linestring, 16);
    if (addr == 0) // Adress 0 is never used
        return 0;

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
    return 1;
}

/*
 * Access and save all memory regions of the game process that are writable.
 * Code taken partly from GDB.
 */

void saveState(pid_t game_pid)
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

    /* Now iterate until end-of-file. */
    while (read_mapping (mapsfile, &addr, &endaddr, &permissions[0], 
                &offset, &device[0], &inode, &filename[0]))
    {
        size = endaddr - addr;

        /* Get the segment's permissions.  */
        readflag  = (strchr (permissions, 'r') != 0);
        writeflag = (strchr (permissions, 'w') != 0);
        execflag  = (strchr (permissions, 'x') != 0);

        fprintf(stderr, 
                "Save segment, %lld bytes at 0x%llx (%c%c%c)", 
                size, addr, 
                readflag  ? 'r' : ' ', 
                writeflag ? 'w' : ' ',
                execflag  ? 'x' : ' ');
        if (filename[0])
            fprintf(stderr, " for %s", filename);
        fprintf(stderr, "\n");

        /* Filter based on permissions */
        if (!writeflag)
            continue;

        /* Move pointer to offset. */
        lseek(memfd, (off_t)addr, SEEK_SET);

        /* Placeholder for storing the memory dump somewhere. */
        char c;
        read(memfd, &c, 1);

    }

    fclose(mapsfile);
    close(memfd);

    /* Detach from the game process */
    detachToGame(game_pid);
}
