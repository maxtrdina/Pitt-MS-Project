//
// Created by izzy on 11/11/20.
//

#include <stdlib.h>
#include <string.h>
#include "constants.h"
#include "topology_manager.h"

void initialize() {
    // TODO
}

Agent* getAgents() {
    Agent* agents = (Agent*)malloc(sizeof(Agent));
    strncpy(agents->location.address, "localhost", strlen("localhost"));
    agents->location.port = AGENT_CONTROL_PORT;
}
