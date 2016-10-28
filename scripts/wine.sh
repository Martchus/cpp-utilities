#!/bin/bash
if [[ $# -lt 1 ]]; then
    echo "Starts the specified executable with wine."
    echo "Usage: $0 executable [additional_dll_path(s)]"
    exit -1
fi
for arg in "${@:2}"; do
    WINEPATH="${arg};${WINEPATH}"
done
export WINEPATH;
wine "${@:1:1}"
