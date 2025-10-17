#!/bin/bash

# Exit immediately if a command exits with a non-zero status.
set -e

echo "--- Testing Zombie Creation and Detection ---"

# Check if executables exist
if [ ! -f ./zombie_creator ] || [ ! -f ./zombie_detector ]; then
    echo "Error: Please compile zombie_creator and zombie_detector first!"
    exit 1
fi

echo "[1] Starting zombie_creator to make 7 zombies in the background..."
./zombie_creator 7 &
CREATOR_PID=$!

# Give the creator a moment to fork all children
sleep 2

echo "[2] Running zombie_detector to find the new zombies..."
# The detector should find 7 zombies with parent PID $CREATOR_PID
./zombie_detector

echo "[3] Cleaning up by killing the zombie_creator process (PID: $CREATOR_PID)..."
kill $CREATOR_PID

# Wait a moment for the OS to clean up the orphaned zombies
sleep 1

echo "--- Test Complete ---"