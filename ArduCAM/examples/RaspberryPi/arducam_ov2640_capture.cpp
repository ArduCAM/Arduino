/*-----------------------------------------

//Update History:
//2016/06/13 	V1.1	by Lee	add support for burst mode

--------------------------------------*/
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <wiringPiI2C.h>
#include <wiringPi.h>
#include "arducam_arch_raspberrypi.h"
#define OV2640_CHIPID_HIGH 	0x0A
#define OV2640_CHIPID_LOW 	0x0B
#define OV2640_MAX_FIFO_SIZE		0x5FFFF			//384KByte
#define BUF_SIZE 4096
#define CAM1_CS 5
uint8_t buf[BUF_SIZE];
bool is_header = false;
ArduCAM myCAM(OV2640,CAM1_CS);
void setup()
{
    uint8_t vid,pid;
    uint8_t temp;
    wiring_init();
    pinMode(CAM1_CS, OUTPUT);
    // Check if the ArduCAM SPI bus is OK
    myCAM.write_reg(ARDUCHIP_TEST1, 0x55);
    temp = myCAM.read_reg(ARDUCHIP_TEST1);
    //printf("temp=%x\n",temp);
    if(temp != 0x55) {
        printf("SPI interface error!\n");
        exit(EXIT_FAILURE);
    }  
    // Change MCU mode
    myCAM.write_reg(ARDUCHIP_MODE, 0x00); 
    // Check if the camera module type is OV2640
    myCAM.rdSensorReg8_8(OV2640_CHIPID_HIGH, &vid);
    myCAM.rdSensorReg8_8(OV2640_CHIPID_LOW, &pid);
	
    if ((vid != 0x26 ) && (( pid != 0x41 ) || ( pid != 0x42 ))){
        printf("Can't find OV2640 module!\n");
        exit(EXIT_FAILURE);
    } else {
        printf("OV2640 detected\n");
    }
}
int main(int argc, char *argv[])
{
	 uint8_t temp = 0, temp_last = 0;
    if (argc == 1)
    {
        printf("Usage: %s [-s <resolution>] | [-c <filename]", argv[0]);
        printf(" -s <resolution> Set resolution, valid resolutions are:\n");
        printf("                   160x120\n");
        printf("                   176x144\n");
        printf("                   320x240\n");
        printf("                   352x288\n");
        printf("                   640x480\n");
        printf("                   800x600\n");
        printf("                   1024x768\n");
        printf("                   1280x1024\n");
        printf("                   1600x1200\n");
        printf(" -c <filename>   Capture image\n");
        exit(EXIT_SUCCESS);
    }
  	if (strcmp(argv[1], "-c") == 0 && argc == 4) 
  	{
      setup(); 
      myCAM.set_format(JPEG);
      myCAM.InitCAM();
      // Change to JPEG capture mode and initialize the OV2640 module   
      if (strcmp(argv[3], "160x120") == 0) myCAM.OV2640_set_JPEG_size(OV2640_160x120);
      else if (strcmp(argv[3], "176x144")  == 0) myCAM.OV2640_set_JPEG_size(OV2640_176x144);
      else if (strcmp(argv[3], "320x240")  == 0) myCAM.OV2640_set_JPEG_size(OV2640_320x240);
      else if (strcmp(argv[3], "352x288")  == 0) myCAM.OV2640_set_JPEG_size(OV2640_352x288);
      else if (strcmp(argv[3], "640x480")  == 0) myCAM.OV2640_set_JPEG_size(OV2640_640x480);
      else if (strcmp(argv[3], "800x600")  == 0) myCAM.OV2640_set_JPEG_size(OV2640_800x600);
      else if (strcmp(argv[3], "1024x768") == 0) myCAM.OV2640_set_JPEG_size(OV2640_1024x768);
      else if (strcmp(argv[3], "1280x960") == 0) myCAM.OV2640_set_JPEG_size(OV2640_1280x1024);
      else if (strcmp(argv[3], "1600x1200")== 0) myCAM.OV2640_set_JPEG_size(OV2640_1600x1200);
      else {
      printf("Unknown resolution %s\n", argv[3]);
      exit(EXIT_FAILURE);
      }
      sleep(1); // Let auto exposure do it's thing after changing image settings
      printf("Changed resolution1 to %s\n", argv[3]);     
      // Flush the FIFO
      myCAM.flush_fifo();    
      // Clear the capture done flag
      myCAM.clear_fifo_flag();
      // Start capture
      printf("Start capture\n");  
      myCAM.start_capture();
      while (!(myCAM.read_reg(ARDUCHIP_TRIG) & CAP_DONE_MASK)) ;
      printf("CAM Capture Done\n");
              
       // Open the new file
      FILE *fp1 = fopen(argv[2], "w+");   
      if (!fp1) {
          printf("Error: could not open %s\n", argv[2]);
          exit(EXIT_FAILURE);
      }  
      printf("Reading FIFO and saving IMG\r\n");    
      size_t length = myCAM.read_fifo_length();
      printf("The length is %d\r\n", length);
      if (length >= OV2640_MAX_FIFO_SIZE){
		   printf("Over size.");
		    exit(EXIT_FAILURE);
		  }else if (length == 0 ){
		    printf("Size is 0.");
		    exit(EXIT_FAILURE);
		  } 
		   
	 int32_t i = 0;
	    myCAM.CS_LOW();  //Set CS low       
      myCAM.set_fifo_burst();
     
      while ( length-- )
		  {
		    temp_last = temp;
		    temp =  myCAM.transfer(0x00);
		    //Read JPEG data from FIFO
		    if ( (temp == 0xD9) && (temp_last == 0xFF) ) //If find the end ,break while,
		    {
		        buf[i++] = temp;  //save the last  0XD9     
		       //Write the remain bytes in the buffer
		        myCAM.CS_HIGH();
		        fwrite(buf, i, 1, fp1);    
		       //Close the file
		        fclose(fp1); 
		        printf("IMG save OK !\n"); 
		        is_header = false;
		        i = 0;
		    }  
		    if (is_header == true)
		    { 
		       //Write image data to buffer if not full
		        if (i < BUF_SIZE)
		        buf[i++] = temp;
		        else
		        {
		          //Write BUF_SIZE bytes image data to file
		          myCAM.CS_HIGH();
		          fwrite(buf, BUF_SIZE, 1, fp1);
		          i = 0;
		          buf[i++] = temp;
		          myCAM.CS_LOW();
		          myCAM.set_fifo_burst();
		        }        
		    }
		    else if ((temp == 0xD8) & (temp_last == 0xFF))
		    {
		      is_header = true;
		      buf[i++] = temp_last;
		      buf[i++] = temp;   
		    } 
		  }
  } 
  else {
      printf("Error: unknown or missing argument.\n");
      exit(EXIT_FAILURE);
  }
  exit(EXIT_SUCCESS);
}
