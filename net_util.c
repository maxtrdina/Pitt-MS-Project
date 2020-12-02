//
// Created by izzy on 11/12/20.
//

#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include "net_util.h"
#include "protocol.h"

void send_message(Message message, char* dstAddr, int dstPort) {
    send_message_r(message, dstAddr, dstPort, 0);
}

Message* send_message_r(Message message, char* dstAddr, int dstPort, int expectResponse) {
    int socket_fd, n;
    struct sockaddr_in serv_addr;

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        printf("ERROR opening socket");
        return NULL;
    }

    printf("Before Addr\n");
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(dstAddr);
    serv_addr.sin_port = htons(dstPort);

    printf("Before Connect\n");
    if (connect(socket_fd, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr_in)) < 0) {
        printf("ERROR connecting");
        return NULL;
    }

    printf("Sending message...\n");
    n = send(socket_fd, &message, sizeof(Message), 0);
    if (n < 0) {
        printf("ERROR writing to socket");
        return NULL;
    }

    if (expectResponse) {
        printf("Receiving response...\n");
        Message* received_message = (Message*)malloc(sizeof(Message));
        recv(socket_fd, received_message, sizeof(Message), 0);
        close(socket_fd);
        return received_message;
    }

    close(socket_fd);
    return NULL;
}
