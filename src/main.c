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

/* redis library */
#include "ae.h"
#include "anet.h"
#include "sds.h"

#include "bras.h"
#include "server.h"
#include "utils.h"

/* the main loop */
aeEventLoop *loop;

/* xl2tpd output buffer */
sds x_buf;

/* timer id */
long long timer_id = -1;

enum BRAS_STATE state;
int xl2tpd_pid, server_fd;
int debug = 0;
struct options_t options;

void kill_xl2tpd();
pid_t init_xl2tpd(int *out, int *err);

static void handle_interupt(int);

/* callbacks for libevent */
static void on_xl2tpd(struct aeEventLoop *eventLoop, int fd, void *clientData, int mask);

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
    if((server_fd = init_server(options.server, atoi(options.port))) < 0) {
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

    /* init buffer */
    x_buf = sdsempty();

    /* init ae */
    loop = aeCreateEventLoop();

    /* listen to xl2tpd's output and input */
    if (AE_ERR == aeCreateFileEvent(loop, l2tp_out, AE_READABLE, on_xl2tpd, NULL))
        fprintf (stderr, "aeCreateEventLoop on %d failed\n", l2tp_out);
    if (AE_ERR == aeCreateFileEvent(loop, l2tp_err, AE_READABLE, on_xl2tpd, NULL))
        fprintf (stderr, "aeCreateEventLoop on %d failed\n", l2tp_err);

    /* listen to the server */
    if (AE_ERR == aeCreateFileEvent(loop, server_fd, AE_READABLE, server_callback, NULL))
        fprintf (stderr, "aeCreateEventLoop on %d failed\n", l2tp_out);

    /* enter loop */
    aeMain(loop);
    aeDeleteEventLoop(loop);

    /* kill xl2tpd */
    kill(xl2tpd_pid, SIGKILL);

    /* close fds and wait to exit */
    close_server(server_fd);
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
    aeStop(loop);
}

static int
on_send_success(struct aeEventLoop *eventLoop, long long id, void *clientData) {
    state = CONNECTED;
    broadcast_state();

    return AE_NOMORE;
}

/* there is new output of xl2tpd */
static void
on_xl2tpd(struct aeEventLoop *eventLoop, int fd, void *clientData, int mask) {
    /* read */
    char buffer[1024];
    int len = read(fd, buffer, 1024);
    if (len <= 0) {
        if (debug) {
            perror("on_xl2tpd(anetRead)");
            fprintf(stderr, "xl2tpd quit\n");
        }

        aeStop(loop);
        return;
    }

    /* push to total and split */
    int count = 0;
    x_buf = sdscatlen(x_buf, buffer, len);
    sds *lines = sdssplitlen(x_buf, sdslen(x_buf), "\n", 1, &count);

    /* translate */
    int i;
    for (i = 0; i < count; i++) {
        translate(lines[i]);
        sdsfree(lines[i]);
    }
    free(lines);

    sdsclear(x_buf);
}

/* translate and broadcast xl2tpd's output */
static void
translate(const char *output) {
    /* skip : */
    while (*output && *output != ':') ++output;
    output += 2;

    /* skip blank line */
    if (!*output)
        return;

    if (strhcmp(output, "Connecting to host")) {
        timer_id = aeCreateTimeEvent(loop, 6000, on_send_success, NULL, NULL);
        state = CONNECTING;
        broadcast_state();
    }
    else if (strhcmp(output, "Maximum retries exceeded")) {
        bras_disconnect();
    }
    else if (strhcmp(output, "Disconnecting from") || 
             strhcmp(output, "Host name lookup failed for "))
    {
        if (timer_id >= 0) /* unschedule sending CONNECTED */
        {
            aeDeleteTimeEvent(loop, timer_id);
            timer_id = -1;
        }

        state = DISCONNECTED;
        broadcast_state();
    }
    else if (strhcmp(output, "call_close:")) {
        bras_disconnect(); /* when invalid auth disconnect */
        fputs("Username or password wrong?", stderr);
    }
    else if (strhcmp(output, "init_network")) {
        state = CRITICAL_ERROR;
        fputs("xl2tpd already runs\n", stderr);
    }
    else if (strhcmp(output, "open_controlfd")) {
        state = CRITICAL_ERROR;
        fputs("I dont't have enough permission\n", stderr);
    }

    if (debug)
        puts(output);
}
