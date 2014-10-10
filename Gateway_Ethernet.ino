/*
Author:  Eric Tsai
License:  CC-BY-SA, https://creativecommons.org/licenses/by-sa/2.0/
Date:  10-10-2014
File: Ethernet_Gateway.ino
This sketch takes data struct from I2C and publishes it to
mosquitto broker. It also subscribes to topics and outputs 
the topic and message to the serial monitor.

Modifications Needed:
1)  Update mac address "mac[]"
2)  Update MQTT broker IP address "server[]"
3.  Update the IP address of this device
4.  Update the 2 subscription areas
*/

#include <SPI.h>
#include <Ethernet.h>
#include <Wire.h>
#include <PubSubClient.h>

//I2C receive device address
const byte MY_ADDRESS = 42;    //I2C comms w/ other Arduino

//Ethernet
byte mac[]    = {  0x90, 0xA2, 0xDA, 0x0D, 0x11, 0x12 }; // assign mac address to this device. It can be anything, use the one on your ethernet shield if it has one, otherwise make it up. Just make sure it's unique on your network.
byte server[] = { 192, 168, 1, 143 }; //IP address of the MQTT server. 
IPAddress ip(192,168,1,107); //IP address of this device


//EthernetClient ethClient;
EthernetClient ethClient;
PubSubClient client(server, 1883, callback, ethClient);

// Used to convert millis() to seconds, for debugging MQTT connection status
unsigned long time=0;

  // MQTT Callback function. This is where the programming goes to respond to payloads on topics you've subscribed to.
  // currently it just prints the topic and payload out to the serial monitor.
  // TODO: figure out how to read the payload, parse it out to NodeID, DeviceID, Command, and send it via I2c to the RFM
  //       gateway to be radio transmitted to the nodes.
void callback(char* topic, byte* payload, unsigned int length)
{
  for (int i=0; i<length; i++) {
    Serial.print((char)payload[i]);
  }
    Serial.println();
}


void setup() 
{

  Wire.begin (MY_ADDRESS);                        // open communication to the RFM gateway over I2c
  Serial.begin (9600);                            // open the serial connection to the console for debugging
  
        Serial.println("starting... long delay means problem with ethernet");
  
  //wait for IP address
    while (Ethernet.begin(mac) != 1)              // while there's no ethernet connection, throw an error message and wait 3 seconds
      {
        Serial.println("Error getting IP address via DHCP, trying again...");
        delay(3000);
      }
  
        Serial.println("ethernet OK");
 
  Wire.onReceive (receiveEvent);                  // On receiving i2c data, call receiveEvent interrupt function at the end of sketch to process incomming data 
   
  while (client.connect("arduinoClient") != 1)    // while there's no MQTT connection, throw an error message and wait 3 seconds
      {
        Serial.println("Error connecting to MQTT");
        delay(3000);
      }
        Serial.println("setup complete");


// ====================================================================================     
// Enter MQTT subscriptions here and also down in the loop.
// The ones here will be active as soon as void setup finishes, until the MQTT reconnection happens
// P.S. use "mosquitto_sub -h 192.168.xxx.xxx -t "#" -v" command line to view all topics in mosquitto


        client.subscribe("1111");
        client.subscribe("2222");
        client.subscribe("3333");
        client.subscribe("4444");

        client.publish("Arduino","Ethernet MQTT Gateway Up");
        
      
    
  

// ====================================================================================     


}  // end of setup



volatile struct 
   {
  int                   nodeID;
  int			sensorID;		
  unsigned long         var1_usl;
  float                 var2_float;
  float			var3_float;		//
  int                   var4_int;
   } SensorNode;

int sendMQTT = 0;
volatile boolean haveData = false;

