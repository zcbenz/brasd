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

	struct node *client = list_append(client_list, client_fd);

    post_state();

    event_set(&client->event, client->fd, EV_READ, client_callback, &client->event);
    event_add(&client->event, NULL);
}

void post_state() {
	struct node *it = client_list->next;
	while(it) {
		if(write(it->fd, description[state], strlen(description[state])) <= 0)
			if(debug) perror("Cannot post state to client");

		it = it->next;
	}
}

static void client_callback(int fd, short event, void *arg) {
    char buffer[512] = { 0 };
    int len;
    if((len = read(fd, buffer, 512)) > 0) {
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
    else { /* client closed */
		if(debug)
			fprintf(stderr, "Close client: %d\n", fd);
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
