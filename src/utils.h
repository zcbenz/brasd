#ifndef UTILS_H
#define UTILS_H

struct options_t {
    int internet;
    char port[64];
    char server[128];
};

int read_options(struct options_t *options);
int strhcmp(const char *str1, const char *str2);

/* set the fd to non-blocking mode */
int set_nonblock(int fd);

/* discarded */
void notify_send(const char *summary, const char *body, const char *icon);

#endif /* end of UTILS_H */
