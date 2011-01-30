#include <stdio.h>
#include <string.h>
#include <config.h>

#include <fcntl.h>

#include "utils.h"

extern int debug;

/* compare only first characters */
int strhcmp(const char *str1, const char *str2) {
    int i = 0;
    while(*str1 && * str2) {
        i++;
        if(*str1++ != *str2++) return 0;
    }

    /* return current position */
    return i;
}

/* read options from /etc/brasd */
int read_options(struct options_t *options) {
    /* fill in default values */
    strcpy(options->server, "127.0.0.1");
    strcpy(options->port, "10086");
    options->internet = 0;

    /* TODO get the path of /etc through autoconf */
    FILE *file = fopen("/etc/brasd", "r+");
    if(!file) {
        if(debug)
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
        /* read port */
        else if(sscanf(buffer, "port %255s", options->port) == 1)
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

    /* print options */
    if(debug) {
        fprintf(stderr, "server runs as %s\n", options->server);
        fprintf(stderr, "server port is %s\n", options->port);
        fprintf(stderr, "internet support is %d\n", options->internet);
    }

    return 0;
}

/* set the fd to non-blocking mode */
int
set_nonblock(int fd) {
    int flags;

    flags = fcntl(fd, F_GETFL);
    if (flags < 0) return flags;

    flags |= O_NONBLOCK;
    if (fcntl(fd, F_SETFL, flags) < 0)
        return -1;

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
