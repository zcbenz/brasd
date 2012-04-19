#include "sds.h"
#include "anet.h"

#include "server.h"
#include "bras.h"
#include "utils.h"
#include "list.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <err.h>

#include <unistd.h>

extern enum BRAS_STATE state;
extern int debug;
extern struct options_t options;
struct node *client_list = NULL;

const char *description[] = {
    "connected\n",
    "connecting\n",
    "disconnected\n",
    "error\n"
};

/* translate client's commands into actions */
static void translate(const char *cmd, struct node *client);

/* tell the client of current state */
static void post_state(struct node *client);

/* callbacks for libevent */
static void client_cb(struct aeEventLoop *eventLoop, int fd, void *clientData, int mask);

/* remove client */
static void client_remove(struct node *client);

int init_server(const char *bindaddr, int port) {
    /* set node as NULL if options.internet is on */
    if(options.internet)
        bindaddr = NULL;

    char error[512] = { 0 };
    int fd = anetTcpServer(error, port, bindaddr);
    if (ANET_ERR == fd) {
        fprintf(stderr, "anetTcpServer: %s\n", error);
        return -1;
    }

    /* create client list */
    if(!client_list)
        client_list = create_list(); /* this list will never be freed */

    return fd;
}

/* close all clients and then close server */
void
close_server(int fd) {
    struct node *it = client_list->next;
    while (it) {
        struct node *next = it->next;
        client_remove(it);

        it = next;
    }
}

/* called by libevent when there is new connection */
void
server_callback(struct aeEventLoop *loop, int fd, void *clientData, int mask) {
    char error[512] = { 0 };

    int child = anetUnixAccept(error, fd);
    if (child == ANET_ERR) {
        fprintf(stderr, "anetUnixAccept: %s\n", error);
        return;
    }

    /* add new client to list */
    struct node *client = list_append(client_list, child);
    client->buffer = sdsempty();
    if (AE_ERR == aeCreateFileEvent(loop, child, AE_READABLE, client_cb, client))
        fprintf (stderr, "aeCreateFileEvent(server_callback) on %d failed\n", child);

    /* tell the client of current state */
    post_state(client);
}

/* tell all clients of current state */
void
broadcast_state() {
    struct node *it = client_list->next;
    while (it) {
        post_state(it);
        it = it->next;
    }
}

/* tell the client of current state */
void
post_state(struct node *client) {
    anetWrite(client->fd, description[state], strlen(description[state]));
}

static void
client_cb(struct aeEventLoop *eventLoop, int fd, void *clientData, int mask) {
    struct node *client = (struct node *)clientData;

    /* read */
    char buffer[512];
    int len = read(fd, buffer, 512);
    if (len <= 0) {
        fprintf(stderr, "Client closed: %d.", fd);
        aeDeleteFileEvent(eventLoop, fd, mask);
        client_remove(client);
        return;
    }

    /* push to total and split */
    int count = 0;
    client->buffer = sdscatlen(client->buffer, buffer, len);
    sds *lines = sdssplitlen(client->buffer, sdslen(client->buffer), "\n", 1, &count);

    /* translate */
    int i;
    for (i = 0; i < count; i++) {
        translate(lines[i], client);
        sdsfree(lines[i]);
    }
    free(lines);

    client->buffer = sdsempty();
}

/* translate client's commands into actions */
static void
translate(const char *cmd, struct node *client) {
    /* skip empty line */
    if (!*cmd) return;

    /* parse the commands */
    char username[16], password[32];

    if (strhcmp(cmd, "STAT"))
        broadcast_state();
    else if (strhcmp(cmd, "CONNECT"))
        bras_connect();
    else if (strhcmp(cmd, "DISCONNECT"))
        bras_disconnect();
    else if (sscanf(cmd, "SET %15s %31s", username, password) == 2)
        bras_set(username, password);
    else {
        fprintf(stderr, "Unrecognized command: %s\n", cmd);

        post_state(client);
    }
}

static void 
client_remove(struct node *client) {
    close(client->fd);
    sdsfree(client->buffer);
    list_remove(client);
}
