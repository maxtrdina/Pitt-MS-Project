//
// Created by izzy on 11/11/20.
//

#ifndef MANAGER_NETWORK_MANAGER_H
#define MANAGER_NETWORK_MANAGER_H

#include "common.h"

typedef struct AddFlowRequest_s {
    Location destination;
    Location switchAddr;
} AddFlowRequest;

typedef struct Flow_s {
    Location switchAddr;
    Location originalDst;
    Location newDst;
    // TODO add backbone info
} Flow;

void nm_initialize();
int create_flow(AddFlowRequest request);
void remove_flow(int flowId);
void nm_print_state();

#endif //MANAGER_NETWORK_MANAGER_H
