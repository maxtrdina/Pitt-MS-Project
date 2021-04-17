//
// Created by izzy on 11/11/20.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "constants.h"
#include "topology_manager.h"
#include <stdutil/stdarr.h>



Agent* agentList;
stdarr nodeArr;
stdarr agentArr;

    /*
    // Goal:    Establish a relationship between sites and agents where a site can have many agents.
    //          Must be able to retrive agent contents quickly.
    //
    // Methods: getAgent(site) -    When given a site, we must return an agent from the site that has 
    //                              available capacity
    */

void initialize() {

    // OVERLAY NODE READ IN

    stdarr_construct(&nodeArr, sizeof(Node), 0);
    printf("Node array Size: %d\n", stdarr_size(&nodeArr));

    int nodeCount; // At this point, agents refer to individual instances of running overlay nodes at sites
    FILE* fPointer = fopen("node_list.txt", "r");

    printf("Reading node List\n");

    if(!fPointer) {
        printf("Failed to open node List\n");
    }

    fscanf(fPointer, "%d\n", &nodeCount);
    printf("The number of nodes is: %d\n", nodeCount);

    //stdarr_resize(&agentArr, agentCount);
    //agentList = (Agent*)malloc(sizeof(Agent)*agentCount);
    Node* temp;

    for (int i = 0; i < nodeCount; i++) {
        temp = malloc(sizeof(Node));
        //printf("Successful Malloc\n");
        fscanf(fPointer, "%15s %d %d %d\n", &temp->location.address, &temp->location.port, &temp->siteID, &temp->throughput);
        //printf("Successful read of data\n");
        printf("%s:%d, located at site %d with a throughput of %d\n", temp->location.address, temp->location.port, temp->siteID, temp->throughput);
        stdarr_push_back(&nodeArr, temp);
        //printf("Successful Insert\n");
    }

    fclose(fPointer);

    // AGENT READ IN

    stdarr_construct(&agentArr, sizeof(Agent), 0);
    printf("Agent array Size: %d\n", stdarr_size(&agentArr));

    int agentCount; // At this point, agents refer to individual instances of running overlay nodes at sites
    fPointer = fopen("site_list.txt", "r");

    printf("Reading agent List\n");

    if(!fPointer) {
        printf("Failed to open agent List\n");
    }

    fscanf(fPointer, "%d\n", &agentCount);
    printf("The number of nodes is: %d\n", agentCount);

    //stdarr_resize(&agentArr, agentCount);
    //agentList = (Agent*)malloc(sizeof(Agent)*agentCount);
    Agent* tempAgent;

    for (int i = 0; i < agentCount; i++) {
        tempAgent = malloc(sizeof(Agent));
        //printf("Successful Malloc\n");
        fscanf(fPointer, "%15s %d %d %d\n", &tempAgent->location.address, &tempAgent->location.port, &tempAgent->siteID);
        //printf("Successful read of data\n");
        printf("%s:%d, located at site %d with a throughput of %d\n", tempAgent->location.address, tempAgent->location.port, tempAgent->siteID);
        stdarr_push_back(&agentArr, tempAgent);
        //printf("Successful Insert\n");
    }

    fclose(fPointer);


}

stdarr returnOverlays(int siteID) {

    int i = 0;
    int size = stdarr_size(&nodeArr);
    stdarr returnArr;
    stdarr_construct(&returnArr, sizeof(Node), 0);

    while (i < size) {
        stdit* temp = (stdarr_get(&nodeArr, NULL, i));
        Node* tempNode = (Node*)stdit_val(temp);
        if ( tempNode->siteID == siteID ) {
            stdarr_insert(&returnArr, NULL, tempNode);
        }

    return returnArr;

    }

}

Agent returnAgent(int siteID) {
    for (int i = 0; i < stdarr_size(&agentArr); i++) {
        stdit* temp = (stdarr_get(&agentArr, NULL, i));
        Agent* tempAgent = (Agent*)stdit_val(temp);

        if (tempAgent->siteID == siteID) {
            return *tempAgent;
        }
    }
}



Agent* getAgents(int bypass) {
    Agent* agents = (Agent*)malloc(sizeof(Agent));
    if (bypass) {
        strcpy(agents->location.address, "10.0.3.3");
    } else {
        // TODO this cannot be `localhost`
        strcpy(agents->location.address, "127.0.0.1\0");
        //return agentList;
    }
    agents->location.port = AGENT_CONTROL_PORT;
    return agents;
}