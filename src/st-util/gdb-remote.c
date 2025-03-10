/*
 * Copyright (c) 2011 Peter Zotov <whitequark@whitequark.org>
 * Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#if defined(_WIN32)
#include <win32_socket.h>
#else
#include <unistd.h>
#include <poll.h>
#endif

#include "gdb-remote.h"

static const char hex[] = "0123456789abcdef";

int32_t gdb_send_packet(int32_t fd, char* data) {
    uint32_t data_length = (uint32_t) strlen(data);
    int32_t length = data_length + 4;
    char* packet = malloc(length); // '$' data (hex) '#' cksum (hex)

    memset(packet, 0, length);

    packet[0] = '$';

    uint8_t cksum = 0;

    for(uint32_t i = 0; i < data_length; i++) {
        packet[i + 1] = data[i];
        cksum += data[i];
    }

    packet[length - 3] = '#';
    packet[length - 2] = hex[cksum >> 4];
    packet[length - 1] = hex[cksum & 0xf];

    while (1) {
        if(write(fd, packet, length) != length) {
            free(packet);
            return (-2);
        }

        char ack;

        if(read(fd, &ack, 1) != 1) {
            free(packet);
            return (-2);
        }

        if(ack == '+') {
            free(packet);
            return (0);
        }
    }
}

#define ALLOC_STEP 1024

int32_t gdb_recv_packet(int32_t fd, char** buffer) {
    uint32_t packet_size = ALLOC_STEP + 1, packet_idx = 0;
    uint8_t cksum = 0;
    char recv_cksum[3] = {0};
    char* packet_buffer = malloc(packet_size);
    uint32_t state;

    if(packet_buffer == NULL) {
        return (-2);
    }

start:
    state = 0;
    packet_idx = 0;
    /*
     * 0: waiting $
     * 1: data, waiting #
     * 2: cksum 1
     * 3: cksum 2
     * 4: fin
     */

    char c;

    while (state != 4) {
        if(read(fd, &c, 1) != 1) {
            free(packet_buffer);
            return (-2);
        }

        switch (state) {
        case 0:

            if(c != '$') { /* ignore */
            } else {
                state = 1;
            }

            break;

        case 1:

            if(c == '#') {
                state = 2;
            } else {
                packet_buffer[packet_idx++] = c;
                cksum += c;

                if(packet_idx == packet_size) {
                    packet_size += ALLOC_STEP;
                    void* p = realloc(packet_buffer, packet_size);

                    if(p != NULL) {
                        packet_buffer = p;
                    } else {
                        free(packet_buffer);
                        return (-2);
                    }
                }
            }

            break;

        case 2:
            recv_cksum[0] = c;
            state = 3;
            break;

        case 3:
            recv_cksum[1] = c;
            state = 4;
            break;
        }
    }

    uint8_t recv_cksum_int = strtoul(recv_cksum, NULL, 16);

    if(recv_cksum_int != cksum) {
        char nack = '-';

        if(write(fd, &nack, 1) != 1) {
            free(packet_buffer);
            return (-2);
        }

        goto start;
    } else {
        char ack = '+';

        if(write(fd, &ack, 1) != 1) {
            free(packet_buffer);
            return (-2);
        }
    }

    packet_buffer[packet_idx] = 0;
    *buffer = packet_buffer;

    return (packet_idx);
}

/*
 * Here we skip any characters which are not \x03, GDB interrupt.
 * As we use the mode with ACK, in a (very unlikely) situation of a packet lost
 * because of this skipping, it will be resent anyway.
 */
int32_t gdb_check_for_interrupt(int32_t fd) {
    struct pollfd pfd;
    pfd.fd = fd;
    pfd.events = POLLIN;

    if(poll(&pfd, 1, 0) != 0) {
        char c;

        if(read(fd, &c, 1) != 1) {
            return (-2);
        }

        if(c == '\x03') {
            return (1); // ^C
        }
    }

    return (0);
}
