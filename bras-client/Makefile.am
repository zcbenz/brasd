SUBDIRS = ui

bin_PROGRAMS = bras-client

bras_client_SOURCES = \
	main.cpp \
	main.h \
	bras.cpp \
	bras.h \
	utils.h \
	utils.cpp \
	logindlg.cpp \
	logindlg.h

bras_client_LDFLAGS  = --as-needed 
bras_client_LDADD    = $(GTKMM_LIBS)

bras_client_CPPFLAGS = -O2 -Wall -DUI_DIR=\""$(pkgdatadir)/ui"\" \
					   $(GTKMM_CFLAGS)

run:
	./bras-client
