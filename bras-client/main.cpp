#include "main.h"

/* global values */
static int bras;
static BRAS_STATE state;
static Gtk::MessageDialog *cur_dlg = NULL;
static sigc::connection *con_response = NULL;

int main(int argc, char *argv[])
{
    Gtk::Main kit (argc, argv);

    bras = init_socket("127.0.0.1", "10086");
    if(bras < 0) {
        Gtk::MessageDialog msg("brasd has not been started", false,
                               Gtk::MESSAGE_ERROR,
                               Gtk::BUTTONS_OK,
                               true);
        msg.run();
        exit(EXIT_FAILURE);
    }

    Glib::signal_io().connect(sigc::ptr_fun(on_bras_state_change),
                              bras, Glib::IO_IN);

    kit.run();
}

inline Gtk::MessageDialog *create_dialog(const Glib::ustring& message,
                                         Gtk::MessageType type,
                                         Gtk::ButtonsType buttons)
{
    cur_dlg = new Gtk::MessageDialog(message, true, type, buttons, true);
    cur_dlg->signal_response().connect(sigc::ptr_fun(on_dlg_response));

    return cur_dlg;
}

static bool on_bras_state_change(Glib::IOCondition) {
    static BRAS_STATE last_state = CONNECTED;

    state = read_state(bras);

    if(cur_dlg) { /* hide current dialog */
        delete cur_dlg;
        cur_dlg = NULL;
    }

    if(con_response) { /* cancel previous no_response call */
        con_response->disconnect();
        delete con_response;
        con_response = NULL;
    }

    /* call functions of each state */
    static state_func_t state_funcs[STATES_COUNT] = {
        on_connected, on_connecting, on_login, on_error, on_quit, on_using
    };

    state_funcs[state] ();

    last_state = state;
    return true;
}

static void on_quit() {
    create_dialog("brasd has quit",
                  Gtk::MESSAGE_ERROR,
                  Gtk::BUTTONS_OK)->show();
}

static void on_login() {
    static LoginDlg dlg(bras);
    static sigc::connection dummy_vaule(dlg.signal_login.connect(
        sigc::bind(sigc::ptr_fun(on_dlg_response), Gtk::RESPONSE_CONNECT)));

    dlg.show();
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

static void on_dlg_response(int response) {
    switch(response) {
        case Gtk::RESPONSE_DISCONNECT:
        case Gtk::RESPONSE_CANCEL:
            bras_disconnect(bras);
            break;
        case Gtk::RESPONSE_CONNECT:
            bras_connect(bras);
            break;
        case Gtk::RESPONSE_CLOSE:
        case Gtk::RESPONSE_OK:
        case Gtk::RESPONSE_DELETE_EVENT:
            close(bras);
            exit(0);
    }

    /* if not receive state in 100ms, show no_response dialog */
    con_response = new sigc::connection(
        Glib::signal_timeout().connect(sigc::ptr_fun(on_brasd_no_response), 100));
}

static bool on_brasd_no_response() {
    if(!cur_dlg)
        create_dialog("Waiting for brasd's response...",
                      Gtk::MESSAGE_OTHER,
                      Gtk::BUTTONS_CLOSE)->show();

    return false;
}

