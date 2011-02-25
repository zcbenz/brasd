#include "main.h"
#include <stdio.h>

/* global values */
static Gtk::MessageDialog *cur_dlg = NULL;
static LoginDlg *logindlg = NULL;

int main(int argc, char *argv[]) {
    Gtk::Main kit (argc, argv);

    Bras *bras;

    try {
        bras = Bras::get();
    } catch (std::exception& error) {
        Gtk::MessageDialog msg(error.what(), false,
                               Gtk::MESSAGE_ERROR,
                               Gtk::BUTTONS_NONE,
                               true);

        msg.add_button("Start brasd", Gtk::RESPONSE_RESTART);
        msg.add_button(GTK_STOCK_CLOSE, Gtk::RESPONSE_CLOSE)->grab_focus();

        /* start brasd, and then restart bras-client */
        if(msg.run() == Gtk::RESPONSE_RESTART) {
            /* this is the easy dirty way */
            pclose(popen("gksu service brasd start", "r"));
            execlp("bras-client", "bras-client", NULL);
        }

        return 0; /* We have not enter main loop, so don't use Gtk::Main::Quit */
    }

    bras->signal_state_changed.connect(sigc::ptr_fun(on_bras_state_change));

    kit.run();
}

inline void hide_current_dialog() {
    if(cur_dlg) { /* hide current dialog */
        delete cur_dlg;
        cur_dlg = NULL;
    }
    
    if (logindlg) {
        logindlg->hide();
    }
}

inline Gtk::MessageDialog *create_dialog(const Glib::ustring& message,
                                         Gtk::MessageType type,
                                         Gtk::ButtonsType buttons)
{
    cur_dlg = new Gtk::MessageDialog(message, true, type, buttons, true);
    cur_dlg->signal_response().connect(sigc::ptr_fun(on_dlg_response));

    return cur_dlg;
}

inline LoginDlg *create_login_dialog() {
    static LoginDlg dlg;
    if(!logindlg) {
        logindlg = &dlg;
        logindlg->signal_login.connect(sigc::bind(sigc::ptr_fun(on_dlg_response), Gtk::RESPONSE_CONNECT));
        logindlg->signal_close.connect(sigc::bind(sigc::ptr_fun(on_dlg_response), Gtk::RESPONSE_CLOSE));
    }

    logindlg->show();

    return logindlg;
}

static void on_bras_state_change(Bras::State state, Bras::State) {
    hide_current_dialog();

    /* call functions of each state */
    static state_func_t state_funcs[Bras::COUNT] = {
        on_connected, on_connecting, on_login, on_error, on_quit, on_using
    };

    state_funcs[state] ();
}

static void on_quit() {
    create_dialog("brasd has quit",
                  Gtk::MESSAGE_ERROR,
                  Gtk::BUTTONS_OK)->show();
}

static void on_login() {
    create_login_dialog();
}

static void on_connecting() {
    create_dialog("Connecting to BRAS...",
                  Gtk::MESSAGE_OTHER,
                  Gtk::BUTTONS_CANCEL)->show();
}

static void on_connected() {
    create_dialog("BRAS is now successfully connected",
                  Gtk::MESSAGE_INFO,
                  Gtk::BUTTONS_NONE);

    cur_dlg->add_button(GTK_STOCK_DISCONNECT, Gtk::RESPONSE_DISCONNECT);
    cur_dlg->add_button(GTK_STOCK_CLOSE, Gtk::RESPONSE_CLOSE)->grab_focus();

    cur_dlg->show();
}

static void on_using() {
    create_dialog("brasd is occupied by another client",
                  Gtk::MESSAGE_ERROR,
                  Gtk::BUTTONS_OK)->show();
}

static void on_error() {
    create_dialog("We got a critical error",
                  Gtk::MESSAGE_ERROR,
                  Gtk::BUTTONS_OK)->show();
}

static void on_brasd_no_response() {
    if(!cur_dlg && (!logindlg || !logindlg->is_show()))
        create_dialog("Waiting for brasd's response...",
                      Gtk::MESSAGE_OTHER,
                      Gtk::BUTTONS_CLOSE)->show();
}

static void on_dlg_response(int response) {
    Bras *bras = Bras::get();

    switch(response) {
        case Gtk::RESPONSE_DISCONNECT:
        case Gtk::RESPONSE_CANCEL:
            bras->disconnect();
            break;
        case Gtk::RESPONSE_CONNECT:
            bras->connect();
            break;
        case Gtk::RESPONSE_CLOSE:
        case Gtk::RESPONSE_OK:
        case Gtk::RESPONSE_DELETE_EVENT:
            Gtk::Main::quit();
    }

    /* if not receive state in 100ms, show no_response dialog */
    Glib::signal_timeout().connect_once(
            sigc::ptr_fun(on_brasd_no_response),
            100);
}
