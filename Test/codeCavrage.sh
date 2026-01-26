#!/bin/bash

# set -e   # Stop on first error

ROOT_DIR="$(pwd)"

find /app \( \
		-name "*.html" -o \
		-name "*.gcno" -o \
		-name "*.gcda" -o \
		-name "*.js"   -o \
		-name "*.css" \
	\) -delete

export PATH=/app/Test:$PATH

find /app/Test -type f -name "Makefile" | while read makefile; do
    DIR=$(dirname "$makefile")
	if [ "$DIR" != "/app/Test" ]; then
		echo "▶ Building in: $DIR"
		cd "$DIR"
		mkdir coverage 2> /dev/null
		make cov
		chmod -R 777 coverage
		echo -----------
		cd "$ROOT_DIR"
	fi
done