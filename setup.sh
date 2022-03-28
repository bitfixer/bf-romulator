echo 'dtparam=spi=on' >> /boot/config.txt
apt-get -y update
apt-get -y install python3-pip
apt-get -y install git
apt-get -y install wget
apt-get -y install imagemagick
sudo -u pi git clone https://github.com/WiringPi/WiringPi
cd WiringPi; ./build; cd ..
pip3 install -U apio==0.5.4
sudo -u pi apio install -a
sudo -u pi git clone https://github.com/bitfixer/bf-romulator.git
cd bf-romulator; git submodule init; git submodule update; cd ..
cd bf-romulator/cc65; make
reboot