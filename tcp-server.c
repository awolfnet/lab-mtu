#include <stdio.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define LISTEN_PORT 10000

int main(int argc, char **argv)
{
    int socker_server_fd;

    struct sockaddr_in servaddr;

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(LISTEN_PORT);

    if ((socker_server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        printf("create socket error: %s(errno: %d)\n", strerror(errno), errno);
        return 1;
    }

    if (bind(socker_server_fd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1)
    {
        printf("bind port(%d) error: %s(errno: %d)\n", LISTEN_PORT, strerror(errno), errno);
        return 1;
    }

    if (listen(socker_server_fd, 10) == -1)
    {
        printf("listen port(%d) error: %s(errno: %d)\n", LISTEN_PORT, strerror(errno), errno);
        return 0;
    }

    printf("done.");
    return 0;
}