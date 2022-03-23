FROM alpine:latest
ARG DOCKER_USER=luser
RUN addgroup -S $DOCKER_USER && adduser -D -S $DOCKER_USER -G $DOCKER_USER
RUN apk add g++ curl-dev cmake meson pkgconfig
USER $DOCKER_USER
WORKDIR /workspace
