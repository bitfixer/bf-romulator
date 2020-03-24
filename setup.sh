sudo echo 'dtparam=spi=on' >> /boot/config.txt
sudo apt-get -y update
sudo apt-get -y install python3-pip
sudo apt-get -y install git
sudo apt-get -y install wiringpi
sudo apt-get -y install wget
sudo apt-get -y install imagemagick
sudo pip3 install -U apio==0.4.1
apio install -a
git clone https://github.com/bitfixer/bf-romulator.git