//
// Created by izzy on 11/18/20.
//

#include <netdb.h>

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <memory.h>
#include "agent_data.h"
#include "protocol.h"
#include "spines/libspines/spines_lib.h"

#define MAX_PKT_SIZE 1472

typedef struct OutBoundData {
    Location target;
    Location node;
    int sPort;
};

int done = 0;
// Start data port
int currentPort = 11567;
//int spinesPort = 8108;

// To configure for testing
char hostname[16];

void *createInbound_thread(void* target);
int createInbound(int port, Location target, Location node);

void *createOutbound_thread(void *target);
int createOutbound(Location target, Location node, int sPort);

void *createBypass_thread(void* target);
int createBypass(int port, Location target);

int getNextPort() {
    return currentPort++;
}

void set_hostname(char *new_hostname) {
    strcpy(hostname, new_hostname);
}

int create_interface(int inbound, Location target, Location node, int bypass, int spinesPort) {
    pthread_t thread_handle;
    done = 0;

    printf("Creating interface...\n");
    if (bypass) {
        pthread_create(&thread_handle, NULL, createBypass_thread, (void*)&target);
    } else if (inbound) {
        struct OutBoundData target_spines;
        strcpy(target_spines.target.address, target.address);
        target_spines.target.port = target.port;
        target_spines.node = node;
        pthread_create(&thread_handle, NULL, createInbound_thread, (void*)&target_spines);
    } else {
        struct OutBoundData target_spines;
        strcpy(target_spines.target.address, target.address);
        target_spines.target.port = target.port;
        target_spines.sPort = spinesPort;
        target_spines.node = node;
        pthread_create(&thread_handle, NULL, createOutbound_thread, (void*)&target_spines); // Creates a socket at the destination agent for sending data to.
    }

    // This is okay because an agent control plane is single threaded
    while (!done) {
        // TODO barrier?
        usleep(500*1000);
    }
    printf("Thread listening on port %d\n", done);

    return done;
}

void *createInbound_thread(void* target) {
    struct OutBoundData *target_spines = (struct OutBoundData*)target;
    Location location = target_spines->target;
    Location node = target_spines->node;
    
    while (!createInbound(getNextPort(), location, node)) { }
//    createInbound(2);
//    return NULL;
}

int createInbound(int port, Location target, Location node) {
    int inbound_sk, spines_sk;
    int ret, bytes;
    char buffer[MAX_PKT_SIZE];
    struct sockaddr_in spines_addr, spines_daemon_addr, inbound_addr;
    struct hostent h_ent;
    struct hostent *host_ptr;
    struct sockaddr *daemon_ptr = NULL;
    fd_set mask, dummy_mask, temp_mask;

    spines_daemon_addr.sin_family = AF_INET;
    spines_daemon_addr.sin_port = htons(8100);
    host_ptr = gethostbyname(node.address);
    memcpy(&spines_daemon_addr.sin_addr, host_ptr->h_addr, sizeof(struct in_addr));
    //daemon_ptr = (struct sockaddr *)&spines_daemon_addr;    // does thsis have to be the port of the overlay node?
    daemon_ptr = (struct sockaddr *)&node.port;;

    memcpy(&h_ent, gethostbyname(target.address), sizeof(h_ent));
    memcpy(&spines_addr.sin_addr, h_ent.h_addr, sizeof(spines_addr.sin_addr));

    spines_addr.sin_family = AF_INET;
    spines_addr.sin_port = htons(target.port);

    if (spines_init(daemon_ptr) < 0) {
        printf("Couldn't spines_init\n");
    } else {
        printf("Spines init complete\n");
    }

    spines_sk = spines_socket(PF_SPINES, SOCK_DGRAM, 0, daemon_ptr);
    if (spines_sk < 0) {
        printf("Client socket error, can't open spines_sk\n");
        return 0;
    } else {
        printf("Spines socket complete\n");
    }

    // Traffic from the sender
    inbound_sk = socket(AF_INET, SOCK_DGRAM, 0);
    if (inbound_sk < 0) {
        printf("Couldn't open socket to get data from the sender\n");
        return 0;
    } else {
        printf("inbound socket open\n");
    }

    inbound_addr.sin_family = AF_INET;
    inbound_addr.sin_addr.s_addr = INADDR_ANY;
    inbound_addr.sin_port = htons(port);

    printf("Binding inbound...\n");
    if (bind(inbound_sk, (struct sockaddr *)&inbound_addr, sizeof(inbound_addr)) < 0) {
        printf("Bind for recv socket failed\n");
        return 0;
    } else {
        printf("Bind for recv sock complete\n");
    }

    FD_ZERO(&mask);
    FD_ZERO(&dummy_mask);
    FD_SET(inbound_sk, &mask);

    // Unblocks the wait. Need a better way to do this.
    done = port;

    for(;;) {
        temp_mask = mask;
        printf("Calling select...\n");
        select(FD_SETSIZE, &temp_mask, &dummy_mask, &dummy_mask, NULL);

        printf("Something exciting happened, receiving...\n");
        bytes = recv(inbound_sk, buffer, sizeof(buffer), 0);
        if (bytes < 0) {
            break;
        } else {
            printf("Got %d bytes.\n", bytes);
        }

        /* Send over spines */
        bytes = spines_sendto(spines_sk, buffer, bytes, 0, (struct sockaddr *)&spines_addr, sizeof(struct sockaddr));
        if (bytes < 0) {
            printf("Some spines_sendto error happened.\n");
        } else {
            printf("Data sent: %d.\n", bytes);
        }
    }

    close(inbound_sk);
    spines_close(spines_sk);
}

