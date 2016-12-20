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
#define OV5640_CHIPID_HIGH 0x300a
#define OV5640_CHIPID_LOW 0x300b
#define OV5640_MAX_FIFO_SIZE		0x7FFFFF		//8MByte
#define BUF_SIZE 4096
#define CAM1_CS 5

#define VSYNC_LEVEL_MASK   		0x02  //0 = High active , 		1 = Low active
uint8_t buf[BUF_SIZE];
bool is_header = false;

ArduCAM myCAM(OV5640,CAM1_CS);
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
    myCAM.wrSensorReg16_8(0xff, 0x01);
    myCAM.rdSensorReg16_8(OV5640_CHIPID_HIGH, &vid);
    myCAM.rdSensorReg16_8(OV5640_CHIPID_LOW, &pid);
    if((vid != 0x56) || (pid != 0x40))
    printf("Can't find OV5640 module!");
     else
     printf("OV5640 detected.\n");
}
int main(int argc, char *argv[])
{
	 uint8_t temp = 0, temp_last = 0;
    if (argc == 1)
    {
        printf("Usage: %s [-s <resolution>] | [-c <filename]", argv[0]);
        printf(" -s <resolution> Set resolution, valid resolutions are:\n");
        printf("                   320x240\n");
		printf("                   352x288\n");
        printf("                   640x480\n");
	    printf("                   800x480\n");
		printf("                   1024x768\n");
        printf("                   1280x960\n");
        printf("                   1600x1200\n");
        printf("                   2048x1536\n");
        printf("                   2592x1944\n");
        printf(" -c <filename>   Capture image\n");
        exit(EXIT_SUCCESS);
    }
  	if (strcmp(argv[1], "-c") == 0 && argc == 4) 
  	{
      setup(); 
      myCAM.set_format(JPEG);
      myCAM.InitCAM();
      // Change to JPEG capture mode and initialize the OV5640 module   
      if (strcmp(argv[3], "320x240")  == 0) myCAM.OV5640_set_JPEG_size(OV5640_320x240);
      else if (strcmp(argv[3], "352x288")  == 0) myCAM.OV5640_set_JPEG_size(OV5640_352x288);
	    else if (strcmp(argv[3], "640x480")  == 0) myCAM.OV5640_set_JPEG_size(OV5640_640x480);
      else if (strcmp(argv[3], "800x480")  == 0) myCAM.OV5640_set_JPEG_size(OV5640_800x480);
      else if (strcmp(argv[3], "1024x768")  == 0) myCAM.OV5640_set_JPEG_size(OV5640_1024x768);
      else if (strcmp(argv[3], "1280x960")  == 0) myCAM.OV5640_set_JPEG_size(OV5640_1280x960);
      else if (strcmp(argv[3], "1600x1200") == 0) myCAM.OV5640_set_JPEG_size(OV5640_1600x1200);
	    else if (strcmp(argv[3], "2048x1536")  == 0) myCAM.OV5640_set_JPEG_size(OV5640_2048x1536);
      else if (strcmp(argv[3], "2592x1944") == 0) myCAM.OV5640_set_JPEG_size(OV5640_2592x1944);
      else {
      printf("Unknown resolution %s\n", argv[3]);
      exit(EXIT_FAILURE);
      }
      sleep(1); // Let auto exposure do it's thing after changing image settings
      printf("Changed resolution1 to %s\n", argv[3]); 
      myCAM.write_reg(ARDUCHIP_TIM, VSYNC_LEVEL_MASK);		//VSYNC is active HIGH   	  
       // Flush the FIFO
      myCAM.flush_fifo();    
      // Clear the capture done flag
      myCAM.clear_fifo_flag();
      // Start capture
      printf("Start capture\n");  
      myCAM.start_capture();
      while (!(myCAM.read_reg(ARDUCHIP_TRIG) & CAP_DONE_MASK)){}
      printf("CAM Capture Done\n");
              
       // Open the new file
      FILE *fp1 = fopen(argv[2], "w+");   
      if (!fp1) {
          printf("Error: could not open %s\n", argv[2]);
          exit(EXIT_FAILURE);
      }
       
      printf("Reading FIFO and saving IMG\n");    
      size_t length = myCAM.read_fifo_length();
      printf("The length is %d\r\n", length);
      if (length >= OV5640_MAX_FIFO_SIZE){
		   printf("Over size.");
		    exit(EXIT_FAILURE);
		  }else if (length == 0 ){
		    printf("Size is 0.");
		    exit(EXIT_FAILURE);
		  } 
	    int32_t i=0;
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
