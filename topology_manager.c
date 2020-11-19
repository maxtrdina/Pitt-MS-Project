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
    //strcpy(agents->location.address, "127.0.0.1\0");
    strcpy(agents->location.address, "10.0.3.3");
    agents->location.port = AGENT_CONTROL_PORT;
    return agents;
}
