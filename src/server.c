#include "server.h"
#include "bras.h"
#include "utils.h"
#include "list.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <err.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include <event.h>

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

/* callbacks for libevent */
static void on_client_read(struct bufferevent *buf_ev, void *arg);
static void on_client_write(struct bufferevent *buf_ev, void *arg);
static void on_client_error(struct bufferevent *buf_ev, short what, void *arg);

/* translate client's commands into actions */
static void translate(const char *cmd, struct bufferevent *buf_ev);

/* tell the client of current state */
static void post_state(struct bufferevent *buf_ev);

int init_server(const char *node, const char *service) {
    /* set node as NULL if options.internet is on */
    if(options.internet)
        node = NULL;

    /* init socket */
    struct addrinfo hints, *result;
    bzero(&hints, sizeof(struct addrinfo));
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags    = AI_PASSIVE;

    if(getaddrinfo(node, service, &hints, &result)) {
        perror("getaddrinfo");
        return -1;
    }

    int sfd = -1;
    struct addrinfo *rp;
    for(rp = result; rp != NULL; rp = rp->ai_next) {
        sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if(sfd == -1)
            continue;

        if(bind(sfd, rp->ai_addr, rp->ai_addrlen) != -1)
            break;          /* Success */

        close(sfd);
    }

    if(!rp) {               /* No address succeeded */
        perror("Could not bind");
        close(sfd);
        return -1;
    }

    freeaddrinfo(result);

    /* avoid "address in use" error */
    int val = 1;
    setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(int));

    if(listen(sfd, 1) == -1) {
        perror("listen");
        close(sfd);
        return -1;
    }

	/* create client list */
	if(!client_list)
		client_list = create_list(); /* this list will never be freed */

    return sfd;
}

/* close all clients and then close server */
void
close_server(int fd) {
	struct node *it = client_list->next;
	while (it) {
        close(it->fd);
        bufferevent_free(it->buf_ev);
        list_remove(it);

		it = it->next;
	}
}

/* called by libevent when there is new connection */
void
server_callback(int fd, short event, void *arg) {
    event_add((struct event*) arg, NULL);

    struct sockaddr client_addr;
    int client_fd;
    size_t length = sizeof(struct sockaddr);
    if((client_fd = accept(fd, &client_addr, &length)) == -1)
        return;

    /* add new client to list */
	struct node *client = list_append(client_list, client_fd);
    set_nonblock(client_fd);
    client->buf_ev = bufferevent_new(client_fd, on_client_read,
            on_client_write, on_client_error, client);
    bufferevent_enable(client->buf_ev, EV_READ);

    /* tell the client of current state */
    post_state(client->buf_ev);
}

/* tell all clients of current state */
void
broadcast_state() {
	struct node *it = client_list->next;
	while (it) {
        bufferevent_write(it->buf_ev, description[state], strlen(description[state]));

		it = it->next;
	}
}

/* tell the client of current state */
void
post_state(struct bufferevent *buf_ev) {
    bufferevent_write(buf_ev, description[state], strlen(description[state]));
}

static void
on_client_read(struct bufferevent *buf_ev, void *arg) {
    /* read lines from client */
    char *cmd = NULL;
    while ((cmd = evbuffer_readline(buf_ev->input))) {
        if (debug)
            fprintf(stderr, "cmd: %s\n", cmd);

        translate(cmd, buf_ev);
        free(cmd);
    }
}

static void
on_client_write(struct bufferevent *buf_ev, void *arg) {
}

static void
on_client_error(struct bufferevent *buf_ev, short what, void *arg) {
    if (!(what & EVBUFFER_EOF))
        warn("Client socket error.");

    struct node *client = (struct node*) arg;

    if (debug)
        warn("Client closed: %d.", client->fd);

    /* free and close */
    close(client->fd);
    list_remove(client);
    bufferevent_free(buf_ev);
}

/* translate client's commands into actions */
static void
translate(const char *cmd, struct bufferevent *buf_ev) {
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
        if (debug)
            warn("Unrecognized command: %s\n", cmd);

        post_state(buf_ev);
    }
}
