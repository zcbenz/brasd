CFLAGS+= -O2 -Wall -DDATA_DIR=\""$(pkgdatadir)"\" -DCONFIGS_DIR=\""$(pkgdatadir)/configs"\"

sbin_PROGRAMS = brasd

brasd_SOURCES = \
	main.c \
	server.c \
	server.h \
	list.c \
	list.h \
	bras.c \
	bras.h \
	utils.c \
	utils.h

brasd_LDADD = -levent

run:
	sudo ./brasd

