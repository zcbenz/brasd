#ifndef SERVER_H
#define SERVER_H

#include <event.h>

int  init_server(const char *node, const char *service);
void server_callback(int fd, short event, void *arg);
void broadcast_state();
void post_state(int fd);

#endif /* end of SERVER_H */
