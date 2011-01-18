#ifndef OPTIONS_H
#define OPTIONS_H

#include <map>
#include <glibmm/ustring.h>

class Options {
public:
    static Options *get();

    static Glib::ustring get_username() { return curt_; }
    static Glib::ustring get_password() { return get_password(curt_); }
    static Glib::ustring get_password(const Glib::ustring& username) { return passwords_[username]; }
    static void add_passwd(const Glib::ustring& username, const Glib::ustring& password) { passwords_[username] = password; curt_ = username; }

private:
    Options();
    Options(const Options&);
    ~Options();

    typedef std::map<Glib::ustring, Glib::ustring> Passwords;

    static Passwords passwords_;
    static Glib::ustring curt_;
};

#endif /* end of OPTIONS_H */
