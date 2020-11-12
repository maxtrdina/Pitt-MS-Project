//
// Created by izzy on 11/8/20.
//

#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include "constants.h"
#include "protocol.h"
#include "network_manager.h"

int main(char argc, char *argv[]) {
    int server_fd, client_fd;
    char buf[MAX_PKT_SIZE];
    //fd_set mask, mask_template, dummy_mask;
    Message* message;

    nm_initialize();

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        printf("Problem opening TCP socket.\n");
        return 1;
    } else {
        printf("Socket open\n");
    }

    struct sockaddr_in server_sockaddr, client_sockaddr;
    server_sockaddr.sin_family = AF_INET;
    server_sockaddr.sin_addr.s_addr = INADDR_ANY;
    server_sockaddr.sin_port = htons(MANAGER_CONTROL_PORT);

    if (bind(server_fd, (struct sockaddr *)&server_sockaddr, sizeof(server_sockaddr)) < 0) {
        printf("Problem binding TCP socket\n");
        return 1;
    } else {
        printf("Socket bound\n");
    }

    if (listen(server_fd, 4) < 0){
        printf("Error while listening\n");
        return 1;
    } else {
        printf("Listening to socket\n");
    }

    socklen_t client_sockaddr_size = sizeof(struct sockaddr_in);
    int terminate = 0;
    while (!terminate) {
        client_fd = accept(server_fd, (struct sockaddr *)&client_sockaddr, &client_sockaddr_size);

        recv(client_fd, buf, sizeof(Message), 0);
        message = (Message*)buf;
        printf("Received message (%d)...\n", message->type);

        if (message->type == TYPE_REGISTER_FLOW) {
            printf("Register Flow message.\n");
            AddFlowRequest request = {
                    .destination = message->registerFlow.dst,
                    .switchAddr = message->registerFlow.switchAddr
            };
            int flowId = create_flow(request);
            Message out = {.type = TYPE_ACK, .ack = { .data = flowId } };
            send(client_fd, &out, sizeof(Message), 0);
        } else if (message->type == TYPE_DELETE_FLOW) {
            printf("Delete Flow message.\n");
            remove_flow(message->deleteFlow.flowId);
        } else if (message->type == TYPE_TERMINATE) {
            printf("Terminate message.\n");
            terminate = 1;
        } else if (message->type == TYPE_PRINT_STATE) {
            nm_print_state();
        }

        close(client_fd);
    }

    close(server_fd);
}
