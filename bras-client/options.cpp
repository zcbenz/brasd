#include <stdio.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>

#include <glibmm/miscutils.h>

#include "options.h"

Options::Passwords Options::passwords_;
Glib::ustring Options::curt_;

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
        char username[256];
        char password[256];

        /* skip comments */
        if(buffer[0] == '#')
            continue;
        /* get stored passwords */
        else if(sscanf(buffer, "auth %255s %255s", username, password) == 2)
            passwords_[ustring(username)] = ustring(password);
        /* get current username */
        else if(sscanf(buffer, "curt %255s", username) == 1)
            curt_ = username;
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
            fprintf(file, "auth %s %s\n", it->first.c_str(), it->second.c_str());
    }

    fclose(file);
    chmod(filename.c_str(), 0600);
}

Options *Options::get() {
    static Options instance;

    return &instance;
}

