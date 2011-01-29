#include "server.h"
#include "bras.h"
#include "utils.h"
#include "list.h"

#include <string.h>
#include <stdio.h>

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

static void client_callback(int fd, short event, void *arg);
static void client_close(int fd);

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

void server_callback(int fd, short event, void *arg) {
    event_add((struct event*) arg, NULL);

    struct sockaddr client_addr;
    int client_fd;
    size_t length = sizeof(struct sockaddr);
    if((client_fd = accept(fd, &client_addr, &length)) == -1)
        return;

    /* add new client to list */
	struct node *client = list_append(client_list, client_fd);
    event_set(&client->event, client->fd, EV_READ, client_callback, &client->event);
    event_add(&client->event, NULL);

    /* tell the client of current state */
    post_state(client_fd);
}

void broadcast_state() {
	struct node *it = client_list->next;
	while(it) {
		if(write(it->fd, description[state], strlen(description[state])) <= 0)
			if(debug) perror("Cannot post state to client");

		it = it->next;
	}
}

void post_state(int fd) {
    if(write(fd, description[state], strlen(description[state])) <= 0)
        client_close(fd);
}

static void client_callback(int fd, short event, void *arg) {
    char buffer[512];
    int len;
    if((len = read(fd, buffer, 512)) > 0) {
        /* turn string into file stream */
        FILE *in = fmemopen((void*)buffer, len, "r");

        char line[512];
        /* get commands line by line */
        while(fgets(line, 512, in)) {
            if(!*line) continue; /* skip empty line */

            /* parse the commands */
            if(strhcmp(line, "STAT"))
                broadcast_state();
            else if(strhcmp(line, "CONNECT"))
                bras_connect();
            else if(strhcmp(line, "DISCONNECT"))
                bras_disconnect();
            else if(strhcmp(line, "SET")) {
                char username[16], password[32];
                if(sscanf(buffer, "SET %15s %31s", username, password) == 3)
                    bras_set(username, password);
            } else
                if(debug) fprintf(stderr, "Unrecognized command: %s\n", line);
        }

        fclose(in);
    } else { /* client closed */
		if(debug) fprintf(stderr, "Client closed: %d\n", fd);

		client_close(fd);
        return;
    }

    event_add((struct event*) arg, NULL);
}

static void client_close(int fd) {
	struct node *it = list_find(client_list, fd);
	/* close and remove client from list, and remove from monitoring */
	if(it) {
		event_del(&it->event);
		close(it->fd);
		list_remove(it);
	} else {
		fprintf(stderr, "Remove invalid client from list: %d\n", fd);
	}
}