void loop() 
{
   if (!client.loop()) 
     {
 
       Serial.print("Client disconnected...");

      if (client.connect("arduinoPublisher"))
        {
          Serial.print("reconnected.");
// ====================================================================================     
// Enter MQTT subscriptions here also.
// These will be refreshed when the MQTT client disconnect/reconnect happens and so stay responsive
// P.S. use "mosquitto_sub -h 192.168.xxx.xxx -t "#" -v" command line to view all topics in mosquitto


        client.subscribe("1111");
        client.subscribe("2222");
        client.subscribe("3333");
        client.subscribe("4444");

        client.publish("Arduino","Ethernet MQTT Gateway Up");
        
      
    
  

// ====================================================================================     
        } 
      
      else
        {
          Serial.print("failed.");
            time = millis();
            Serial.println(time/1000);
        }
    
  
     }                                // end if client.loop
   
     
  if (haveData)                      // "recieveEvent" interrupt routine sets this "haveData" variable true when it's complete so the code progresses beyond it
  {

    Serial.println ();
    Serial.print ("Received Node ID      = ");
    Serial.println (SensorNode.nodeID);  
    Serial.print ("Received Device ID    = ");
    Serial.println (SensorNode.sensorID);  
    Serial.print ("1. Time               = ");
    Serial.print (SensorNode.var1_usl);
    Serial.print (" millis, ");
    Serial.print (SensorNode.var1_usl/1000);
    Serial.print (" seconds, or ");
    Serial.print ((SensorNode.var1_usl / 1000) /60);
    Serial.println (" minutes");
    Serial.print ("2. var2_float         = ");
    Serial.println (SensorNode.var2_float);
    Serial.print ("3. var3_float         = ");
    Serial.println (SensorNode.var3_float);    
    Serial.print ("4. RSSI               = ");
    Serial.println (SensorNode.var4_int); 


      int varnum;
      char buff_topic[6];
      char buff_message[12];      

      
    Serial.println (); 

      //send var1_usl - This is used to pass millis() value at the time the sensor reading was transmitted
      varnum = 1;
      buff_topic[6];
      buff_message[12];
      sprintf(buff_topic, "%02d%01d%01d", SensorNode.nodeID, SensorNode.sensorID, varnum);
      Serial.print("MQTT publish topic: "); 
      Serial.println(buff_topic);
      dtostrf (SensorNode.var1_usl, 10, 1, buff_message);
      client.publish(buff_topic, buff_message);
      
      
      //send var2_float - sensor data
      varnum = 2;
      buff_topic[6];
      buff_message[7];
      sprintf(buff_topic, "%02d%01d%01d", SensorNode.nodeID, SensorNode.sensorID, varnum);
      Serial.print("MQTT publish topic: "); 
      Serial.println(buff_topic);
      dtostrf (SensorNode.var2_float, 2, 1, buff_message);
      client.publish(buff_topic, buff_message);
      
      delay(200);
      
      //send var3_float - sensor data
      varnum = 3;
      sprintf(buff_topic, "%02d%01d%01d", SensorNode.nodeID, SensorNode.sensorID, varnum);
      Serial.print("MQTT publish topic: "); 
      Serial.println(buff_topic);
      dtostrf (SensorNode.var3_float, 2, 1, buff_message);
      client.publish(buff_topic, buff_message);

      delay(200);
      
      //send var4_int, RSSI - This passess on the node's RSSI at the time of sending 
      varnum = 4;
      sprintf(buff_topic, "%02d%01d%01d", SensorNode.nodeID, SensorNode.sensorID, varnum);
      Serial.print("MQTT publish topic: "); 
      Serial.println(buff_topic);
      sprintf(buff_message, "%04d%", SensorNode.var4_int);
      client.publish(buff_topic, buff_message);

      Serial.println ();
      Serial.println("finished MQTT send");
      Serial.println ();
          
  haveData = false;  // set "haveData" false since the data taken from I2C, parsed by the recieveEvent interrupt, has been published to MQTT
  
}  // end if haveData

}  // end of loop




// called by interrupt service routine when incoming data arrives
void receiveEvent (int howMany)
{
  if (howMany < sizeof SensorNode)
    return;
    
  // read into structure
  byte * p = (byte *) &SensorNode;
  for (byte i = 0; i < sizeof SensorNode; i++)
    *p++ = Wire.read ();
    
  haveData = true;     // once "haveData" is set true, the sketch will proceed to publish the data to MQTT
}
