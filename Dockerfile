# Start from scratch
FROM docker.io/debian:stretch

# Label this image
# LABEL name="registry.arpa.local/processi/elab_msg_batch"
LABEL name="elab_batch"
LABEL version="1.0"
LABEL maintainer="Luca Paganotti <luca.paganotti@gmail.com>"
LABEL description="image for radar process elab_msg_batch"

## Set a default user. Available via runtime flag `--user docker` 
## Add user to 'staff' group, granting them write privileges to /usr/local/lib/R/site.library
## User should also have & own a home directory (for rstudio or linked volumes to work properly). 
RUN useradd docker \
	&& mkdir /home/docker \
	&& chown docker:docker /home/docker \
	&& addgroup docker staff

RUN apt-get update \ 
	&& apt-get install -y --no-install-recommends \
                apt-utils \
		ed \
		less \
		locales \
		vim-tiny \
		wget \
		ca-certificates \
		fonts-texgyre \
	&& rm -rf /var/lib/apt/lists/*

## Configure default locale, see https://github.com/rocker-org/rocker/issues/19
RUN echo "en_US.UTF-8 UTF-8" >> /etc/locale.gen \
	&& locale-gen en_US.utf8 \
	&& /usr/sbin/update-locale LANG=en_US.UTF-8

ENV LC_ALL en_US.UTF-8
ENV LANG en_US.UTF-8

## Use Debian unstable via pinning -- new style via APT::Default-Release
#RUN echo "deb http://http.debian.net/debian sid main" > /etc/apt/sources.list.d/debian-unstable.list \
#	&& echo 'APT::Default-Release "testing";' > /etc/apt/apt.conf.d/default

ENV R_BASE_VERSION 3.3.3

## Now install R and littler, and create a link for littler in /usr/local/bin
## Also set a default CRAN repo, and make sure littler knows about it too
RUN apt-get update \
	#&& apt-get install -t unstable -y --no-install-recommends \
	&& apt-get install -y --no-install-recommends \
		littler \
                r-cran-littler \
		r-base=${R_BASE_VERSION}* \
		r-base-dev=${R_BASE_VERSION}* \
		r-recommended=${R_BASE_VERSION}* \
        && echo 'options(repos = c(CRAN = "https://cloud.r-project.org/"), download.file.method = "libcurl")' >> /etc/R/Rprofile.site \
        && echo 'source("/etc/R/Rprofile.site")' >> /etc/littler.r \
	&& ln -s /usr/share/doc/littler/examples/install.r /usr/local/bin/install.r \
	&& ln -s /usr/share/doc/littler/examples/install2.r /usr/local/bin/install2.r \
	&& ln -s /usr/share/doc/littler/examples/installGithub.r /usr/local/bin/installGithub.r \
	&& ln -s /usr/share/doc/littler/examples/testInstalled.r /usr/local/bin/testInstalled.r \
	&& install.r docopt \
	&& rm -rf /tmp/downloaded_packages/ /tmp/*.rds \
	&& rm -rf /var/lib/apt/lists/*

# Il presente dockerfile potrebbe essere spezzato qui per creare l'immagine intermedia r-base

# Install useful packages
RUN apt-get update
# RUN apt-get install -y apt-utils
# RUN apt-get install -y curl
# RUN apt-get install -y openssl
# RUN apt-get install -y libssl-dev
# RUN apt-get install -y libcurl4-openssl-dev
# # RUN apt-get install -y libmysqlclient-dev
# RUN apt-get install -y libmariadb-dev-compat
# RUN apt-get install -y libmariadbclient-dev-compat
# RUN apt-get install -y libmysql++-dev
# RUN apt-get install -y libpq-dev
# RUN apt-get install -y ncftp
# RUN apt-get install -y ssh
# RUN apt-get install -y libgdal-dev
# RUN apt-get install -y rsync
# RUN apt-get install -y libgtk2.0-dev

RUN apt-get install -y apt-utils curl openssl libssl-dev libcurl4-openssl-dev libmariadbclient-dev-compat \
      libmariadbclient-dev libmysql++-dev libpq-dev ncftp ssh libgdal-dev rsync libgtk2.0-dev libqt4-dev

# Copy locally necessary files
# COPY ./scripts /scripts
# COPY ./conf /conf
# RUN mkdir -p /development/c/navig
# COPY ./dev /development
# COPY ./dev/c /development/c
# COPY ./dev/c/navig /development/c/navig
# COPY ./dev/c/cumulata /development/c/cumulata
# COPY ./dev/c/minutes /development/c/minutes
# COPY ./dev/c/minutes/src /development/c/minutes/src
# COPY ./scripts/install_lib.R /usr/local/src/myscripts/
# RUN mkdir -p /development/c/msg/xrit-0.2.5
# COPY ./dev/c/msg/xrit-0.2.5 /development/c/msg/xrit-0.2.5
# RUN mkdir -p /data/rss && mkdir -p /data/modelli
# RUN mkdir -p /data/msg/tmp && mkdir -p /data/msg/archive \
#       && mkdir -p /data/msg/imgs && mkdir -p /data/msg/europe \
#       && mkdir -p /data/msg/analisi_msg && mkdir -p /data/msg/input
ADD ./elab_msg_batch.tar.xz /
COPY ./scripts/install_lib.R /usr/local/src/myscripts/
WORKDIR /usr/local/src/myscripts/

# Install R packages
RUN R -e "install.packages('RPostgreSQL', repos = 'http://cran.us.r-project.org')"
RUN R -e "install.packages('lubridate', repos = 'http://cran.us.r-project.org')"
RUN R -e "install.packages('jsonlite', repos = 'http://cran.us.r-project.org')"
RUN R -e "install.packages('curl', repos = 'http://cran.us.r-project.org')"
RUN R -e "install.packages('openssl', repos = 'http://cran.us.r-project.org')"
RUN R -e "install.packages('httr', repos = 'http://cran.us.r-project.org')"
RUN R -e "install.packages('fields', repos = 'http://cran.us.r-project.org')"
RUN R -e "install.packages('maps', repos = 'http://cran.us.r-project.org')"
RUN R -e "install.packages('raster', repos = 'http://cran.us.r-project.org')"
RUN R -e "install.packages('rgdal', repos = 'http://cran.us.r-project.org')"
RUN R -e "install.packages('RMySQL', repos = 'http://cran.us.r-project.org')"
RUN R -e "install.packages('R2HTML', repos = 'http://cran.us.r-project.org')"

# Update system
RUN apt-get -y autoremove && \
      apt-get -y update --fix-missing && \
      apt-get -y upgrade --fix-missing && \
      apt-get -y autoremove && \
      apt-get -y dist-upgrade && \
      apt-get -y autoremove

# Must install Qt4/5
# RUN apt-get -y install libqt4-dev

#  1) Change directory to /development/c/navig
#  2) Build navig c executable
#  3) Copy executable file in /bin
#  4) Change directory to /development/c/cumulata
#  5) Build cumulata c executable
#  6) Copy executable file in /bin
#  7) Change directory to /development/c/minutes
#  8) Build minutes c executable
#  9) Change directory to development/c/jpeg812_src/
# 10) and build libjpeg812
# 11) Copy libjpeg812.so to /lib

RUN cd /development/c/navig && \
      make && \
      cp /development/c/navig/navig /bin && \
      cd /development/c/cumulata && \
      make && \
      cp /development/c/cumulata/cumulata /bin && \
      cd /development/c/minutes && \
      make && \
      cp /development/c/minutes/minutes /bin && \
      cd /development/c/jpeg812_src && \
      make && \
      cp /development/c/jpeg812_src/libjpeg812.so /lib
      
# Copy executable file in /bin
# RUN cp /development/c/navig/navig /bin

# Change directory to /development/c/cumulata
# Build cumulata c executable
# RUN cd /development/c/cumulata && make

# Copy executable file in /bin
# RUN cp /development/c/cumulata/cumulata /bin

# Change directory to /development/c/minutes
# Build minutes c executable
# RUN cd /development/c/minutes && make

# Copy executable file in /bin
# RUN cp /development/c/minutes/minutes /bin

# Change directory to development/c/jpeg812_src/
# and build libjpeg812
# RUN cd /development/c/jpeg812_src && make

# Copy libjpeg812.so to /lib
# RUN cp /development/c/jpeg812_src/libjpeg812.so /lib

# Copy libwvt.so to /lib
COPY ./development/c/decompr_64/libwvt.so /lib

# Change directory to /development/c/xrit2pic/sgtk/sgtk_src and make
# RUN cd /development/c/xrit2pic/sgtk/sgtk_src && make 

# 1) Change directory to /development/c/xrit2pic/sgtk/sgtk_src and make
# 2) Copy /development/c/msg/xrit-0.2.5/bin/put_xrit_segments_together.pl, 
#    /development/c/msg/bin/read_seviri_prologue,
#    /development/c/msg/bin/read_xrit_header,
#    /development/c/msg/bin/xrit2raw            to /bin
# 3) Copy /development/c/msg/xrit-0.2.5/lib/libxrit.a to /lib
# 4) Setup linker path
# 5) Change directory to /development/c/xrit2pic/xrit/main_src and make
# 6) Copy /development/c/xrit2pic/xrit/main_src/xrit2pic to /bin
# 7) Change directory to /development/c/msg/xrit-0.2.5 && make clean && make
RUN cd /development/c/xrit2pic/sgtk/sgtk_src \
      && make \
      && cp /development/c/msg/xrit-0.2.5/bin/put_xrit_segments_together.pl /bin \
      && cp /development/c/msg/xrit-0.2.5/bin/read_seviri_prologue /bin \
      && cp /development/c/msg/xrit-0.2.5/bin/read_xrit_header /bin \
      && cp /development/c/msg/xrit-0.2.5/bin/xrit2raw /bin \
      && cp /development/c/msg/xrit-0.2.5/lib/libxrit.a /lib \
      && touch /etc/ld.so.conf.d/xrit2piclibs.conf \
      && echo "/lib" > /etc/ld.so.conf.d/xrit2pic.conf \
      && ldconfig \
      && cd /development/c/xrit2pic/xrit/main_src \
      && make \
      && cp /development/c/xrit2pic/xrit/main_src/xrit2pic /bin \
      && cd /development/c/msg/xrit-0.2.5 && make clean \
      && make

# Copy /development/c/msg/xrit-0.2.5/lib/libxrit.a to /lib
# RUN cp /development/c/msg/xrit-0.2.5/lib/libxrit.a /lib

# Setup linker path
# RUN touch /etc/ld.so.conf.d/xrit2piclibs.conf && echo "/lib" > /etc/ld.so.conf.d/xrit2pic.conf && ldconfig

# Change directory to /development/c/xrit2pic/xrit/main_src and make
# RUN cd /development/c/xrit2pic/xrit/main_src && make 

# Copy /development/c/xrit2pic/xrit/main_src/xrit2pic to /bin
# RUN cp /development/c/xrit2pic/xrit/main_src/xrit2pic /bin

# Change directory to /development/c/msg/xrit-0.2.5 && make clean && make
# RUN cd /development/c/msg/xrit-0.2.5 && make clean && make

#RUN /bin/navig
#RUN /bin/cumulata
#RUN /bin/minutes
#RUN /bin/xrit2pic -h

#COPY ./conf/id_rsa* /root/.ssh/
#COPY ./conf/known_hosts /root/.ssh
#RUN gcc /prisma/src/cumula_ora.c -o /prisma/cumula_ora
WORKDIR /scripts

