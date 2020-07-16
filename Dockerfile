FROM balenalib/armv7hf-debian

RUN [ "cross-build-start" ]

RUN apt-get update && apt-get install python-setuptools python-pip && apt-get clean && rm -rf /var/lib/apt/lists/*

COPY . /opt/rover/

#RUN apt-get update
#RUN apt-get install gcc
RUN apt-get update && apt-get install gcc python-dev && \
    pip install wheel && \
    pip install -e /opt/rover/ && \
    apt-get purge --auto-remove gcc python-dev && \
    apt-get clean && rm -rf /var/lib/apt/lists/*

ENV FLASK_APP=SGVHAK_Rover

RUN [ "cross-build-end" ]

EXPOSE 5000

WORKDIR /opt/rover

CMD [ "flask", "run", "--host=0.0.0.0" ]