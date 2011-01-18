#ifndef LOGINDLG_H
#define LOGINDLG_H

#include <gtkmm/liststore.h>
#include <sigc++/signal.h>

namespace Gtk {
    class Window;
    class ComboBoxEntry;
    class Entry;
    class Button;
    class CheckButton;
}

class Bras;

class LoginDlg {
public:
    LoginDlg();

    void show();
    void hide();
    bool is_show() { return shown_; }

    /* emitted when login */
    sigc::signal<void> signal_login;
    sigc::signal<void> signal_close;

protected:
    class ModelColumns: public Gtk::TreeModel::ColumnRecord {
    public:
        ModelColumns() { add(username_); }

        Gtk::TreeModelColumn<Glib::ustring> username_;
    };

    ModelColumns columns_;
    Glib::RefPtr<Gtk::ListStore> tree_model_;

    void add_column(const Glib::ustring&);

private:
    void on_login();
    void on_close();
    bool on_delete_event(GdkEventAny*);
    void on_username_changed();

private:
    Gtk::Window *window_;
    Gtk::Button *button_ok_;
    Gtk::Button *button_close_;
    Gtk::CheckButton *button_remember_;
    Gtk::ComboBoxEntry *entry_username_;
    Gtk::Entry *entry_password_;

    Bras *bras_;
    bool shown_;
};

#endif
