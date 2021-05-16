//
// Created by izzy on 11/11/20.
//

#ifndef MANAGER_NETWORK_MANAGER_H
#define MANAGER_NETWORK_MANAGER_H

#include "common.h"
#include "protocol.h"

typedef struct Flow_s {
    char switchAddr[16];
    Location originalDst;
    Location outDst;
    Location inDst;
    // TODO add backbone info
} Flow;

void nm_initialize();
int create_flow(RegisterFlow flow);
void remove_flow(int flowId);
void nm_print_state();

#endif //MANAGER_NETWORK_MANAGER_H
