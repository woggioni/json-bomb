#!/bin/bash

set -e

docker build -t alpine:builder docker/builder

USER_ID=$(id -u)
GROUP_ID=$(id -g)
docker run --rm --user $USER_ID:$GROUP_ID -v "$(realpath .):/workspace" alpine:builder /workspace/docker/builder/build.sh
docker build -t json-bomb docker/runner

