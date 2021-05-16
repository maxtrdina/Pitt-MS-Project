//
// Created by izzy on 11/11/20.
//

#ifndef MANAGER_TOPOLOGY_MANAGER_H
#define MANAGER_TOPOLOGY_MANAGER_H

#include "common.h"
#include <stdutil/stdarr.h>

typedef struct Agent_s {
    Location location;
    int siteID;
    int throughput;
} Agent;

typedef struct Node_s {
    Location location;
    int siteID;
    int throughput;
} Node;

typedef struct Site_s {
    Location location;
    int siteID;
    // Should sites have a space for a name?
} Site;

void initialize();
stdarr returnOverlays(int siteID);
Agent returnAgent(int siteID);
void reduceResources(int overlayPort, int amount);
void increaseResources(int overlayPort, int amount);
Agent* getAgents(int bypass);

#endif // MANAGER_TOPOLOGY_MANAGER_H
