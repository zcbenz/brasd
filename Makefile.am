SUBDIRS = src data debian $(CLIENT_DIR)
DIST_SUBDIRS = src data bras-client debian

EXTRA_DIST = brasd.init \
			 ANNOUCEMENT README README.chinese CREDITS \
			 mkdeb mkppa \
			 packaging

install-service:
	cp $(srcdir)/brasd.init /etc/init.d/brasd
	ln -f -s /etc/init.d/brasd /etc/rc2.d/S80brasd
	ln -f -s /etc/init.d/brasd /etc/rc3.d/S80brasd
	ln -f -s /etc/init.d/brasd /etc/rc4.d/S80brasd
	ln -f -s /etc/init.d/brasd /etc/rc5.d/S80brasd
	ln -f -s $(bindir)/brasd /usr/sbin/brasd
	/etc/init.d/brasd start

uninstall-service:
	/etc/init.d/brasd stop
	rm -f /etc/rc2.d/S80brasd
	rm -f /etc/rc3.d/S80brasd
	rm -f /etc/rc4.d/S80brasd
	rm -f /etc/rc5.d/S80brasd
	rm -f /etc/init.d/brasd
	rm -f /usr/sbin/brasd

