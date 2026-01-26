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

export PATH=/app/Test:$PATH

find . -type f -name "Makefile" | while read makefile; do
    DIR=$(dirname "$makefile")
	if [ "$DIR" != "." ]; then 
		echo "▶ Building in: $DIR"
		cd "$DIR"
		make re 2> /dev/null
		./a.out
		echo 
		cd "$ROOT_DIR"
	fi
done