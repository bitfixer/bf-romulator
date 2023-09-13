brew install python
brew install git
brew install wget
sudo pip3 install apio==0.8.1
apio install -a
git clone https://github.com/bitfixer/bf-romulator.git
cd bf-romulator; git submodule init; git submodule update; cd ..
cd bf-romulator/cc65; make
