#ifndef SERVER_H
#define SERVER_H

#include "ae.h"

/* init socket */
int init_server(const char *bindaddr, int port);

/* close all clients and then close server */
void close_server(int fd);

/* called by libevent when there is new connection */
void server_callback(struct aeEventLoop *eventLoop, int fd, void *clientData, int mask);

/* tell all clients of current state */
void broadcast_state();

#endif /* end of SERVER_H */
