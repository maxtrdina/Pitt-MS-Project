//
// Created by izzy on 11/11/20.
//

#ifndef MANAGER_COMMON_H
#define MANAGER_COMMON_H

typedef struct Location_s {
    // 4 segments of 3, 3 dots, and the null terminator max
    char address[16];
    int port;
} Location;

#endif // MANAGER_COMMON_H
