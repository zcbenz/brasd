#ifndef BRAS_H
#define BRAS_H

int bras_get_default_gateway(char *buffer);
int bras_add_route();
int bras_connect();
int bras_disconnect();
int bras_set(const char *username, const char *password);

#endif /* end of BRAS_H */
