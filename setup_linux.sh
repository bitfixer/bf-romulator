apt-get -y update
apt-get -y python3-pip
apt-get -y install git
apt-get -y install wget
pip3 install -u apio==0.5.4
apio install -a
git clone https://github.com/bitfixer/bf-romulator.git
cd bf-romulator; git submodule init; git submodule update; cd ..
cd bf-romulator/cc65; make