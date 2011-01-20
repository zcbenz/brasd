#ifndef UTILS_H
#define UTILS_H

struct options_t {
    int internet;
    char server[128];
};

int read_options(struct options_t *options);
int strhcmp(const char *str1, const char *str2);

/* discarded */
void notify_send(const char *summary, const char *body, const char *icon);

#endif /* end of UTILS_H */
