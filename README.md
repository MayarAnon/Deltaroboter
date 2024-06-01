# Delta Robot Project

Welcome to the Delta Robot Project repository. This project is developed as part of a DHBW Mosbach initiative, undertaken by two Infortronics students. We were responsible for the software and electronics aspects of the delta robot. The project demonstrates a comprehensive approach to robot control and simulation, integrating advance software engineering and electronic design.

## Project Overview

This project utilizes a modular web application architecture, developed using React, which supports highly scalable and maintainable code. The system is designed around a microservices architecture, enhancing flexibility and allowing individual services to be developed, deployed, and scaled independently.

Key components of the application communicate in real-time through WebSockets for instantaneous user interaction and MQTT for reliable inter-service messaging. This architecture not only facilitates robust, real-time communication but also ensures that the system can easily adapt to changes in processing demands or functionality.

![Architecture Diagram](Docs/architecture.jpg)

## Features

- **Real-time Control and Visualization**: Control and observe the delta robot within an interactive 3D environment.
- **G-Code Support**: Load, edit, and execute G-Code to control the robot movements.
- **Manual Control**: Direct control over the robot's movements through manual inputs.
- **System State Monitoring**: Monitor and display the current state of the robot.

## Technologies

- **Frontend**:
   - **React**: Used for building the web app interface.
   - **Recoil**: Manages state within the app, providing a more dynamic user experience.
   - **ThreeJS**: Utilized for rendering the digital twin in a 3D environment.
   - **WebSocket**: Facilitates real-time communication between the backend and frontend.

- **Backend**:
   - **Express**: Serves as the HTTP server and manages the WebSocket server for real-time web communication.
   - **MQTT (Mosquitto and Paho)**: Manages communication between all backend services, ensuring efficient message dispatch and handling.
   - **PIGPIO**: Allows for advanced real-time GPIO control using Direct Memory Access in C, optimizing performance for critical operations.
   - **RPI.GPIO**: Used for general GPIO control via Python, providing a flexible interface for less time-sensitive tasks.


## Installation

Follow these steps to set up the project locally:

### Preparing the Raspberry Pi SD Card
1. **Create a Raspberry Pi Image on an SD Card**  
   The Raspberry Pi is preconfigured as follows:
   - Hostname: deltarobot
   - User: pi
   - Password: raspberry
   - SSH authentication via password
   - WLAN network name: Hotspot
   - WLAN network password: ChangeMe
   - Location: DE

### Configuring the Raspberry Pi as an Access Point (AP-STA Mode)
2. **Configure the Raspberry Pi as an Access Point**
   - Connect to the Raspberry Pi via the preconfigured "Hotspot" WLAN network and then via SSH.
   - Install RaspAP to set up the Raspberry Pi to operate simultaneously as both an Access Point (AP) and a Wireless Client/Station (STA):
     ```bash
     curl -sL https://install.raspap.com | bash
     ```
   - After installation, reboot the Raspberry Pi. And connect again over the preconfigured wlan-network.
   - Access the admin interface via deltarobot.local or via the IP address (username: admin, password: secret).
   - Configure the Raspberry Pi for AP-STA mode:
     - Ensure the Wireless Client dashboard widget shows an active connection.
     - Go to Hotspot > Advanced and enable the "WiFi client AP mode" option.
     - You may also change the static IP address, the SSID, and the password. (The preconfigured static IP address is 192.168.50.1. If you configure another one, make sure to update the IP in Frontend/.env and in Backend/Webserver/server.js.)
   - After another reboot, the Raspberry Pi AP-SSID should appear (default SSID: raspi-webgui, password: ChangeMe).

### Software Installation and Configuration
3. **Install and configure Git**
   - Install Git and configure your user information:
     ```bash
     sudo apt install git
     git config --global user.name "Your Name"
     git config --global user.email "your.email@example.com"
     ```
4. **Clone the Repository**
   ```bash
   git clone https://github.com/MayarAnon/Deltaroboter.git
      ```
5. **setup the deltarobot**
Navigate to the script directory and run the setup script:
      ```bash
      cd Deltarobot 
      cd Bash  
      ./setup.sh
      ```
6. **add the deltarobot services**
Run the services script:
       ```bash
      cd Deltarobot 
      cd Bash  
      ./services.sh
      ```
7. sudo reboot
      ```bash
      sudo reboot
      ```
## Usage
After starting the application, you can access the web interface via http://192.168.50.1:3000 to control and observe the delta robot.
under http://192.168.50.1:80 you can access the admin interface for the accesspoint

## License
This project is licensed under the MIT License. See the LICENSE file for details.

## Authors
Dennis Roth & 
Mayar Hanhon
## Contact
For more information, please contact us.
