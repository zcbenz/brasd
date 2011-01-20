#include <stdio.h>
#include <string.h>
#include <config.h>

#include "utils.h"

/* compare only first characters */
int strhcmp(const char *str1, const char *str2) {
    while(*str1 && * str2)
        if(*str1++ != *str2++) return 0;

    return 1;
}

/* read options from /etc/brasd */
int read_options(struct options_t *options) {
    /* fill in default values */
    strcpy(options->server, "bras.seu.edu.cn");
    options->internet = 0;

    /* TODO get the path of /etc through autoconf */
    FILE *file = fopen("/etc/brasd", "r+");
    if(!file) {
        perror("Cannot open /etc/brasd");
        return -1;
    }

    char buffer[512];
    while(fgets(buffer, 512, file)) {
        char arg[256];

        /* skip comments */
        if(buffer[0] == '#')
            continue;
        /* read server name */
        else if(sscanf(buffer, "server %255s", options->server) == 1)
            ;
        /* read whether enable internet control */
        else if(sscanf(buffer, "internet %s", arg) == 1) {
            if(!strcmp(arg, "on"))
                options->internet = 1;
            else if(!strcmp(arg, "off"))
                options->internet = 0;
        }
    }

    fclose(file);

    return 0;
}

/* discarded */
void notify_send(const char *summary, const char *body, const char *icon) {
#if HAVE_NOTIFY_OSD == 1
    char buffer[512];
    sprintf(buffer, "notify-send \"%s\" \"%s\" -i \"%s\"", summary, body, icon);
    pclose(popen(buffer, "r"));
#endif
}
