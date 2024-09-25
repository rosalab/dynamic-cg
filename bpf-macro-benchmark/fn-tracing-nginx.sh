#!/bin/bash

echo "Starting NGINX..."
trace-cmd record -p function_graph -b 100000 -c -F nginx -g "daemon off;" &
sleep 5
sed -i "s/listen       80;/listen       8089;/g" /etc/nginx/nginx.conf
nginx -s reload

# ps -axjf

# Get the PID of the NGINX master process
NGINX_PID=$(pgrep nginx | head -n 1)

if lsof -i :8089 > /dev/null; then
    echo "NGINX is running and bound to port 8089"
else
    echo "NGINX failed to bind to port 8089"
    exit 1
fi

# trace-cmd reset
# trace-cmd start -p function_graph -e sched_switch -b 10000 -P $NGINX_PID

wrk -t 1 -c 1 -d 1s http://localhost:8089/

# trace-cmd stop
# trace-cmd extract

#AAAAAa
kill $NGINX_PID

sleep 5

echo "Processing trace data..."
trace-cmd report > /linux/nginx_trace.txt