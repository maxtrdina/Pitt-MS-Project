//
// Created by izzy on 11/11/20.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "network_manager.h"
#include "topology_manager.h"
#include "common.h"
#include "map.h"
#include "net_util.h"

int flow_ct;
Map* map;

int fileExists(char *filename);

void nm_initialize() {
    flow_ct = 0;
    map = map_create();
}

int create_flow(RegisterFlow flow) {
    int flowId = flow_ct++;

    //int agentCount = 1;
    Agent* agents = getAgents();
    Agent target = *agents;

    printf("Creating flow, %s\n", flow.dst.address);
    Flow* newFlow = (Flow*)malloc(sizeof(Flow));
    strcpy(newFlow->switchAddr, flow.switchAddr);
    newFlow->originalDst = flow.dst;
    newFlow->newDst = target.location;
    printf("Flow populated\n");

    Message message = { .type = TYPE_ADD_FLOW_ROUTING, .flowRouting = {
            .flowId = flowId, .spinesPort = 9999, .exitPoint = 5555
    } };
    printf("Message prepped\n");
    Message* response = send_message_r(message, newFlow->newDst.address, newFlow->newDst.port, 1);
    // The port the agent opened for this flow
    newFlow->newDst.port = response->ack.data;
    printf("Agent opened port #%d\n", newFlow->newDst.port);

    // Write the file the controller will read
    // 1 million possible requests, increase if needed
    char filename[21+6];
    sprintf(filename, "ext_controller/reqs/r%d", flowId);
    FILE *fptr = fopen(filename, "w");
    fprintf(fptr, "%s", flow.switchAddr);
    fprintf(fptr, "\n%s", flow.myIp);
    fprintf(fptr, "\n%s", flow.myMac);
    fprintf(fptr, "\n%s:%d-%s:%d", flow.dst.address, flow.dst.port, newFlow->newDst.address, newFlow->newDst.port);
    fclose(fptr);

    char ackFilename[23+6];
    sprintf(ackFilename, "ext_controller/reqs/ack%d", flowId);
    while (!fileExists(ackFilename)) {
        usleep(50*1000);
    }
    remove(ackFilename);

    map_insert(map, flowId, newFlow);

    return flowId;
}

void remove_flow(int flowId) {
    // TODO kill agent thread
    map_delete(map, flowId);
}

void nm_print_state() {
    printf("Printing state...\n");
    for (int i = 0; i < map->_capacity; i++) {
        if (map->keys[i] != -1) {
            Flow flow = *((Flow*)(map->values[i]));
            printf(
                    "\tFlow #%d: from %s, %d to %s, %d\n",
                    map->keys[i],
                    flow.originalDst.address, flow.originalDst.port,
                    flow.newDst.address, flow.newDst.port
            );
        }
    }
}

int fileExists(char *filename) {
    return access(filename, F_OK) != -1;
}
