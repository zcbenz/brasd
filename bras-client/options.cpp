#include <stdio.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>

#include <glibmm/miscutils.h>

#include "options.h"

Options::Passwords Options::passwords_;
Glib::ustring Options::curt_;
Glib::ustring Options::server_ = "127.0.0.1";
Glib::ustring Options::port_ = "10086";

using Glib::ustring;

Options::Options() {
    ustring filename = Glib::get_home_dir() + "/.brasd";

    FILE *file = fopen(filename.c_str(), "r+");
    if(!file) {
        perror("Cannot open ~/.brasd");
        return;
    }

    char buffer[512];
    while(fgets(buffer, 512, file)) {
        char arg1[256];
        char arg2[256];

        /* skip comments */
        if(buffer[0] == '#')
            continue;
        /* get stored passwords */
        else if(sscanf(buffer, "pair %255s %255s", arg1, arg2) == 2)
            passwords_[ustring(arg1)] = ustring(arg2);
        /* get current username */
        else if(sscanf(buffer, "curt %255s", arg1) == 1)
            curt_ = arg1;
        /* get brasd's address */
        else if(sscanf(buffer, "serv %255s", arg1) == 1)
            server_ = arg1;
        /* get brasd's port */
        else if(sscanf(buffer, "port %255s", arg1) == 1)
            port_ = arg1;
    }

    fclose(file);

    /* Set the first one as current username if we don't have one */
    if(curt_.empty() && !passwords_.empty())
        curt_ = passwords_.begin()->first;
}

Options::~Options() {
    ustring filename = Glib::get_home_dir() + "/.brasd";

    FILE *file = fopen(filename.c_str(), "w+");
    if(!file) {
        perror("Cannot open ~/.brasd");
        return;
    }

    /* Output configuration file */
    fputs("# BRAS Client configuration file \n\n", file);

    fputs("# brasd's server address\n", file);
    fprintf(file, "serv %s\n\n", server_.c_str());

    fputs("# brasd's port\n", file);
    fprintf(file, "port %s\n\n", port_.c_str());

    if(!curt_.empty()) {
        fputs("# current user\n", file);
        fprintf(file, "curt %s\n\n", curt_.c_str());
    }

    if(!passwords_.empty())
        fputs("# stored usernames and passwords\n", file);

    for(Passwords::const_iterator it = passwords_.begin();
        it != passwords_.end(); ++it)
    {
        if(!it->first.empty() && !it->second.empty())
            fprintf(file, "pair %s %s\n", it->first.c_str(), it->second.c_str());
    }

    fclose(file);
    chmod(filename.c_str(), 0600);
}

Options *Options::get() {
    static Options instance;

    return &instance;
}

Glib::ustring Options::encode(const Glib::ustring& str) {
    char *buffer = g_base64_encode((const guchar*)str.c_str(), str.length() + 1);
    Glib::ustring ret(buffer);
    g_free(buffer);
    return ret;
}

Glib::ustring Options::decode(const Glib::ustring& str) {
    gsize size;
    char *buffer = (char *)g_base64_decode((const gchar*)str.c_str(), &size);
    Glib::ustring ret(buffer);
    g_free(buffer);
    return ret;
}
