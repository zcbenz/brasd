SUBDIRS = ui

bin_PROGRAMS = bras-client

bras_client_SOURCES = \
	main.cpp \
	main.h \
	bras.cpp \
	bras.h \
	options.h \
	options.cpp \
	logindlg.cpp \
	logindlg.h

bras_client_LDADD    = $(GTKMM_LIBS)
bras_client_CPPFLAGS = -Wall -DUI_DIR=\""$(pkgdatadir)/ui"\" \
					   $(GTKMM_CFLAGS)

run:
	./bras-client
