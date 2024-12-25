#!/bin/bash

term_handler() {
	echo "clean up and exit"
	exit 143; # SIGTERM
}

trap 'kill ${!}; term_handler' SIGTERM

CHARDEV=/dev/watchdog_timer

exec 5>$CHARDEV

#run it until SIGTERM
while true
do
  tail -f /dev/null & wait ${!}
done

#EOF
