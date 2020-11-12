//
// Created by izzy on 11/11/20.
//

#include <stdio.h>
#include <stdlib.h>
#include "net_util.h"
#include "constants.h"
#include "protocol.h"

void send_command(int option);

int port = 5647;

int main(int argc, char *argv[]) {
    int select;
    do {
        printf("Option: ");
        scanf("%d", &select);
        send_command(select);
    } while (select);
}

void send_command(int option) {
    Message message;
    if (option == 0) {
        message = (Message){ .type = TYPE_TERMINATE };
    } else if (option == 1) {
        message = (Message){ .type = TYPE_REGISTER_FLOW, .registerFlow = {
                .dst = { .address = "127.0.0.1\0", .port = ++port },
                .switchAddr = { .address = "127.0.0.1\0", .port = 6666 },
        } };
    } else if (option == 2) {
        message = (Message){ .type = TYPE_DELETE_FLOW, .deleteFlow = { .flowId = 0 } };
        printf("Flow? ");
        scanf("%d", &message.deleteFlow.flowId);
    } else if (option == 3) {
        message = (Message){ .type = TYPE_PRINT_STATE };
    } else {
        return;
    }

    printf("Sending...\n");
    Message* received = send_message_r(message, "127.0.0.1\0", MANAGER_CONTROL_PORT, option == 1);
    if (received != NULL) {
        printf("Got flow ID: %d\n", received->ack.data);
        free(received);
    }
}
