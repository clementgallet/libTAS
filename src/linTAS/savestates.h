#include <sys/ptrace.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>
#include <inttypes.h>

void attachToGame(pid_t game_pid);
void detachToGame(pid_t game_pid);
int 
read_mapping (FILE *mapfile, 
          unsigned long long int *addr, 
          unsigned long long int *endaddr, 
          char *permissions, 
          unsigned long long int *offset, 
          char *device, 
          unsigned long long int *inode, 
          char *filename);

void saveState(pid_t game_pid);
