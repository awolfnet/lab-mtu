#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <zlib.h>

#define CONNECT_PORT 10000
#define LENGTH 6000

#define DEFAULT_TAG 0x07

typedef unsigned char BYTE;

typedef struct
{
    BYTE tag;
    BYTE syn[8];
    int data_length;
    uLong data_checksum;
    uLong header_checksum;
    BYTE *data
} MYPACKET;

int main(int argc, char *argv[])
{
    char *address = NULL;
    if (4 == argc && argv[1] != NULL)
    {
        address = argv[1];
    }
    else
    {
        address = "127.0.0.1";
    }

    int port = CONNECT_PORT;
    if (4 == argc && argv[2] != NULL)
    {
        port = atoi(argv[2]);
    }

    int length = LENGTH;
    if (4 == argc && argv[3] != NULL)
    {
        length = atoi(argv[3]);
    }

    printf("Connect to %s:%d, send data length %d\r\n", address, port, length);

    struct sockaddr_in servaddr;
    memset((void *)&servaddr, 0x00, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    if (inet_pton(AF_INET, address, &servaddr.sin_addr) == -1)
    {
        printf("ip addess not correct %s\r\n", address);
        return EXIT_FAILURE;
    }

    //Socket
    int socket_client_fd;
    if ((socket_client_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        printf("socket error(%d): %s\r\n", errno, strerror(errno));
        return EXIT_FAILURE;
    }

    //Connect to server
    if (connect(socket_client_fd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1)
    {
        printf("connect to server error(%d): %s\r\n", errno, strerror(errno));
        return EXIT_FAILURE;
    }

    printf("Connected.\r\n");

    BYTE *buffer = calloc(length, sizeof(BYTE));
    memset((void *)buffer, 'A', length - 1);



    if (send(socket_client_fd, buffer, length, 0) < 0)
    {
        printf("send error(%d): %s\r\n", errno, strerror(errno));
        close(socket_client_fd);
        free(buffer);
        return EXIT_FAILURE;
    }
    close(socket_client_fd);
    free(buffer);
    return EXIT_SUCCESS;
}