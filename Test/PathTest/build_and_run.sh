#!/bin/bash
set -e
ROOT_DIR=$(cd "$(dirname "$0")/../.." && pwd)
cd "$ROOT_DIR"

# Build project objects
make -f Makefile.ilyas re

# Create archive of all object files except the main object
OBJ_LIST=$(find . -maxdepth 2 -name "*.o" | grep -v "mainIlyas.o")
if [ -z "$OBJ_LIST" ]; then
  echo "No object files found to archive. Build failed?"
  exit 1
fi

echo "Creating static archive libwebsrv.a (excluding mainIlyas.o)..."
rm -f libwebsrv.a
ar rcs libwebsrv.a $OBJ_LIST

# Compile test
TEST_SRC="Test/PathTest/PathTest.cpp"
OUT_BIN="Test/PathTest/PathTest"

g++ -g3 -Wall -Wextra -Werror -std=c++98 -I. -o "$OUT_BIN" "$TEST_SRC" libwebsrv.a

# Run test
echo "Running Path tests..."
"$OUT_BIN"
