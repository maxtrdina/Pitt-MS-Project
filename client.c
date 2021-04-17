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

int firstSwitchPort = 50050;        // Switch port initializes to 50050
char switchAddr[16];
char myIp[16];
char myMac[18];

int main(int argc, char *argv[]) {
    char* hostIdStr = argv[1];      // Read value of CLI argument and set it as hostIdStr
    int hostId = atoi(hostIdStr);   // Convert it to an integer

    sprintf(switchAddr, "127.0.0.1:%d", firstSwitchPort+hostId);    // Sets switchAddr to 127.0.0.1 w/ a port number of 50050 + the number passed on the CLI
    sprintf(myIp, "10.0.%d.%d", hostId, hostId);                    // Sets myIp to 10.0.x.x where x is the number passed on the CLI
    sprintf(myMac, "08:00:00:00:0%d:%d%d", hostId, hostId, hostId); // Sets myMac to 08:00:00:00:0x:xx where x is the number passed on the CLI

    // Let's not worry about this for now
    // char nextMac[18];
    // sprintf(nextMac, "08:00:00:00:0%d:%d%d", hostId, hostId, hostId);

    printf("Switch: %s\n", switchAddr);
    printf("My IP: %s\n", myIp);
    printf("My MAC: %s\n", myMac);

    int select;
    printf("Option: ");
    scanf("%d", &select);   // Reads input from keyboard
    send_command(select);   // Calls send_command, passing keyboard input
}

void send_command(int option) {
    Message message; // Create a message. Defined in protocol.h
    Location target = { .address = "10.0.3.3", .port = MANAGER_CONTROL_PORT }; // Creates a target. Defined in common.h. Manager Control Port defined in constants.h
    int expect_response = 0;
    if (option == 0) {                                                                  // If user inputs 0
        message = (Message){ .type = TYPE_TERMINATE };
    } else if (option == 1) {                                                           // If user inputs 1
        //  Specify the message type as well as values for registerFlow, a struct contained in Message
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
        expect_response = 1; // We are expecting a response
    } else if (option == 2) {                                                           // If user inputs 2
        
        int usrPort;
        int resources;
        int inboundSite;
        int outboundSite;

        printf("Please specify a port number: ");
        scanf("%d", &usrPort);   // Reads input from keyboard
        printf("Please specify a resource requirement (int): ");
        scanf("%d", &resources);   // Reads input from keyboard
        printf("Please specify an inbound site (int): ");
        scanf("%d", &inboundSite);   // Reads input from keyboard
        printf("Please specify an outbound site (int): ");
        scanf("%d", &outboundSite);   // Reads input from keyboard


        message = (Message){ .type = TYPE_REGISTER_FLOW, .registerFlow = {
                .switchAddr = "\0", // 127.0.0.1:5005X
                .myIp = "\0",
                .myMac = "\0",
                .dst = { .address = "127.0.0.1\0", .port = usrPort },
                .bypass = 0,
                .resources = resources,
                .inboundSite = inboundSite,
                .outboundSite = outboundSite
        } };
        strcpy(message.registerFlow.switchAddr, switchAddr);
        strcpy(message.registerFlow.myIp, myIp);
        strcpy(message.registerFlow.myMac, myMac);
        strcpy(target.address, "127.0.0.1\0");
        expect_response = 1;
    } else if (option == 3) {                                                           // If user inputs 3
        message = (Message){ .type = TYPE_DELETE_FLOW, .deleteFlow = { .flowId = 0 } };
        printf("Flow? ");
        scanf("%d", &message.deleteFlow.flowId);
    } else if (option == 4) {                                                           // If user inputs 4
        printf("PING!\n");
        message = (Message){ .type = TYPE_PING };
        Message* rec = send_message_r(message, "10.0.2.2", 5647, 1);
        if (rec->type == TYPE_PONG) {
            printf("PONG!\n");
        }
    } else if (option == 5) {                                                           // If user inputs 5
        printf("Creating outbound flow...\n");
        message = (Message) {.type = TYPE_ADD_FLOW_ROUTING_OUTBOUND, .flowRouting = {
                .flowId = 999, .target = {.address = "127.0.0.1", .port = 11999}
        }};
        strcpy(target.address, "10.0.3.3\0");
        target.port = AGENT_CONTROL_PORT;
        expect_response = 1;
    } else if (option == 6) {                                                           // If user inputs 6
        printf("Creating inbound flow...\n");
        message = (Message) {.type = TYPE_ADD_FLOW_ROUTING_INBOUND, .flowRouting = {
                .flowId = 999, .target = {.address = "localhost", .port = 8108}
        }};
        strcpy(target.address, "10.0.3.3\0");
        target.port = AGENT_CONTROL_PORT;
        expect_response = 1;
    } else if (option == 7) { // For local tests only                                   // If user inputs 7
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
    } else if (option == 99) {                                                          // If user inputs 99
        message = (Message){ .type = TYPE_PRINT_STATE };
    } else {
        return;
    }

    printf("Sending...\n");
    printf("The target port is %d\n", target.port);
    Message* received = send_message_r(message, target.address, target.port, expect_response); // Sends message to the MANAGER!!!
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
}
