FROM alpine:latest
RUN apk add libstdc++ libcurl
RUN adduser -D luser
COPY jsonbomb /bin
USER luser
WORKDIR /home/luser
COPY start.sh /home/luser/start.sh
CMD /home/luser/start.sh
