#include <SPI.h>
#include <SC16IS750.h>
#include <WiFly.h>

// Enabe debug tracing to Serial port.
#define DEBUGGING

// Here we define a maximum framelength to 64 bytes. Default is 256.
#define MAX_FRAME_LENGTH 64

// Define how many callback functions you have. Default is 1.
#define CALLBACK_FUNCTIONS 1

#include <WebSocketServer.h>

WiFlyServer server(80);
WebSocketServer webSocketServer;


// Called when a new message from the WebSocket is received
// Looks for a message in this form:
//
// DPV
//
// Where: 
//        D is either 'd' or 'a' - digital or analog
//        P is a pin #
//        V is the value to apply to the pin
//

void handleClientData(String &dataString) {
  bool isDigital = dataString[0] == 'd';
  int pin = dataString[1] - '0';
  int value;

  value = dataString[2] - '0';

    
  pinMode(pin, OUTPUT);
   
  if (isDigital) {
    digitalWrite(pin, value);
  } else {
    analogWrite(pin, value);
  }
    
  Serial.println(dataString);
}

// send the client the analog value of a pin
void sendClientData(int pin) {
  String data = "a";
  
  pinMode(pin, INPUT);
  data += String(pin) + String(analogRead(pin));
  webSocketServer.sendData(data);  
}

void setup() {
  

  Serial.begin(9600);
  SC16IS750.begin();
  
  WiFly.setUart(&SC16IS750);
  
  WiFly.begin();
  
  // This is for an unsecured network
  // For a WPA1/2 network use auth 3, and in another command send 'set wlan phrase PASSWORD'
  // For a WEP network use auth 2, and in another command send 'set wlan key KEY'
  WiFly.sendCommand(F("set wlan auth 1"));
  WiFly.sendCommand(F("set wlan channel 0"));
  WiFly.sendCommand(F("set ip dhcp 1"));
  
  server.begin();
  Serial.println(F("Joining WiFi network..."));
  

  // Here is where you set the network name to join
  if (!WiFly.sendCommand(F("join arduino_wifi"), "Associated!", 20000, false)) {
    Serial.println(F("Association failed."));
    while (1) {
      // Hang on failure.
    }
  }
  
  if (!WiFly.waitForResponse("DHCP in", 10000)) {  
    Serial.println(F("DHCP failed."));
    while (1) {
      // Hang on failure.
    }
  }

  // This is how you get the local IP as an IPAddress object
  Serial.println(WiFly.localIP());
  
  // This delay is needed to let the WiFly respond properly
  delay(100);
}

void loop() {
  String data;
  WiFlyClient client = server.available();
  
  if (client.connected() && webSocketServer.handshake(client)) {
    
    while (client.connected()) {
      data = webSocketServer.getData();

      if (data.length() > 0) {
        handleClientData(data);
      }

      sendClientData(1);
      sendClientData(2);
      sendClientData(3);
    }
  }
  
  // wait to fully let the client disconnect
  delay(100);
}
