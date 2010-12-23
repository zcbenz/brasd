#! /bin/sh

### BEGIN INIT INFO
# Provides:          brasd
# Required-Start:    $network $syslog $remote_fs
# Required-Stop:     $network $syslog $remote_fs
# Should-Start:      ipsec
# Should-Stop:       ipsec
# Default-Start:     2 3 4 5
# Default-Stop:      0 1 6
# Short-Description: SoutheastUniversity BRAS daemon
# Description:       SoutheastUniversity BRAS daemon
### END INIT INFO

PATH=/usr/local/sbin:/usr/local/bin:/sbin:/bin:/usr/sbin:/usr/bin
DAEMON=/usr/sbin/brasd
NAME=brasd
DESC=brasd

test -x $DAEMON || exit 0

PIDFILE=/var/run/$NAME.pid

set -e

case "$1" in
  start)
	echo -n "Starting $DESC: "
	start-stop-daemon --start --quiet --pidfile $PIDFILE \
		--exec $DAEMON -- $DAEMON_OPTS
	echo "$NAME."
	;;
  stop)
	echo -n "Stopping $DESC: "
	start-stop-daemon --oknodo --stop --quiet --name xl2tpd 
	echo "$NAME."
	;;
  force-reload)
	start-stop-daemon --stop --test --quiet --pidfile \
		$PIDFILE --exec $DAEMON \
	&& $0 restart \
	|| exit 0
	;;
  restart)
	echo -n "Restarting $DESC: "
	start-stop-daemon --stop --quiet --pidfile \
		$PIDFILE --exec $DAEMON
	sleep 1
	start-stop-daemon --start --quiet --pidfile \
		$PIDFILE --exec $DAEMON -- $DAEMON_OPTS
	echo "$NAME."
	;;
  *)
	N=/etc/init.d/$NAME
	echo "Usage: $N {start|stop|restart}" >&2
	exit 1
	;;
esac

exit 0
