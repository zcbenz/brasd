#ifndef BRAS_H
#define BRAS_H

enum BRAS_STATE
{
    CONNECTED,
    CONNECTING,
    DISCONNECTED,
    CRITICAL_ERROR,
    CLOSED,
    INUSE
};

int init_socket(const char *node, const char *port);

BRAS_STATE read_state(int fd);  /* read state when socket is ready */
int bras_state(int fd);   /* get state by sending STAT */
int bras_connect(int fd);
int bras_disconnect(int fd);
int bras_set(int fd, const char *username, const char *password);

#endif /* end of BRAS_H */
