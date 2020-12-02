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

Agent* getAgents(int bypass) {
    Agent* agents = (Agent*)malloc(sizeof(Agent));
    if (bypass) {
        strcpy(agents->location.address, "10.0.3.3");
    } else {
        // TODO this cannot be `localhost`
        strcpy(agents->location.address, "127.0.0.1\0");
    }
    agents->location.port = AGENT_CONTROL_PORT;
    return agents;
}
