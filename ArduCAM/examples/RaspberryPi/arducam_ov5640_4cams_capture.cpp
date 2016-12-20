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
#define CAM1_CS 0
#define CAM2_CS 4
#define CAM3_CS 3
#define CAM4_CS 5

#define VSYNC_LEVEL_MASK   		0x02  //0 = High active , 		1 = Low active
uint8_t buf[BUF_SIZE];
bool is_header = false;

ArduCAM myCAM1(OV5640,CAM1_CS);
ArduCAM myCAM2(OV5640,CAM2_CS);
ArduCAM myCAM3(OV5640,CAM3_CS);
ArduCAM myCAM4(OV5640,CAM4_CS);
void setup()
{
    uint8_t vid,pid;
    uint8_t temp;
    wiring_init();
    pinMode(CAM1_CS, OUTPUT);
    pinMode(CAM2_CS, OUTPUT);
    pinMode(CAM3_CS, OUTPUT);
    pinMode(CAM4_CS, OUTPUT);
      // Check if the ArduCAM SPI1 bus is OK
    myCAM1.write_reg(ARDUCHIP_TEST1, 0x55);
    temp = myCAM1.read_reg(ARDUCHIP_TEST1);
    //printf("temp=%x\n",temp);
    if(temp != 0x55) {
        printf("SPI1 interface error!\n");
        exit(EXIT_FAILURE);
    } 	
    // Check if the ArduCAM SPI2 bus is OK
    myCAM2.write_reg(ARDUCHIP_TEST1, 0x55);
    temp = myCAM2.read_reg(ARDUCHIP_TEST1);
    //printf("temp=%x\n",temp);      //debug
    if(temp != 0x55) {
        printf("SPI2 interface error!\n");
        exit(EXIT_FAILURE);
    }   	
    // Check if the ArduCAM SPI3 bus is OK
    myCAM3.write_reg(ARDUCHIP_TEST1, 0x55);
    temp = myCAM3.read_reg(ARDUCHIP_TEST1);
    //printf("temp=%x\n",temp);
    if(temp != 0x55) {
        printf("SPI3 interface error!\n");
        exit(EXIT_FAILURE);
    } 
     // Check if the ArduCAM SPI4 bus is OK
    myCAM4.write_reg(ARDUCHIP_TEST1, 0x55);
    temp = myCAM4.read_reg(ARDUCHIP_TEST1);
    //printf("temp=%x\n",temp);
    if(temp != 0x55) {
        printf("SPI4 interface error!\n");
        exit(EXIT_FAILURE);
    }
    // Change MCU mode
    myCAM1.write_reg(ARDUCHIP_MODE, 0x00); 
    myCAM2.write_reg(ARDUCHIP_MODE, 0x00); 
    myCAM3.write_reg(ARDUCHIP_MODE, 0x00); 
    myCAM4.write_reg(ARDUCHIP_MODE, 0x00); 
    myCAM1.wrSensorReg16_8(0xff, 0x01);
    myCAM1.rdSensorReg16_8(OV5640_CHIPID_HIGH, &vid);
    myCAM1.rdSensorReg16_8(OV5640_CHIPID_LOW, &pid);
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
  	if (strcmp(argv[1], "-c") == 0 && argc == 7) 
  	{
      setup(); 
      myCAM1.set_format(JPEG);
      myCAM1.InitCAM();
      // Change to JPEG capture mode and initialize the OV5640 module   
      if (strcmp(argv[6], "320x240")  == 0) myCAM1.OV5640_set_JPEG_size(OV5640_320x240);
      else if (strcmp(argv[6], "352x288")  == 0) myCAM1.OV5640_set_JPEG_size(OV5640_352x288);
	    else if (strcmp(argv[6], "640x480")  == 0) myCAM1.OV5640_set_JPEG_size(OV5640_640x480);
      else if (strcmp(argv[6], "800x480")  == 0) myCAM1.OV5640_set_JPEG_size(OV5640_800x480);
      else if (strcmp(argv[6], "1024x768")  == 0) myCAM1.OV5640_set_JPEG_size(OV5640_1024x768);
      else if (strcmp(argv[6], "1280x960")  == 0) myCAM1.OV5640_set_JPEG_size(OV5640_1280x960);
      else if (strcmp(argv[6], "1600x1200") == 0) myCAM1.OV5640_set_JPEG_size(OV5640_1600x1200);
	    else if (strcmp(argv[6], "2048x1536")  == 0) myCAM1.OV5640_set_JPEG_size(OV5640_2048x1536);
      else if (strcmp(argv[6], "2592x1944") == 0) myCAM1.OV5640_set_JPEG_size(OV5640_2592x1944);
      else {
      printf("Unknown resolution %s\n", argv[6]);
      exit(EXIT_FAILURE);
      }
      sleep(1); // Let auto exposure do it's thing after changing image settings
      printf("Changed resolution1 to %s\n", argv[6]); 
      myCAM1.write_reg(ARDUCHIP_TIM, VSYNC_LEVEL_MASK);		//VSYNC is active HIGH  
      myCAM2.write_reg(ARDUCHIP_TIM, VSYNC_LEVEL_MASK);		//VSYNC is active HIGH 
	  myCAM3.write_reg(ARDUCHIP_TIM, VSYNC_LEVEL_MASK);		//VSYNC is active HIGH 
      myCAM4.write_reg(ARDUCHIP_TIM, VSYNC_LEVEL_MASK);		//VSYNC is active HIGH    	  
     // Flush the FIFO
        myCAM1.flush_fifo();    
        // Clear the capture done flag
        myCAM1.clear_fifo_flag();
        // Start capture
        printf("CAM1 start capture\n");  
        myCAM1.start_capture();
        while (!(myCAM1.read_reg(ARDUCHIP_TRIG) & CAP_DONE_MASK)) ;
         printf("CAM1 Capture Done\n");
        
        // Flush the FIFO
        myCAM2.flush_fifo();    
        // Clear the capture done flag
        myCAM2.clear_fifo_flag();
        // Start capture
        printf("CAM2 start capture\n");  
        myCAM2.start_capture();
        while (!(myCAM2.read_reg(ARDUCHIP_TRIG) & CAP_DONE_MASK)) ;
        printf("CAM2 Capture Done\n");
        
         // Flush the FIFO
        myCAM3.flush_fifo();    
        // Clear the capture done flag
        myCAM3.clear_fifo_flag();
        // Start capture
        printf("CAM3 start capture\n");  
        myCAM3.start_capture();
        while (!(myCAM3.read_reg(ARDUCHIP_TRIG) & CAP_DONE_MASK)) ;
        printf("CAM3 Capture Done\n");
        
         // Flush the FIFO
        myCAM4.flush_fifo();    
        // Clear the capture done flag
        myCAM4.clear_fifo_flag();
        // Start capture
        printf("CAM4 start capture\n");  
        myCAM4.start_capture();
        while (!(myCAM4.read_reg(ARDUCHIP_TRIG) & CAP_DONE_MASK)) ;
        printf("CAM4 Capture Done\n");
         
            
         // Open the new file
        FILE *fp1 = fopen(argv[2], "w+");
         FILE *fp2 = fopen(argv[3], "w+");
          FILE *fp3 = fopen(argv[4], "w+");
           FILE *fp4 = fopen(argv[5], "w+");
        
        if (!fp1) {
            printf("Error: could not open %s\n", argv[2]);
            exit(EXIT_FAILURE);
        }
        if (!fp2) {
            printf("Error: could not open %s\n", argv[3]);
            exit(EXIT_FAILURE);
        }
          if (!fp3) {
            printf("Error: could not open %s\n", argv[4]);
            exit(EXIT_FAILURE);
        }
        if (!fp4) {
            printf("Error: could not open %s\n", argv[5]);
            exit(EXIT_FAILURE);
        } 
         printf("Reading FIFO and saving IMG\n");  
        
        size_t len1 = myCAM1.read_fifo_length();
        size_t len2 = myCAM2.read_fifo_length();
        size_t len3 = myCAM3.read_fifo_length();
        size_t len4 = myCAM4.read_fifo_length();
		printf("The len1 is %d\r\n",len1);
		printf("The len2 is %d\r\n",len2);
		printf("The len3 is %d\r\n",len3);
		printf("The len4 is %d\r\n",len4);
      if ((len1>=OV5640_MAX_FIFO_SIZE)||(len2>=OV5640_MAX_FIFO_SIZE)||(len3>=OV5640_MAX_FIFO_SIZE)||(len4>=OV5640_MAX_FIFO_SIZE)){
		   printf("Over size.");
		    exit(EXIT_FAILURE);
		  }
		  if (len1 == 0 ) printf("Size1 is 0."); 
		  if (len2 == 0 ) printf("Size2 is 0."); 
		  if (len3 == 0 ) printf("Size3 is 0."); 
		  if (len4 == 0 ) printf("Size4 is 0.");  
      int32_t i = 0; 
      myCAM1.CS_LOW();  //Set CS low       
      myCAM1.set_fifo_burst();
      while ( len1-- )
		  {
		    temp_last = temp;
		    temp =  myCAM1.transfer(0x00);
		    //Read JPEG data from FIFO
		    if ( (temp == 0xD9) && (temp_last == 0xFF) ) //If find the end ,break while,
		    {
		        buf[i++] = temp;  //save the last  0XD9     
		       //Write the remain bytes in the buffer
		        myCAM1.CS_HIGH();
		        fwrite(buf, i, 1, fp1);    
		       //Close the file
		        fclose(fp1); 
		        printf("IMG1 save OK !\n"); 
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
		          myCAM1.CS_HIGH();
		          fwrite(buf, BUF_SIZE, 1, fp1);
		          i = 0;
		          buf[i++] = temp;
		          myCAM1.CS_LOW();
		          myCAM1.set_fifo_burst();
		        }        
		    }
		    else if ((temp == 0xD8) & (temp_last == 0xFF))
		    {
		      is_header = true;
		      buf[i++] = temp_last;
		      buf[i++] = temp;   
		    } 
		  }
		  
       temp = 0;temp_last = 0; i = 0;
      myCAM2.CS_LOW();  //Set CS low       
      myCAM2.set_fifo_burst();
       while ( len2-- )
		  {
		    temp_last = temp;
		    temp =  myCAM2.transfer(0x00);
		    //Read JPEG data from FIFO
		    if ( (temp == 0xD9) && (temp_last == 0xFF) ) //If find the end ,break while,
		    {
		        buf[i++] = temp;  //save the last  0XD9     
		       //Write the remain bytes in the buffer
		        myCAM2.CS_HIGH();
		        fwrite(buf, i, 1, fp2);    
		       //Close the file
		        fclose(fp2); 
		        printf("IMG2 save OK !\n"); 
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
		          myCAM2.CS_HIGH();
		          fwrite(buf, BUF_SIZE, 1, fp2);
		          i = 0;
		          buf[i++] = temp;
		          myCAM2.CS_LOW();
		          myCAM2.set_fifo_burst();
		        }        
		    }
		    else if ((temp == 0xD8) & (temp_last == 0xFF))
		    {
		      is_header = true;
		      buf[i++] = temp_last;
		      buf[i++] = temp;   
		    } 
		  }
       
       temp = 0;temp_last = 0; i = 0;
       myCAM3.CS_LOW();  //Set CS low       
       myCAM3.set_fifo_burst();
       while ( len3-- )
		  {
		    temp_last = temp;
		    temp =  myCAM3.transfer(0x00);
		    //Read JPEG data from FIFO
		    if ( (temp == 0xD9) && (temp_last == 0xFF) ) //If find the end ,break while,
		    {
		        buf[i++] = temp;  //save the last  0XD9     
		       //Write the remain bytes in the buffer
		        myCAM3.CS_HIGH();
		        fwrite(buf, i, 1, fp3);    
		       //Close the file
		        fclose(fp3); 
		        printf("IMG3 save OK !\n"); 
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
		          myCAM3.CS_HIGH();
		          fwrite(buf, BUF_SIZE, 1, fp3);
		          i = 0;
		          buf[i++] = temp;
		          myCAM3.CS_LOW();
		          myCAM3.set_fifo_burst();
		        }        
		    }
		    else if ((temp == 0xD8) & (temp_last == 0xFF))
		    {
		      is_header = true;
		      buf[i++] = temp_last;
		      buf[i++] = temp;   
		    } 
		  }
       
       temp = 0;temp_last = 0; i = 0;
      myCAM4.CS_LOW();  //Set CS low       
      myCAM4.set_fifo_burst();
       while ( len4-- )
		  {
		    temp_last = temp;
		    temp =  myCAM4.transfer(0x00);
		    //Read JPEG data from FIFO
		    if ( (temp == 0xD9) && (temp_last == 0xFF) ) //If find the end ,break while,
		    {
		        buf[i++] = temp;  //save the last  0XD9     
		       //Write the remain bytes in the buffer
		        myCAM4.CS_HIGH();
		        fwrite(buf, i, 1, fp4);    
		       //Close the file
		        fclose(fp4); 
		        printf("IMG4 save OK !\n"); 
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
		          myCAM4.CS_HIGH();
		          fwrite(buf, BUF_SIZE, 1, fp4);
		          i = 0;
		          buf[i++] = temp;
		          myCAM4.CS_LOW();
		          myCAM4.set_fifo_burst();
		        }        
		    }
		    else if ((temp == 0xD8) & (temp_last == 0xFF))
		    {
		      is_header = true;
		      buf[i++] = temp_last;
		      buf[i++] = temp;   
		    } 
		  }
      
    } else {
        printf("Error: unknown or missing argument.\n");
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
}

