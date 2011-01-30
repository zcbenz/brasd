#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <err.h>

#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>

#include <event.h>

#include "bras.h"
#include "server.h"
#include "utils.h"

enum BRAS_STATE state;
int xl2tpd_pid, server_fd;
int debug = 0;
struct options_t options;

void kill_xl2tpd();
pid_t init_xl2tpd(int *out, int *err);

static void handle_interupt(int);
static void on_send_success(int);

/* callbacks for libevent */
static void on_xl2tpd_read(struct bufferevent *buf_ev, void *arg);
static void on_xl2tpd_write(struct bufferevent *buf_ev, void *arg);
static void on_xl2tpd_error(struct bufferevent *buf_ev, short what, void *arg);

/* translate and broadcast xl2tpd's output */
static void translate(const char *);

int main(int argc, char *argv[]) {
    /* use "brasd -D" to enter debug mode,
     * bras will not init as daemon */
    if(argc == 2 && !strcmp(argv[1], "-D")) {
        debug = 1;
    } else {
        if(daemon(0, 1)) {
            fputs("Cannot start as daemon", stderr);
            exit(EXIT_FAILURE);
        }
    }

    /* read options from /etc/brasd */
    read_options(&options);

    /* find and kill xl2tpd */
    kill_xl2tpd();

    /* start xl2tpd and hold the stdout of xl2tpd */
    int l2tp_out, l2tp_err; /* file descriptors of pipe */
    if((xl2tpd_pid = init_xl2tpd(&l2tp_out, &l2tp_err)) < 0) {
        fputs("Cannot start xl2tpd", stderr);
        exit(EXIT_FAILURE);
    }

    /* init state */
    state = DISCONNECTED;

    /* init server */
    if((server_fd = init_server(options.server, options.port)) < 0) {
        fputs("Cannot init server, brasd already runs?\n", stderr);
        kill(xl2tpd_pid, SIGKILL);
        exit(EXIT_FAILURE);
    }

    /* anti-zombies */
    signal(SIGCHLD, SIG_IGN);

    /* gracefully exit on CTRL+C */
    signal(SIGINT  , handle_interupt);
    signal(SIGHUP  , handle_interupt);
    signal(SIGTERM , handle_interupt);
    signal(SIGQUIT , handle_interupt);

    /* init libevent */
    event_init();

    /* server's fd */
    struct event ev_server;

    /* xl2tpd's stdout and stderr */
    struct bufferevent *ev_err, *ev_out;

    /* set bufferevent */
    set_nonblock(l2tp_err);
    set_nonblock(l2tp_out);

    ev_err = bufferevent_new(l2tp_err, on_xl2tpd_read,
        on_xl2tpd_write, on_xl2tpd_error, NULL);
    ev_out = bufferevent_new(l2tp_out, on_xl2tpd_read,
        on_xl2tpd_write, on_xl2tpd_error, NULL);

    bufferevent_enable(ev_err, EV_READ);
    bufferevent_enable(ev_out, EV_READ);

    /* use libevent to listen*/
    event_set(&ev_server, server_fd, EV_READ, server_callback, &ev_server);
    event_add(&ev_server, NULL);

    /* use sigalarm to delay sending CONNECTED state */
    signal(SIGALRM, on_send_success);

    /* enter loop */
    event_dispatch();

    /* kill xl2tpd */
    kill(xl2tpd_pid, SIGKILL);

    /* close fds and wait to exit */
    close(l2tp_err);
    close(l2tp_out);
    wait(NULL);

    return 0;
}

void kill_xl2tpd() {
    pclose(popen("killall -9 xl2tpd > /dev/null 2>&1", "r"));
}

