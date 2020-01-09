#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <wait.h>

#define DEST_IP "192.168.1.10"
#define TEST_IP "127.0.0.1"
#define MYPORT 35   /* the port users will be connecting to */
#define SERVER_PORT 50
#define BUFSZ 500
#define BACKLOG 1
#define CLI_DISC -1

int main()
{
    int sockfd, client_fd, server_fd;
    struct sockaddr_in my_addr, client_addr, server_addr;
    uint32_t sin_size;
    int recv_bytes;
    char recv_buf[BUFSZ], send_buf[BUFSZ];

    memset(recv_buf,'\0', sizeof(recv_buf));
    memset(send_buf,'\0', sizeof(send_buf));
    memset(&my_addr, 0, sizeof(my_addr));
    memset(&client_addr, 0, sizeof(client_addr));
    memset(&server_addr, 0, sizeof(server_addr));

    if ((sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    my_addr.sin_family = AF_INET;         /* host byte order */
    my_addr.sin_port = htons(MYPORT);     /* short, network byte order */
    my_addr.sin_addr.s_addr = INADDR_ANY; /* auto-fill with my IP */
    bzero(&(my_addr.sin_zero), 8);        /* zero the rest of the struct */

    if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) \
                                                                      == -1) {
        perror("[bind]");
        exit(EXIT_FAILURE);
    }

    if (listen(sockfd, BACKLOG) == -1) {
        perror("[listen]");
        exit(EXIT_FAILURE);
    }
        sin_size = sizeof(struct sockaddr_in);

    if ((client_fd = accept(sockfd, (struct sockaddr *)&client_addr, \
                                                          &sin_size)) == -1) {
            perror("[accept]");
        }
        printf("[app] : got connection from %s\n", \
                                               inet_ntoa(client_addr.sin_addr));

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;      /* host byte order */
    server_addr.sin_port = htons(SERVER_PORT);    /* short, network byte order */
    server_addr.sin_addr.s_addr = inet_addr(DEST_IP);
    bzero(&(server_addr.sin_zero), 8);     /* zero the rest of the struct */

    if (connect(server_fd, (struct sockaddr *)&server_addr, \
                                              sizeof(struct sockaddr)) == -1) {
        perror("connect");
        exit(EXIT_FAILURE);
    }
    while(1) {  /* main accept() loop */
        recv_bytes = recv (client_fd,recv_buf, sizeof(recv_buf),0);
        if (recv_bytes == -1) {
            perror("received from Client");
        }
        if ((recv_bytes - 1) == CLI_DISC)
        {
            printf("Client disconnect");
            close(client_fd);
            close(sockfd);
            close(server_fd);
            exit (SO_ERROR);
        }
        recv_buf[recv_bytes-1] = '>';
        snprintf(send_buf,BUFSZ+strlen("Received[] <\n"),"Received[%d] <%s\n",recv_bytes-1,recv_buf);
        printf("%s",send_buf);
        if (send(server_fd, send_buf, strlen(send_buf), 0) == -1) {
            perror("send to Server");
        }
            if (strncmp(recv_buf,"exit",strlen(recv_buf)-1) == 0)
        {
            close(client_fd);
            close(sockfd);
            close(server_fd);
            exit(EXIT_SUCCESS);
        }
        memset(recv_buf,'\0', sizeof(recv_buf));
        memset(send_buf,'\0', sizeof(recv_buf));
        while(waitpid(-1,NULL,WNOHANG) > 0); /* clean up child processes */
    }
}
