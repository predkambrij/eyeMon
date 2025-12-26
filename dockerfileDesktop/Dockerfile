FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

ARG ARG_UID
ARG ARG_GID

ADD cfgs/supervisord.conf /etc/supervisord.conf

COPY script1.sh /
RUN /script1.sh
RUN apt-get install -y pwgen python3-pip

COPY script2.sh /
RUN /script2.sh

COPY script3.sh /
RUN /script3.sh
#RUN wget https://github.com/Itseez/opencv/archive/2.4.10.4.zip -O /2.4.10.4.zip

USER developer

COPY script4.sh /
RUN /script4.sh

USER root


EXPOSE 22
CMD ["supervisord", "-n", "-c", "/etc/supervisord.conf"]
