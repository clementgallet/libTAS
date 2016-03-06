#include "savestates_criu.h"

void init_criu(int pid)
{
    criu_init_opts();
    criu_set_service_address(NULL);
    criu_set_pid(pid);
    int fd = open("./", O_DIRECTORY);
    criu_set_images_dir_fd(fd); /* must be set for dump/restore */

    criu_set_log_file("restore.log");
    criu_set_log_level(4);
}

int dump_criu(void)
{
    return criu_dump();
}

int restore_criu(void)
{
    return criu_restore();
}

