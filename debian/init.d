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

. /lib/lsb/init-functions

set -e

brasd_start () {
	log_daemon_msg "Starting $DESC"
	start-stop-daemon --start --quiet --exec $DAEMON
	log_end_msg $?
}

brasd_stop () {
	log_daemon_msg "Stopping $DESC"
	start-stop-daemon --oknodo --stop --quiet --name xl2tpd
	log_end_msg $?
}

case "$1" in
  start)
    brasd_start
	;;
  stop)
    brasd_stop
	;;
  force-reload)
	start-stop-daemon --stop --test --quiet --pidfile \
		$PIDFILE --exec $DAEMON \
	&& $0 restart \
	|| exit 0
	;;
  restart)
    brasd_stop
	sleep 1
    brasd_start
	;;
  *)
	N=/etc/init.d/$NAME
	echo "Usage: $N {start|stop|restart}" >&2
	exit 1
	;;
esac

exit 0
