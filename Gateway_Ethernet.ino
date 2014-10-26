/*
 Based on work from author:  Eric Tsai
 Gateway ncorporating both the RFM69 and the ethernet part
 Revised by Alexandre Bouillot
 
 License:  CC-BY-SA, https://creativecommons.org/licenses/by-sa/2.0/
 Date:  10-23-2014
 File: Gateway.ino
 This sketch receives RFM wireless data and forwards it to Mosquitto relay.
 
 It also subscribes to MQTT topics and sends the payload on to nodes. The payload should be in the format 
 "node#","device#","value#". For ex. 15,01,01 would send to node 15 only.
 
 Modifications Needed:
 1)  Update encryption string "ENCRYPTKEY"
 2)  Adjust SS - Chip Select - for RFM69
 3)  Adjust MQTT server address
 */

/*
RFM69 Pinout:
 MOSI = 11
 MISO = 12
 SCK = 13
 SS = 8
 */

/*
Ethernet Pinout:
 MOSI = 11
 MISO = 12
 SCK = 13
 SS = 10
 */

//general --------------------------------
#define SERIAL_BAUD   115200
#if 1
#define DEBUG1(expression)  Serial.print(expression)
#define DEBUG2(expression, arg)  Serial.print(expression, arg)
#define DEBUGLN1(expression)  Serial.println(expression)
#else
#define DEBUG1(expression)
#define DEBUG2(expression, arg)
#define DEBUGLN1(expression)
#endif
//RFM69  ----------------------------------
#include <RFM69.h>
#include <SPI.h>
#define NODEID        2    //unique for each node on same network
#define NETWORKID     101  //the same on all nodes that talk to each other
#define FREQUENCY   RF69_433MHZ
//#define FREQUENCY   RF69_868MHZ
//#define FREQUENCY     RF69_915MHZ
#define ENCRYPTKEY    "1111111111111111" //exactly the same 16 characters/bytes on all nodes!
#define IS_RFM69HW    //uncomment only for RFM69HW! Leave out if you have RFM69W!
#define ACK_TIME      30 // max # of ms to wait for an ack
#define RFM69_SS  8
RFM69 radio(RFM69_SS);
bool promiscuousMode = false; //set to 'true' to sniff all packets on the same network

#include <Ethernet.h>

//Ethernet
byte mac[]    = {  
  0x90, 0xA2, 0xDA, 0x0D, 0x11, 0x12 };
byte server[] = { 
  192, 168, 1, 143 };

IPAddress ip(192,168,1,161);
EthernetClient ethClient;
#define DHCP_RETRY 500

// Mosquitto---------------
#include <PubSubClient.h>
PubSubClient client(server, 1883, callback, ethClient);
#define MQTT_CLIENT_ID "arduinoClient"
#define MQTT_RETRY 500
int sendMQTT = 0;

void MQTTSendInt(PubSubClient* _client, int node, int sensor, int var, int val);
void MQTTSendLong(PubSubClient* _client, int node, int sensor, int var, long val);

//use LED for indicating MQTT connection status.
int led = 13;

typedef struct {		
  int                   nodeID; 
  int			sensorID;
  unsigned long         var1_usl; 
  float                 var2_float; 
  float			var3_float;	
} 
Payload;
Payload theData;

volatile struct 
{
  int                   nodeID;
  int			sensorID;		
  unsigned long         var1_usl;
  float                 var2_float;
  float			var3_float;		//
  int                   var4_int;
} 
SensorNode;

void setup() 
{
  Serial.begin(SERIAL_BAUD); 

  //Ethernet -------------------------
  //Ethernet.begin(mac, ip);

  //wait for IP address
  while (Ethernet.begin(mac) != 1) {
    DEBUGLN1("Error getting IP address via DHCP, trying again...");
    delay(DHCP_RETRY);
  }

  DEBUGLN1("ethernet OK");
  // print your local IP address:
  DEBUGLN1("My IP address: ");
  for (byte thisByte = 0; thisByte < 4; thisByte++) {
    // print the value of each byte of the IP address:
    DEBUG2(Ethernet.localIP()[thisByte], DEC);
    DEBUG1("."); 
  }
  DEBUGLN1();

  // Mosquitto ------------------------------
  while (client.connect(MQTT_CLIENT_ID) != 1) {
    DEBUGLN1("Error connecting to MQTT");
    delay(MQTT_RETRY);
  }
  // ====================================================================================     
  // Enter MQTT subscriptions here and also down in the loop.
  // The ones here will be active as soon as void setup finishes, until the MQTT reconnection happens
  // P.S. use "mosquitto_sub -h 192.168.xxx.xxx -t "#" -v" command line to view all topics in mosquitto


        client.subscribe("1111");
        client.subscribe("2222");
        client.subscribe("3333");
        client.subscribe("4444");

        client.publish("Arduino","Ethernet MQTT Gateway Up");
        
      
  //RFM69 ---------------------------
  radio.initialize(FREQUENCY,NODEID,NETWORKID);
#ifdef IS_RFM69HW
  radio.setHighPower(); //uncomment only for RFM69HW!
#endif
  radio.encrypt(ENCRYPTKEY);
  radio.promiscuous(promiscuousMode);
  char buff[50];
  sprintf(buff, "\nListening at %d Mhz...", FREQUENCY==RF69_433MHZ ? 433 : FREQUENCY==RF69_868MHZ ? 868 : 915);
  DEBUGLN1(buff);

  DEBUGLN1("setup complete");
}  // end of setup

