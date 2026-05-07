#!/bin/bash
cd /home/debian/RFID/build
pkill -9 -f "./server" 2>/dev/null
sleep 1
RFID_DB_HOST=127.0.0.1 RFID_DB_PORT=5432 RFID_DB_USER=rfid RFID_DB_PASS=rfid123 RFID_DB_NAME=rfid nohup ./server > server_v33.log 2>&1 &
sleep 2
ps aux | grep "./server" | grep -v grep
tail -5 /home/debian/RFID/build/server_v33.log

