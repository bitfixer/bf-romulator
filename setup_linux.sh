sudo apt-get -y update
sudo apt-get -y install python3-pip
sudo apt-get -y install git
sudo apt-get -y install wget
sudo pip3 install apio==0.5.4
apio install -a
git clone https://github.com/bitfixer/bf-romulator.git
cd bf-romulator; git submodule init; git submodule update; cd ..
cd bf-romulator/cc65; make