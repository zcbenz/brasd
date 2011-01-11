#include "utils.h"
#include "bras.h"

#include <stdio.h>
#include <string.h>

#include <stdexcept>

#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <netdb.h>

#include <gtkmm/main.h>

int Bras::bras_ = -1;
Bras::State Bras::state_ = Bras::CONNECTED;
sigc::signal<void, Bras::State, Bras::State> Bras::signal_state_changed;

/* state code strings */
const char *Bras::state_strings[COUNT] = {
    "connected", "connecting", "disconnected", "error", "closed", "IN USE"
};

Bras::Bras(const char *domain, const char *port) {
    /* Connect to brasd */
    struct addrinfo hints, *result;
    bzero(&hints, sizeof(struct addrinfo));
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags    = AI_PASSIVE;

    if(getaddrinfo(domain, port, &hints, &result))
        throw std::runtime_error("Cannot resolve server name");

    struct addrinfo *rp;
    for(rp = result; rp != NULL; rp = rp->ai_next) {
        bras_ = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if(bras_ == -1)
            continue;

        if(::connect(bras_, rp->ai_addr, rp->ai_addrlen) != -1)
            break;          /* Success */

        close(bras_);
    }

    freeaddrinfo(result);

    if(!rp) {               /* No address succeeded */
        close(bras_);
        throw std::runtime_error("Cannot connect to brasd.");
    }

    /* Monitoring brasd socket */
    Glib::signal_io().connect(sigc::hide(sigc::ptr_fun(on_state_changed)),
                              bras_, Glib::IO_IN | Glib::IO_ERR);
}

Bras::~Bras() {
    if(bras_ > 0) close(bras_);
}

Bras *Bras::get() {
    static Bras instance("127.0.0.1", "10086");

    return &instance;
}

void Bras::set(const char *username, const char *password) {
    char buffer[128];
    snprintf(buffer, 128, "SET %s %s\n", username, password);

    write(buffer);
}

void Bras::connect() {
    write("CONNECT\n");
}

void Bras::connect(const char *username, const char *password) {
    set(username, password);
    Glib::usleep(10000);
    connect();
}

void Bras::disconnect() {
    write("DISCONNECT\n");
}

bool Bras::on_state_changed() {
    char buffer[128];
    read(buffer, 128);

    /* Get state code from string */
    State new_state = CRITICAL_ERROR;
    for(int i = 0; i < COUNT; i++)
        if(strhcmp(buffer, state_strings[i])) {
            new_state = (State)i;
            break;
        }

    emit_signal(new_state);

    /* don't remove this function */
    return true;
}

void Bras::emit_signal(State new_state) {
    signal_state_changed.emit(new_state, state_);
    state_ = new_state;
}

void Bras::write(const char *str) {
    if(::write(bras_, str, strlen(str) + 1) <= 0)
        emit_signal(CLOSED);
}

void Bras::read(char *buffer, size_t size) {
    if(::read(bras_, buffer, size) <= 0)
        emit_signal(CLOSED);
}
