#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <zlib.h>

#define LISTEN_PORT 10000
#define LENGTH 6000
#define REQUEST_QUEUE 10
#define RECV_TIMEOUT 30

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

    printf("port %d, length %d \r\n", port, length);
    //Socket
    int socket_server_fd;
    if ((socket_server_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        printf("socket error(%d): %s\r\n", errno, strerror(errno));
        return EXIT_FAILURE;
    }

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

    printf("Bind on port:%d\r\n", LISTEN_PORT);

    while (true)
    {
        printf("Waiting for data.\r\n");

        while (true)
        {
            struct sockaddr_in cliaddr;

            HEADER header;
            memset((void *)&header, 0x00, sizeof(header));

            //receive header
            int sockaddr_len = sizeof(cliaddr);
            int n_header = recvfrom(socket_server_fd, (void *)&header, sizeof(header), 0, (struct sockaddr *)&cliaddr, &sockaddr_len);
            if (0 == n_header)
            {
                printf("the peer has performed an orderly shutdown.\r\n");
                break;
            }
            else if (-1 == n_header)
            {
                printf("receive header error(%d): %s\r\n", errno, strerror(errno));
                break;
            }

            printf("Header received, syn(%x), datalength(%u), datachecksum(%u), headerchecksum(%u)\r\n", *header.syn, header.data_length, header.data_checksum, header.header_checksum);

            uLong header_crc_original = header.header_checksum;
            header.header_checksum = 0L;
            uLong header_crc = crc32(0L, (const Bytef *)&header, sizeof(header));

            //Check header checksum, make sure header is correct, can receive correct data with length
            if (header_crc_original != header_crc)
            {
                printf("Header crc was not matched, header maybe wrong.\r\n");
                break;
            }

            BYTE buffer[header.data_length + 1]; //Add 1 for end of string '\0
            memset(buffer, 0x00, header.data_length + 1);

            //receive data until length is enough
            int data_receive = 0;
            while (data_receive < header.data_length)
            {
                //bytes to receive
                int to_recv = header.data_length - data_receive;
                int n_data = recvfrom(socket_server_fd, (void *)&buffer[data_receive], to_recv, 0, (struct sockaddr *)&cliaddr, &sockaddr_len);
                if (0 == n_data)
                {
                    printf("the peer has performed an orderly shutdown.\r\n");
                    break;
                }
                else if (-1 == n_data)
                {
                    int er = errno;
                    printf("receive data error(%d): %s\r\n", er, strerror(er));
                    break;
                }
                data_receive += n_data;
                printf("Received data length(%d),total(%d), expect(%d) \r\n", n_data, data_receive, header.data_length);
            }

            uLong data_crc = crc32(0L, (const Bytef *)buffer, header.data_length);
            if (header.data_checksum != data_crc)
            {
                printf("Data crc was not matched, data maybe wrong.\r\n");
            }
            else
            {
                printf("Total received length(%d) and checksum correct\r\n", data_receive);
            }
        }
        printf("Connection closed.\r\n");
    }

    close(socket_server_fd);

    printf("done.");
    return EXIT_SUCCESS;
}
