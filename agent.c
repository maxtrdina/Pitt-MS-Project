#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "constants.h"
#include "agent_control.h"
#include "agent_data.h"
#include "map.h"

#define MAX_PKT_SIZE 1472
#define IN_PORT      5678

int main(int argc, char *argv[]) {
    /*int tcp_in_sock, udp_in_sock;
    int bytes;
    char buf[MAX_PKT_SIZE];
    fd_set mask, mask_template, dummy_mask;
    struct sockaddr_in tcp_in_sockaddr;*/

    if (argc < 2) {
        printf("Please, pass the local address as a parameter:\n");
        printf("\t./agent 127.0.0.1\n");
        return 0;
    }
    set_hostname(argv[1]);
    run_control();

    /*printf("Done with control");
    
    tcp_in_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (tcp_in_sock < 0) {
        printf("Problem opening TCP socket.\n");
        return -1;
    }
    
    tcp_in_sockaddr.sin_family = AF_INET;
    tcp_in_sockaddr.sin_addr.s_addr = INADDR_ANY;
    tcp_in_sockaddr.sin_port = htons(IN_PORT);
    
    if (bind(tcp_in_sock, (struct sockaddr *)&tcp_in_sockaddr, sizeof(tcp_in_sockaddr)) < 0) {
        printf("Problem binding TCP socket.\n");
        return -1;
    }
    
    FD_ZERO(&mask_template);
    FD_ZERO(&mask);
    FD_SET(tcp_in_sock, &mask_template);

    Map* map = map_create();
    Message* message;
    
    for (;;) {
        mask = mask_template;
        
        select(FD_SETSIZE, &mask, &dummy_mask, &dummy_mask, NULL);
        
        bytes = recv(tcp_in_sock, buf, sizeof(buf), 0);
        message = (Message*)&bytes;


        
        buf[MAX_PKT_SIZE-1] = '\0';
        printf("Message: %s\n", buf);
    }*/
}