byte ackCount=0;
long watchdogInterval = 2000;
long watchdog = 0;

void loop() {
  
  // calling client.loop too often block the system at some point quite early (~up to 5 loop)
  // Here is a temporized call to it on a regular interval
  // This need to be as fast as the fastest sensor received
  if (millis() > watchdog) {
//    Serial.print("loop "); 
//    Serial.println(millis());
    watchdog += watchdogInterval;
    //client.loop needs to run every iteration.  Previous version did not.  Big opps.
   // client.loop();
// }



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
        }
    
     }
     }                                // end if client.loop


  if (radio.receiveDone()) {
    DEBUG1('[');
    DEBUG2(radio.SENDERID, DEC);
    DEBUG1("] ");
    if (promiscuousMode) {
      DEBUG1("to [");
      DEBUG2(radio.TARGETID, DEC);
      DEBUG1("] ");
    }
    DEBUGLN1();

    if (radio.DATALEN != sizeof(Payload))
      Serial.println(F("Invalid payload received, not matching Payload struct!"));
    else {
      theData = *(Payload*)radio.DATA; //assume radio.DATA actually contains our struct and not something else

      DEBUG1(theData.sensorID);
      DEBUG1(", ");
      DEBUG1(theData.var1_usl);
      DEBUG1(", ");
      DEBUG1(theData.var2_float);
      DEBUG1(", ");
      DEBUG1(" var2(temperature)=");
      DEBUG1(", ");
      DEBUG1(theData.var3_float);

      //printFloat(theData.var2_float, 5); Serial.print(", "); printFloat(theData.var3_float, 5);

      DEBUG1(", RSSI= ");
      DEBUGLN1(radio.RSSI);

      //save it for i2c:
      SensorNode.nodeID = theData.nodeID;
      SensorNode.sensorID = theData.sensorID;
      SensorNode.var1_usl = theData.var1_usl;
      SensorNode.var2_float = theData.var2_float;
      SensorNode.var3_float = theData.var3_float;
      SensorNode.var4_int = radio.RSSI;

      /*
      DEBUG1("Received Device ID = ");
      DEBUGLN1(SensorNode.sensorID);  
       DEBUG1 ("    Time = ");
       DEBUGLN1 (SensorNode.var1_usl);
       DEBUG1 ("    var2_float ");
       DEBUGLN1 (SensorNode.var2_float);
       */
      sendMQTT = 1;
    }


    if (radio.ACK_REQUESTED)
    {
      byte theNodeID = radio.SENDERID;
      radio.sendACK();

      // When a node requests an ACK, respond to the ACK
      // and also send a packet requesting an ACK (every 3rd one only)
      // This way both TX/RX NODE functions are tested on 1 end at the GATEWAY
      if (ackCount++%3==0)
      {
        //Serial.print(" Pinging node ");
        //Serial.print(theNodeID);
        //Serial.print(" - ACK...");
        //delay(3); //need this when sending right after reception .. ?
        //if (radio.sendWithRetry(theNodeID, "ACK TEST", 8, 0))  // 0 = only 1 attempt, no retries
        //  Serial.print("ok!");
        //else Serial.print("nothing");
      }
    }//end if radio.ACK_REQESTED
  } //end if radio.receive

  if (sendMQTT == 1) {
    DEBUGLN1("starting MQTT send");

    if (!client.connected()) {
      while (client.connect(MQTT_CLIENT_ID) != 1)
      {
        digitalWrite(led, LOW);
        DEBUGLN1("Error connecting to MQTT");
        delay(500);
        digitalWrite(led, HIGH);
      }
      client.publish("outTopic","hello world");
    } 

    digitalWrite(led, HIGH);
    
    int varnum;
    char buff_topic[6];
    char buff_message[12];      

    
      //send var1_usl - This is used to pass millis() value at the time the sensor reading was transmitted
     varnum = 1;
     buff_topic[6];
     buff_message[12];
     sprintf(buff_topic, "%02d%01d%01d", SensorNode.nodeID, SensorNode.sensorID, varnum);
     Serial.println(buff_topic);
     dtostrf (SensorNode.var1_usl, 10, 1, buff_message);
     client.publish(buff_topic, buff_message);
     

    //send var2_float - sensor data
    MQTTSendLong(&client, SensorNode.nodeID, SensorNode.sensorID, 2, SensorNode.var2_float);

    //send var3_float - sensor data
    MQTTSendInt(&client, SensorNode.nodeID, SensorNode.sensorID, 3, SensorNode.var3_float);

    //send var4_int, RSSI
    MQTTSendInt(&client, SensorNode.nodeID, SensorNode.sensorID, 4, SensorNode.var4_int);

    sendMQTT = 0;
    DEBUGLN1("finished MQTT send");
    digitalWrite(led, LOW);
  }//end if sendMQTT
}//end loop

