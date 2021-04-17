//
// Created by izzy on 11/8/20.
//

#ifndef AGENT_PROTOCOL_H
#define AGENT_PROTOCOL_H

#include "constants.h"
#include "common.h"

// Generic
#define TYPE_ACK 1

// Network Manager
#define TYPE_REGISTER_FLOW 2
#define TYPE_DELETE_FLOW 3

// Agent's Control Plane
#define TYPE_ADD_FLOW_ROUTING_INBOUND 4
#define TYPE_ADD_FLOW_ROUTING_OUTBOUND 5
#define TYPE_REMOVE_FLOW_ROUTING 6
#define TYPE_TERMINATE 7


// Debug
#define TYPE_PING 97
#define TYPE_PONG 98
#define TYPE_PRINT_STATE 99

typedef struct Ack_s {
    int data;
} Ack;

typedef struct FlowRouting_s {
    int flowId;
    // Outbound: Flow destination. ie. when the packets come out of spines, where do we send it?
    // Inbound: Spines destination. ie. where does the packet go when it comes out of the spines network?
    Location target;
    Location node;
    int bypass;
    int spinesPort;
    int resources;
} FlowRouting;

typedef struct RemoveFlowRouting_s {
    int flowId;
} RemoveFlowRouting;

typedef struct RegisterFlow_s {
    char switchAddr[16]; // 127.0.0.1:5005X
    char myIp[16];
    char myMac[18];
    Location dst;
    int bypass;
    int resources;
    int inboundSite;
    int outboundSite;
} RegisterFlow;

typedef struct DeleteFlow_s {
    int flowId;
} DeleteFlow;

typedef struct Message_s {
    int type;
    union {
        Ack ack;
        FlowRouting flowRouting;
        RemoveFlowRouting removeFlowRouting;
        RegisterFlow registerFlow;
        DeleteFlow deleteFlow;
    };
} Message;

#endif //AGENT_PROTOCOL_H
