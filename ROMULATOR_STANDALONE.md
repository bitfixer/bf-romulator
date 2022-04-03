# Programming/Debug using Standalone Programmer

The standalone programmer for the ROMulator consists of a D1 Mini board and an interface board to connect the D1 Mini to the ROMulator's 10-pin header.
First step is to install the build tools on your Win/Mac/Linux machine. You will build new firmware images here. For Mac and Linux the install is fairly straightforward and just requires running a script. On Windows, however, some build dependencies are not natively supported, and requires installing a Linux distro using WSL (windows subsystem for linux). 
Instructions for each OS:

## Windows

Use the Microsoft Store to install a linux distribution with apt as a package manager. Either Ubuntu or Debian recommended.\
After installation, your windows filesystem is accessible in /mnt, i.e. C: is at /mnt/c/, etc.\
Create a directory for your installation somewhere easily accessible to both linux and windows, i.e. /mnt/c/home/Users/username/romulator. Change to this directory and follow the instructions for Linux at this point.

## Linux

Fetch the linux setup script from linux command line with\
```wget https://raw.githubusercontent.com/bitfixer/bf-romulator/master/setup_linux.sh```\
then run the script:\
```./setup_linux.sh```\
You will need to enter your password as some commands in the setup script require sudo.

## Mac (10.15 or higher)

If not installed already, install homebrew using instructions here:\
(https://raw.githubusercontent.com/bitfixer/bf-romulator/master/setup.sh)[https://raw.githubusercontent.com/bitfixer/bf-romulator/master/setup.sh]
Then open Terminal, change to a directory where you want to install, and download the install script with:\
```curl https://raw.githubusercontent.com/bitfixer/bf-romulator/master/setup_linux.sh > setup_linux.sh```\
and run the script:\
```./setup_mac.sh```



