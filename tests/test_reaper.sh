#!/bin/bash
set -e

echo "--- Testing All Three Zombie Reaping Strategies ---"

if [ ! -f ./zombie_reaper ]; then
    echo "Error: Please compile zombie_reaper first!"
    exit 1
fi

for strategy in 1 2 3; do
    echo ""
    echo "================================="
    echo "      Testing Strategy $strategy"
    echo "================================="
    ./zombie_reaper $strategy
    sleep 1 # Small pause for clarity
done

echo ""
echo "--- All Reaping Tests Complete ---"