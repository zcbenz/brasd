#ifndef LOGINDLG_H
#define LOGINDLG_H

#include <sigc++/signal.h>

namespace Gtk {
    class Window;
    class ComboBoxEntry;
    class Entry;
    class Button;
}

class Bras;

class LoginDlg {
public:
    LoginDlg();

    void show();
    void hide();

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

    Bras *bras_;
};

#endif