void MQTTSendInt(PubSubClient* _client, int node, int sensor, int var, int val) {
    char buff_topic[6];
    char buff_message[7];

    sprintf(buff_topic, "%02d%01d%01d", node, sensor, var);
    sprintf(buff_message, "%04d%", val);
    _client->publish(buff_topic, buff_message);
}

void MQTTSendLong(PubSubClient* _client, int node, int sensor, int var, long val) {
    char buff_topic[6];
    char buff_message[7];

    sprintf(buff_topic, "%02d%01d%01d", node, sensor, var);
    dtostrf (val, 2, 1, buff_message);
    _client->publish(buff_topic, buff_message);
}

// Handing of Mosquitto messages
void callback(char* topic, byte* payload, unsigned int length) {
  // handle message arrived
  DEBUGLN1(F("Mosquitto Callback"));

Serial.println("received MQTT");
Serial.print(topic);

payload[length] = '\0';

//convert payload to string
String strPayload = String((char*)payload); //returns number or text string exactly as input
  
Serial.print(" = ");
Serial.println(strPayload); //returns number or text string exactly as input


// -------------------------------------------------------------
//Strings have a method called indexOf() which allows you to search for the index in the String's character array of a particular character. If the character is not found, the method should return -1. A second parameter can be added to the function call to indicate a starting point for the search. In your case, since your delimiters are commas, you would call:

int commaIndex = strPayload.indexOf(',');
//  Search for the next comma just after the first
int secondCommaIndex = strPayload.indexOf(',', commaIndex+1);
//Then you could use that index to create a substring using the String class's substring() method. This returns a new String beginning at a particular starting index, and ending just before a second index (Or the end of a file if none is given). So you would type something akin to:
int lastCommaIndex = strPayload.lastIndexOf(',');

String firstValue = strPayload.substring(0, commaIndex);
String secondValue = strPayload.substring(commaIndex+1, secondCommaIndex);
String thirdValue = strPayload.substring(lastCommaIndex+1); // To the end of the string
//Finally, the integer values can be retrieved using the String class's undocumented method, toInt():

int node = firstValue.toInt();
int device = secondValue.toInt();
int value = thirdValue.toInt();
//More information on the String object and its various methods can be found int the Arduino documentation.

Serial.print("Sending radio message, node:");
Serial.print(node);
Serial.print(", ");
Serial.print("device: ");
Serial.print(device);
Serial.print(", ");
Serial.print("vaule: ");
Serial.print(value);
Serial.print(", ");
Serial.print("Length = ");
Serial.println(length);


// transmits the MQTT payload to the node# specified in the payload. The payload should match the format
// node#,device#,value# for ex. 015,12,200
radio.sendWithRetry(node, payload, length);

Serial.println("send done");

}

//-----------------------------------------NODE CODE-----------------------------------------
/* Below is code that needs to be put into your -node- to recieve radio communicated payload
   and parse the data into variables


  if (radio.receiveDone())
  {
    Serial.print('[');Serial.print(radio.SENDERID, DEC);Serial.print("] ");
    if (promiscuousMode)
    {
      Serial.print("to [");Serial.print(radio.TARGETID, DEC);Serial.print("] ");
    }
    for (byte i = 0; i < radio.DATALEN; i++)
      Serial.print((char)radio.DATA[i]);
    Serial.print("   [RX_RSSI:");Serial.print(radio.RSSI);Serial.print("]");
 Serial.println();


//convert payload to string
String strPayload = String((char*)radio.DATA); //returns number or text string exactly as input
Serial.print(" = ");
Serial.println(strPayload); //returns number or text string exactly as input

//Strings have a method called indexOf() which allows you to search for the index in the String's character array of a particular character. If the character is not found, the method should return -1. A second parameter can be added to the function call to indicate a starting point for the search. In your case, since your delimiters are commas, you would call:

int commaIndex = strPayload.indexOf(',');
//  Search for the next comma just after the first
int secondCommaIndex = strPayload.indexOf(',', commaIndex+1);
//Then you could use that index to create a substring using the String class's substring() method. This returns a new String beginning at a particular starting index, and ending just before a second index (Or the end of a file if none is given). So you would type something akin to:
int lastCommaIndex = strPayload.lastIndexOf(',');

String firstValue = strPayload.substring(0, commaIndex);
String secondValue = strPayload.substring(commaIndex+1, secondCommaIndex);
String thirdValue = strPayload.substring(lastCommaIndex+1); // To the end of the string
//Finally, the integer values can be retrieved using the String class's undocumented method, toInt():

int node = firstValue.toInt();
int device = secondValue.toInt();
int value = thirdValue.toInt();
//More information on the String object and its various methods can be found int the Arduino documentation.

Serial.print("Recieve radio message, node:");
Serial.print(node);
Serial.print(", ");
Serial.print("device: ");
Serial.print(device);
Serial.print(", ");
Serial.print("vaule: ");
Serial.print(value);
Serial.print(", ");
Serial.print("Length = ");
Serial.println(radio.DATALEN);

*/ //end node code
