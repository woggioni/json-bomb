#!/bin/sh
set -e
meson /tmp/build /workspace
meson configure /tmp/build -Dbuildtype=release -Db_lto=true -Dstrip=true
meson compile -C /tmp/build
install -D /tmp/build/src/jsonbomb /workspace/docker/runner

