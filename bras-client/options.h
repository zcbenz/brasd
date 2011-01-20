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
    static Glib::ustring get_password() { if(curt_.empty()) return ""; else return get_password(curt_); }
    static Glib::ustring get_password(const const_iterator& it) { return decode(it->second); }
    static Glib::ustring get_password(const Glib::ustring& username) { return decode(passwords_[username]); }
    static void set(const Glib::ustring& username) { curt_ = username; }
    static void add_passwd(const Glib::ustring& username, const Glib::ustring& password) { passwords_[username] = encode(password); curt_ = username; }

    static const char *get_server() { return server_.c_str(); }
    static const char *get_port() { return port_.c_str(); }

    static iterator begin() { return passwords_.begin(); }
    static iterator end() { return passwords_.end(); }
    static iterator find(const Glib::ustring& username) { return passwords_.find(username); }
    static void     erase(const iterator& it) { passwords_.erase(it); }

private:
    Options();
    Options(const Options&);
    ~Options();

    static Glib::ustring encode(const Glib::ustring& str);
    static Glib::ustring decode(const Glib::ustring& str);

    static Passwords passwords_;
    static Glib::ustring curt_;
    static Glib::ustring server_;
    static Glib::ustring port_;
};

#endif /* end of OPTIONS_H */