/* start xl2tpd and redirect its output */
pid_t init_xl2tpd(int *out, int *err) {
    /* check if /var/run/xl2tpd exists */
    struct stat buf;
    if(stat("/var/run/xl2tpd/l2tp-control", &buf))
        mkdir("/var/run/xl2tpd", 0755);

    int fd_err[2], fd_out[2];
    pid_t pid;

    if(pipe(fd_err))
        goto err_err;

    if(pipe(fd_out))
        goto err_out;

    pid = fork();
    if(pid > 0) {
        close(fd_err[1]);
        close(fd_out[1]);

        *err = fd_err[0];
        *out = fd_out[0];
    }
    else if (pid == 0) {
        close(fd_err[0]);
        close(fd_out[0]);
        dup2(fd_out[1], 1);
        dup2(fd_err[1], 2);

        execlp("xl2tpd", "xl2tpd", "-D", NULL);
        perror("Cannot start xl2tpd");
        exit(EXIT_FAILURE);
    }
    else
        goto err_fork;

    if(debug) fprintf(stderr, "xl2tpd started: %d\n", pid);
    return pid;

err_fork:
    close(fd_out[0]);
    close(fd_out[1]);
err_out:
    close(fd_err[0]);
    close(fd_err[1]);
err_err:
    return -1;
}

/* disconnect bras and exit loop when received exit signal */
static void
handle_interupt(int signum) {
    if(state == CONNECTED) bras_disconnect();

    usleep(100000);
    event_loopbreak();
}

static void on_send_success(int sig) {
    alarm(0);

    state = CONNECTED;
    notify_send("BRAS", "You have connected to BRAS", "notification-network-wireless-full");
    broadcast_state();
}

/* there is new output of xl2tpd */
static void
on_xl2tpd_read(struct bufferevent *buf_ev, void *arg) {
    /* get the length of "xl2tpd[123]: " */
    char cmd_head[64];
    int head_len = snprintf(cmd_head, 64, "xl2tpd[%d]: ", xl2tpd_pid);

    /* readline */
    char *cmd = NULL;
    while((cmd = evbuffer_readline(buf_ev->input))) {
        translate(cmd + head_len);
        free(cmd);
    }
}

/* required by libevent, ignore it */
static void
on_xl2tpd_write(struct bufferevent *buf_ev, void *arg) {
}

/* an error occurred, may be EOF */
static void
on_xl2tpd_error(struct bufferevent *buf_ev, short what, void *arg) {
    if(!(what & EVBUFFER_EOF))
        warn("xl2tpd error, will exit.");

    /* free data */
    bufferevent_free(buf_ev);

    /* exit loop, and close fds at main() */
    event_loopbreak();
}

/* translate and broadcast xl2tpd's output */
static void
translate(const char *output) {
    if(strhcmp(output, "start_pppd")) {
        /* start_pppd doesn't mean auth is ok,
         * so if we do not get call_close in one second, 
         * we can know connection is successfully established */
        alarm(6);
    }
    else if (strhcmp(output, "Connecting to host")) {
        state = CONNECTING;
        broadcast_state();
    }
    else if (strhcmp(output, "Maximum retries exceeded")) {
        bras_disconnect();
    }
    else if (strhcmp(output, "Disconnecting from") || 
             strhcmp(output, "Host name lookup failed for "))
    {
        alarm(0); /* unschedule sending CONNECTED */

        if(state == CONNECTED) /* show notification when disconnected */
            notify_send("BRAS", "BRAS disconnected", "notification-network-wireless-disconnected");

        state = DISCONNECTED;
        broadcast_state();
    }
    else if (strhcmp(output, "call_close:")) {
        alarm(0); /* unschedule sending CONNECTED */

        state = DISCONNECTED;
        bras_disconnect(); /* when invalid auth disconnect */
        broadcast_state();
        puts("Username or password wrong?");
    }
    else if (strhcmp(output, "init_network")) {
        state = CRITICAL_ERROR;
        fputs("xl2tpd already runs\n", stderr);
    }
    else if (strhcmp(output, "open_controlfd")) {
        state = CRITICAL_ERROR;
        fputs("I dont't have enough permission\n", stderr);
    }

    if(debug) puts(output);
}

