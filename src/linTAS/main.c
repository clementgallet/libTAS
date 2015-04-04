#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#define MAGIC_NUMBER 42
#define SOCKET_FILENAME "/tmp/libTAS.socket"

void draw_cli(void);
int proceed_command(unsigned int command, int socket_fd);

enum key_codes
{
    UP,
    LEFT,
    DOWN,
    RIGHT,
    SPACE,
    SHIFT,
    KEYS_NUMBER // Number of keys.
};
unsigned char keys[KEYS_NUMBER] = {0};
unsigned int speed_divisor = 1;
unsigned char running = 0;
unsigned long int frame_counter = 0;

int main(void)
{
    const struct sockaddr_un addr = { AF_UNIX, SOCKET_FILENAME };
    int socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);

    printf("Connecting to libTAS...\n");

    if (connect(socket_fd, (const struct sockaddr*)&addr, sizeof(struct sockaddr_un)))
    {
        printf("Couldn’t connect to socket.\n");
        return 1;
    }

    printf("Connected.\n");

    while (1)
    {
        unsigned int command;

        draw_cli();

        do
            printf("(SuperMeatBoyTaser) ");
        while (!scanf("%u", &command));

        if (proceed_command(command, socket_fd))
            break;
    }

    close(socket_fd);
    return 0;
}

void draw_cli(void)
{
    printf("\n        \033[%cm[ UP ]\033[0m                 \033[%cm[SPACE]\033[0m\n\n",
           keys[UP] ? '7' : '0', keys[SPACE] ? '7' : '0');
    printf("\033[%cm[LEFT]\033[0m  \033[%cm[DOWN]\033[0m  \033[%cm[RIGHT]\033[0m        \033[%cm[SHIFT]\033[0m\n\n",
           keys[LEFT] ? '7' : '0', keys[DOWN] ? '7' : '0', keys[RIGHT] ? '7' : '0', keys[SHIFT] ? '7' : '0');
    printf("%s      Speed divisor: %u     Frame counter: %lu\n\n",
           running ? "\033[7m[RUNNING ]\033[0m" : "[ PAUSED ]", speed_divisor, frame_counter);
    printf("Available commands:\n\n");
    printf("1 - Toggle UP.\n");
    printf("2 - Toggle DOWN.\n");
    printf("3 - Toggle LEFT.\n");
    printf("4 - Toggle RIGHT.\n\n");
    printf("5 - Toggle SPACE.\n");
    printf("6 - Toggle SHIFT.\n\n");
    printf("7 - Toggle PAUSE/RUNNING.\n");
    printf("8 - Advance 1 frame.\n");
    printf("9 - Set speed divisor.\n\n");
    printf("10 - Save inputs.\n");
    printf("11 - Load inputs.\n\n");
    printf("0 - Exit.\n\n");
}

int proceed_command(unsigned int command, int socket_fd)
{
    if (!command)
        return 1;

    if (command > 11)
    {
        printf("This command does not exist.\n");
        return 0;
    }

    send(socket_fd, &command, sizeof(unsigned int), 0);

    char filename_buffer[1024];

    switch (command)
    {
    case 1:
        keys[UP] = !keys[UP];
        break;

    case 2:
        keys[DOWN] = !keys[DOWN];
        break;

    case 3:
        keys[LEFT] = !keys[LEFT];
        break;

    case 4:
        keys[RIGHT] = !keys[RIGHT];
        break;

    case 5:
        keys[SPACE] = !keys[SPACE];
        break;

    case 6:
        keys[SHIFT] = !keys[SHIFT];
        break;

    case 7:
        running = !running;
        break;

    case 9:
        speed_divisor = 0;
        do
        {
            printf("Enter non-null speed divisor factor: ");
            scanf("%u", &speed_divisor);
        }
        while (!speed_divisor);

        send(socket_fd, &speed_divisor, sizeof(unsigned int), 0);
        break;

    case 10:
        printf("Enter filename to save inputs in: ");
        scanf("%s", filename_buffer);
        send(socket_fd, filename_buffer, 1024, 0);

        unsigned long first_frame = 0;
        do
            printf("Enter first frame to record: ");
        while (!scanf("%lu", &first_frame));

        send(socket_fd, &first_frame, sizeof(unsigned long), 0);
        break;

    case 11:
        printf("Enter filename from which to load inputs: ");
        scanf("%s", filename_buffer);
        send(socket_fd, filename_buffer, 1024, 0);

        /* Check if loading can be done. */
        unsigned char answer;
        recv(socket_fd, &answer, sizeof(unsigned char), 0);
        if (!answer)
        {
            printf("libTAS couldn’t load inputs.\n");
            break;
        }

        /* Update inputs. */
        recv(socket_fd, &answer, sizeof(unsigned char), 0);
        for (unsigned int i = 0; i < KEYS_NUMBER; ++i)
        {
            keys[i] = answer & 0x1;
            answer >>= 1;
        }

    default:;
    }

    command = 0;
    send(socket_fd, &command, sizeof(unsigned int), 0);
    recv(socket_fd, &frame_counter, sizeof(unsigned long), 0);

    return 0;
}
