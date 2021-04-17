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

        recv(client_fd, buf, sizeof(Message), 0);               // Manager receives message from the client
        message = (Message*)buf;
        printf("Received message (%d)...\n", message->type);    // Prints message type

        if (message->type == TYPE_REGISTER_FLOW) {          // If type is Register Flow
            printf("Register Flow message.\n");
            int flowId = create_flow(message->registerFlow);                // Create new flow using message data, returns flowID. create_flow is defined in Network_Manager.c
            Message out = {.type = TYPE_ACK, .ack = { .data = flowId } };   // Create message to send back to Client
            send(client_fd, &out, sizeof(Message), 0);
        } else if (message->type == TYPE_DELETE_FLOW) {     // If type is Delete Flow
            printf("Delete Flow message.\n");
            remove_flow(message->deleteFlow.flowId);    // Calls remove_flow, defined in Network_Manager.c
        } else if (message->type == TYPE_TERMINATE) {       // If type is Terminate
            printf("Terminate message.\n");
            terminate = 1;
        } else if (message->type == TYPE_PRINT_STATE) {     // If type is Print State
            nm_print_state();
        }

        close(client_fd);
    }

    close(server_fd);
}
