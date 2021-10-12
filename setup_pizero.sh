echo 'dtparam=spi=on' >> /boot/config.txt
apt-get -y update
apt-get -y install git
apt-get -y install wiringpi
apt-get -y install wget
sudo -u pi git clone https://github.com/bitfixer/bf-romulator.git
cd bf-romulator; git submodule init; git submodule update; cd ..
reboot
