#!/bin/bash

# set -e   # Stop on first error

ROOT_DIR="$(pwd)"

find ../ \( \
		-name "*.html" -o \
		-name "*.gcov" -o \
		-name "*.gcno" -o \
		-name "*.gcda" -o \
		-name "*.js"   -o \
		-name "*.css" \
	\) -delete

export PATH="$HOME/.local/bin:$PATH"

find . -type f -name "Makefile" | while read makefile; do
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