void *createOutbound_thread(void *target) {
    struct OutBoundData *target_spines = (struct OutBoundData*)target;
    Location location = target_spines->target;
    Location node = target_spines->node;
    printf("%16s: %d, spines port: %d\n", target_spines->target.address, target_spines->target.port, target_spines->sPort);
    createOutbound(location, node, target_spines->sPort);
}

int createOutbound(Location target, Location node, int sPort) {
    int out_socket, spines_sk;
    int ret, bytes;
    char buffer[MAX_PKT_SIZE];
    struct sockaddr_in spines_daemon_addr, spines_addr;
    struct sockaddr_in out_addr;
    struct hostent  *host_ptr;
    struct sockaddr *daemon_ptr = NULL;
    fd_set  mask, dummy_mask, temp_mask;
    //int sPort = spinesPort;
    //spinesPort++;

    struct timeval *timeout_ptr;

    spines_daemon_addr.sin_family = AF_INET;
    spines_daemon_addr.sin_port = htons(8100); // Default Spines port
    host_ptr = gethostbyname(node.address);
    memcpy(&spines_daemon_addr.sin_addr, host_ptr->h_addr, sizeof(struct in_addr));
    //daemon_ptr = (struct sockaddr *)&spines_daemon_addr;       // Should this be the port number indicating the overlay instance
    daemon_ptr = (struct sockaddr *)&node.port;;

    // HANDLE BAD PORT NUMBER

    if (sPort == -1) {
        printf("Can't initialize, bad port number\n");
        return 0;
    }

    if (spines_init(daemon_ptr) < 0) {
        printf("flooder_client: socket error\n");
        return 0;
    } else {
        printf("Spines init complete\n");
    }

    spines_sk = spines_socket(PF_INET, SOCK_DGRAM, 0, daemon_ptr);
    if (spines_sk <= 0) {
        printf("Couldn't create spines socket...\n");
        return 0;
    } else {
        printf("Spines socket created\n");
    }

    spines_addr.sin_family = AF_INET;
    spines_addr.sin_addr.s_addr = INADDR_ANY;
    spines_addr.sin_port = htons(sPort);

    if (spines_bind(spines_sk, (struct sockaddr *)&spines_addr, sizeof(spines_addr)) < 0) {
        printf("spines_bind error\n");
        return 0;
    } else {
        printf("Spines socket bound\n");
    }

    out_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (out_socket < 0) {
        printf("couldn't open outbound socket\n");
        return 0;
    } else {
        printf("Outbound socket created.\n");
    }

    out_addr.sin_family = AF_INET;
    out_addr.sin_addr.s_addr = inet_addr(target.address);
    out_addr.sin_port = htons(target.port);

    FD_ZERO(&mask);
    FD_ZERO(&dummy_mask);
    FD_SET(spines_sk, &mask);

    // Unblocks the wait. Need a better way to do this.
    done = sPort;

    for(;;) {
        temp_mask = mask;

        printf("Calling select...\n");
        ret = select(FD_SETSIZE, &temp_mask, &dummy_mask, &dummy_mask, NULL);
        if (ret > 0) {
            printf("Something exciting happened... receiving\n");
            bytes = spines_recvfrom(spines_sk, buffer, sizeof(buffer), 0, NULL, 0);
            if (bytes <= 0) {
                printf("Disconnected by spines...\n");
                break;
            } else {
                buffer[bytes] = '\0';
                printf("OUTBOUND | Got %d bytes data from spines\n", bytes);
                printf("%s\n", buffer);
            }

            sendto(out_socket, buffer, bytes, 0, (struct sockaddr *)&out_addr, sizeof(out_addr));
        }
    }

    spines_close(spines_sk);
    close(out_socket);

    return 1;
}

