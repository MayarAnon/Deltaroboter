#!/bin/bash

#Mosquitto herunterladen 
sudo apt install mosquitto mosquitto-clients -y

#config Datei austauschen 

sudo rm /etc/mosquitto/mosquitto.conf

sudo cp mosquitto.conf /etc/mosquitto

#mosquitto neustarten 

sudo service mosquitto restart


