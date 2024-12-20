#!/bin/bash

term_handler() {
    exit 143; # SIGTERM
}

trap 'kill ${!}; term_handler' SIGTERM

# start our super server :)
cd /app
./server 8080

#run it until SIGTERM
while true
do
  tail -f /dev/null & wait ${!}
done



