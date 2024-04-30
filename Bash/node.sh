#!/bin/bash

# Node installieren 

sudo apt install nodejs npm


# alle Packages installieren 

FrontendPath=$(realpath ../Frontend)

BackendPath=$(realpath ../Backend/WebServer)

cd $FrontendPath

npm install # installiert alle Benötigten Packages im Frontend 

cd $BackendPath 

npm install # installiert alle Benötigten Packages im Backend 

