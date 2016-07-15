/***
 *         _   _   _ _____ ___  __  __    _  _____ ___ ____    _    
 *        / \ | | | |_   _/ _ \|  \/  |  / \|_   _|_ _/ ___|  / \   
 *       / _ \| | | | | || | | | |\/| | / _ \ | |  | | |     / _ \  
 *      / ___ \ |_| | | || |_| | |  | |/ ___ \| |  | | |___ / ___ \ 
 *     /_/   \_\___/  |_| \___/|_|  |_/_/   \_\_| |___\____/_/   \_\
 *     
 *     
 *     Automatica is an application for controlling the light and other electronics at home 
 *     over the internet.
 *     
 *     Automatica is using the MQTT framework and requires the use of Amazon IOT shadow system.
 *     It will update the shadow automatically each second (OBS! default value it can be changed.) 
 *     
 *     For changing the Amazon setup, go to the aws_iot_config.h and put in the values for your 
 *     project.
 *     
 *     Author: Oliver Nybroe (S153558@student.dtu.dk)
 *                                                            
 */


#include <aws_iot_mqtt.h>
#include "aws_iot_config.h"
#include <ArduinoJson.h>

aws_iot_mqtt_client myClient; // init iot_mqtt_client

/*
 * Device pins and setup.
 * 
 * Theese values can be changed to match your requirement.
 */
const int MOTION_PIN = 2; // Pin connected to motion detector
const int LIGHTSENSOR_PIN = A0; //pin connected to lightsensor
const int RELAY_PIN = 3; //pin connected to the relay

const int DELAY = 50; //The delay for each loop in ms.
int SEND_DATA_TIMER = 1000;// The delay between each message to be send in ms.
int MOTION_DETECT_MIN = 10; //The percentage of time the motion sensor should detect between each SEND_DATA_TIMER before it send there is motion.

/*
 * TEMP VALUES
 * 
 * DO NOT MODIFY!
 */
int lightSensorTemp = 0; //Temp store for lightsensor values.
int motionDetectionTemp = 0; //Temp store for detected motion
int loopCounter = 0; //Temp store for counting loops.


void setup() {
  pinMode(MOTION_PIN, INPUT_PULLUP); //Init the motion sensor as a pullup resistor.
  pinMode(RELAY_PIN, OUTPUT); //Init the relay.
  digitalWrite(RELAY_PIN,HIGH); //Turn off the relay.
  SEND_DATA_TIMER /= DELAY; //Translates SEND_DATA_TIMER in ms to number of rounds of delay.
  MOTION_DETECT_MIN = SEND_DATA_TIMER/MOTION_DETECT_MIN; //Translates percentage to rounds.

  // Set up the client
  if((myClient.setup(AWS_IOT_CLIENT_ID)) != 0){
    while(true);
  }
  // Load user configuration
  if((myClient.config(AWS_IOT_MQTT_HOST, AWS_IOT_MQTT_PORT, AWS_IOT_ROOT_CA_PATH, AWS_IOT_PRIVATE_KEY_PATH, AWS_IOT_CERTIFICATE_PATH)) != 0){
    while(true);
  }
  // Use default connect: 60 sec for keepalive
  if((myClient.connect()) != 0){
    while(true);
  }

  // initiate shadow for thing.
  if(myClient.shadow_init(AWS_IOT_MY_THING_NAME) !=0){
    while(true);
  }

  if(myClient.shadow_register_delta_func(AWS_IOT_MY_THING_NAME, delta_callback) != 0){
    while(true); 
  }

  //Serial.println("Setup completed");

  // Delay to make sure SUBACK is received, delay time could vary according to the server
  delay(2000);


}

void delta_callback(char* src, unsigned int len, Message_status_t flag) {
  //Return if the delta message is corrupted.
  if(flag != STATUS_NORMAL) {
    return;
  }
 
  // Return if the delta key doesn't exist and binds the buffer to relay 1 state.
  char JSON_buf[100]; //bufer
  if(myClient.getDeltaValueByKey(src, "Power\"1\"state", JSON_buf, 100) != 0){
    return; 
  }
  String json_received = JSON_buf;

  //if relay 1 delta state is false turn off relay and update shadow.
  if(json_received == "false"){
    digitalWrite(RELAY_PIN,HIGH); //Turn off the relay.
    String json = "{\"state\":{\"reported\":{\"Power\":{\"1\":{\"state\":false}}}}}";
    json.toCharArray(JSON_buf, 100);
    myClient.shadow_update(AWS_IOT_MY_THING_NAME, JSON_buf, strlen(JSON_buf), NULL, 5);
  }
  // if relay 1 delte state is false turn on relay and update shadow.
  else if(json_received == "true"){
    digitalWrite(RELAY_PIN,LOW); //Turn on the relay.
    String json = "{\"state\":{\"reported\":{\"Power\":{\"1\":{\"state\":true}}}}}";
    json.toCharArray(JSON_buf, 100);
    myClient.shadow_update(AWS_IOT_MY_THING_NAME, JSON_buf, strlen(JSON_buf), NULL, 5);
  }



}

void loop() {
  /*
   * Main loop
   * 
   * This part of the loop runs each time the code loops.
   */
  //adds 1 to motionDection temp store when it detects.
  if (digitalRead(MOTION_PIN) == LOW) {
    motionDetectionTemp++;
  }
  //Adds lightsensor value to lightSensor temp store.
  lightSensorTemp += analogRead(LIGHTSENSOR_PIN);

  //Count up the loopCounter and wait the delay.
  loopCounter++;
  delay(DELAY); 

  
  //Run loop again if SEND_DATA_TIMER is not reached yet.
  if (loopCounter < SEND_DATA_TIMER) {
    return;
  }

  /*
   * Data sender
   * 
   * This part of the loop only runs when SEND_DATA_TIMER is reached.
   * This code calculates the last values and sends the data to AWS.
   * Runs the code for processing and sending the data to AWS.
   */

  //Process the data before sending it
  int motionDetected = motionDetectionTemp >= MOTION_DETECT_MIN; //Check if MOTION_DETECT_MIN is met.
  lightSensorTemp /= SEND_DATA_TIMER; //Calculates the average light over the given SEND_DATA_Timer time.

  //Creates the JSON object.
  StaticJsonBuffer<100> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  JsonObject& state = root.createNestedObject("state");
  JsonObject& reported = state.createNestedObject("reported");
  JsonObject& lightSensor = reported.createNestedObject("LightSensor");
  JsonObject& motionSensor = reported.createNestedObject("MotionSensor");
  lightSensor.set("Value", lightSensorTemp);
  motionSensor.set("Value", motionDetected);

  //Creates the JSON buffer
  char buffer[root.measureLength() + 1];
  root.printTo(buffer, sizeof(buffer));

  //Sends the data to the shadow.
  myClient.shadow_update(AWS_IOT_MY_THING_NAME, buffer, sizeof(buffer), NULL, 5);

  //Empties the data buffer and print of the yield messages.
  myClient.yield();

  //Resets the temp values.
  lightSensorTemp = 0;
  motionDetectionTemp = 0;
  loopCounter = 0;
}