void *createBypass_thread(void* target) {
    Location *location = (Location*)target;
    while (!createBypass(getNextPort(), *location)) {}
}

int createBypass(int port, Location target) {
    int inbound_sk, outbound_sk;
    int ret, bytes;
    char buffer[MAX_PKT_SIZE];
    struct sockaddr_in inbound_addr, outbound_addr;
    fd_set mask, dummy_mask, temp_mask;

    outbound_sk = socket(AF_INET, SOCK_DGRAM, 0);
    if (outbound_sk < 0) {
        printf("couldn't open outbound socket\n");
        return 0;
    } else {
        printf("Outbound socket created.\n");
    }

    outbound_addr.sin_family = AF_INET;
    outbound_addr.sin_addr.s_addr = inet_addr(target.address);
    outbound_addr.sin_port = htons(target.port);

    // Traffic from the sender
    inbound_sk = socket(AF_INET, SOCK_DGRAM, 0);
    if (inbound_sk < 0) {
        printf("Couldn't open socket to get data from the sender\n");
        return 0;
    } else {
        printf("inbound socket open\n");
    }

    inbound_addr.sin_family = AF_INET;
    inbound_addr.sin_addr.s_addr = INADDR_ANY;
    inbound_addr.sin_port = htons(port);

    printf("Binding inbound...\n");
    if (bind(inbound_sk, (struct sockaddr *)&inbound_addr, sizeof(inbound_addr)) < 0) {
        printf("Bind for recv socket failed\n");
        return 0;
    } else {
        printf("Bind for recv sock complete\n");
    }

    FD_ZERO(&mask);
    FD_ZERO(&dummy_mask);
    FD_SET(inbound_sk, &mask);

    // Unblocks the wait. Need a better way to do this.
    done = port;

    for(;;) {
        temp_mask = mask;
        printf("Calling select...\n");
        select(FD_SETSIZE, &temp_mask, &dummy_mask, &dummy_mask, NULL);

        printf("Something exciting happened, receiving...\n");
        bytes = recv(inbound_sk, buffer, sizeof(buffer), 0);
        if (bytes < 0) {
            break;
        } else {
            printf("Got %d bytes.\n", bytes);
        }

        bytes = sendto(outbound_sk, buffer, bytes, 0, (struct sockaddr *)&outbound_addr, sizeof(outbound_addr));
        if (bytes < 0) {
            printf("Some spines_sendto error happened.\n");
        } else {
            printf("Data sent: %d.\n", bytes);
        }
    }

    close(inbound_sk);
    close(outbound_sk);
}
