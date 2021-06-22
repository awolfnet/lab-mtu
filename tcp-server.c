#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <zlib.h>

#include "MD5.h"

#define LISTEN_PORT 10000
#define LENGTH 6000
#define REQUEST_QUEUE 10
#define RECV_TIMEOUT 30

typedef unsigned char BYTE;

int main(int argc, char *argv[])
{
    int port = LISTEN_PORT;
    if (3 == argc && argv[1] != NULL)
    {
        port = atoi(argv[1]);
    }

    int length = LENGTH;
    if (3 == argc && argv[2] != NULL)
    {
        length = atoi(argv[2]);
    }

    printf("Listen on %d, length %d \r\n", port, length);
    //Socket
    int socket_server_fd;
    if ((socket_server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        printf("socket error(%d): %s\r\n", errno, strerror(errno));
        return EXIT_FAILURE;
    }

    //Set recv timeout
    struct timeval tv;
    tv.tv_sec = RECV_TIMEOUT;
    tv.tv_usec = 0;
    setsockopt(socket_server_fd, SOL_SOCKET, SO_RCVTIMEO, (const void *)&tv, sizeof(tv));

    //listen local address
    struct sockaddr_in servaddr;
    memset((void *)&servaddr, 0x00, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(LISTEN_PORT);

    //bind local port
    if (bind(socket_server_fd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1)
    {
        printf("bind port(%d) error(%d): %s\r\n", LISTEN_PORT, errno, strerror(errno));
        return EXIT_FAILURE;
    }

    //listen port
    if (listen(socket_server_fd, REQUEST_QUEUE) == -1)
    {
        printf("listen port(%d) error(%d): %s\r\n", LISTEN_PORT, errno, strerror(errno));
        return EXIT_FAILURE;
    }

    printf("Listen on port:%d\r\n", LISTEN_PORT);

    while (true)
    {
        printf("Waiting for connection.\r\n");
        int connection_fd;
        if ((connection_fd = accept(socket_server_fd, (struct sockaddr *)NULL, NULL)) == -1)
        {
            printf("accept connection error(%d): %s\r\n", errno, strerror(errno));
            continue;
        }

        printf("Connection accepted.\r\n");

        while (true)
        {
            BYTE *buffer;
            buffer = calloc(length + 1, sizeof(BYTE)); //add 1 for the end of string \0

            int n = recv(connection_fd, (void *)buffer, length, 0);
            if (0 == n)
            {
                printf("Connection was closed by peer.\r\n");
                break;
            }
            else if (-1 == n)
            {
                printf("receive data error(%d): %s\r\n", errno, strerror(errno));
                break;
            }
            
            free(buffer);
            printf("received length(%d) buff: %s\n", n, buff);
        }
        printf("Connection closed.\r\n");
        close(connection_fd);
    }

    close(socket_server_fd);

    printf("done.");
    return EXIT_SUCCESS;
}
