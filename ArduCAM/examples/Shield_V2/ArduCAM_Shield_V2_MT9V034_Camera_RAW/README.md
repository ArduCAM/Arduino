#How to use MTV9V034 demo

1. The MT9V034 default 8bit I2C slave address is 0x90

2. This demo is only support for ArduCAM_Shield_V2 platform and MT9V034 sensor.
Before using it,you should enable  ARDUCAM_SHIELD_V2 flatform and MT9V034_CAM 
camera module in the ../libraries/ArduCAM/memorysaver.h file.
        
3. Capture and buffer the image into FIFO when shutter button pressed shortly.

4. Store the image to Micro SD/TF card with RAW format with the default 752x480 resolution.