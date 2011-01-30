#ifndef SERVER_H
#define SERVER_H

/* init socket */
int init_server(const char *node, const char *service);

/* called by libevent when there is new connection */
void server_callback(int fd, short event, void *arg);

/* tell all clients of current state */
void broadcast_state();

#endif /* end of SERVER_H */
