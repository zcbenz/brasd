SUBDIRS = src data $(CLIENT_DIR)
DIST_SUBDIRS = src data bras-client

EXTRA_DIST = brasd.init \
			 README.md CREDITS Changelog \
			 brasd.recipe packaging \
			 installer/generate_installer.sh \
			 installer/makeself-header.sh \
			 installer/makeself.sh

install-service:
	cp $(srcdir)/brasd.init /etc/init.d/brasd
	ln -f -s /etc/init.d/brasd /etc/rc2.d/S80brasd
	ln -f -s /etc/init.d/brasd /etc/rc3.d/S80brasd
	ln -f -s /etc/init.d/brasd /etc/rc4.d/S80brasd
	ln -f -s /etc/init.d/brasd /etc/rc5.d/S80brasd
	/etc/init.d/brasd start

uninstall-service:
	/etc/init.d/brasd stop
	rm -f /etc/rc2.d/S80brasd
	rm -f /etc/rc3.d/S80brasd
	rm -f /etc/rc4.d/S80brasd
	rm -f /etc/rc5.d/S80brasd
	rm -f /etc/init.d/brasd

