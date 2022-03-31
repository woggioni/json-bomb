FROM python:3.10-alpine
RUN adduser -D luser
COPY main.py /home/luser
USER luser
WORKDIR /home/luser
RUN pip install --user flask
ENV FLASK_APP=main.py
EXPOSE 1180
CMD python -m flask run -h 0.0.0.0 -p 1180
