#include <gtkmm/main.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/image.h>

#include <unistd.h>

#include "bras.h"
#include "logindlg.h"

static int bras;
static BRAS_STATE state;
static Gtk::MessageDialog *con_dlg = NULL;

static bool on_bras_state_change(Glib::IOCondition io_condition);
static void on_quit();
static void on_login();
static void on_connecting();
static void on_connected();
static void on_using();
static void on_error();
static void on_dlg_response(int response);

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
        exit(1);
    }

    Glib::signal_io().connect(sigc::ptr_fun(on_bras_state_change),
                              bras, Glib::IO_IN);

    kit.run ();
}

static void on_quit() {
    Gtk::MessageDialog msg("brasd has quit", false,
                           Gtk::MESSAGE_ERROR,
                           Gtk::BUTTONS_OK,
                           true);

    msg.run();
    close(bras);
    exit(0);
}

static bool on_bras_state_change(Glib::IOCondition) {
    static BRAS_STATE last_state = CONNECTED;

    state = read_state(bras);

    if(con_dlg) { /* hide connecting dialog */
        delete con_dlg;
        con_dlg = NULL;
    }

    switch(state) {
        case DISCONNECTED:
            on_login();
            break;
        case CONNECTED:
            on_connected();
            break;
        case CONNECTING:
            on_connecting();
            break;
        case CLOSED:
            on_quit();
            break;
        case INUSE:
            on_using();
            break;
        default:
            on_error();
    }

    last_state = state;
    return true;
}

static void on_login() {
    static LoginDlg dlg(bras);

    dlg.show();
}

static void on_connecting() {
    con_dlg = new Gtk::MessageDialog("Connecting to BRAS...", false,
                                     Gtk::MESSAGE_OTHER,
                                     Gtk::BUTTONS_CANCEL,
                                     true);
    con_dlg->signal_response().connect(sigc::ptr_fun(on_dlg_response));
    con_dlg->show();
}

static void on_connected() {
    Gtk::MessageDialog msg("BRAS is now successfully connected", false,
                           Gtk::MESSAGE_INFO,
                           Gtk::BUTTONS_NONE,
                           true);
    msg.add_button(GTK_STOCK_DISCONNECT, 1);
    msg.add_button(GTK_STOCK_CLOSE, 2)->grab_focus();

    switch(msg.run()) {
        case 1:
            bras_disconnect(bras);
            break;
        default:
            exit(0);
    }
}

static void on_using() {
    Gtk::MessageDialog msg("brasd is occupied by another client", false,
                           Gtk::MESSAGE_ERROR,
                           Gtk::BUTTONS_OK,
                           false);
    msg.run();
    exit(0);
}
static void on_error() {
    Gtk::MessageDialog msg("We got a critical error", false,
                           Gtk::MESSAGE_ERROR,
                           Gtk::BUTTONS_OK,
                           false);
    msg.run();
    on_quit();
}

static void on_dlg_response(int response) {
    if(response == Gtk::RESPONSE_CANCEL)
        bras_disconnect(bras);
}
