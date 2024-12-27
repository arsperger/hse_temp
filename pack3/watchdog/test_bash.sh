#!/bin/bash

term_handler() {
	echo "clean up and exit"
	exit 143; # SIGTERM
}

trap 'term_handler' SIGTERM

CHARDEV=/dev/watchdog_timer

exec 5>$CHARDEV

#run it until SIGTERM
while true
do
	wait ${!}
done

#EOF
