#!/bin/bash
set -e

echo "--- Running Zombie Detector ---"
echo "Note: This test is most effective when zombie processes"
echo "      (like from ./zombie_creator) are already running."

if [ ! -f ./zombie_detector ]; then
    echo "Error: Please compile zombie_detector first!"
    exit 1
fi

./zombie_detector

echo "--- Detection Complete ---"