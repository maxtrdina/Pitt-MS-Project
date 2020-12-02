//
// Created by izzy on 11/11/20.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "net_util.h"
#include "constants.h"
#include "protocol.h"

void send_command(int option);

int port = 5647;

int firstSwitchPort = 50050;
char switchAddr[16];
char myIp[16];
char myMac[18];

int main(int argc, char *argv[]) {
    char* hostIdStr = argv[1];
    int hostId = atoi(hostIdStr);

    sprintf(switchAddr, "127.0.0.1:%d", firstSwitchPort+hostId);
    sprintf(myIp, "10.0.%d.%d", hostId, hostId);
    sprintf(myMac, "08:00:00:00:0%d:%d%d", hostId, hostId, hostId);

    // Let's not worry about this for now
    // char nextMac[18];
    // sprintf(nextMac, "08:00:00:00:0%d:%d%d", hostId, hostId, hostId);

    printf("Switch: %s\n", switchAddr);
    printf("My IP: %s\n", myIp);
    printf("My MAC: %s\n", myMac);

    int select;
    do {
        printf("Option: ");
        scanf("%d", &select);
        send_command(select);
    } while (select);
}

void send_command(int option) {
    Message message;
    Location target = { .address = "10.0.3.3", .port = MANAGER_CONTROL_PORT };
    int expect_response = 0;
    if (option == 0) {
        message = (Message){ .type = TYPE_TERMINATE };
    } else if (option == 1) {
        message = (Message){ .type = TYPE_REGISTER_FLOW, .registerFlow = {
                .switchAddr = "\0", // 127.0.0.1:5005X
                .myIp = "\0",
                .myMac = "\0",
                .dst = { .address = "10.0.2.2\0", .port = 11999 },
                .bypass = 1
        } };
        strcpy(message.registerFlow.switchAddr, switchAddr);
        strcpy(message.registerFlow.myIp, myIp);
        strcpy(message.registerFlow.myMac, myMac);
        expect_response = 1;
    } else if (option == 2) {
        message = (Message){ .type = TYPE_REGISTER_FLOW, .registerFlow = {
                .switchAddr = "\0", // 127.0.0.1:5005X
                .myIp = "\0",
                .myMac = "\0",
                .dst = { .address = "127.0.0.1\0", .port = 11999 },
                .bypass = 0
        } };
        strcpy(message.registerFlow.switchAddr, switchAddr);
        strcpy(message.registerFlow.myIp, myIp);
        strcpy(message.registerFlow.myMac, myMac);
        expect_response = 1;
    } else if (option == 3) {
        message = (Message){ .type = TYPE_DELETE_FLOW, .deleteFlow = { .flowId = 0 } };
        printf("Flow? ");
        scanf("%d", &message.deleteFlow.flowId);
    } else if (option == 4) {
        printf("PING!\n");
        message = (Message){ .type = TYPE_PING };
        Message* rec = send_message_r(message, "10.0.2.2", 5647, 1);
        if (rec->type == TYPE_PONG) {
            printf("PONG!\n");
        }
    } else if (option == 5) {
        printf("Creating outbound flow...\n");
        message = (Message) {.type = TYPE_ADD_FLOW_ROUTING_OUTBOUND, .flowRouting = {
                .flowId = 999, .target = {.address = "127.0.0.1", .port = 11999}
        }};
        strcpy(target.address, "10.0.3.3\0");
        target.port = AGENT_CONTROL_PORT;
        expect_response = 1;
    } else if (option == 6) {
        printf("Creating inbound flow...\n");
        message = (Message) {.type = TYPE_ADD_FLOW_ROUTING_INBOUND, .flowRouting = {
                .flowId = 999, .target = {.address = "localhost", .port = 8108}
        }};
        strcpy(target.address, "10.0.3.3\0");
        target.port = AGENT_CONTROL_PORT;
        expect_response = 1;
    } else if (option == 7) { // For local tests only
        message = (Message){ .type = TYPE_REGISTER_FLOW, .registerFlow = {
                .switchAddr = "\0", // 127.0.0.1:5005X
                .myIp = "\0",
                .myMac = "\0",
                .dst = { .address = "127.0.0.1", .port = 11999 }
        } };
        strcpy(message.registerFlow.switchAddr, switchAddr);
        strcpy(message.registerFlow.myIp, "127.0.0.1\0");
        strcpy(message.registerFlow.myMac, myMac);
        strcpy(target.address, "127.0.0.1\0");
        expect_response = 1;
    } else if (option == 99) {
        message = (Message){ .type = TYPE_PRINT_STATE };
    } else {
        return;
    }

    printf("Sending...\n");
    Message* received = send_message_r(message, target.address, target.port, expect_response);
    if (received != NULL) {
        printf("Got flow ID: %d\n", received->ack.data);
        if (option == 4) {
            printf("Outbound flow open on port %d\n", received->ack.data);
        } else if (option == 5) {
            printf("Inbound flow open on port %d\n", received->ack.data);
        } else if (option == 6) {
            printf("Flow created; id: %d\n", received->ack.data);
        }
        free(received);
    }

    if (option == 1) {
        printf("PING!\n");
        Location loc = message.registerFlow.dst;
        message = (Message){ .type = TYPE_PING };
        received = send_message_r(message, /*"10.0.2.2\0"*/ loc.address, loc.port, 1);
        if (received->type == TYPE_PONG) {
            printf("PONG!\n");
        }
        free(received);
    }
}
