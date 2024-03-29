#include "bras.h"

#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/* xl2tpd control file */
#define XL2TPD_CONTORL   "/var/run/xl2tpd/l2tp-control"
/* install path */
#define XL2TPDCONF_PATH  "/etc/xl2tpd/xl2tpd.conf"
#define CHAPSECRET_PATH  "/etc/ppp/chap-secrets"
#define PPPOPTIONS_PATH  "/etc/ppp/options.l2tpd"
/* template path */
#define XL2TPDCONF_TPL   CONFIGS_DIR "/xl2tpd.conf"
#define CHAPSECRET_TPL   CONFIGS_DIR "/chap-secrets"
#define PPPOPTIONS_TPL   CONFIGS_DIR "/options.l2tpd"
/* route table file path */
#define ADD_ROUTE_TABLE_PATH DATA_DIR "/add-route-table"
#define RM_ROUTE_TABLE_PATH  DATA_DIR "/rm-route-table"

/* remember default gateway */
static char default_gateway[64] = { 0 };

static int write_to(const char *string, const char *path);
static int get_file_size(int fd);
static int read_file(char *buffer, int fd);
static int print_tpl(const char *tpl, const char *file,
                     const char *username, const char *password);

int bras_get_default_gateway(char *buffer)
{
    FILE *fp = popen("route -n | grep -e '^0.0.0.0' | awk '{print $2}'", "r");
    if(!fp) return 1;

    if(fscanf(fp, "%s", buffer) != 1 ||
       !strcmp(buffer, "") ||
       !strcmp(buffer, "0.0.0.0"))
    {
        pclose(fp);
        return 1;
    }

    pclose(fp);
    return 0;
}

int bras_restore_route()
{
    /* read route table from file */
    FILE *table = fopen(RM_ROUTE_TABLE_PATH, "r");
    if(!table) {
        perror("Can not open rm-route-table file");
        return -1;
    }

    /* rm route table line by line */
    char line[512], command[512];
    while(fgets(line, 512, table)) {
        sprintf(command, line, default_gateway);
        pclose(popen(command, "r"));
    }

    fclose(table);

    return 0;
}

int bras_add_route()
{
    if(bras_get_default_gateway(default_gateway)) return -1;

    /* read route table from file */
    FILE *table = fopen(ADD_ROUTE_TABLE_PATH, "r");
    if(!table) {
        perror("Can not open add-route-table file");
        return -1;
    }

    /* add route table line by line */
    char line[512], command[512];
    while(fgets(line, 512, table)) {
        sprintf(command, line, default_gateway);
        pclose(popen(command, "r"));
    }

    fclose(table);

    return 0;
}

int bras_connect()
{
    bras_add_route();
    return write_to("c seubras", XL2TPD_CONTORL);
}

int bras_disconnect()
{
    bras_restore_route();
    return write_to("d seubras", XL2TPD_CONTORL);
}

int bras_set(const char *username, const char *password)
{
    if(!access("/etc/ppp", F_OK)) mkdir("/etc/ppp", 0751);
    if(!access("/etc/xl2tpd", F_OK)) mkdir("/etc/xl2tpd", 0751);

    if(print_tpl(XL2TPDCONF_TPL, XL2TPDCONF_PATH, username, password))
        goto go_error;
    if(print_tpl(PPPOPTIONS_TPL, PPPOPTIONS_PATH, username, password))
        goto go_error;
    if(print_tpl(CHAPSECRET_TPL, CHAPSECRET_PATH, username, password))
        goto go_error;

    return 0;

go_error:
    perror("Cannot write config file");
    return -1;
}

static int write_to(const char *string, const char *path)
{
    int fd = open(path, O_WRONLY);
    if(fd < 0)
    {
        perror("cannot open xl2tpd control file");
        return fd;
    }

    size_t written = strlen(string);
    if((written = write(fd, string, written)) <= 0)
    {
        perror("cannot write to xl2tpd control file");
        return -1;
    }

    close(fd);
    return written;
}

static int get_file_size(int fd)
{
    struct stat buf;
    if(fstat(fd, &buf))
        return -1;
    else
        return buf.st_size;
}

static int read_file(char *buffer, int fd)
{
    int count;
    while((count = read(fd, buffer, 128)) > 0)
        buffer += count;
    buffer[count] = 0;

    return count;
}

static int print_tpl(const char *tpl, const char *file,
                     const char *username, const char *password)
{
    int fd = open(tpl, O_RDONLY);
    if(fd < 0) return -1;

    int size = get_file_size(fd);
    if(size < 0) return -1;

    char buffer[size];
    if(read_file(buffer, fd) < 0)
        return -1;

    FILE *target = fopen(file, "w");
    if(!target)
    {
        close(fd);
        return -1;
    }

    fprintf(target, buffer, username, password);
    close(fd);
    fclose(target);
    chmod(file, 0600);

    return 0;
}
