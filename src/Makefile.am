CFLAGS+= -g -Wall -DDATA_DIR=\""$(pkgdatadir)"\" -DCONFIGS_DIR=\""$(pkgdatadir)/configs"\"

sbin_PROGRAMS = brasd

brasd_SOURCES = \
	ae.h \
	ae.c \
	sds.h \
	sds.c \
	anet.h \
	anet.c \
	main.c \
	server.c \
	server.h \
	list.c \
	list.h \
	bras.c \
	bras.h \
	utils.c \
	utils.h

run:
	sudo ./brasd

