//
// Created by izzy on 11/18/20.
//

#ifndef AGENT_DATA_H
#define AGENT_DATA_H

#include "common.h"

void set_hostname(char* hostname);
int create_interface(int inbound, Location target, Location node, int bypass, int spinesPort);

#endif //AGENT_DATA_H
