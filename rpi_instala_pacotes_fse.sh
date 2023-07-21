#! /bin/bash
echo "Instala pacotes FSE"

echo "Instalando o vim, tmux"
sudo apt install vim tmux -y

echo "Instalando a Biblioteca BCM2835 C/C++"
wget http://www.airspayce.com/mikem/bcm2835/bcm2835-1.71.tar.gz
tar -xvzf bcm2835-1.71.tar.gz
rm bcm2835-1.71.tar.gz
cd bcm2835-1.71/
./configure
make
sudo make check
sudo make install
cd ~

echo "Instalando a Biblioteca WiringPi em C/C++"
cd ~ 
git clone https://github.com/WiringPi/WiringPi.git
cd WiringPi
./build
cd ~

echo "Instalando a Biblioteca WiringPi em Python"
pip3 install wiringpi
pip install wiringpi

echo "Instala ncurses"
sudo apt-get install libncurses5-dev libncursesw5-dev -y