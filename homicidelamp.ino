#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino

//needed for library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager

#include <PubSubClient.h>
WiFiClient espClient;
PubSubClient client(espClient);

//VARS
const char* mqtt_server = "SERVER";
const char* mqtt_username = "USERNAME";
const char* mqtt_pass = "PASSWORD";
int light_pin = 5; //pin D1 on NodeMCU v1.0
String lamp_id;
char lamp_id_buf[100];

void blink() {
  int count=0;
  while (count < 4) {
    digitalWrite(light_pin, LOW);
    delay(500);
    digitalWrite(light_pin, HIGH);
    delay(500);
    count++;
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client", mqtt_username, mqtt_pass)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      lamp_id.toCharArray(lamp_id_buf, lamp_id.length()+1);
      client.publish("lamp", lamp_id_buf);
      // ... and resubscribe
      client.subscribe("homicide");
      blink();
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      Serial.println(WiFi.localIP());
      Serial.println (espClient.status());
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  //Debugging
  Serial.print ("Callback ");
  Serial.print(topic);
  Serial.print(" ");
  Serial.println((char)payload[0]);
  
  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(light_pin, LOW); //Turn LED on
  } else {
    digitalWrite(light_pin, HIGH); //Turn LED off
  }
}

void setup() {
  pinMode(light_pin, OUTPUT);
  
  // put your setup code here, to run once:
  Serial.begin(115200);
  
  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;
  //reset settings - for testing
  //wifiManager.resetSettings();

  //sets timeout until configuration portal gets turned off
  //useful to make it all retry or go to sleep
  //in seconds
  wifiManager.setTimeout(180);
  
  //fetches ssid and pass and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP"
  //and goes into a blocking loop awaiting configuration
  if(!wifiManager.autoConnect("HomicideLampAP")) {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(5000);

    
  } 

  //if you get here you have connected to the WiFi
  Serial.println("Connected to WiFi");

  client.setServer(mqtt_server, 11213);
  client.setCallback(callback);

  lamp_id = WiFi.macAddress();
  lamp_id.replace(":" , "");
  Serial.print("lamp_id: ");
  Serial.println(lamp_id);
 
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
    client.loop();
}
