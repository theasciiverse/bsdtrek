#!/bin/sh
# Builds trek into out/. Plain C on standard I/O, so one compiler call
# serves every target (see .github/workflows/release.yml):
#
#   asciiverse/build.sh                                         # Linux, macOS
#   LDFLAGS="-static -s" asciiverse/build.sh                    # Alpine, static
#   CC=x86_64-w64-mingw32-gcc LDFLAGS="-static -s" EXE=.exe asciiverse/build.sh  # Windows
set -eu
cd "$(dirname "$0")/.."
CC="${CC:-cc}"
CFLAGS="${CFLAGS:--O2}"
LDFLAGS="${LDFLAGS:-}"
EXE="${EXE:-}"
mkdir -p out
$CC -std=c99 -D_XOPEN_SOURCE=600 $CFLAGS -Wno-unused-result *.c $LDFLAGS -lm -o "out/trek$EXE"
echo "built trek$EXE"
