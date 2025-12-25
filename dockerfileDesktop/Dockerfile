FROM ubuntu:14.04

ENV DEBIAN_FRONTEND noninteractive

ARG ARG_UID
ARG ARG_GID

ADD cfgs/supervisord.conf /etc/supervisord.conf

COPY script1.sh /
RUN /script1.sh

COPY script2.sh /
RUN /script2.sh

COPY script3.sh /
RUN /script3.sh

USER developer

COPY script4.sh /
RUN /script4.sh

USER root

EXPOSE 22
CMD ["supervisord", "-n", "-c", "/etc/supervisord.conf"]
