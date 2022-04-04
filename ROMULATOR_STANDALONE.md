# Programming/Debug using Standalone Programmer

The standalone programmer for the ROMulator consists of a D1 Mini board and an interface board to connect the D1 Mini to the ROMulator's 10-pin header.
First step is to install the build tools on your Win/Mac/Linux machine. You will build new firmware images here. For Mac and Linux the install is fairly straightforward and just requires running a script. On Windows, however, some build dependencies are not natively supported, and requires installing a Linux distro using WSL (windows subsystem for linux). 
Instructions for each OS:

## Windows

Use the Microsoft Store to install a linux distribution with apt as a package manager. Either Ubuntu or Debian recommended.\
After installation, your windows filesystem is accessible in /mnt, i.e. C: is at /mnt/c/, etc.\
Create a directory for your installation somewhere easily accessible to both linux and windows, i.e. /mnt/c/home/Users/username/romulator. Change to this directory and follow the instructions for Linux at this point.

## Linux

Create a directory for your install.\
Then fetch and run the linux setup script from linux command line in this directory with (one line)\
```wget https://raw.githubusercontent.com/bitfixer/bf-romulator/master/setup_linux.sh; ./setup_linux.sh```\
You will need to enter your password as some commands in the setup script require sudo.

## Mac (10.15 or higher)

If not installed already, install homebrew using instructions here.:\
[homebrew installation](https://brew.sh)\
Create a directory for your romulator installation.\
Then open Terminal, change to the install directory, and run this (one line):\
```curl https://raw.githubusercontent.com/bitfixer/bf-romulator/master/setup_mac.sh > setup_mac.sh; chmod 755 setup_mac.sh; ./setup_mac.sh```
You will need to enter your password at some point during the installation.

