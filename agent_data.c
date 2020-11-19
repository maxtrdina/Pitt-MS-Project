//
// Created by izzy on 11/18/20.
//

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "agent_data.h"
#include "protocol.h"

int done = 0;
// Start data port
int currentPort = 11567;

void *createSrcIn_entry(void* arg);
int createSrcIn(int port);

int create_interface() {
    pthread_t thread_handle;
    done = 0;

    printf("Creating interface...\n");
    pthread_create(&thread_handle, NULL, createSrcIn_entry, (void*)1);

    while (!done) {
        // TODO barrier?
        usleep(500*1000);
    }
    printf("Thread listening on port %d\n", done);

    return done;
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
