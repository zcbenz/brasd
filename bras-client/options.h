#ifndef OPTIONS_H
#define OPTIONS_H

#include <map>
#include <glibmm/ustring.h>

class Options {
public:
    static Options *get();

    typedef std::map<Glib::ustring, Glib::ustring> Passwords;
    typedef Passwords::iterator iterator;
    typedef Passwords::const_iterator const_iterator;

    static Glib::ustring get_username() { return curt_; }
    static Glib::ustring get_password() { return get_password(curt_); }
    static Glib::ustring get_password(const Glib::ustring& username) { return passwords_[username]; }
    static void add_passwd(const Glib::ustring& username, const Glib::ustring& password) { passwords_[username] = password; curt_ = username; }

    static iterator begin() { return passwords_.begin(); }
    static iterator end() { return passwords_.end(); }

private:
    Options();
    Options(const Options&);
    ~Options();

    static Passwords passwords_;
    static Glib::ustring curt_;
};

#endif /* end of OPTIONS_H */
