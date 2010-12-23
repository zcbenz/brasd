CFLAGS+= -O2 -Wall -DCONFIGS_DIR=\""$(pkgdatadir)/configs"\"

sbin_PROGRAMS = brasd

brasd_SOURCES = \
	main.c \
	brasd.h \
	server.c \
	server.h \
	bras.c \
	bras.h \
	utils.c \
	utils.h

brasd_LDADD = -levent

run:
	sudo ./brasd

