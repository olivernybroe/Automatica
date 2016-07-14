                                _   _   _ _____ ___  __  __    _  _____ ___ ____    _    
                               / \ | | | |_   _/ _ \|  \/  |  / \|_   _|_ _/ ___|  / \   
                              / _ \| | | | | || | | | |\/| | / _ \ | |  | | |     / _ \  
                             / ___ \ |_| | | || |_| | |  | |/ ___ \| |  | | |___ / ___ \ 
                            /_/   \_\___/  |_| \___/|_|  |_/_/   \_\_| |___\____/_/   \_\

# Automatica-IOT
This is a project for DTU IOT course.
It communicates with the Amazon IoT and uses the Amazon shadow.

# Requirements
  - Arduino Yún
  - Light sensor ([Only tested with Groove Light sensor v1.1](http://www.seeedstudio.com/wiki/Grove_-_Light_Sensor))
  - PIR sensor ([Only tested with PIR Motion Sensor (JST)](https://www.sparkfun.com/products/13285))
  - Relay single port
  - Amazon IOT account
  - Webserver (Can also be hosted local)

# Installation
  1. Setup the Arduino Yun WiFi and connect it to Amazon. [Follow all the steps on Amazon's SDK for Arduino yun](https://github.com/aws/aws-iot-device-sdk-arduino-yun#installation)
  2. Open the `Arduino/Automatica/aws_iot_config.example.h` file and setup it to match your settings.
  3. Open the `js/config.example`fiel and setup it to match your settings.
  4. Connect the sensors and relay to the Arduino.
  5. Change the ports for the devices connected to the Arduino, so it matches the settings in the `Arduino/Automatica/Automatica.ino`.
  6. `cd` to the folder and run `npm install` and `bower install` to install all the dependencies.
  7. Start the webserver and open up the `index.html` file and your are done.
