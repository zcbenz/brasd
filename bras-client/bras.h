#ifndef BRAS_H
#define BRAS_H

#include <sigc++/signal.h>

#include <glibmm/refptr.h>
#include <glibmm/iochannel.h>

/* Forward declaration */
namespace Gio {
    class SocketConnection;
    class InputStream;
    class OutputStream;
    class AsyncResult;
}

class Bras {
public:
    enum State {
        CONNECTED,
        CONNECTING,
        DISCONNECTED,
        CRITICAL_ERROR,
        CLOSED,
        INUSE,
        COUNT
    };

    static Bras *get();
    static void set(const char *username, const char *password);
    static void connect();
    static void connect(const char *username, const char *password);
    static void disconnect();
    static State get_state() { return state_; }

    static sigc::signal<void, State, State> signal_state_changed;

private:
    Bras (const char *domain, int port);
    Bras (const Bras&){}
    ~Bras();

    static const Glib::ustring state_strings[COUNT];
    static void on_state_changed(Glib::RefPtr<Gio::AsyncResult>&);

    static State state_;
    static char buffer_[256];
    static Glib::RefPtr<Gio::SocketConnection> connection_;
    static Glib::RefPtr<Gio::InputStream> input_;
    static Glib::RefPtr<Gio::OutputStream> output_;
};

#endif /* end of BRAS_H */
