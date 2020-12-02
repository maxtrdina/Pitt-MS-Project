//
// Created by izzy on 11/11/20.
//

#ifndef MANAGER_TOPOLOGY_MANAGER_H
#define MANAGER_TOPOLOGY_MANAGER_H

#include "common.h"

typedef struct Agent_s {
    Location location;
} Agent;

void initialize();
Agent* getAgents(int bypass);

#endif // MANAGER_TOPOLOGY_MANAGER_H
