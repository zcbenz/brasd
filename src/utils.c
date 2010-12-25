#include <stdio.h>

#include <config.h>

#include "utils.h"

/* compare only first characters */
int strhcmp(const char *str1, const char *str2)
{
    while(*str1 && * str2)
        if(*str1++ != *str2++) return 0;

    return 1;
}

void notify_send(const char *summary, const char *body, const char *icon) {
#if HAVE_NOTIFY_OSD == 1
    char buffer[512];
    sprintf(buffer, "notify-send \"%s\" \"%s\" -i \"%s\"", summary, body, icon);
    pclose(popen(buffer, "r"));
#endif
}
