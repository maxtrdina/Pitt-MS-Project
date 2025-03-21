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


    stdarr_construct(&agentArr, sizeof(Agent), 0);
    stdarr_construct(&nodeArr, sizeof(Node), 0);
    // OVERLAY NODE READ IN

    //stdarr_construct(&nodeArr, sizeof(Node), 0);
    //printf("Node array Size: %d\n", stdarr_size(&nodeArr));

    int nodeCount; // At this point, agents refer to individual instances of running overlay nodes at sites
    FILE* fPointer = fopen("overlay_list.txt", "r");

    printf("Reading Overlay List\n");

    if(!fPointer) {
        printf("Failed to open Overlay List\n");
    }

    fscanf(fPointer, "%d\n", &nodeCount);
    printf("The number of Overlay sites is: %d\n", nodeCount);

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

    //stdarr_construct(&agentArr, sizeof(Agent), 0);
    //printf("Agent array Size: %d\n", stdarr_size(&agentArr));

    int agentCount; // At this point, agents refer to individual instances of running overlay nodes at sites
    fPointer = fopen("agent_list.txt", "r");

    printf("Reading Agent List\n");

    if(!fPointer) {
        printf("Failed to open Agent List\n");
    }

    fscanf(fPointer, "%d\n", &agentCount);
    printf("The number of Agents is: %d\n", agentCount);

    //stdarr_resize(&agentArr, agentCount);
    //agentList = (Agent*)malloc(sizeof(Agent)*agentCount);
    Agent* tempAgent;

    for (int i = 0; i < agentCount; i++) {
        tempAgent = malloc(sizeof(Agent));
        //printf("Successful Malloc\n");
        fscanf(fPointer, "%15s %d %d\n", &tempAgent->location.address, &tempAgent->location.port, &tempAgent->siteID);
        //printf("Successful read of data\n");
        printf("%s:%d, located at site %d\n", tempAgent->location.address, tempAgent->location.port, tempAgent->siteID);
        stdarr_push_back(&agentArr, tempAgent);
        //printf("Successful Insert\n");
    }

    fclose(fPointer);


}

stdarr returnOverlays(int siteID) {

    int i = 0;
    int size = stdarr_size(&nodeArr);

    printf("Size of node array is %d.\n", size);

    stdarr returnArr;
    stdarr_construct(&returnArr, sizeof(Node), 0);

    //printf("return array initialized\n");

    for (int i = 0; i < size; i++) {
	printf("currently viewing node %d\n", i);
        stdit temp = *(stdarr_get(&nodeArr, &temp, i));
	//stdit* temp;
	//stdit_ptr(temp, NULL, sizeof(Node));
	//stdarr_get(&nodeArr, temp, i);
	//printf("successfully created a stdit value for node %d\n", i);
        Node* tempNode = (Node*)stdit_val(&temp);
	//printf("successfully created a temporary node for node %d\n", i);
	printf("node %d has a site id of %d and a port number of %d\n", i, tempNode->siteID, tempNode->location.port);
        if ( tempNode->siteID == siteID ) {
            stdarr_push_back(&returnArr, tempNode);
	    }


    }

return returnArr;

}

void reduceResources(int overlayPort, int amount) {

    int i = 0;
    int size = stdarr_size(&nodeArr);

    while (i < size) {
        stdit temp = *(stdarr_get(&nodeArr, &temp, i));
        Node* tempNode = (Node*)stdit_val(&temp);
        if (tempNode->location.port == overlayPort) {
            tempNode->throughput = tempNode->throughput - amount;
            }
        i++;
    }
}

void increaseResources(int overlayPort, int amount) {

    int i = 0;
    int size = stdarr_size(&nodeArr);

    while (i < size) {
        stdit temp = *(stdarr_get(&nodeArr, &temp, i));
        Node* tempNode = (Node*)stdit_val(&temp);
        if (tempNode->location.port == overlayPort) {
            tempNode->throughput = tempNode->throughput + amount;
            }
        i++;
    }
}

Agent returnAgent(int siteID) {
    for (int i = 0; i < stdarr_size(&agentArr); i++) {
        stdit temp = *(stdarr_get(&agentArr, &temp, i));
        Agent* tempAgent = (Agent*)stdit_val(&temp);

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
