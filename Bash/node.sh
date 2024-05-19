#!/bin/bash

# Node installieren 

sudo apt install -y nodejs npm


# alle Packages installieren 

FrontendPath=$(realpath ../Frontend)

BackendPath=$(realpath ../Backend/WebServer)

cd $FrontendPath

npm install --no-optional --silent # installiert alle Benötigten Packages im Frontend 

npm run build

cd $BackendPath 

npm install --no-optional --silent # installiert alle Benötigten Packages im Backend 

