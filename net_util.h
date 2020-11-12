//
// Created by izzy on 11/12/20.
//

#ifndef COMMON_NET_UTIL_H
#define COMMON_NET_UTIL_H

#include "protocol.h"

// Sends a message expecting no response
void send_message(Message message, char* dstAddr, int dstPort);

// Sends a message, response is optional
Message* send_message_r(Message message, char* dstAddr, int dstPort, int expectResponse);

#endif // COMMON_NET_UTIL_H
