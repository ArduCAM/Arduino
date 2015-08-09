/*
In this sketch folder is a basic webpage and a copy of zepto.js, a
 minimized version of jQuery.  When you upload your sketch, place the 
 ArduCAMYun floder in the /arduino/www folder on your SD card.Then you can 
 access link such as http;//arduino.local/sd/ArduCAMYun to see the basic webpage.
 It can control the ArduCAM to take photos.
*/

#include <Bridge.h>
#include <YunServer.h>
#include <YunClient.h>
#include <FileIO.h>
#include <Process.h>
#include <Wire.h>
#include <ArduCAM.h>
#include <SPI.h>
#include "memorysaver.h"

const int CS1 = 4;
uint8_t capture = 0;
uint8_t save_done = 0;
char ip[16];
int m;
char filename[20];
ArduCAM myCAM1(OV5642,CS1);  

// Listen on default port 5555, the webserver on the Yún
// will forward there all the HTTP requests for us.
YunServer server;

void setup() 
{
  uint8_t vid,pid;
  uint8_t temp;
  
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  Serial.println("ArduCAM Start!");
  
  // Bridge startup
  Bridge.begin();
  
  // Listen for incoming connection only from localhost
  // (no one from the external network could connect)
  server.listenOnLocalhost();
  server.begin();
  
  Wire.begin(); 
  
  pinMode(CS1, OUTPUT);
  
  SPI.begin(); 
  //Check if the ArduCAM SPI bus is OK
  myCAM1.write_reg(ARDUCHIP_TEST1, 0x55);
  temp = myCAM1.read_reg(ARDUCHIP_TEST1);
  //Serial.println(temp);
  if(temp != 0x55)
  {
  	Serial.println("SPI1 interface Error!");
  	//while(1);
  }
  //Check if the camera module type is OV5642
  myCAM1.rdSensorReg16_8(OV5642_CHIPID_HIGH, &vid);
  myCAM1.rdSensorReg16_8(OV5642_CHIPID_LOW, &pid);
  
  if((vid != 0x56) || (pid != 0x42))
  	Serial.println("Can't find OV5642 module!");
  else
  	Serial.println("OV5642 detected");
  
  //Change to JPEG capture mode and initialize the OV2640 module	
  myCAM1.set_format(JPEG);
  myCAM1.InitCAM();
  myCAM1.write_reg(ARDUCHIP_TIM, VSYNC_LEVEL_MASK);
  
  myCAM1.clear_fifo_flag();
  myCAM1.write_reg(ARDUCHIP_FRAMES,0x00);
  
  //Initialize SD Card
  if (! FileSystem.begin()) 
  {
    Serial.println("SD Card Error!");
    //while(1);
  }
  else
  {
    Serial.println("SD Card detected!");
  }
  if(!FileSystem.exists("/mnt/sda1/arduino/"))//chick if arduino floder exist
  {
    if(FileSystem.mkdir("/mnt/sda1/arduino"))
    {
      Serial.println("Make arduino floder done.");
    }
    else
    {
      Serial.println("Make arduino floder failed!");
      //while(1);
    }
  }
  else
  {
    Serial.println("The arduino floder is already existed.");
  }
  if(!FileSystem.exists("/mnt/sda1/DCIM/"))//chick is DCIM floder exist
  {
    if(FileSystem.mkdir("/mnt/sda1/DCIM"))
    {
      Serial.println("Make DCIM floder done.");
    }
    else
    {
      Serial.println("Make DCIM floder failed!");
      //while(1);
    }
  }
  else
  {
    Serial.println("The DCIM floder is already existed.");
  }
  Process p;
  p.runShellCommand("ln -s /mnt/sda1/DCIM /www/DCIM");
  p.running();
  Process q;
  //if you use wlan, change the eth1 to wlan0
  q.runShellCommand("/sbin/ifconfig wlan0 | awk '/inet addr/ {print $2}' | cut -f2 -d ':'");
  q.running();
  m = 0;
  while (q.available())
  {
    ip[m] = q.read();
    m += 1;
  }
  Serial.print("IP address is ");
  Serial.println(ip);
}

void loop() 
{
  uint8_t temp,temp_last;
  char path[40];
  byte buf[256];
  
 
  // Get clients coming from server
  YunClient client = server.accept();

  // There is a new client?
  if (client) 
  {
    // Process request
    process(client);
    
    // Close connection and free resources.
    client.stop();
  }
  if(capture==1)
  {
    //Clear the capture done flag 
    myCAM1.clear_fifo_flag();	 
    //Start capture
    myCAM1.start_capture();
    capture=0;	 
  }
  if(myCAM1.get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK))
  {
    Serial.println("CAM1 Capture Done!");
    memset(path,0,40);
    memset(filename,0,20);
    String time = gettime();
    strcat(filename,time.c_str());
    filename[strlen(filename)-1]=0;
    //Serial.println(filename);
    strcat(filename,".jpg");
    strcat(path,"/mnt/sda1/DCIM/");
    strcat(path,filename);
    Serial.println(path);
    //Open the new file  
    File outFile = FileSystem.open(path,FILE_WRITE);
    if (! outFile) 
    { 
      Serial.println("open file failed");
      myCAM1.clear_fifo_flag();
      return;
    }
    
    temp = 0;
    int i=0;
    while( (temp != 0xD9) | (temp_last != 0xFF) )
    {
        temp_last = temp;
	temp = myCAM1.read_fifo();
        if(i < 256)
        {
          buf[i] = temp;
          i++;
        }
        else
        {
          //Write 256 bytes image data to file
          outFile.write(buf,256);
          i = 0;
          buf[i] = temp;
          i++;
        }
    }
    if(i>0)
      outFile.write(buf,i);

    //Clear the capture done flag 
    myCAM1.clear_fifo_flag();
    save_done = 1;
    Serial.println("Save done.");
  }
}

void process(YunClient client) 
{
  // read the command
  String command = client.readStringUntil('/');//the url is like http ://1.1.1.1.1/command/1

  if (command == "capture") 
  {
    capture = 1;
    save_done = 0;
    client.print(F("capture"));
  }
  else if(command == "save")
  {
    if(save_done==0)
      client.print("wait");
    else
    {
      client.print("http://");//return the file path
      client.print(ip);
      client.print("/DCIM/");
      client.print(filename); 
    }
  }
}

String gettime()
{
    Process ptime;
    String time="";
    ptime.begin("date");
    ptime.addParameter("+%Y%m%d%H%M%S");
    ptime.run();
    while(ptime.available() > 0) 
    {
      char c = ptime.read();
      time += c;
    }
    return time;
}

