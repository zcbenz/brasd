#ifndef SERVER_H
#define SERVER_H

int init_server(const char *node, const char *service);
void server_callback(int fd, short event, void *arg);
void broadcast_state();

#endif /* end of SERVER_H */
