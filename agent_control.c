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
#include "agent_control.h"
#include "agent_data.h"
#include "protocol.h"
#include "map.h"
#include "constants.h"

Map* flowRoutingMap;

//int resourceCapactiy = 10;

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

        recv(client_fd, buf, sizeof(Message), 0);           // Receive message from the Manager
        printf("Message: %s\n", buf);
        fprintf(stderr, "Value of errno: %d\n", errno);
        message = (Message*)buf;                            // Buffer message info

        printf("Received type: %d\n", message->type);           // Print message type
        if (message->type == TYPE_ADD_FLOW_ROUTING_INBOUND) {           // If we are adding an inbound flow
            int port;
            printf("Resources requirement is: %d\n", message->flowRouting.resources);
            //if (resourceCapactiy >= message->flowRouting.resources){ // If we have capacity remaining
            //if (NULL == NULL) {
                Location target = message->flowRouting.target;      
                printf("Request to add inbound flow routing to %s:%d\n", target.address, target.port);
                port = create_interface(1, target, message->flowRouting.node, message->flowRouting.bypass, 0);
                printf("Interface created at %d.\n", port);
                add_flow_routing(message->flowRouting);
                Message out = {.type = TYPE_ACK, .ack = { .data = port } };
                send(client_fd, &out, sizeof(Message), 0);
                //resourceCapactiy = resourceCapactiy - message->flowRouting.resources; // Decrement capacity
            /*} else { // If we don't have capacity
                port = -1;
                printf("This agent doesn't have the required capacity to create this flow.\n");
                Message out = {.type = TYPE_ACK, .ack = { .data = port } }; // return a port # of -1 to the manager to indicate a failure
                send(client_fd, &out, sizeof(Message), 0);
            }*/
            
        } else if (message->type == TYPE_ADD_FLOW_ROUTING_OUTBOUND) {   // If we are adding an outbound flow
            int port;
            printf("Resources requirement is: %d\n", message->flowRouting.resources);
            //if (resourceCapactiy >= message->flowRouting.resources){
            //if (NULL == NULL) {    
                Location target = message->flowRouting.target;      // This is the target specified by the client
                printf("Request to add outbound flow routing to %s:%d\n", target.address, target.port);
                port = create_interface(0, target, message->flowRouting.node, message->flowRouting.bypass, message->flowRouting.spinesPort);    // Calls create_interface, in agent_data.c. This creates a socket at the dest. agent
                printf("Interface created at %d.\n", port);
                add_flow_routing(message->flowRouting);
                Message out = {.type = TYPE_ACK, .ack = { .data = port } };
                send(client_fd, &out, sizeof(Message), 0);
                //resourceCapactiy = resourceCapactiy - message->flowRouting.resources; // Decrement capacity
            /*} else {
                port = -1;
                printf("This agent doesn't have the required capacity to create this flow.\n");
                Message out = {.type = TYPE_ACK, .ack = { .data = port } };
                send(client_fd, &out, sizeof(Message), 0);
            }*/
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
    map_insert(flowRoutingMap, flowRouting.flowId, &flowRouting);
}

void remove_flow_routing(RemoveFlowRouting removeFlowRouting) {
    printf("Removing Flow Routing.");
    map_delete(flowRoutingMap, removeFlowRouting.flowId);
}
