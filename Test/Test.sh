#!/bin/bash

set -e   # Stop on first error

ROOT_DIR="$(pwd)"

find . -type f -name "Makefile" | while read makefile; do
    DIR=$(dirname "$makefile")
    echo "▶ Building in: $DIR"
    cd "$DIR"
    make
	echo execute $DIR/a.out
	./a.out
	echo -----------
    cd "$ROOT_DIR"
done