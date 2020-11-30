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

int done = 0;
// Start data port
int currentPort = 11567;

void *createInbound_thread(void* target);
int createInbound(int port);

void *createOutbound_thread(void *target);
int createOutbound(int port, Location target);

void *createSrcIn_entry(void* arg);
int createSrcIn(int port);

int getNextPort() {
    return currentPort++;
}

int create_interface(int inbound, Location target) {
    pthread_t thread_handle;
    done = 0;

    printf("Creating interface...\n");
    if (inbound) {
        pthread_create(&thread_handle, NULL, createInbound_thread, (void*)&target);
    } else {
        pthread_create(&thread_handle, NULL, createOutbound_thread, (void*)&target);
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
    Location *location = (Location*)target;
    while (!createInbound(getNextPort())) { }
//    createInbound(2);
//    return NULL;
}

int createInbound(int port) {
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
    host_ptr = gethostbyname("localhost");
    memcpy(&spines_daemon_addr.sin_addr, host_ptr->h_addr, sizeof(struct in_addr));
    daemon_ptr = (struct sockaddr *)&spines_daemon_addr;

    memcpy(&h_ent, gethostbyname("localhost"), sizeof(h_ent));
    memcpy(&spines_addr.sin_addr, h_ent.h_addr, sizeof(spines_addr.sin_addr));

    spines_addr.sin_family = AF_INET;
    spines_addr.sin_port = htons(8108);

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
    if (bind(inbound_sk, (struct sockaddr *)&inbound_addr, sizeof(inbound_addr)) < 0 ) {
        printf("Bind for recv socket failed\n");
        return 0;
    } else {
        printf("Bind for recv sock complete\n");
    }

    FD_ZERO(&mask);
    FD_ZERO(&dummy_mask);
    FD_SET(inbound_sk, &mask);

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
    Location *location = (Location*)target;
    while (!createOutbound(getNextPort(), *location)) { }
}

int createOutbound(int port, Location target) {
    int out_socket, spines_sk;
    int ret, bytes;
    char buffer[MAX_PKT_SIZE];
    struct sockaddr_in spines_daemon_addr, spines_addr;
    struct sockaddr_in out_addr;
    struct hostent  *host_ptr;
    struct sockaddr *daemon_ptr = NULL;
    fd_set  mask, dummy_mask, temp_mask;

    struct timeval *timeout_ptr;

    spines_daemon_addr.sin_family = AF_INET;
    spines_daemon_addr.sin_port = htons(8100); // Default Spines port
    host_ptr = gethostbyname("localhost" /* 10.0.3.3 */); // TODO I'm only going to be running spines in that node
    memcpy(&spines_daemon_addr.sin_addr, host_ptr->h_addr, sizeof(struct in_addr));
    daemon_ptr = (struct sockaddr *)&spines_daemon_addr;

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
    spines_addr.sin_port = htons(port);

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
