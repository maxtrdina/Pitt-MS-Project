//
// Created by izzy on 11/3/20.
//

#include <stdio.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include "control.h"
#include "protocol.h"
#include "map.h"
#include "constants.h"

Map* flowRoutingMap;

void add_flow_routing(FlowRouting flow_routing);
void remove_flow_routing(RemoveFlowRouting remove_flow_routing);

void run_control() {
    int server_fd, client_fd;
    char buf[MAX_PKT_SIZE];
    //fd_set mask, mask_template, dummy_mask;
    Message* message;

    flowRoutingMap = map_create();

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        printf("Problem opening TCP socket.\n");
        return;
    } else {
        printf("Socket open\n");
    }

    struct sockaddr_in server_sockaddr, client_sockaddr;
    server_sockaddr.sin_family = AF_INET;
    server_sockaddr.sin_addr.s_addr = INADDR_ANY;
    server_sockaddr.sin_port = htons(AGENT_CONTROL_PORT);

    if (bind(server_fd, (struct sockaddr *)&server_sockaddr, sizeof(server_sockaddr)) < 0) {
        printf("Problem binding TCP socket\n");
        return;
    } else {
        printf("Socket bound\n");
    }

//    FD_ZERO(&mask_template);
//    FD_ZERO(&mask);
//    FD_SET(tcp_in_fd, &mask_template);

    if (listen(server_fd, 4) < 0){
        printf("Error while listening\n");
        return;
    } else {
        printf("Listening to socket\n");
    }

    socklen_t client_sockaddr_size = sizeof(struct sockaddr_in);
    int terminate = 0;
    while (!terminate) {
        client_fd = accept(server_fd, (struct sockaddr *)&client_sockaddr, &client_sockaddr_size);

        printf("a\n");
        recv(client_fd, buf, sizeof(Message), 0);
        printf("Message: %s\n", buf);
        fprintf(stderr, "Value of errno: %d\n", errno);
        printf("a\n");
        message = (Message*)buf;

        printf("Received type: %d\n", message->type);
        if (message->type == TYPE_ADD_FLOW_ROUTING) {
            //add_flow_routing(message->flowRouting);
            printf("Request to add flow routing to %d\n", message->flowRouting.spinesPort);
            Message out = {.type = TYPE_ACK, .ack = { .data = 1234 } };
            send(client_fd, &out, sizeof(Message), 0);
        } else if (message->type == TYPE_REMOVE_FLOW_ROUTING) {
            remove_flow_routing(message->removeFlowRouting);
        } else if (message->type == TYPE_TERMINATE) {
            terminate = 1;
        } else if (message->type == TYPE_PRINT_STATE) {
            map_print(flowRoutingMap);
        }

        close(client_fd);
    }

    close(server_fd);
    map_destroy(flowRoutingMap);
}

void add_flow_routing(FlowRouting flowRouting) {
    printf("Adding Flow Routing.\n");
    map_insert(flowRoutingMap, flowRouting.spinesPort, &flowRouting);
}

void remove_flow_routing(RemoveFlowRouting removeFlowRouting) {
    printf("Removing Flow Routing.");
    map_delete(flowRoutingMap, removeFlowRouting.srcAddr);
}
