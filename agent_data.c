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
int createOutbound(int port);

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
        pthread_create(&thread_handle, NULL, createInbound_thread, (void *) &target);
    } else {
        pthread_create(&thread_handle, NULL, createOutbound_thread, (void *) &target);
    }

    while (!done) {
        // TODO barrier?
        usleep(500*1000);
    }
    printf("Thread listening on port %d\n", done);

    return done;
}

void *createInbound_thread(void* target) {
    createInbound(2);
    return NULL;
}

int createInbound(int port) {
    int recv_sk, spines_sk;
    int ret, bytes;
    char buffer[MAX_PKT_SIZE];
    struct sockaddr_in host, serv_addr, name;
    struct hostent h_ent;
    struct hostent *host_ptr;
    struct sockaddr *daemon_ptr = NULL;
    fd_set mask, dummy_mask, temp_mask;

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8100);
    host_ptr = gethostbyname("127.0.0.1");
    memcpy(&serv_addr.sin_addr, host_ptr->h_addr, sizeof(struct in_addr));
    daemon_ptr = (struct sockaddr *)&serv_addr;

    memcpy(&h_ent, gethostbyname("127.0.0.1"), sizeof(h_ent));
    memcpy(&host.sin_addr, h_ent.h_addr, sizeof(host.sin_addr));

    host.sin_family = AF_INET;
    host.sin_port   = htons(8108);

    spines_sk = spines_socket(PF_SPINES, SOCK_DGRAM, 0, daemon_ptr);
    if (spines_sk < 0) {
        printf("Client socket error, can't open spines_sk\n");
        return 0;
    }

    // Traffic from the sender
    recv_sk = socket(AF_INET, SOCK_DGRAM, 0);
    if (recv_sk < 0) {
        printf("Couldn't open socket to get data from the sender\n");
        return 0;
    }

    name.sin_family = AF_INET;
    name.sin_addr.s_addr = INADDR_ANY;
    name.sin_port = htons(8109);

    if ( bind( recv_sk, (struct sockaddr *)&name, sizeof(name) ) < 0 ) {
        printf("Bind for recv socket failed\n");
        return 0;
    }

    FD_ZERO( &mask );
    FD_ZERO( &dummy_mask );
    FD_SET( recv_sk, &mask );

    for(;;) {
        temp_mask = mask;
        select( FD_SETSIZE, &temp_mask, &dummy_mask, &dummy_mask, NULL);

        bytes = recv( recv_sk, buffer, sizeof(buffer), 0);
        if (bytes < 0) {
            break;
        }

        /* Send over spines */
        spines_sendto(spines_sk, buffer, bytes, 0, (struct sockaddr *)&host, sizeof(struct sockaddr));
    }

    close(recv_sk);
    spines_close(spines_sk);
}

void *createOutbound_thread(void *target) {
    while (!createOutbound(getNextPort())) { }
}

int createOutbound(int port) {
    int out_socket, spines_sk;
    int ret, bytes;
    char buffer[MAX_PKT_SIZE];
    struct sockaddr_in serv_addr, name;
    struct sockaddr_in out_addr;
    struct hostent  *host_ptr;
    struct sockaddr *daemon_ptr = NULL;
    fd_set  mask, dummy_mask, temp_mask;

    struct timeval *timeout_ptr;

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8100); // Default Spines port
    host_ptr = gethostbyname("10.0.3.3"); // I'm only going to be running spines in that node
    memcpy(&serv_addr.sin_addr, host_ptr->h_addr, sizeof(struct in_addr));
    daemon_ptr = (struct sockaddr *)&serv_addr;

    spines_sk = spines_socket(PF_INET, SOCK_DGRAM, 0, daemon_ptr);
    if (spines_sk <= 0) {
        printf("Couldn't create spines socket...");
        return 0;
    }

    name.sin_family = AF_INET;
    name.sin_addr.s_addr = INADDR_ANY;
    name.sin_port = htons(port);

    if (spines_bind(spines_sk, (struct sockaddr *)&name, sizeof(name)) < 0) {
        printf("spines_bind error\n");
        return 0;
    }

    /*out_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (out_socket < 0) {
        printf("couldn't open outbound socket");
        return 0;
    }*/

    out_addr.sin_family = AF_INET;
    out_addr.sin_addr.s_addr = inet_addr("" /* NEED */);
    out_addr.sin_port = htons(55 /* NEED */);

    FD_ZERO(&mask);
    FD_ZERO(&dummy_mask);
    FD_SET(spines_sk, &mask);

    for(;;) {
        temp_mask = mask;

        ret = select(FD_SETSIZE, &temp_mask, &dummy_mask, &dummy_mask, NULL);
        if (ret > 0) {
            bytes = spines_recvfrom(spines_sk, buffer, sizeof(buffer), 0, NULL, 0);
            if (bytes <= 0) {
                printf("Disconnected by spines...\n");
                break;
            } else {
                printf("Got some data from spines\n");
                printf("%s\n", buffer);
            }

            //send(out_socket, buffer, sizeof(buffer), 0);
        }
    }

    spines_close(spines_sk);
    close(out_socket);

    return 1;
}

void *createSrcIn_entry(void* arg) {
    while (!createSrcIn(++currentPort)) { }
    // createSrcIn(++currentPort);
}

int createSrcIn(int port) {
    int server_fd, client_fd;
    char buf[MAX_PKT_SIZE];
    //fd_set mask, mask_template, dummy_mask;
    Message* message;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        printf("Problem opening TCP socket.\n");
        return 0;
    } else {
        printf("Socket open\n");
    }

    struct sockaddr_in server_sockaddr, client_sockaddr;
    server_sockaddr.sin_family = AF_INET;
    server_sockaddr.sin_addr.s_addr = INADDR_ANY;
    server_sockaddr.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&server_sockaddr, sizeof(server_sockaddr)) < 0) {
        printf("Problem binding TCP socket\n");
        return 0;
    } else {
        printf("Socket bound\n");
    }

//    FD_ZERO(&mask_template);
//    FD_ZERO(&mask);
//    FD_SET(tcp_in_fd, &mask_template);

    if (listen(server_fd, 4) < 0){
        printf("Error while listening\n");
        return 0;
    } else {
        printf("Listening to socket\n");
    }

    socklen_t client_sockaddr_size = sizeof(struct sockaddr_in);
    int terminate = 0;
    while (!terminate) {
        // Triggers create_interface to move on
        done = port;

        client_fd = accept(server_fd, (struct sockaddr *)&client_sockaddr, &client_sockaddr_size);

        recv(client_fd, buf, sizeof(Message), 0);
        fprintf(stderr, "Value of errno: %d\n", errno);
        message = (Message*)buf;

        printf("Received type: %d\n", message->type);
        if (message->type == TYPE_PING) {
            printf("PING!\n");
            Message out = {.type = TYPE_PONG, .ack = { .data = 1234 } };
            send(client_fd, &out, sizeof(Message), 0);
        }else if (message->type == TYPE_TERMINATE) {
            terminate = 1;
        }

        close(client_fd);
    }

    close(server_fd);

    return 1;
}
