#include <gtkmm/main.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/image.h>

#include <unistd.h>

#include "bras.h"
#include "logindlg.h"

namespace Gtk {
    enum MY_RESPONESE {
        RESPONSE_DISCONNECT = 1,
        RESPONSE_CONNECT,
        RESPONSE_RESTART
    }; 
}

typedef void (*state_func_t)();

inline void hide_current_dialog();
inline Gtk::MessageDialog *create_dialog(const Glib::ustring& message,
                                         Gtk::MessageType type,
                                         Gtk::ButtonsType buttons);
inline LoginDlg *create_login_dialog();

static void on_bras_state_change(Bras::State, Bras::State);
static void on_quit();
static void on_login();
static void on_connecting();
static void on_connected();
static void on_using();
static void on_error();
static void on_dlg_response(int response);

static bool on_brasd_no_response();
