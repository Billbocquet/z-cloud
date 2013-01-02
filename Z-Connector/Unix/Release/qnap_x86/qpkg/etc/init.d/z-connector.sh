#!/bin/sh
CONF=/etc/config/qpkg.conf
QPKG_NAME="z-connector"
NAME=z-connector
DAEMON=/usr/sbin/$NAME
DAEMON_ARGS="-s z-cloud.z-wave.me --cacert /etc/z-wave.me/Certificates/cacert.pem --cert /etc/z-wave.me/Certificates/cert.pem --key /etc/z-wave.me/Certificates/cert.key -d /dev/ttyUSB0 --debug --log=/var/log/z-connector.log"
PIDFILE=/var/run/$NAME.pid

case "$1" in
  start)
    ENABLED=$(/sbin/getcfg $QPKG_NAME Enable -u -d FALSE -f $CONF)
    if [ "$ENABLED" != "TRUE" ]; then
        echo "$QPKG_NAME is disabled."
        exit 1
    fi
	echo "starting daemon"
    : ADD START ACTIONS HERE
    
	start-stop-daemon --start --quiet --pidfile $PIDFILE --exec $DAEMON --test > /dev/null || exit 1
    start-stop-daemon --start --quiet --pidfile $PIDFILE --exec $DAEMON -- $DAEMON_ARGS    || exit 2
	;;
  stop)
    : ADD STOP ACTIONS HERE
	echo "Stopping daemon"
	# Return
	#   0 if daemon has been stopped
	#   1 if daemon was already stopped
	#   2 if daemon could not be stopped
	#   other if a failure occurred
	start-stop-daemon --stop --quiet --retry=TERM/30/KILL/5 --pidfile $PIDFILE --name $NAME 
	RETVAL="$?"
	[ "$RETVAL" = 2 ] && exit 2
	# Wait for children to finish too if this is a daemon that forks
	# and if the daemon is only ever run from this initscript.
	# If the above conditions are not satisfied then add some other code
	# that waits for the process to drop all resources that could be
	# needed by services started subsequently.  A last resort is to
	# sleep for some time.
	start-stop-daemon --stop --quiet --oknodo --retry=0/30/KILL/5 --exec $DAEMON
	[ "$?" = 2 ] && exit 2
	# Many daemons don't delete their pidfiles when they exit.
	rm -f $PIDFILE
	exit "$RETVAL"
	;;
  status)
    
	echo "is running"
	;;
  restart)
    $0 stop
    $0 start
    ;;
  *)
    echo "Usage: $0 {start|stop|restart|status}"
    exit 1
esac

exit 0
