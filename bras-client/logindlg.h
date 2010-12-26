#ifndef LOGINDLG_H
#define LOGINDLG_H

#include <sigc++/signal.h>

namespace Gtk {
    class Window;
    class ComboBoxEntry;
    class Entry;
    class Button;
}

class LoginDlg
{
public:
    LoginDlg(int bras);

    void show();

    /* emitted when login */
    sigc::signal<void> signal_login;

private:
    void on_login();
    void on_close();
    bool on_delete_event(GdkEventAny*);

private:
    Gtk::Window *window_;
    Gtk::Button *button_ok_;
    Gtk::Button *button_close_;
    Gtk::ComboBoxEntry *entry_username_;
    Gtk::Entry *entry_password_;

    int bras_;
};

#endif
