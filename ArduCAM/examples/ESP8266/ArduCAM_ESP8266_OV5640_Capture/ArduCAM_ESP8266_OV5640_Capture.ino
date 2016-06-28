// ArduCAM Mini demo (C)2016 Lee
// web: http://www.ArduCAM.com
// This program is a demo of how to use most of the functions
// of the library with ArduCAM ESP8266 5MP camera.
// This demo was made for ArduCAM ESP8266 OV5642 5MP Camera.
// It can take photo and send to the Web.
// It can take photo continuously as video streaming and send to the Web.
// The demo sketch will do the following tasks:
// 1. Set the camera to JEPG output mode.
// 2. if server.on("/capture", HTTP_GET, serverCapture),it can take photo and send to the Web.
// 3.if server.on("/stream", HTTP_GET, serverStream),it can take photo continuously as video 
//streaming and send to the Web.

// This program requires the ArduCAM V3.4.1 (or later) library and ArduCAM ESP8266 5MP camera
// and use Arduino IDE 1.5.8 compiler or above
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#include <Wire.h>
#include <ArduCAM.h>
#include <SPI.h>
#include "memorysaver.h"

#define   FIFO_SIZE     0x07FFFFF

// Enabe debug tracing to Serial port.
#define DEBUGGING

// Here we define a maximum framelength to 64 bytes. Default is 256.
#define MAX_FRAME_LENGTH 64

// Define how many callback functions you have. Default is 1.
#define CALLBACK_FUNCTIONS 1

// set GPIO16 as the slave select :
const int CS = 16;

int wifiType = 0; // 0:Station  1:AP
const char* ssid = "JL_network"; // Put your SSID here
const char* password = "00180605"; // Put your PASSWORD here

ESP8266WebServer server(80);

ArduCAM myCAM(OV5640, CS);

void start_capture(){
  myCAM.clear_fifo_flag();
  myCAM.start_capture();
}

void camCapture(ArduCAM myCAM){
  WiFiClient client = server.client();
  
  size_t len = myCAM.read_fifo_length();
  if (len >= FIFO_SIZE){
    Serial.println("Over size.");
    return;
  }else if (len == 0 ){
    Serial.println("Size is 0.");
    return;
  }
  
  myCAM.CS_LOW();
  myCAM.set_fifo_burst();
  SPI.transfer(0xFF);
  
  if (!client.connected()) return;
  String response = "HTTP/1.1 200 OK\r\n";
  response += "Content-Type: image/jpeg\r\n";
  response += "Content-Length: " + String(len) + "\r\n\r\n";
  server.sendContent(response);
  
  static const size_t bufferSize = 4096;
  static uint8_t buffer[bufferSize] = {0xFF};
  
  while (len) {
      size_t will_copy = (len < bufferSize) ? len : bufferSize;
      SPI.transferBytes(&buffer[0], &buffer[0], will_copy);
      if (!client.connected()) break;
      client.write(&buffer[0], will_copy);
      len -= will_copy;
  }
  
  myCAM.CS_HIGH();
}

void serverCapture(){
  myCAM.clear_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);//disable low power
  delay(2000);
  start_capture();
  Serial.println("CAM Capturing");

  int total_time = 0;
  total_time = millis();
  while (!myCAM.get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK));
  total_time = millis() - total_time;
  Serial.print("capture total_time used (in miliseconds):");
  Serial.println(total_time, DEC);
  total_time = 0;
  Serial.println("CAM Capture Done!");
  total_time = millis();
  camCapture(myCAM);
  myCAM.set_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);  //enable low power
  total_time = millis() - total_time;
  Serial.print("send total_time used (in miliseconds):");
  Serial.println(total_time, DEC);
  Serial.println("CAM send Done!");
  myCAM.set_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);  //enable low power
}

