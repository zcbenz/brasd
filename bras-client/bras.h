#ifndef BRAS_H
#define BRAS_H

#include <sigc++/signal.h>
#include <sigc++/connection.h>

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
    static void close();
    static void set(const char *username, const char *password);
    static void connect();
    static void connect(const char *username, const char *password);
    static void disconnect();
    static State get_state() { return state_; }

    static sigc::signal<void, State, State> signal_state_changed;

private:
    Bras (const char *domain, const char *port);
    Bras (const Bras&){}
    ~Bras();

    static const char *state_strings[COUNT];
    static bool on_state_changed();
    static void emit_signal(State);

    /* wrap original calls so if failed then emit CLOSED signal */
    static void write(const char*);
    static void read(char*, size_t);

    static State state_;
    static int bras_;
    static sigc::connection connection_io_;
};

#endif /* end of BRAS_H */
