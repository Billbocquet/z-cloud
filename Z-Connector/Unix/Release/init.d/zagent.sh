 #! /bin/sh
# /etc/init.d/noip


# If you want a command to always run, put it here


# Carry out specific functions when asked to by the system
case "$1" in
  start)
    echo "Starting zagent"
    # run application you want to start
    /usr/local/Run_Z-Agent.sh >> /var/log/z-agent.log &
    ;;
  stop)
    echo "Stopping zagent"
    # kill application you want to stop
    killall Run_Z-Agent.sh
    ;;
  *)
    echo "Usage: /etc/init.d/zagent {start|stop}"
    exit 1
    ;;
esac


exit 0
