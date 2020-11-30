//
// Created by izzy on 11/23/20.
//

#include "agent_data.h"

int createOutbound(int port, Location target);

int main(int argc, char *argv[]) {

    Location location = {.address = "127.0.0.1\0", .port = 11999};
    createOutbound(8108, location);
    return 0;
}
