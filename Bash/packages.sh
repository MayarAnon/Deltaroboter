sudo apt-get install cmake
git clone https://github.com/DaveGamble/cJSON.git
cd cJSON
mkdir build
cd build
cmake ..
make
sudo make install

cd ..

cd ..

sudo apt-get install build-essential git
git clone https://github.com/eclipse/paho.mqtt.c.git
cd paho.mqtt.c
make
sudo make install

