#include <gtkmm/builder.h>
#include <gtkmm/window.h>
#include <gtkmm/button.h>
#include <gtkmm/entry.h>
#include <gtkmm/comboboxentrytext.h>
using sigc::mem_fun;
using Glib::ustring;

#include "logindlg.h"
#include "bras.h"
#include "utils.h"

LoginDlg::LoginDlg(int bras) :bras_(bras)
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
    config_t config;
    if(!read_config(&config)) {
        entry_username_->get_entry()->set_text(config.username);
        entry_password_->set_text(config.password);
    }
}

void LoginDlg::show()
{
    window_->show();
}

void LoginDlg::on_login()
{
    ustring username = entry_username_->get_active_text();
    ustring password = entry_password_->get_text();

    if(username.empty()) {
        entry_username_->grab_focus();
        return;
    } else if(password.empty()) {
        entry_password_->grab_focus();
        return;
    }

    bras_set(bras_, username.c_str(), password.c_str());
    usleep(10000);
    bras_connect(bras_);

    config_t config;
    strcpy(config.username, username.c_str());
    strcpy(config.password, password.c_str());
    write_config(&config);

    window_->hide();
}

void LoginDlg::on_close() {
    exit(0);
}

bool LoginDlg::on_delete_event(GdkEventAny*) {
    on_close();

    return false;
}
