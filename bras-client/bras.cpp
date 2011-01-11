#include "utils.h"
#include "bras.h"

#include <iostream>

#include <glibmm/ustring.h>
using Glib::ustring;

#include <giomm/error.h>
#include <giomm/socket.h>
#include <giomm/socketconnection.h>
#include <giomm/socketclient.h>
#include <giomm/inputstream.h>
#include <giomm/outputstream.h>

Bras::State Bras::state_ = Bras::CONNECTED;
char Bras::buffer_[256];
sigc::signal<void, Bras::State, Bras::State> Bras::signal_state_changed;
Glib::RefPtr<Gio::InputStream> Bras::input_;
Glib::RefPtr<Gio::OutputStream> Bras::output_;
Glib::RefPtr<Gio::SocketConnection> Bras::connection_;

/* state code strings */
const ustring Bras::state_strings[COUNT] = {
    "connected", "connecting", "disconnected", "error", "closed", "IN USE"
};

Bras::Bras(const char *domain, int port) {
    /* Connect to brasd */
    Glib::RefPtr<Gio::SocketClient> client = Gio::SocketClient::create();

    if(!(connection_ = client->connect_to_host(domain, port)))
        throw Gio::Error(Gio::Error::CONNECTION_REFUSED, "brasd has not been started.");

    /* Create IOStream */
    input_  = connection_->get_input_stream();
    output_ = connection_->get_output_stream();

    /* Monitoring IOStream */
    input_->read_async(buffer_, 256, sigc::ptr_fun(on_state_changed));
}

Bras::~Bras() {
    connection_->close();
}

Bras *Bras::get() {
    static Bras instance("127.0.0.1", 10086);

    return &instance;
}

void Bras::set(const char *username, const char *password) {
    ustring command = ustring::compose("SET %1 %2\n", username, password);
    output_->write(command);
}

void Bras::connect() {
    output_->write("CONNECT\n");
}

void Bras::connect(const char *username, const char *password) {
    set(username, password);
    Glib::usleep(10000);
    connect();
}

void Bras::disconnect() {
    output_->write("DISCONNECT\n");
}

void Bras::on_state_changed(Glib::RefPtr<Gio::AsyncResult>& result) {
    size_t size;
    if((size = input_->read_finish(result)) <= 0)
        throw Gio::Error(Gio::Error::CLOSED, "brasd has exited.");

    ustring buffer(buffer_, size - 1);

    /* Get state code from string */
    State new_state = CRITICAL_ERROR;
    for(int i = 0; i < COUNT; i++) {
        if(!buffer.compare(0, state_strings[i].length(), state_strings[i])) {
            new_state = (State)i;
            break;
        }
    }

    signal_state_changed.emit(new_state, state_);
    state_ = new_state;

    input_->read_async(buffer_, 256, sigc::ptr_fun(on_state_changed));
}

