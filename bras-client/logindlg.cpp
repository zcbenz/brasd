#include <gtkmm/builder.h>
#include <gtkmm/window.h>
#include <gtkmm/button.h>
#include <gtkmm/entry.h>
#include <gtkmm/comboboxentry.h>
using sigc::mem_fun;
using Glib::ustring;

#include "logindlg.h"
#include "bras.h"
#include "options.h"

LoginDlg::LoginDlg(): bras_(Bras::get()), shown_(false)
{
    /* initialize widgets */
    Glib::RefPtr<Gtk::Builder> builder =
                Gtk::Builder::create_from_file(UI_DIR "/login.ui");
    builder->get_widget("login"    , window_);
    builder->get_widget("ok"       , button_ok_);
    builder->get_widget("close"    , button_close_);
    builder->get_widget("clear"    , button_clear_);
    builder->get_widget("remember" , button_remember_);
    builder->get_widget("username" , entry_username_);
    builder->get_widget("password" , entry_password_);

    /* create tree model */
    tree_model_ = Gtk::ListStore::create(columns_);

    /* set username combobox */
    entry_username_->set_model(tree_model_);
    entry_username_->set_text_column(columns_.username_);

    /* connect signals */
    button_ok_->signal_clicked().connect(mem_fun(*this, &LoginDlg::on_login));
    button_close_->signal_clicked().connect(mem_fun(*this, &LoginDlg::on_close));
    button_clear_->signal_clicked().connect(mem_fun(*this, &LoginDlg::on_clear));
    window_->signal_delete_event().connect(mem_fun(*this, &LoginDlg::on_delete_event));
    entry_username_->get_entry()->signal_activate().connect(mem_fun(*this, &LoginDlg::on_login));
    entry_username_->signal_changed().connect(mem_fun(*this, &LoginDlg::on_username_changed));
    entry_password_->signal_activate().connect(mem_fun(*this, &LoginDlg::on_login));

    /* set window icon */
    window_->set_icon_from_file(UI_DIR "/bras-client.png");

    /* set username and password to previously saved one */
    Options *options = Options::get();
    entry_username_->get_entry()->set_text(options->get_username());
    entry_password_->set_text(options->get_password());

    /* add stored usernames */
    for(Options::const_iterator it = options->begin();
        it != options->end(); add_row(it->first), ++it)
        ;
}

void LoginDlg::show() {
    window_->show_all();
    shown_ = true;
}

void LoginDlg::hide() {
    window_->hide();
    shown_ = false;
}

void LoginDlg::add_row(const Glib::ustring& username) {
    Gtk::TreeModel::Row row = *(tree_model_->append());
    row[columns_.username_] = username;
}

void LoginDlg::erase_row(const Glib::ustring& username) {
    Gtk::TreeModel::Children children = tree_model_->children();
    for(Gtk::TreeModel::iterator it = children.begin();
        it != children.end(); ++it)
        if((*it)[columns_.username_] == username) {
            tree_model_->erase(it);
            break;
        }
}

void LoginDlg::on_login() {
    ustring username = entry_username_->get_active_text();
    ustring password = entry_password_->get_text();

    if(username.empty()) {
        entry_username_->grab_focus();
        return;
    } else if(password.empty()) {
        entry_password_->grab_focus();
        return;
    }

    /* save the username and password */
    if(button_remember_->get_active()) {
        Options *options = Options::get();

        /* append it in column if not added before */
        if(options->find(username) == options->end())
            add_row(username);

        options->add_passwd(username, password);
    }

    bras_->set(username.c_str(), password.c_str());

    window_->hide();

    usleep(10000);
    signal_login.emit();
}

void LoginDlg::on_close() {
    signal_close.emit();
}

void LoginDlg::on_clear() {
    ustring username = entry_username_->get_active_text();
    if(username.empty()) return;

    /* erase saved username and password */
    Options *options = Options::get();
    Options::iterator it = options->find(username);
    if(it != options->end())
        options->erase(it);

    /* erase model and entry */
    entry_username_->get_entry()->set_text("");
    entry_password_->set_text("");
    erase_row(username);
}

bool LoginDlg::on_delete_event(GdkEventAny*) {
    on_close();

    return false;
}

void LoginDlg::on_username_changed() {
    ustring username = entry_username_->get_active_text();

    /* peek stored password */
    Options *options = Options::get();
    Options::const_iterator it = options->find(username);
    if(it != options->end())
        /* change password */
        entry_password_->set_text(it->second);
}
