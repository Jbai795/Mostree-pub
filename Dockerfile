FROM ubuntu:20.04
ENV TZ=Europe/Amsterdam
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

RUN apt-get update \
    && apt-get install --no-install-recommends -y \
	git \
	python3 \
	wget \
	build-essential \ 
	libssl-dev \
	g++ \
	python-dev \
	autotools-dev \
	libicu-dev \
	libbz2-dev \
	libboost-all-dev \
	libgmp-dev \
	sudo \
	iproute2 \
	iputils-ping \
	vim 

## CMAKE
WORKDIR /home/mostree
RUN wget --no-check-certificate https://github.com/Kitware/CMake/releases/download/v3.27.3/cmake-3.27.3.tar.gz \
&& tar -zxvf cmake-3.27.3.tar.gz \
&& cd cmake-3.27.3 \
&& ./bootstrap \
&& make \
&& make install

## BOOST
WORKDIR /tmp/boost
RUN wget --no-check-certificate https://boostorg.jfrog.io/artifactory/main/release/1.75.0/source/boost_1_75_0.tar.bz2 \
&& tar -xvf boost_1_75_0.tar.bz2 \
&& cd boost_1_75_0 \
&& ./bootstrap.sh --prefix=/usr/ \
&& ./b2 install

##NTL
WORKDIR /tmp/ntl
RUN wget --no-check-certificate https://libntl.org/ntl-11.5.1.tar.gz \
&& gunzip ntl-11.5.1.tar.gz \
&& tar xf ntl-11.5.1.tar \
&& cd ntl-11.5.1/src \
&& ./configure \
&& make \
&& make install

RUN apt-get install --reinstall ca-certificates -y \
&& mkdir /usr/local/share/ca-certificates/cacert.org \
&& wget -P /usr/local/share/ca-certificates/cacert.org http://www.cacert.org/certs/root.crt http://www.cacert.org/certs/class3.crt \
&& update-ca-certificates \
&& git config --global http.sslCAinfo /etc/ssl/certs/ca-certificates.crt

##ABY3
RUN git clone https://github.com/ladnir/aby3.git /home/mostree/aby3 
WORKDIR /home/mostree/aby3
RUN python3 build.py --setup \
	&& python3 build.py 

##BUILD
WORKDIR /home/mostree
RUN wget --no-check-certificate https://raw.githubusercontent.com/emp-toolkit/emp-readme/master/scripts/install.py \
&& python install.py --deps --tool \
&& git clone https://github.com/Jbai795/Mostree-pub.git \
&& cd Mostree-pub \
&& python3 build.py