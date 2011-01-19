#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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

void kill_xl2tpd();
pid_t init_xl2tpd(int *out, int *err);
void on_out(int fd, short event, void *arg);
void breakdown_output(const char *, size_t);
void translate(const char *);

static void handle_interupt(int);
static void on_send_success(int);

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
    if((server_fd = init_server("127.0.0.1", "10086")) < 0) {
        fputs("Cannot init server, brasd already runs?\n", stderr);
        kill(xl2tpd_pid, SIGKILL);
        exit(EXIT_FAILURE);
    }

    /* anti-zombies */
    signal(SIGCHLD, SIG_IGN);
    /* kill xl2tpd when quit */
    signal(SIGINT, handle_interupt);

    /* init libevent */
    struct event ev_out, ev_err, ev_server;
    event_init();

    /* use libevent to listen on pipe */
    event_set(&ev_err, l2tp_err, EV_READ, on_out, &ev_err);
    event_set(&ev_out, l2tp_out, EV_READ, on_out, &ev_out);
    event_set(&ev_server, server_fd, EV_READ, server_callback, &ev_server);
    event_add(&ev_err, NULL);
    event_add(&ev_out, NULL);
    event_add(&ev_server, NULL);

    /* use sigalarm to delay sending CONNECTED state */
    signal(SIGALRM, on_send_success);

    /* enter loop */
    event_dispatch();
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

/* called when there is new output of xl2tpd */
void on_out(int fd, short event, void *arg) {
    char buffer[512] = { 0 };
    size_t len;
    if((len = read(fd, buffer, 512)) > 0)
        breakdown_output(buffer, len);
    else {
        if(debug) fputs("l2tpd ended\n", stderr);
        close(fd);
        exit(EXIT_SUCCESS);
    }

    event_add((struct event*) arg, NULL);
}

/* break down output of xl2tpd into commands */
void breakdown_output(const char *output, size_t length) {
    /* turn string into file stream */
    FILE *f_pipe = fmemopen((void*)output, length, "r");

    char buffer[128];
    int ret, pid;
    while((ret = fscanf(f_pipe, "xl2tpd[%d]: ", &pid)) != EOF) {
        if(ret != 1) continue;
        if(!fgets(buffer, 128, f_pipe)) break;
        translate(buffer);
    }
}

/* get state from xl2tpd's output */
void translate(const char *output) {
    if(strhcmp(output, "start_pppd")) {
        /* start_pppd doesn't mean auth is ok,
         * so if we do not get call_close in one second, 
         * we can know connection is successfully established */
        alarm(6);
    }
    else if (strhcmp(output, "Connecting to host")) {
        state = CONNECTING;
        post_state();
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
        post_state();
    }
    else if (strhcmp(output, "call_close:")) {
        alarm(0); /* unschedule sending CONNECTED */

        state = DISCONNECTED;
        bras_disconnect(); /* when invalid auth disconnect */
        post_state();
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

static void handle_interupt(int signum) {
    if(state == CONNECTED) bras_disconnect();
    usleep(100000);
    kill(xl2tpd_pid, SIGKILL);
}

static void on_send_success(int sig) {
    alarm(0);

    state = CONNECTED;
    notify_send("BRAS", "You have connected to BRAS", "notification-network-wireless-full");
    post_state();
}

