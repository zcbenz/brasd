#ifndef UTILS_H
#define UTILS_H

int strhcmp(const char *str1, const char *str2);

typedef struct _config {
    char username[16];
    char password[64];
} config_t;

const char *get_home();
int read_config(config_t*);
int write_config(config_t*);

#endif /* end of UTILS_H */
