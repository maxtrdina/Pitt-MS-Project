//
// Created by izzy on 11/11/20.
//

#include <stdio.h>
#include <stdlib.h>
#include "network_manager.h"
#include "topology_manager.h"
#include "common.h"
#include "map.h"
#include "net_util.h"

int flow_ct;
Map* map;

void nm_initialize() {
    flow_ct = 0;
    map = map_create();
}

int create_flow(AddFlowRequest request) {
    //int agentCount = 1;
    Agent* agents = getAgents();
    Agent target = *agents;

    printf("Creating flow, %s\n", request.destination.address);
    Flow* newFlow = (Flow*)malloc(sizeof(Flow));
    newFlow->switchAddr = request.switchAddr;
    newFlow->originalDst = request.destination;
    newFlow->newDst = target.location;
    printf("Flow populated\n");

    Message message = { .type = TYPE_ADD_FLOW_ROUTING, .flowRouting = {
            .spinesPort = 9999, .exitPoint = 5555
    } };
    printf("Message prepped\n");
    Message* response = send_message_r(message, newFlow->newDst.address, newFlow->newDst.port, 1);
    // The port the agent opened for this flow
    newFlow->newDst.port = response->ack.data;
    printf("Agent opened port #%d\n", response->ack.data);

    int flowId = flow_ct++;
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
