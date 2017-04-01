#ArduCAM Library and examples porting for Raspberry Pi 

##Edit config.txt 
sudo nano /boot/config.txt<br> 
uncomment dtparam=spi=on line<br>
sudo reboot<br>

##Install related libraries and tools
sudo apt-get install wiringpi <br>
sudo apt-get install i2c-tools <br>
sudo apt-get install libi2c-dev <br>
sudo apt-get install python-smbus <br>

##Make the library and examples
'memorysaver.h' must be configured to enable 'RASPBERRY_PI' definition before make the library.
cd /home/pi/ArduCAM/example/RaspberryPi 
sudo make 

##Run the examples
Examples to run the demo code, using different executable files for different camera models.
sudo ./ov2640_capture -c test.jpg 320x240 
sudo ./ov5640_capture -c test.jpg 320x240
sudo ./ov5642_capture -c test.jpg 320x240 
sudo ./ov2640_4cams_capture -c test1,jpg test2.jpg test3.jpg test4.jpg 320x240 