#!/bin/sh
# Packs the build into dist/bsdtrek-<target>.tar.gz (.zip for Windows)
# for av, with the man page, LICENSE and README.md beside the binary.
#
#   asciiverse/package.sh x86_64-unknown-linux-musl
set -eu
cd "$(dirname "$0")/.."
target="$1"
case "$target" in *windows*) exe=.exe ;; *) exe= ;; esac
stage=stage/bsdtrek
rm -rf "$stage" && mkdir -p "$stage" dist
cp "out/trek$exe" trek.6 LICENSE README.md "$stage/"
if [ -n "$exe" ]; then
  (cd "$stage" && zip -qr "../../dist/bsdtrek-$target.zip" .)
else
  tar -czf "dist/bsdtrek-$target.tar.gz" -C "$stage" .
fi
ls dist
