#include "server.h"
#include "bras.h"
#include "utils.h"

#include <string.h>
#include <stdio.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include <event.h>

extern enum BRAS_STATE state;
extern int debug;
struct event ev_client;
int cfd = -1; /* client fd */

const char *description[] = {
    "connected\n",
    "connecting\n",
    "disconnected\n",
    "error\n"
};

static void client_callback(int fd, short event, void *arg);

int init_server(const char *node, const char *service)
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

    int sfd = -1;
    struct addrinfo *rp;
    for(rp = result; rp != NULL; rp = rp->ai_next)
    {
        sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if(sfd == -1)
            continue;

        if(bind(sfd, rp->ai_addr, rp->ai_addrlen) != -1)
            break;          /* Success */

        close(sfd);
    }

    if(!rp)                 /* No address succeeded */
    {
        perror("Could not bind");
        close(sfd);
        return -1;
    }

    freeaddrinfo(result);

    /* avoid "address in use" error */
    int val = 1;
    setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(int));

    if(listen(sfd, 1) == -1)
    {
        perror("listen");
        close(sfd);
        return -1;
    }

    return sfd;
}

void server_callback(int fd, short event, void *arg)
{
    event_add((struct event*) arg, NULL);

    struct sockaddr client_addr;
    int new_cfd;
    size_t length = sizeof(struct sockaddr);
    if((new_cfd = accept(fd, &client_addr, &length)) == -1)
        return;

    /* only allow one connection one time */
    if(cfd > 0)
    {
        int tmp = write(new_cfd, "IN USE\n", 8);
        (void) tmp; /* suppress warnings */
        close(new_cfd);
        return;
    }

    cfd = new_cfd;
    post_state();

    event_set(&ev_client, cfd, EV_READ, client_callback, &ev_client);
    event_add(&ev_client, NULL);
}

void post_state()
{
    if(cfd < 0) return;

    int tmp = write(cfd, description[state], strlen(description[state]) + 1);
    (void) tmp; /* suppress warnings */
}

static void client_callback(int fd, short event, void *arg)
{
    char buffer[512] = { 0 };
    int len;
    if((len = read(cfd, buffer, 512)) > 0)
    {
        if(strhcmp(buffer, "STAT"))
            post_state();
        else if(strhcmp(buffer, "CONNECT")) {
            bras_connect();
        } else if(strhcmp(buffer, "DISCONNECT")) {
            bras_disconnect();
        } else if(strhcmp(buffer, "SET")) {
            char tmp[4], username[16], password[32];
            if(sscanf(buffer, "%s %s %s", tmp, username, password) == 3)
                bras_set(username, password);
        } else {
            fprintf(stderr, "Unrecognized command: %s\n", buffer);
        }
    }
    else /* client closed */
    {
        close(cfd);
        cfd = -1;
        return;
    }

    event_add((struct event*) arg, NULL);
}
