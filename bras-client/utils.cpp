#include <stdio.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>

#include "utils.h"

/* compare only first characters */
int strhcmp(const char *str1, const char *str2) {
    while(*str1 && * str2)
        if(*str1++ != *str2++) return 0;

    return 1;
}

const char *get_home() {
    struct passwd *pw = getpwuid(getuid());

    return pw->pw_dir;
}

int read_config(config_t *config) {
    char config_path[512];
    sprintf(config_path, "%s/.brasd", get_home());

    FILE *fc = fopen(config_path, "r");
    if(!fc) return -1;

    if(fscanf(fc, "auth %s %s", config->username, config->password) != 2) {
        fclose(fc);
        return -1;
    } else {
        fclose(fc);
        return 0;
    }
}

int write_config(config_t *config) {
    char config_path[512];
    sprintf(config_path, "%s/.brasd", get_home());

    FILE *fc = fopen(config_path, "w+");
    if(!fc) return -1;

    fprintf(fc, "auth %s %s\n", config->username, config->password);
    fclose(fc);
    chmod(config_path, 0600);

    return 0;
}
