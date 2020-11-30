//
// Created by izzy on 11/23/20.
//

#include "agent_data.h"

int createInbound(int port, Location target);

int main(int argc, char *argv[]) {
    set_hostname("localhost\0");
    Location location = {.address = "127.0.0.1\0", .port = 8108};
    createInbound(11678, location);
    return 0;
}
