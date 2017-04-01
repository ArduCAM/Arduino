# ArduCAM Library and examples porting for Raspberry Pi 

## 1. Edit config.txt 
sudo nano /boot/config.txt<br> 
uncomment dtparam=spi=on line<br>
sudo reboot<br>

## 2. Install related libraries and tools
sudo apt-get install wiringpi <br>
sudo apt-get install i2c-tools <br>
sudo apt-get install libi2c-dev <br>
sudo apt-get install python-smbus <br>

## 3. Make the library and examples
`memorysaver.h` must be configured to enable `RASPBERRY_PI` definition before make the library.<br>
cd /home/pi/ArduCAM/example/RaspberryPi <br>
sudo make <br>

## 4. Run the examples
Examples to run the demo code, using different executable files for different camera models.<br>
sudo ./ov2640_capture -c test.jpg 320x240 <br>
sudo ./ov5640_capture -c test.jpg 320x240 <br>
sudo ./ov5642_capture -c test.jpg 320x240 <br>
sudo ./ov2640_4cams_capture -c test1,jpg test2.jpg test3.jpg test4.jpg 320x240 <br>