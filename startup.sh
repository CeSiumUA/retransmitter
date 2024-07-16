#! /bin/sh

case "$1" in
  start)
    echo "Starting retransmitter"
    start-stop-daemon -S -n retransmitter -a /usr/bin/retransmitter
    ;;
  stop)
    echo "Stopping retransmitter"
    start-stop-daemon -K -n retransmitter
    ;;
  *)
    echo "Usage: $0 {start|stop}"
    exit 1
    ;;
esac

exit 0