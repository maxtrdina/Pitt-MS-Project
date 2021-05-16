//
// Created by izzy on 11/11/20.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "network_manager.h"
#include "topology_manager.h"
#include "common.h"
#include "map.h"
#include "net_util.h"
#include <stdutil/stdhash.h>
#include <stdutil/stdarr.h>


int flow_ct;
Map* map;
int port = 8108;
stdhash portHash;

int fileExists(char *filename);

void nm_initialize() {
    flow_ct = 0;
    map = map_create();
    initialize();

    
    stdhash_construct(&portHash, sizeof(int), 0, NULL, NULL, 0);
}

int create_flow(RegisterFlow flow) {
    int flowId = flow_ct++; // Sets current FlowID

    printf("The requested resources: %d\n", flow.resources);

    //int agentCount = 1;
    //Agent* agents = getAgents(flow.bypass); // Returns a list of agents. Currently returns 127.0.0.1:5679
    //Agent target = *agents;                 // Will this work when there's more than one agent?
    // I'm assuming this is the point where selecting the correct agent will need to occur

    stdarr entrySites = returnOverlays(flow.entryOverlaySite); // We are actually returning instanes of overlay nodes
    stdarr exitSites = returnOverlays(flow.exitOverlaySite);

    // Outbound Agent Selection

    Node entrySite;
    Node exitSite;
    printf("Getting Agents\n");
    Agent outboundAgent = returnAgent(flow.outboundAgent);
    printf("outbound agent: %s\n", outboundAgent.location.address);
    Agent inboundAgent = returnAgent(flow.inboundAgent);
    printf("inbound agent: %s\n", inboundAgent.location.address);
    
    printf("Selecting Nodes at each site\n");
    int found = 0;
    for (int i = 0; i < stdarr_size(&entrySites); i++) {

        printf("currently considering outbound node\n");
        stdit temp = *stdarr_get(&entrySites, &temp, i);
        Node* potentialNode = (Node*)stdit_val(&temp);
        if (potentialNode->throughput >= flow.resources) {
            entrySite = *potentialNode;
            
            for (int j = 0; j < stdarr_size(&exitSites); j++) {

                printf("currently considering inbound node\n");
                stdit temp2 = *stdarr_get(&exitSites, &temp2, j);
                Node* potentialNode2 = (Node*)stdit_val(&temp2);
                if (potentialNode2->throughput >= flow.resources && potentialNode2->location.port == entrySite.location.port) {
                    exitSite = *potentialNode2;
                    found = 1;
                    break;
                }
            }

            if(found) {
                break;
            }

        }
    }

    if (!found) {
        printf("No suitable pair of overlay nodes could be found.\n");
        flow_ct--;
        return -1;
    }

    printf("Entry Overlay: %s, port %d\n", entrySite.location.address, entrySite.location.port);
    printf("Exit Overlay: %s, port %d\n", exitSite.location.address, exitSite.location.port);
    // Inbound Agent Selection

    //stdarr inboundSite = returnAgents(flow.inboundSite);

    //Agent inboundAgent;
    /*found = 0;
    for (int i = 0; i < stdarr_size(&inboundSite); i++) {

        Agent* potentialAgent = stdarr_get(&inboundSite, NULL, i);
        if (potentialAgent->throughput > 0 && potentialAgent->location.port == outboundAgent.location.port) {
            inboundAgent = *potentialAgent;
            found = 1;
            break;
        }
    }

    if (!found) {
        printf("No suitable agent at the inbound site was available.\n");
        flow_ct--;
        return -1;
    }
    */

    //Agent outboundAgent;
    //Agent inboundAgent;



    // Prep parameters
    printf("Creating flow, %s\n", flow.dst.address);
    Flow* newFlow = (Flow*)malloc(sizeof(Flow));        // Creates a new flow struct
    strcpy(newFlow->switchAddr, flow.switchAddr);       // Sets flow Switch Addr to that of passed flow info
    newFlow->originalDst = flow.dst;                    // Sets flow original destination to be that of passed flow info. This is defined by the client
    newFlow->outDst = outboundAgent.location;                  // Sets new flow destination to be that of the agent best suited for delivery
    printf("Flow populated\n");
    printf("Sending data to outbound agent: IP: %15s, Port: %d\n", newFlow->outDst.address, newFlow->outDst.port);

    int newPort = requestPort();
    if (newPort == -1) {
        flow_ct--;
        return -1;
    }

    // Outbound
    Message message = { .type = TYPE_ADD_FLOW_ROUTING_OUTBOUND, .flowRouting = { // prep message to the agent to create a new outbound flow, passing the client specified target
            .flowId = flowId, .target = newFlow->originalDst, .bypass = flow.bypass, .flowPort = newPort, .resources = flow.resources, 
            .overlay = exitSite.location
    } };

    printf("There are currently %d registered flows\n", (int) stdhash_size(&portHash));

    printf("Outbound message prepped\n");
    Message* response = send_message_r(message, newFlow->outDst.address, newFlow->outDst.port, 1); // Send message to agent. Create new flow and a port on dest. agent
    // The port the agent opened for this flow
    int spinesPort = response->ack.data;                        // Document the port Spines created
    if (spinesPort > 0) {
        printf("Agent opened port #%d\n", spinesPort);
    } else {
        printf("Couldn't open port\n");
        free(response);
        flow_ct--;
        return -1;
    }
    free(response);

    // Inbound
    message = (Message){ .type = TYPE_ADD_FLOW_ROUTING_INBOUND, .flowRouting = { // prep message to the agent to create a new inbound flow, passes target agent address and 
            .flowId = flowId, .target = { .address = exitSite.location.address, .port = newPort }, .bypass = flow.bypass, .flowPort = newPort, .resources = flow.resources,
            .overlay = entrySite.location
    } };
    strcpy(message.flowRouting.target.address, exitSite.location.address);
    newFlow->inDst = inboundAgent.location;
    printf("Inbound message prepped\n");
    printf("Sending data to inbound agent: IP: %15s, Port: %d\n", newFlow->inDst.address, newFlow->inDst.port);
    response = send_message_r(message, newFlow->inDst.address, newFlow->inDst.port, 1);
    printf("Response port from inbound flow creation: %d\n", response->ack.data);
    // The port the agent opened for this flow
    int inboundPort = response->ack.data;
    if (inboundPort > 0){
        newFlow->inDst.port = inboundPort;
        printf("Agent opened port #%d\n", newFlow->inDst.port);
    } else {
        printf("Couldn't open port.\n");
        flow_ct--;
        return -1;
    }

    if (flow.bypass) {
        // Write the file the controller will read
        // 1 million possible requests, increase if needed
        char filename[21 + 6];
        sprintf(filename, "ext_controller/reqs/r%d", flowId);
        FILE *fptr = fopen(filename, "w");
        fprintf(fptr, "%s", flow.switchAddr);
        fprintf(fptr, "\n%s", flow.myIp);
        fprintf(fptr, "\n%s", flow.myMac);
        fprintf(fptr, "\n%s:%d-%s:%d", flow.dst.address, flow.dst.port, newFlow->outDst.address, newFlow->outDst.port);
        fclose(fptr);

        char ackFilename[23 + 6];
        sprintf(ackFilename, "ext_controller/reqs/ack%d", flowId);
        while (!fileExists(ackFilename)) {
            usleep(50 * 1000);
        }
        remove(ackFilename);
    }

    map_insert(map, flowId, newFlow);
    reduceResources(entrySite.location.port, flow.resources);

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
                    flow.outDst.address, flow.outDst.port
            );
        }
    }
}

int fileExists(char *filename) {
    return access(filename, F_OK) != -1;
}

int requestPort() {

    int returnVal = port;

    if (port < 4000 || port > 65536) { // In the event we have some kind of overflow/wrapping of port numbers
        return -1; 
    }

    void* portNum = malloc(sizeof(int));
    memcpy(portNum, &port, sizeof(port));

    stdhash_insert(&portHash, NULL, portNum, NULL);
    port++;
    return returnVal;
}
