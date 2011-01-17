#include <gtkmm/builder.h>
#include <gtkmm/window.h>
#include <gtkmm/button.h>
#include <gtkmm/entry.h>
#include <gtkmm/comboboxentrytext.h>
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
    builder->get_widget("username" , entry_username_);
    builder->get_widget("password" , entry_password_);

    /* connect signals */
    button_ok_->signal_clicked().connect(mem_fun(*this, &LoginDlg::on_login));
    button_close_->signal_clicked().connect(mem_fun(*this, &LoginDlg::on_close));
    window_->signal_delete_event().connect(mem_fun(*this, &LoginDlg::on_delete_event));
    entry_username_->get_entry()->signal_activate().connect(mem_fun(*this, &LoginDlg::on_login));
    entry_password_->signal_activate().connect(mem_fun(*this, &LoginDlg::on_login));

    /* set window icon */
    window_->set_icon_from_file(UI_DIR "/bras-client.png");

    /* set username and password to previously saved one */
    Options *options = Options::get();
    entry_username_->get_entry()->set_text(options->get_username());
    entry_password_->set_text(options->get_password());
}

void LoginDlg::show() {
    window_->show();
    shown_ = true;
}

void LoginDlg::hide() {
    window_->hide();
    shown_ = false;
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

    bras_->set(username.c_str(), password.c_str());

    /* save the username and password */
    Options *options = Options::get();
    options->add_passwd(username, password);

    /* set current username */
    options->set_curt(username);

    window_->hide();

    usleep(10000);
    signal_login.emit();
}

void LoginDlg::on_close() {
    signal_close.emit();
}

bool LoginDlg::on_delete_event(GdkEventAny*) {
    on_close();

    return false;
}
