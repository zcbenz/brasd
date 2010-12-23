#include "utils.h"
#include "bras.h"

#include <stdio.h>
#include <string.h>

#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <netdb.h>

int init_socket(const char *node, const char *service)
{
    struct addrinfo hints, *result;
    bzero(&hints, sizeof(struct addrinfo));
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags    = AI_PASSIVE;

    if(getaddrinfo(node, service, &hints, &result))
    {
        perror("getaddrinfo");
        return -1;
    }

    int sfd = 0;
    struct addrinfo *rp;
    for(rp = result; rp != NULL; rp = rp->ai_next)
    {
        sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if(sfd == -1)
            continue;

        if(connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1)
            break;          /* Success */

        close(sfd);
    }

    freeaddrinfo(result);

    if(!rp)                 /* No address succeeded */
    {
        perror("Could not connect to brasd");
        close(sfd);
        return -1;
    }

    /* disable Nagle algorithm */
    int v = 1; 
    setsockopt(sfd, SOL_TCP, TCP_NODELAY, &v, sizeof(v));

    return sfd;
}

BRAS_STATE read_state(int fd)
{
    char buffer[128];
    if(read(fd, buffer, 128) <= 0)
        return CLOSED;

    if(strhcmp(buffer, "connected"))
        return CONNECTED;
    else if(strhcmp(buffer, "disconnected"))
        return DISCONNECTED;
    else if(strhcmp(buffer, "connecting"))
        return CONNECTING;
    else if(strhcmp(buffer, "error"))
        return CRITICAL_ERROR;
    else if(strhcmp(buffer, "IN USE"))
        return INUSE;
    else
        return CRITICAL_ERROR;
}

int bras_state(int fd)
{
    return write(fd, "STAT\n", 6);
}

int bras_connect(int fd)
{
    return write(fd, "CONNECT\n", 9);
}

int bras_disconnect(int fd)
{
    return write(fd, "DISCONNECT\n", 12);
}

int bras_set(int fd, const char *username, const char *password)
{
    char buffer[128];
    snprintf(buffer, 128, "SET %s %s\n", username, password);

    return write(fd, buffer, strlen(buffer) + 1);
}
