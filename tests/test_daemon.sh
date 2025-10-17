#!/bin/bash
set -e

echo "--- Testing Long-Running Daemon ---"

if [ ! -f ./process_daemon ]; then
    echo "Error: Please compile process_daemon first!"
    exit 1
fi

# Clean up any old daemon or log file
echo "[1] Cleaning up old processes and logs..."
killall -q process_daemon || true
rm -f /tmp/daemon.log

echo "[2] Starting the daemon in the background..."
./process_daemon

sleep 1 # Give it a moment to daemonize

# Check if the daemon is running
DAEMON_PID=$(pgrep process_daemon)
if [ -z "$DAEMON_PID" ]; then
    echo "Error: Daemon failed to start!"
    exit 1
fi
echo "Daemon is running with PID: $DAEMON_PID"

echo "[3] Monitoring for zombies for 15 seconds. There should be NONE."
for i in {1..15}; do
    # Check for any zombie processes on the system
    ZOMBIE_COUNT=$(ps aux | awk '$8 ~ /Z/' | wc -l)
    if [ "$ZOMBIE_COUNT" -ne 0 ]; then
        echo "Error: ZOMBIE DETECTED!"
        ps aux | awk '$8 ~ /Z/'
        killall process_daemon
        exit 1
    fi
    echo -n "."
    sleep 1
done
echo "\nNo zombies were detected. Success!"

echo "[4] Sending SIGTERM to shut down the daemon gracefully..."
killall process_daemon
sleep 1

echo "[5] Final check of the log file:"
echo "--------------------------------"
cat /tmp/daemon.log
echo "--------------------------------"

echo "--- Daemon Test Complete ---"