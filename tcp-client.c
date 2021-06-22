#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <zlib.h>

#define CONNECT_PORT 10000
#define LENGTH 6000
#define SEND_TIMES 5

#define DEFAULT_TAG 0x21

typedef unsigned char BYTE;

typedef struct
{
    BYTE tag;
    BYTE syn[8];
    int data_length;
    uLong data_checksum;
    uLong header_checksum;
} HEADER;

int main(int argc, char *argv[])
{
    char *address = NULL;
    if (5 == argc && argv[1] != NULL)
    {
        address = argv[1];
    }
    else
    {
        address = "127.0.0.1";
    }

    int port = CONNECT_PORT;
    if (5 == argc && argv[2] != NULL)
    {
        port = atoi(argv[2]);
    }

    int length = LENGTH;
    if (5 == argc && argv[3] != NULL)
    {
        length = atoi(argv[3]);
    }

    int times = SEND_TIMES;
    if (5 == argc && argv[4] != NULL)
    {
        times = atoi(argv[4]);
    }

    printf("Send to %s:%d, send data length %d, times %d\r\n", address, port, length, times);

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
    if ((socket_client_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
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
    while (times-- > 0)
    {
        memset((void *)buffer, 'A' + times, length - 1);
        uLong buffer_crc = crc32(0L, buffer, length);

        HEADER header;
        memset((void *)&header, 0x00, sizeof(header));

        //TAg
        header.tag = DEFAULT_TAG;

        //SYN
        sprintf((char *)header.syn, "%x", times);

        //Data length
        header.data_length = length;

        //Data checksum
        header.data_checksum = buffer_crc;

        //Header checksum
        header.header_checksum = 0L;
        uLong header_crc = crc32(0L, (const Bytef *)&header, sizeof(header));
        header.header_checksum = header_crc;

        int header_length = sizeof(header);
        printf("Sendig %d\r\n", times);
        printf("Sending header, length(%d), syn(%x), datalength(%u), datachecksum(%u), headerchecksum(%u)\r\n", header_length, *header.syn, header.data_length, header.data_checksum, header.header_checksum);
        if (send(socket_client_fd, &header, header_length, 0) == -1)
        {
            printf("send header error(%d): %s\r\n", errno, strerror(errno));
            break;
        }
        printf("Sending data, length(%d)\r\n", length);
        if (send(socket_client_fd, buffer, length, 0) == -1)
        {
            printf("send data error(%d): %s\r\n", errno, strerror(errno));
            break;
        }
    }

    close(socket_client_fd);
    free(buffer);
    return EXIT_SUCCESS;
}