void serverStream(){
  myCAM.set_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);  //enable low power
  WiFiClient client = server.client();
  
  String response = "HTTP/1.1 200 OK\r\n";
  response += "Content-Type: multipart/x-mixed-replace; boundary=frame\r\n\r\n";
  server.sendContent(response);
  
  while (1){
    myCAM.clear_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK); //disable low power
    delay(100);
    start_capture();   
    while (!myCAM.get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK)); 
    size_t len = myCAM.read_fifo_length();
    if (len >= FIFO_SIZE){
      Serial.println("Over size.");
      continue;
    }else if (len == 0 ){
      Serial.println("Size is 0.");
      continue;
    }
    
    myCAM.CS_LOW();
    myCAM.set_fifo_burst(); 
    SPI.transfer(0xFF);   
    if (!client.connected()) break;
    response = "--frame\r\n";
    response += "Content-Type: image/jpeg\r\n\r\n";
    server.sendContent(response);
    
    static const size_t bufferSize = 4096;
    static uint8_t buffer[bufferSize] = {0xFF};
    
    while (len) {
      size_t will_copy = (len < bufferSize) ? len : bufferSize;
      SPI.transferBytes(&buffer[0], &buffer[0], will_copy);
      if (!client.connected()) break;
      client.write(&buffer[0], will_copy);
      len -= will_copy;
    }
    myCAM.CS_HIGH();
    myCAM.set_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);  //enable low power
    if (!client.connected()) break;
  }
}

void handleNotFound(){
  String message = "Server is running!\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  server.send(200, "text/plain", message);
  
  if (server.hasArg("ql")){
    myCAM.clear_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK); //disable low power
    int ql = server.arg("ql").toInt();
    myCAM.OV5640_set_JPEG_size(ql);
    myCAM.set_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);//enable low power
    Serial.println("QL change to: " + server.arg("ql"));
  }
}

void setup() {
  uint8_t vid, pid;
  uint8_t temp;
#if defined(__SAM3X8E__)
  Wire1.begin();
#else
  Wire.begin();
#endif
  Serial.begin(115200);
  Serial.println("ArduCAM Start!");

  // set the CS as an output:
  pinMode(CS, OUTPUT);

  // initialize SPI:
  SPI.begin();
  SPI.setFrequency(4000000); //4MHz

  //Check if the ArduCAM SPI bus is OK
  myCAM.write_reg(ARDUCHIP_TEST1, 0x55);
  temp = myCAM.read_reg(ARDUCHIP_TEST1);
  if (temp != 0x55){
    Serial.println("SPI1 interface Error!");
    while(1);
  }
 myCAM.clear_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK); //disable low power
 delay(100);
  //Check if the camera module type is OV5640
  myCAM.wrSensorReg16_8(0xff, 0x01);
  myCAM.rdSensorReg16_8(OV5640_CHIPID_HIGH, &vid);
  myCAM.rdSensorReg16_8(OV5640_CHIPID_LOW, &pid);
   if((vid != 0x56) || (pid != 0x40)){
   Serial.println("Can't find OV5640 module!");
   while(1);
   }
   else
   Serial.println("OV5640 detected.");
 

  //Change to JPEG capture mode and initialize the OV5642 module
  myCAM.set_format(JPEG);
   myCAM.InitCAM();
   myCAM.write_reg(ARDUCHIP_TIM, VSYNC_LEVEL_MASK);   //VSYNC is active HIGH
   myCAM.OV5642_set_JPEG_size(OV5640_320x240);
   myCAM.set_bit(ARDUCHIP_GPIO,GPIO_PWDN_MASK);//enable low power
  if (wifiType == 0){
    // Connect to WiFi network
    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("WiFi connected");
    Serial.println("");
    Serial.println(WiFi.localIP());
  }else if (wifiType == 1){
    Serial.println();
    Serial.println();
    Serial.print("Share AP: ");
    Serial.println(ssid);
    
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssid, password);
    Serial.println("");
    Serial.println(WiFi.softAPIP());
  }
  
  // Start the server
  server.on("/capture", HTTP_GET, serverCapture);
  server.on("/stream", HTTP_GET, serverStream);
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("Server started");
}
void loop() {

  server.handleClient();
}

