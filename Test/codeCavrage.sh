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
    echo "▶ Building in: $DIR"
    cd "$DIR"
	mkdir coverage 2> /dev/null
    make re
	# echo execute $DIR/a.out
	# ./a.out
	echo -----------
    cd "$ROOT_DIR"
done