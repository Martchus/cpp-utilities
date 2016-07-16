#!/bin/bash
for arg in "${@:2}"; do
    WINEPATH="${arg};${WINEPATH}"
done
export WINEPATH;
wine "${@:1:1}"
