

#include <Wire.h>
#include <SD.h>
#include <SPI.h>

const int chipSelect = 53;        
const int HardwareSSPin = 53;  // For Arduino Mega

int PhotoTakenCount = 0;

// Serial Input 
const int BUFFERLENGTH = 255;
char IncomingByte[BUFFERLENGTH];   // for incoming serial data

// VGA Default 
int PHOTO_WIDTH  =  640;   
int PHOTO_HEIGHT =  480; 
int PHOTO_BYTES_PER_PIXEL = 2;

// Command and Parameter related Strings
String RawCommandLine = "";
String Command  = "QVGA";
String FPSParam = "ThirtyFPS";
String AWBParam = "SAWB";
String AECParam = "HistAEC";
String YUVMatrixParam = "YUVMatrixOn";
String DenoiseParam = "DenoiseNo";
String EdgeParam = "EdgeNo";
String ABLCParam = "AblcON";


enum ResolutionType
{
  VGA,
  VGAP,
  QVGA,
  QQVGA,
  None
};

ResolutionType Resolution = None; 

// Camera input/output pin connection to Arduino
#define WRST  25      // Output Write Pointer Reset
#define RRST  23      // Output Read Pointer Reset
#define WEN   24      // Output Write Enable
#define VSYNC 22      // Input Vertical Sync marking frame capture
#define RCLK  26      // Output FIFO buffer output clock
// set OE to low gnd

// FIFO Ram input pins
#define DO7   28     
#define DO6   29   
#define DO5   30   
#define DO4   31   
#define DO3   32   
#define DO2   33   
#define DO1   34
#define DO0   35 


// SDCARD
// MISO, MOSI, and SCK are also available in a consistent physical location on the ICSP header; 
// this is useful, for example, in designing a shield that works on the Uno and the Mega. 
// On the Arduino Mega, this is 
// 50 (MISO) 
// 51 (MOSI) 
// 52 (SCK) 
// 53 (SS) 



// Register addresses and values
#define CLKRC                 0x11 
#define CLKRC_VALUE_VGA       0x01  // Raw Bayer
#define CLKRC_VALUE_QVGA      0x01
#define CLKRC_VALUE_QQVGA     0x01
#define CLKRC_VALUE_NIGHTMODE_FIXED   0x03 // Fixed Frame
#define CLKRC_VALUE_NIGHTMODE_AUTO    0x80 // Auto Frame Rate Adjust

#define COM7                                   0x12 
#define COM7_VALUE_VGA                         0x01   // Raw Bayer
#define COM7_VALUE_VGA_COLOR_BAR               0x03   // Raw Bayer
#define COM7_VALUE_VGA_PROCESSED_BAYER         0x05   // Processed Bayer
#define COM7_VALUE_QVGA                        0x00
#define COM7_VALUE_QVGA_COLOR_BAR              0x02
#define COM7_VALUE_QVGA_PREDEFINED_COLOR_BAR   0x12
#define COM7_VALUE_QQVGA                       0x00
#define COM7_VALUE_QQVGA_COLOR_BAR             0x02   
#define COM7_VALUE_QCIF                        0x08     // Predefined QCIF format
#define COM7_VALUE_COLOR_BAR_QCIF              0x0A     // Predefined QCIF Format with ColorBar
#define COM7_VALUE_RESET                       0x80


#define COM3                            0x0C 
#define COM3_VALUE_VGA                  0x00 // Raw Bayer
#define COM3_VALUE_QVGA                 0x04
#define COM3_VALUE_QQVGA                0x04  // From Docs
#define COM3_VALUE_QQVGA_SCALE_ENABLED  0x0C  // Enable Scale and DCW
#define COM3_VALUE_QCIF                 0x0C  // Enable Scaling and enable DCW

#define COM14                            0x3E 
#define COM14_VALUE_VGA                  0x00 // Raw Bayer
#define COM14_VALUE_QVGA                 0x19
#define COM14_VALUE_QQVGA                0x1A
#define COM14_VALUE_MANUAL_SCALING       0x08   // Manual Scaling Enabled
#define COM14_VALUE_NO_MANUAL_SCALING    0x00   // Manual Scaling DisEnabled

#define SCALING_XSC                                  0x70
#define SCALING_XSC_VALUE_VGA                        0x3A  // Raw Bayer
#define SCALING_XSC_VALUE_QVGA                       0x3A
#define SCALING_XSC_VALUE_QQVGA                      0x3A
#define SCALING_XSC_VALUE_QQVGA_SHIFT1               0x3A
#define SCALING_XSC_VALUE_COLOR_BAR                  0xBA  
#define SCALING_XSC_VALUE_QCIF_COLOR_BAR_NO_SCALE    0x80 // Predefined QCIF with Color Bar and NO Scaling

#define SCALING_YSC                                   0x71 
#define SCALING_YSC_VALUE_VGA                         0x35 // Raw Bayer 
#define SCALING_YSC_VALUE_QVGA                        0x35
#define SCALING_YSC_VALUE_QQVGA                       0x35
#define SCALING_YSC_VALUE_COLOR_BAR                   0x35  // 8 bar color bar
#define SCALING_YSC_VALUE_COLOR_BAR_GREY              0xB5  // fade to grey color bar
#define SCALING_YSC_VALUE_COLOR_BAR_SHIFT1            0xB5  // fade to grey color bar
#define SCALING_YSC_VALUE_QCIF_COLOR_BAR_NO_SCALE     0x00  // Predefined QCIF with Color Bar and NO Scaling

#define SCALING_DCWCTR               0x72 
#define SCALING_DCWCTR_VALUE_VGA     0x11  // Raw Bayer
#define SCALING_DCWCTR_VALUE_QVGA    0x11
#define SCALING_DCWCTR_VALUE_QQVGA   0x22  

#define SCALING_PCLK_DIV              0x73  
#define SCALING_PCLK_DIV_VALUE_VGA    0xF0 // Raw Bayer
#define SCALING_PCLK_DIV_VALUE_QVGA   0xF1
#define SCALING_PCLK_DIV_VALUE_QQVGA  0xF2

#define SCALING_PCLK_DELAY              0xA2
#define SCALING_PCLK_DELAY_VALUE_VGA    0x02 // Raw Bayer
#define SCALING_PCLK_DELAY_VALUE_QVGA   0x02
#define SCALING_PCLK_DELAY_VALUE_QQVGA  0x02


// Controls YUV order Used with COM13
// Need YUYV format for Android Decoding- Default value is 0xD
#define TSLB                                         0x3A
#define TSLB_VALUE_YUYV_AUTO_OUTPUT_WINDOW_ENABLED   0x01 // No custom scaling
#define TSLB_VALUE_YUYV_AUTO_OUTPUT_WINDOW_DISABLED  0x00 // For adjusting HSTART, etc. YUYV format
#define TSLB_VALUE_UYVY_AUTO_OUTPUT_WINDOW_DISABLED  0x08 
#define TSLB_VALUE_TESTVALUE                         0x04 // From YCbCr Reference 


// Default value is 0x88
// ok if you want YUYV order, no need to change
#define COM13                      0x3D
#define COM13_VALUE_DEFAULT        0x88
#define COM13_VALUE_NOGAMMA_YUYV   0x00
#define COM13_VALUE_GAMMA_YUYV     0x80
#define COM13_VALUE_GAMMA_YVYU     0x82
#define COM13_VALUE_YUYV_UVSATAUTOADJ_ON 0x40



// Works with COM4
#define COM17                                 0x42
#define COM17_VALUE_AEC_NORMAL_NO_COLOR_BAR   0x00
#define COM17_VALUE_AEC_NORMAL_COLOR_BAR      0x08 // Activate Color Bar for DSP

#define COM4   0x0D

// RGB Settings and Data format
#define COM15    0x40


// Night Mode
#define COM11                             0x3B
#define COM11_VALUE_NIGHTMODE_ON          0x80     // Night Mode
#define COM11_VALUE_NIGHTMODE_OFF         0x00 
#define COM11_VALUE_NIGHTMODE_ON_EIGHTH   0xE0     // Night Mode 1/8 frame rate minimum
#define COM11_VALUE_NIGHTMODE_FIXED       0x0A 
#define COM11_VALUE_NIGHTMODE_AUTO        0xEA     // Night Mode Auto Frame Rate Adjust


// Color Matrix Control YUV
#define MTX1	   	0x4f 
#define MTX1_VALUE	0x80

#define MTX2	   	0x50 
#define MTX2_VALUE	0x80

#define MTX3	   	0x51 
#define MTX3_VALUE	0x00

#define MTX4	   	0x52 
#define MTX4_VALUE	0x22

#define MTX5	   	0x53 
#define MTX5_VALUE	0x5e

#define MTX6	   	0x54 
#define MTX6_VALUE	0x80

#define CONTRAS	   	0x56 
#define CONTRAS_VALUE	0x40

#define MTXS	   	0x58 
#define MTXS_VALUE	0x9e



// COM8
#define COM8                    0x13
#define COM8_VALUE_AWB_OFF      0xE5
#define COM8_VALUE_AWB_ON       0xE7




// Automatic White Balance
#define AWBC1	   	0x43 
#define AWBC1_VALUE	0x14

#define AWBC2	   	0x44 
#define AWBC2_VALUE	0xf0

#define AWBC3	   	0x45 
#define AWBC3_VALUE  	0x34

#define AWBC4	   	0x46 
#define AWBC4_VALUE	0x58

#define AWBC5	        0x47 
#define AWBC5_VALUE	0x28

#define AWBC6	   	0x48 
#define AWBC6_VALUE	0x3a

#define AWBC7           0x59
#define AWBC7_VALUE     0x88

#define AWBC8          0x5A
#define AWBC8_VALUE    0x88

#define AWBC9          0x5B
#define AWBC9_VALUE    0x44

#define AWBC10         0x5C
#define AWBC10_VALUE   0x67

#define AWBC11         0x5D
#define AWBC11_VALUE   0x49

#define AWBC12         0x5E
#define AWBC12_VALUE   0x0E

#define AWBCTR3        0x6C
#define AWBCTR3_VALUE  0x0A

#define AWBCTR2        0x6D
#define AWBCTR2_VALUE  0x55

#define AWBCTR1        0x6E
#define AWBCTR1_VALUE  0x11

#define AWBCTR0                0x6F
#define AWBCTR0_VALUE_NORMAL   0x9F
#define AWBCTR0_VALUE_ADVANCED 0x9E


// Gain
#define COM9                        0x14
#define COM9_VALUE_MAX_GAIN_128X    0x6A
#define COM9_VALUE_4XGAIN           0x10    // 0001 0000

#define BLUE          0x01    // AWB Blue Channel Gain
#define BLUE_VALUE    0x40

#define RED            0x02    // AWB Red Channel Gain
#define RED_VALUE      0x40

#define GGAIN            0x6A   // AWB Green Channel Gain
#define GGAIN_VALUE      0x40

#define COM16	   	0x41 
#define COM16_VALUE	0x08 // AWB Gain on

#define GFIX	   	0x69 
#define GFIX_VALUE	0x00

// Edge Enhancement Adjustment
#define EDGE	   	0x3f 
#define EDGE_VALUE	0x00

#define REG75	   	0x75 
#define REG75_VALUE	0x03

#define REG76	   	0x76 
#define REG76_VALUE	0xe1

// DeNoise 
#define DNSTH	   	0x4c 
#define DNSTH_VALUE	0x00

#define REG77	   	0x77 
#define REG77_VALUE	0x00

// Denoise and Edge Enhancement
#define COM16_VALUE_DENOISE_OFF_EDGE_ENHANCEMENT_OFF_AWBGAIN_ON     0x08 // Denoise off, AWB Gain on
#define COM16_VALUE_DENOISE_ON__EDGE_ENHANCEMENT_OFF__AWBGAIN_ON    0x18
#define COM16_VALUE_DENOISE_OFF__EDGE_ENHANCEMENT_ON__AWBGAIN_ON    0x28
#define COM16_VALUE_DENOISE_ON__EDGE_ENHANCEMENT_ON__AWBGAIN_ON     0x38 // Denoise on,  Edge Enhancement on, AWB Gain on


// 30FPS Frame Rate , PCLK = 24Mhz
#define CLKRC_VALUE_30FPS  0x80

#define DBLV               0x6b
#define DBLV_VALUE_30FPS   0x0A

#define EXHCH              0x2A
#define EXHCH_VALUE_30FPS  0x00

#define EXHCL              0x2B
#define EXHCL_VALUE_30FPS  0x00

#define DM_LNL               0x92
#define DM_LNL_VALUE_30FPS   0x00

#define DM_LNH               0x93
#define DM_LNH_VALUE_30FPS   0x00

#define COM11_VALUE_30FPS    0x0A   


// Saturation Control
#define SATCTR	   	0xc9 
#define SATCTR_VALUE	0x60


// AEC/AGC - Automatic Exposure/Gain Control
#define GAIN		0x00 
#define GAIN_VALUE	0x00

#define AEW	   	0x24 
#define AEW_VALUE	0x95

#define AEB	   	0x25 
#define AEB_VALUE	0x33

#define VPT	   	0x26 
#define VPT_VALUE	0xe3


/*
// Gamma
#define SLOP	   	0x7a 
#define SLOP_VALUE	0x20

#define GAM1	   	0x7b 
#define GAM1_VALUE	0x10

#define GAM2	   	0x7c 
#define GAM2_VALUE      0x1e

#define GAM3	   	0x7d 
#define GAM3_VALUE	0x35

#define GAM4	   	0x7e 
#define GAM4_VALUE	0x5a

#define GAM5	   	0x7f 
#define GAM5_VALUE	0x69

#define GAM6	   	0x80 
#define GAM6_VALUE	0x76

#define GAM7	   	0x81 
#define GAM7_VALUE	0x80

#define GAM8	   	0x82 
#define GAM8_VALUE	0x88

#define GAM9	   	0x83 
#define GAM9_VALUE	0x8f

#define GAM10	   	0x84 
#define GAM10_VALUE	0x96

#define GAM11	   	0x85 
#define GAM11_VALUE	0xa3

#define GAM12	   	0x86 
#define GAM12_VALUE	0xaf

#define GAM13	   	0x87 
#define GAM13_VALUE	0xc4

#define GAM14	   	0x88 
#define GAM14_VALUE	0xd7

#define GAM15	   	0x89 
#define GAM15_VALUE	0xe8
*/



// AEC/AGC Control- Histogram
#define HAECC1	   	0x9f 
#define HAECC1_VALUE	0x78

#define HAECC2	   	0xa0 
#define HAECC2_VALUE	0x68

#define HAECC3	   	0xa6 
#define HAECC3_VALUE	0xd8

#define HAECC4	   	0xa7 
#define HAECC4_VALUE	0xd8

#define HAECC5	   	0xa8 
#define HAECC5_VALUE	0xf0

#define HAECC6	   	0xa9 
#define HAECC6_VALUE	0x90

#define HAECC7	                        0xaa  // AEC Algorithm selection
#define HAECC7_VALUE_HISTOGRAM_AEC_ON	0x94 
#define HAECC7_VALUE_AVERAGE_AEC_ON     0x00



/*
// Gamma
#define SLOP	   	0x7a 
#define SLOP_VALUE	0x20

#define GAM1	   	0x7b 
#define GAM1_VALUE	0x10

#define GAM2	   	0x7c 
#define GAM2_VALUE      0x1e

#define GAM3	   	0x7d 
#define GAM3_VALUE	0x35

#define GAM4	   	0x7e 
#define GAM4_VALUE	0x5a

#define GAM5	   	0x7f 
#define GAM5_VALUE	0x69

#define GAM6	   	0x80 
#define GAM6_VALUE	0x76

#define GAM7	   	0x81 
#define GAM7_VALUE	0x80

#define GAM8	   	0x82 
#define GAM8_VALUE	0x88

#define GAM9	   	0x83 
#define GAM9_VALUE	0x8f

#define GAM10	   	0x84 
#define GAM10_VALUE	0x96

#define GAM11	   	0x85 
#define GAM11_VALUE	0xa3

#define GAM12	   	0x86 
#define GAM12_VALUE	0xaf

#define GAM13	   	0x87 
#define GAM13_VALUE	0xc4

#define GAM14	   	0x88 
#define GAM14_VALUE	0xd7

#define GAM15	   	0x89 
#define GAM15_VALUE	0xe8

*/

// Array Control
#define CHLF	   	0x33 
#define CHLF_VALUE	0x0b

#define ARBLM	   	0x34 
#define ARBLM_VALUE	0x11



// ADC Control
#define ADCCTR1	   	0x21 
#define ADCCTR1_VALUE	0x02

#define ADCCTR2	   	0x22 
#define ADCCTR2_VALUE	0x91

#define ADC	   	0x37 
#define ADC_VALUE       0x1d

#define ACOM	   	0x38 
#define ACOM_VALUE	0x71

#define OFON	   	0x39 
#define OFON_VALUE	0x2a


// Black Level Calibration
#define ABLC1	   	0xb1 
#define ABLC1_VALUE	0x0c

#define THL_ST		0xb3 
#define THL_ST_VALUE	0x82


// Window Output 
#define HSTART               0x17
#define HSTART_VALUE_DEFAULT 0x11
#define HSTART_VALUE_VGA     0x13     
#define HSTART_VALUE_QVGA    0x13   
#define HSTART_VALUE_QQVGA   0x13   // Works

#define HSTOP                0x18
#define HSTOP_VALUE_DEFAULT  0x61
#define HSTOP_VALUE_VGA      0x01   
#define HSTOP_VALUE_QVGA     0x01  
#define HSTOP_VALUE_QQVGA    0x01   // Works 

#define HREF                  0x32
#define HREF_VALUE_DEFAULT    0x80
#define HREF_VALUE_VGA        0xB6   
#define HREF_VALUE_QVGA       0x24
#define HREF_VALUE_QQVGA      0xA4  

#define VSTRT                0x19
#define VSTRT_VALUE_DEFAULT  0x03
#define VSTRT_VALUE_VGA      0x02
#define VSTRT_VALUE_QVGA     0x02
#define VSTRT_VALUE_QQVGA    0x02  
 
#define VSTOP                0x1A
#define VSTOP_VALUE_DEFAULT  0x7B
#define VSTOP_VALUE_VGA      0x7A
#define VSTOP_VALUE_QVGA     0x7A
#define VSTOP_VALUE_QQVGA    0x7A  

#define VREF                 0x03
#define VREF_VALUE_DEFAULT   0x03
#define VREF_VALUE_VGA       0x0A   
#define VREF_VALUE_QVGA      0x0A
#define VREF_VALUE_QQVGA     0x0A  


// I2C 
#define OV7670_I2C_ADDRESS                 0x21
#define I2C_ERROR_WRITING_START_ADDRESS      11
#define I2C_ERROR_WRITING_DATA               22

#define DATA_TOO_LONG                  1      // data too long to fit in transmit buffer 
#define NACK_ON_TRANSMIT_OF_ADDRESS    2      // received NACK on transmit of address 
#define NACK_ON_TRANSMIT_OF_DATA       3      // received NACK on transmit of data 
#define OTHER_ERROR                    4      // other error 

#define I2C_READ_START_ADDRESS_ERROR        33
#define I2C_READ_DATA_SIZE_MISMATCH_ERROR   44





byte ReadRegisterValue(int RegisterAddress)
{
  byte data = 0;
  
  Wire.beginTransmission(OV7670_I2C_ADDRESS);         
  Wire.write(RegisterAddress);                        
  Wire.endTransmission();
  Wire.requestFrom(OV7670_I2C_ADDRESS, 1);            
  while(Wire.available() < 1);              
  data = Wire.read(); 

  return data;  
}

void ReadRegisters()
{
  byte data = 0;
  
  data = ReadRegisterValue(CLKRC);
  Serial.print(F("CLKRC = "));
  Serial.println(data,HEX);
  
  data = ReadRegisterValue(COM7);
  Serial.print(F("COM7 = "));
  Serial.println(data,HEX);
  
  data = ReadRegisterValue(COM3);
  Serial.print(F("COM3 = "));
  Serial.println(data,HEX);
  
  data = ReadRegisterValue(COM14);
  Serial.print(F("COM14 = "));
  Serial.println(data,HEX);
   
  data = ReadRegisterValue(SCALING_XSC);
  Serial.print(F("SCALING_XSC = "));
  Serial.println(data,HEX);
  
  data = ReadRegisterValue(SCALING_YSC);
  Serial.print(F("SCALING_YSC = "));
  Serial.println(data,HEX);
  
  data = ReadRegisterValue(SCALING_DCWCTR);
  Serial.print(F("SCALING_DCWCTR = "));
  Serial.println(data,HEX);
 
  data = ReadRegisterValue(SCALING_PCLK_DIV);
  Serial.print(F("SCALING_PCLK_DIV = "));
  Serial.println(data,HEX);
   
  data = ReadRegisterValue(SCALING_PCLK_DELAY);
  Serial.print(F("SCALING_PCLK_DELAY = "));
  Serial.println(data,HEX);
  
  //data = ReadRegisterValue(COM10);
  //Serial.print(F("COM10 (Vsync Polarity) = "));
  //Serial.println(data,HEX);
  
  // default value D
  data = ReadRegisterValue(TSLB);
  Serial.print(F("TSLB (YUV Order- Higher Bit, Bit[3]) = "));
  Serial.println(data,HEX);
  
  // default value 88
  data = ReadRegisterValue(COM13);
  Serial.print(F("COM13 (YUV Order - Lower Bit, Bit[1]) = "));
  Serial.println(data,HEX);
  
  data = ReadRegisterValue(COM17);
  Serial.print(F("COM17 (DSP Color Bar Selection) = "));
  Serial.println(data,HEX);
  
  data = ReadRegisterValue(COM4);
  Serial.print(F("COM4 (works with COM 17) = "));
  Serial.println(data,HEX);
  
  data = ReadRegisterValue(COM15);
  Serial.print(F("COM15 (COLOR FORMAT SELECTION) = "));
  Serial.println(data,HEX);
  
  data = ReadRegisterValue(COM11);
  Serial.print(F("COM11 (Night Mode) = "));
  Serial.println(data,HEX);
  
  data = ReadRegisterValue(COM8);
  Serial.print(F("COM8 (Color Control, AWB) = "));
  Serial.println(data,HEX);
  
  data = ReadRegisterValue(HAECC7);
  Serial.print(F("HAECC7 (AEC Algorithm Selection) = "));
  Serial.println(data,HEX);
 
  data = ReadRegisterValue(GFIX);
  Serial.print(F("GFIX = "));
  Serial.println(data,HEX);
  
   
  // Window Output
  data = ReadRegisterValue(HSTART);
  Serial.print(F("HSTART = "));
  Serial.println(data,HEX);
  //Serial.print(F(", "));
  //Serial.println(data, DEC);
  
  data = ReadRegisterValue(HSTOP);
  Serial.print(F("HSTOP = "));
  Serial.println(data,HEX);
  
  data = ReadRegisterValue(HREF);
  Serial.print(F("HREF = "));
  Serial.println(data,HEX);
  
  data = ReadRegisterValue(VSTRT);
  Serial.print(F("VSTRT = "));
  Serial.println(data,HEX);
  
  data = ReadRegisterValue(VSTOP);
  Serial.print(F("VSTOP = "));
  Serial.println(data,HEX);
 
  data = ReadRegisterValue(VREF);
  Serial.print(F("VREF = "));
  Serial.println(data,HEX);
}


void ResetCameraRegisters()
{
  // Reset Camera Registers
  // Reading needed to prevent error
  byte data = ReadRegisterValue(COM7);
  
  int result = OV7670WriteReg(COM7, COM7_VALUE_RESET );
  String sresult = ParseI2CResult(result);
  Serial.println("RESETTING ALL REGISTERS BY SETTING COM7 REGISTER to 0x80: " + sresult);

  // Delay at least 500ms 
  delay(500);
}



// Main Call to Setup the ov7670 Camera
void SetupCamera()
{
  Serial.println(F("In SetupCamera() ..."));
  InitializeOV7670Camera();
}


void InitializeOV7670Camera()
{
  Serial.println(F("Initializing OV7670 Camera ..."));
  
  //Set WRST to 0 and RRST to 0 , 0.1ms after power on.
  int DurationMicroSecs =  1;
  
  // Set mode for pins wither input or output
  pinMode(WRST , OUTPUT);
  pinMode(RRST , OUTPUT);
  pinMode(WEN  , OUTPUT);
  pinMode(VSYNC, INPUT);
  pinMode(RCLK , OUTPUT);
  
  // FIFO Ram output pins
  pinMode(DO7 , INPUT);
  pinMode(DO6 , INPUT);
  pinMode(DO5 , INPUT);
  pinMode(DO4 , INPUT);
  pinMode(DO3 , INPUT);
  pinMode(DO2 , INPUT);
  pinMode(DO1 , INPUT);
  pinMode(DO0 , INPUT);
  
  // Delay 1 ms 
  delay(1); 
  
  PulseLowEnabledPin(WRST, DurationMicroSecs); 
  
  //PulseLowEnabledPin(RRST, DurationMicroSecs); 
  // Need to clock the fifo manually to get it to reset
  digitalWrite(RRST, LOW);
  PulsePin(RCLK, DurationMicroSecs); 
  PulsePin(RCLK, DurationMicroSecs); 
  digitalWrite(RRST, HIGH);  
}

void SetupCameraAdvancedAutoWhiteBalanceConfig()
{
   int result = 0;
   String sresult = "";
   
   Serial.println(F("........... Setting Camera Advanced Auto White Balance Configs ........"));
  
   result = OV7670WriteReg(AWBC1, AWBC1_VALUE);
   sresult = ParseI2CResult(result);
   Serial.print(F("AWBC1: "));
   Serial.println(sresult);
   
   result = OV7670WriteReg(AWBC2, AWBC2_VALUE);
   sresult = ParseI2CResult(result);
   Serial.print(F("AWBC2: "));
   Serial.println(sresult);
   
   result = OV7670WriteReg(AWBC3, AWBC3_VALUE);
   sresult = ParseI2CResult(result);
   Serial.print(F("AWBC3: "));
   Serial.println(sresult);
   
   result = OV7670WriteReg(AWBC4, AWBC4_VALUE);
   sresult = ParseI2CResult(result);
   Serial.print(F("AWBC4: "));
   Serial.println(sresult);
   
   result = OV7670WriteReg(AWBC5, AWBC5_VALUE);
   sresult = ParseI2CResult(result);
   Serial.print(F("AWBC5: "));
   Serial.println(sresult);
   
   result = OV7670WriteReg(AWBC6, AWBC6_VALUE);
   sresult = ParseI2CResult(result);
   Serial.print(F("AWBC6: "));
   Serial.println(sresult);
   
   result = OV7670WriteReg(AWBC7, AWBC7_VALUE);
   sresult = ParseI2CResult(result);
   Serial.print(F("AWBC7: "));
   Serial.println(sresult);
 
   result = OV7670WriteReg(AWBC8, AWBC8_VALUE);
   sresult = ParseI2CResult(result);
   Serial.print(F("AWBC8: "));
   Serial.println(sresult);
 
   result = OV7670WriteReg(AWBC9, AWBC9_VALUE);
   sresult = ParseI2CResult(result);
   Serial.print(F("AWBC9: "));
   Serial.println(sresult);
 
   result = OV7670WriteReg(AWBC10, AWBC10_VALUE);
   sresult = ParseI2CResult(result);
   Serial.print(F("AWBC10: "));
   Serial.println(sresult);
 
   result = OV7670WriteReg(AWBC11, AWBC11_VALUE);
   sresult = ParseI2CResult(result);
   Serial.print(F("AWBC11: "));
   Serial.println(sresult);
 
   result = OV7670WriteReg(AWBC12, AWBC12_VALUE);
   sresult = ParseI2CResult(result);
   Serial.print(F("AWBC12: "));
   Serial.println(sresult);
 
   result = OV7670WriteReg(AWBCTR3, AWBCTR3_VALUE);
   sresult = ParseI2CResult(result);
   Serial.print(F("AWBCTR3: "));
   Serial.println(sresult);
 
   result = OV7670WriteReg(AWBCTR2, AWBCTR2_VALUE);
   sresult = ParseI2CResult(result);
   Serial.print(F("AWBCTR2: "));
   Serial.println(sresult);
 
   result = OV7670WriteReg(AWBCTR1, AWBCTR1_VALUE);
   sresult = ParseI2CResult(result);
   Serial.print(F("AWBCTR1: "));
   Serial.println(sresult);
}

void SetupCameraUndocumentedRegisters()
{ 
   // Write(0xb0,0x84); //adding this improve the color a little bit
   int result = 0;
   String sresult = "";
   
   Serial.println(F("........... Setting Camera Undocumented Registers ........"));
   result = OV7670WriteReg(0xB0, 0x84);
   sresult = ParseI2CResult(result);
   Serial.print(F("Setting B0 UNDOCUMENTED register to 0x84:= "));
   Serial.println(sresult);
}

void SetupCameraFor30FPS()
{
   int result = 0;
   String sresult = "";
   
   Serial.println(F("........... Setting Camera to 30 FPS ........"));
   result = OV7670WriteReg(CLKRC, CLKRC_VALUE_30FPS);
   sresult = ParseI2CResult(result);
   Serial.print(F("CLKRC: "));
   Serial.println(sresult);

   result = OV7670WriteReg(DBLV, DBLV_VALUE_30FPS);
   sresult = ParseI2CResult(result);
   Serial.print(F("DBLV: "));
   Serial.println(sresult);
 
   result = OV7670WriteReg(EXHCH, EXHCH_VALUE_30FPS);
   sresult = ParseI2CResult(result);
   Serial.print(F("EXHCH: "));
   Serial.println(sresult);

   result = OV7670WriteReg(EXHCL, EXHCL_VALUE_30FPS);
   sresult = ParseI2CResult(result);
   Serial.print(F("EXHCL: "));
   Serial.println(sresult);
   
   result = OV7670WriteReg(DM_LNL, DM_LNL_VALUE_30FPS);
   sresult = ParseI2CResult(result);
   Serial.print(F("DM_LNL: "));
   Serial.println(sresult);

   result = OV7670WriteReg(DM_LNH, DM_LNH_VALUE_30FPS);
   sresult = ParseI2CResult(result);
   Serial.print(F("DM_LNH: "));
   Serial.println(sresult);

   result = OV7670WriteReg(COM11, COM11_VALUE_30FPS);
   sresult = ParseI2CResult(result);
   Serial.print(F("COM11: "));
   Serial.println(sresult);
   
}

void SetupCameraABLC()
{
   int result = 0;
   String sresult = "";
   
   // If ABLC is off then return otherwise
   // turn on ABLC.
   if (ABLCParam == "AblcOFF")
   {
     return;
   }
   
   Serial.println(F("........ Setting Camera ABLC ......."));
   
   result = OV7670WriteReg(ABLC1, ABLC1_VALUE);
   sresult = ParseI2CResult(result);
   Serial.print(F("ABLC1: "));
   Serial.println(sresult);
  
   result = OV7670WriteReg(THL_ST, THL_ST_VALUE);
   sresult = ParseI2CResult(result);
   Serial.print(F("THL_ST: "));
   Serial.println(sresult);
}

void SetupOV7670ForVGARawRGB()
{
   int result = 0;
   String sresult = "";
   
   Serial.println(F("--------------------------- Setting Camera for VGA (Raw RGB) ---------------------------"));
  
   PHOTO_WIDTH  = 640;
   PHOTO_HEIGHT = 480; 
   PHOTO_BYTES_PER_PIXEL = 1;

   Serial.print(F("Photo Width = "));
   Serial.println(PHOTO_WIDTH);
   
   Serial.print(F("Photo Height = "));
   Serial.println(PHOTO_HEIGHT);
   
   Serial.print(F("Bytes Per Pixel = "));
   Serial.println(PHOTO_BYTES_PER_PIXEL);
   
   
   // Basic Registers
   result = OV7670WriteReg(CLKRC, CLKRC_VALUE_VGA);
   sresult = ParseI2CResult(result);
   Serial.print(F("CLKRC: "));
   Serial.println(sresult);
 
   result = OV7670WriteReg(COM7, COM7_VALUE_VGA );
   //result = OV7670WriteReg(COM7, COM7_VALUE_VGA_COLOR_BAR );
   sresult = ParseI2CResult(result);
   Serial.print(F("COM7: "));
   Serial.println(sresult);

   result = OV7670WriteReg(COM3, COM3_VALUE_VGA);   
   sresult = ParseI2CResult(result);
   Serial.print(F("COM3: "));
   Serial.println(sresult);
   
   result = OV7670WriteReg(COM14, COM14_VALUE_VGA );
   sresult = ParseI2CResult(result);
   Serial.print(F("COM14: "));
   Serial.println(sresult);
  
   result = OV7670WriteReg(SCALING_XSC,SCALING_XSC_VALUE_VGA );
   sresult = ParseI2CResult(result);
   Serial.print(F("SCALING_XSC: "));
   Serial.println(sresult);
  
   result = OV7670WriteReg(SCALING_YSC,SCALING_YSC_VALUE_VGA );    
   sresult = ParseI2CResult(result);
   Serial.print(F("SCALING_YSC: "));
   Serial.println(sresult);
  
   result = OV7670WriteReg(SCALING_DCWCTR, SCALING_DCWCTR_VALUE_VGA );
   sresult = ParseI2CResult(result);
   Serial.print(F("SCALING_DCWCTR: "));
   Serial.println(sresult);
  
   result = OV7670WriteReg(SCALING_PCLK_DIV, SCALING_PCLK_DIV_VALUE_VGA);
   sresult = ParseI2CResult(result);
   Serial.print(F("SCALING_PCLK_DIV: "));
   Serial.println (sresult);
  
   result = OV7670WriteReg(SCALING_PCLK_DELAY,SCALING_PCLK_DELAY_VALUE_VGA);
   sresult = ParseI2CResult(result);
   Serial.print(F("SCALING_PCLK_DELAY: "));
   Serial.println(sresult);
 
   // COM17 - DSP Color Bar Enable/Disable
   // 0000 1000 => to Enable
   // 0x08  
   // COM17_VALUE  0x08 // Activate Color Bar for DSP
   //result = OV7670WriteReg(COM17, COM17_VALUE_AEC_NORMAL_COLOR_BAR);
   result = OV7670WriteReg(COM17, COM17_VALUE_AEC_NORMAL_NO_COLOR_BAR);
   sresult = ParseI2CResult(result);
   Serial.print(F("COM17: "));
   Serial.println(sresult);
  
 
   // Set Additional Parameters
   
   // Set Camera Frames per second
   SetCameraFPSMode();
    
   // Set Camera Automatic Exposure Control
   SetCameraAEC();
   
   // Needed Color Correction, green to red
   result = OV7670WriteReg(0xB0, 0x8c);
   sresult = ParseI2CResult(result);
   Serial.print(F("Setting B0 UNDOCUMENTED register to 0x84:= "));
   Serial.println(sresult);
   
   // Set Camera Saturation
   SetCameraSaturationControl();
     
   // Setup Camera Array Control
   SetupCameraArrayControl();
   
    // Set ADC Control
   SetupCameraADCControl();
   
   // Set Automatic Black Level Calibration
   SetupCameraABLC();
    
 
 
   Serial.println(F("........... Setting Camera Window Output Parameters  ........"));
 
   // Change Window Output parameters after custom scaling
   result = OV7670WriteReg(HSTART, HSTART_VALUE_VGA );
   sresult = ParseI2CResult(result);
   Serial.print(F("HSTART: "));
   Serial.println(sresult);
  
   result = OV7670WriteReg(HSTOP, HSTOP_VALUE_VGA );
   sresult = ParseI2CResult(result);
   Serial.print(F("HSTOP: "));
   Serial.println(sresult);
  
   result = OV7670WriteReg(HREF, HREF_VALUE_VGA );
   sresult = ParseI2CResult(result);
   Serial.print(F("HREF: "));
   Serial.println(sresult);
  
   result = OV7670WriteReg(VSTRT, VSTRT_VALUE_VGA );
   sresult = ParseI2CResult(result);
   Serial.print(F("VSTRT: "));
   Serial.println(sresult);
  
   result = OV7670WriteReg(VSTOP, VSTOP_VALUE_VGA );
   sresult = ParseI2CResult(result);
   Serial.print(F("VSTOP: "));
   Serial.println(sresult);
  
   result = OV7670WriteReg(VREF, VREF_VALUE_VGA );
   sresult = ParseI2CResult(result);
   Serial.print(F("VREF: "));
   Serial.println(sresult);
}

void SetupOV7670ForVGAProcessedBayerRGB()
{
   int result = 0;
   String sresult = "";
   
   // Call Base for VGA Raw Bayer RGB Mode
   SetupOV7670ForVGARawRGB();
  
   Serial.println(F("------------- Setting Camera for VGA (Processed Bayer RGB) ----------------"));
    
   // Set key register for selecting processed bayer rgb output
   result = OV7670WriteReg(COM7, COM7_VALUE_VGA_PROCESSED_BAYER );
   //result = OV7670WriteReg(COM7, COM7_VALUE_VGA_COLOR_BAR );
   sresult = ParseI2CResult(result);
   Serial.print(F("COM7: "));
   Serial.println(sresult);
   
   result = OV7670WriteReg(TSLB, 0x04);
   sresult = ParseI2CResult(result);
   Serial.print(F("Initializing TSLB register result = "));
   Serial.println(sresult);
   
   // Needed Color Correction, green to red
   result = OV7670WriteReg(0xB0, 0x8c);
   sresult = ParseI2CResult(result);
   Serial.print(F("Setting B0 UNDOCUMENTED register to 0x84:= "));
   Serial.println(sresult);
  
   // Set Camera Automatic White Balance
   SetupCameraAWB();
   
   // Denoise and Edge Enhancement
   SetupCameraDenoiseEdgeEnhancement();
}

void SetupCameraAverageBasedAECAGC()
{
   int result = 0;
   String sresult = "";
   
   Serial.println(F("-------------- Setting Camera Average Based AEC/AGC Registers ---------------"));
  
   result = OV7670WriteReg(AEW, AEW_VALUE);
   sresult = ParseI2CResult(result);
   Serial.print(F("AEW: "));
   Serial.println(sresult);

   result = OV7670WriteReg(AEB, AEB_VALUE);
   sresult = ParseI2CResult(result);
   Serial.print(F("AEB: "));
   Serial.println(sresult);
   
   result = OV7670WriteReg(VPT, VPT_VALUE);
   sresult = ParseI2CResult(result);
   Serial.print(F("VPT: "));
   Serial.println(sresult);
   
   result = OV7670WriteReg(HAECC7, HAECC7_VALUE_AVERAGE_AEC_ON);
   sresult = ParseI2CResult(result);
   Serial.print(F("HAECC7: "));
   Serial.println(sresult);   
}

void SetCameraHistogramBasedAECAGC()
{
   int result = 0;
   String sresult = "";
   
   Serial.println(F("-------------- Setting Camera Histogram Based AEC/AGC Registers ---------------"));
  
   result = OV7670WriteReg(AEW, AEW_VALUE);
   sresult = ParseI2CResult(result);
   Serial.print(F("AEW: "));
   Serial.println(sresult);

   result = OV7670WriteReg(AEB, AEB_VALUE);
   sresult = ParseI2CResult(result);
   Serial.print(F("AEB: "));
   Serial.println(sresult);
    
   result = OV7670WriteReg(HAECC1, HAECC1_VALUE);
   sresult = ParseI2CResult(result);
   Serial.print(F("HAECC1: "));
   Serial.println(sresult);
  
   result = OV7670WriteReg(HAECC2, HAECC2_VALUE);
   sresult = ParseI2CResult(result);
   Serial.print(F("HAECC2: "));
   Serial.println(sresult);
  
   result = OV7670WriteReg(HAECC3, HAECC3_VALUE);
   sresult = ParseI2CResult(result);
   Serial.print(F("HAECC3: "));
   Serial.println(sresult);
  
   result = OV7670WriteReg(HAECC4, HAECC4_VALUE);
   sresult = ParseI2CResult(result);
   Serial.print(F("HAECC4: "));
   Serial.println(sresult);
  
   result = OV7670WriteReg(HAECC5, HAECC5_VALUE);
   sresult = ParseI2CResult(result);
   Serial.print(F("HAECC5: "));
   Serial.println(sresult);
  
   result = OV7670WriteReg(HAECC6, HAECC6_VALUE);
   sresult = ParseI2CResult(result);
   Serial.print(F("HAECC6: "));
   Serial.println(sresult);
  
   result = OV7670WriteReg(HAECC7, HAECC7_VALUE_HISTOGRAM_AEC_ON);
   sresult = ParseI2CResult(result);
   Serial.print(F("HAECC7: "));
   Serial.println(sresult);  
}

void SetupOV7670ForQVGAYUV()
{
   int result = 0;
   String sresult = "";
   
   Serial.println(F("--------------------------- Setting Camera for QVGA (YUV) ---------------------------"));
  
   PHOTO_WIDTH  = 320;
   PHOTO_HEIGHT = 240; 
   PHOTO_BYTES_PER_PIXEL = 2;

   Serial.print(F("Photo Width = "));
   Serial.println(PHOTO_WIDTH);
   
   Serial.print(F("Photo Height = "));
   Serial.println(PHOTO_HEIGHT);
   
   Serial.print(F("Bytes Per Pixel = "));
   Serial.println(PHOTO_BYTES_PER_PIXEL);
   
   
   // Basic Registers
   result = OV7670WriteReg(CLKRC, CLKRC_VALUE_QVGA);
   sresult = ParseI2CResult(result);
   Serial.print(F("CLKRC: "));
   Serial.println(sresult);
 
   result = OV7670WriteReg(COM7, COM7_VALUE_QVGA );
   //result = OV7670WriteReg(COM7, COM7_VALUE_QVGA_COLOR_BAR );
   sresult = ParseI2CResult(result);
   Serial.print(F("COM7: "));
   Serial.println(sresult);

   result = OV7670WriteReg(COM3, COM3_VALUE_QVGA);  
   sresult = ParseI2CResult(result);
   Serial.print(F("COM3: "));
   Serial.println(sresult);
   
   result = OV7670WriteReg(COM14, COM14_VALUE_QVGA );
   sresult = ParseI2CResult(result);
   Serial.print(F("COM14: "));
   Serial.println(sresult);
  
   result = OV7670WriteReg(SCALING_XSC,SCALING_XSC_VALUE_QVGA );
   sresult = ParseI2CResult(result);
   Serial.print(F("SCALING_XSC: "));
   Serial.println(sresult);
  
   result = OV7670WriteReg(SCALING_YSC,SCALING_YSC_VALUE_QVGA );    
   sresult = ParseI2CResult(result);
   Serial.print(F("SCALING_YSC: "));
   Serial.println(sresult);
  
   result = OV7670WriteReg(SCALING_DCWCTR, SCALING_DCWCTR_VALUE_QVGA );
   sresult = ParseI2CResult(result);
   Serial.print(F("SCALING_DCWCTR: "));
   Serial.println(sresult);
  
   result = OV7670WriteReg(SCALING_PCLK_DIV, SCALING_PCLK_DIV_VALUE_QVGA);
   sresult = ParseI2CResult(result);
   Serial.print(F("SCALING_PCLK_DIV: "));
   Serial.println (sresult);
  
   result = OV7670WriteReg(SCALING_PCLK_DELAY,SCALING_PCLK_DELAY_VALUE_QVGA );
   sresult = ParseI2CResult(result);
   Serial.print(F("SCALING_PCLK_DELAY: "));
   Serial.println(sresult);
   
   // YUV order control change from default use with COM13
   //result = OV7670WriteReg(TSLB, TSLB_VALUE_YUYV_AUTO_OUTPUT_WINDOW_ENABLED);
   //result = OV7670WriteReg(TSLB, TSLB_VALUE_YUYV_AUTO_OUTPUT_WINDOW_DISABLED); // Works
   //result = OV7670WriteReg(TSLB, TSLB_VALUE_UYVY_AUTO_OUTPUT_WINDOW_DISABLED);
   //result = OV7670WriteReg(TSLB, TSLB_VALUE_TESTVALUE); 
   result = OV7670WriteReg(TSLB, 0x04);
   sresult = ParseI2CResult(result);
   Serial.print(F("TSLB: "));
   Serial.println(sresult);
  
   //COM13
   //result = OV7670WriteReg(COM13, COM13_VALUE_GAMMA_YUYV);
   //result = OV7670WriteReg(COM13, COM13_VALUE_DEFAULT);
   //result = OV7670WriteReg(COM13, COM13_VALUE_GAMMA_YVYU);
   //result = OV7670WriteReg(COM13, COM13_VALUE_NOGAMMA_YUYV);
   //result = OV7670WriteReg(COM13, COM13_VALUE_YUYV_UVSATAUTOADJ_ON); 
   // { REG_COM13, /*0xc3*/0x48 } // error in datasheet bit 3 controls YUV order
   //result = OV7670WriteReg(COM13, 0x48);
   //result = OV7670WriteReg(COM13, 0xC8); // needed for correct color bar
   result = OV7670WriteReg(COM13, 0xC2);   // from YCbCr reference specs 
   //result = OV7670WriteReg(COM13, 0x82);
   sresult = ParseI2CResult(result);
   Serial.print(F("COM13: "));
   Serial.println(sresult);
  
   // COM17 - DSP Color Bar Enable/Disable
   // 0000 1000 => to Enable
   // 0x08  
   // COM17_VALUE  0x08 // Activate Color Bar for DSP
   //result = OV7670WriteReg(COM17, COM17_VALUE_AEC_NORMAL_COLOR_BAR);
   result = OV7670WriteReg(COM17, COM17_VALUE_AEC_NORMAL_NO_COLOR_BAR);
   sresult = ParseI2CResult(result);
   Serial.print(F("COM17: "));
   Serial.println(sresult);
  
 
 
   // Set Additional Parameters
   
   // Set Camera Frames per second
   SetCameraFPSMode();
    
   // Set Camera Automatic Exposure Control
   SetCameraAEC();
   
   // Set Camera Automatic White Balance
   SetupCameraAWB();
   
   // Setup Undocumented Registers - Needed Minimum
   SetupCameraUndocumentedRegisters();
   
   // Set Color Matrix for YUV
   if (YUVMatrixParam == "YUVMatrixOn")
   {
     SetCameraColorMatrixYUV();
   }
  
   // Set Camera Saturation
   SetCameraSaturationControl();
   
    // Denoise and Edge Enhancement
   SetupCameraDenoiseEdgeEnhancement();
   
   
   // Set up Camera Array Control
   SetupCameraArrayControl();
   
   
   // Set ADC Control
   SetupCameraADCControl();
   
   
   // Set Automatic Black Level Calibration
   SetupCameraABLC();
    
  
   Serial.println(F("........... Setting Camera Window Output Parameters  ........"));
 
   // Change Window Output parameters after custom scaling
   result = OV7670WriteReg(HSTART, HSTART_VALUE_QVGA );
   sresult = ParseI2CResult(result);
   Serial.print(F("HSTART: "));
   Serial.println(sresult);
  
   result = OV7670WriteReg(HSTOP, HSTOP_VALUE_QVGA );
   sresult = ParseI2CResult(result);
   Serial.print(F("HSTOP: "));
   Serial.println(sresult);
  
   result = OV7670WriteReg(HREF, HREF_VALUE_QVGA );
   sresult = ParseI2CResult(result);
   Serial.print(F("HREF: "));
   Serial.println(sresult);
  
   result = OV7670WriteReg(VSTRT, VSTRT_VALUE_QVGA );
   sresult = ParseI2CResult(result);
   Serial.print(F("VSTRT: "));
   Serial.println(sresult);
  
   result = OV7670WriteReg(VSTOP, VSTOP_VALUE_QVGA );
   sresult = ParseI2CResult(result);
   Serial.print(F("VSTOP: "));
   Serial.println(sresult);
  
   result = OV7670WriteReg(VREF, VREF_VALUE_QVGA );
   sresult = ParseI2CResult(result);
   Serial.print(F("VREF: "));
   Serial.println(sresult);
}

void SetupCameraNightMode()
{
   int result = 0;
   String sresult = "";
   
   Serial.println(F("......... Turning NIGHT MODE ON ........"));
   result = OV7670WriteReg(CLKRC, CLKRC_VALUE_NIGHTMODE_AUTO);
   sresult = ParseI2CResult(result);
   Serial.print(F("CLKRC: "));
   Serial.println(sresult);
 
   result = OV7670WriteReg(COM11, COM11_VALUE_NIGHTMODE_AUTO);
   sresult = ParseI2CResult(result);
   Serial.print(F("COM11: "));
   Serial.println(sresult); 
}


void SetupCameraSimpleAutomaticWhiteBalance()
{
 /*
   i2c_salve_Address = 0x42;
   write_i2c(0x13, 0xe7); //AWB on
   write_i2c(0x6f, 0x9f); // Simple AWB
 */
 
   int result = 0;
   String sresult = "";
   
   Serial.println(F("........... Setting Camera to Simple AWB ........"));
  
   // COM8
   //result = OV7670WriteReg(0x13, 0xE7);
   result = OV7670WriteReg(COM8, COM8_VALUE_AWB_ON);
   sresult = ParseI2CResult(result);
   Serial.print(F("COM8(0x13): "));
   Serial.println(sresult);
 
   // AWBCTR0
   //result = OV7670WriteReg(0x6f, 0x9f);
   result = OV7670WriteReg(AWBCTR0, AWBCTR0_VALUE_NORMAL);
   sresult = ParseI2CResult(result);
   Serial.print(F("AWBCTR0 Control Register 0(0x6F): "));
   Serial.println(sresult);
}

void SetupCameraAdvancedAutomaticWhiteBalance()
{
   int result = 0;
   String sresult = "";
   
   Serial.println(F("........... Setting Camera to Advanced AWB ........"));
  
   // AGC, AWB, and AEC Enable
   result = OV7670WriteReg(0x13, 0xE7);
   sresult = ParseI2CResult(result);
   Serial.print(F("COM8(0x13): "));
   Serial.println(sresult);
 
   // AWBCTR0 
   result = OV7670WriteReg(0x6f, 0x9E);
   sresult = ParseI2CResult(result);
   Serial.print(F("AWB Control Register 0(0x6F): "));
   Serial.println(sresult);
}

void SetupCameraGain()
{
   int result = 0;
   String sresult = "";
   
   Serial.println(F("........... Setting Camera Gain ........"));
   
   // Set Maximum Gain
   //result = OV7670WriteReg(COM9, COM9_VALUE_MAX_GAIN_128X);
   result = OV7670WriteReg(COM9, COM9_VALUE_4XGAIN);
   //result = OV7670WriteReg(COM9, 0x18);
   sresult = ParseI2CResult(result);
   Serial.print(F("COM9: "));
   Serial.println(sresult);
   
   // Set Blue Gain
   //{ REG_BLUE, 0x40 },
   result = OV7670WriteReg(BLUE, BLUE_VALUE);
   sresult = ParseI2CResult(result);
   Serial.print(F("BLUE GAIN: "));
   Serial.println(sresult);
   
   // Set Red Gain
   //{ REG_RED, 0x60 },
   result = OV7670WriteReg(RED, RED_VALUE);
   sresult = ParseI2CResult(result);
   Serial.print(F("RED GAIN: "));
   Serial.println(sresult);
   
   
   // Set Green Gain
   //{ 0x6a, 0x40 }, 
   result = OV7670WriteReg(GGAIN, GGAIN_VALUE);
   sresult = ParseI2CResult(result);
   Serial.print(F("GREEN GAIN: "));
   Serial.println(sresult);
   
   
   // Enable AWB Gain
   // REG_COM16	0x41	/* Control 16 */
   // COM16_AWBGAIN   0x08	  /* AWB gain enable */
   // { REG_COM16, COM16_AWBGAIN }, 
   result = OV7670WriteReg(COM16, COM16_VALUE);
   sresult = ParseI2CResult(result);
   Serial.print(F("COM16(ENABLE GAIN): "));
   Serial.println(sresult);
   
}

void SetCameraSaturationControl()
{
  int result = 0;
  String sresult = "";
  
  Serial.println(F("........... Setting Camera Saturation Level ........"));
  result = OV7670WriteReg(SATCTR, SATCTR_VALUE);
  sresult = ParseI2CResult(result);
  Serial.print(F("SATCTR: "));
  Serial.println(sresult);
}

void SetCameraColorMatrixYUV()
{
  int result = 0;
  String sresult = "";
  
  Serial.println(F("........... Setting Camera Color Matrix for YUV ........"));
  
  result = OV7670WriteReg(MTX1, MTX1_VALUE);
  sresult = ParseI2CResult(result);
  Serial.print(F("MTX1: "));
  Serial.println(sresult);
 
  result = OV7670WriteReg(MTX2, MTX2_VALUE);
  sresult = ParseI2CResult(result);
  Serial.print(F("MTX2: "));
  Serial.println(sresult);
 
  result = OV7670WriteReg(MTX3, MTX3_VALUE);
  sresult = ParseI2CResult(result);
  Serial.print(F("MTX3: "));
  Serial.println(sresult);
 
  result = OV7670WriteReg(MTX4, MTX4_VALUE);
  sresult = ParseI2CResult(result);
  Serial.print(F("MTX4: "));
  Serial.println(sresult);
 
  result = OV7670WriteReg(MTX5, MTX5_VALUE);
  sresult = ParseI2CResult(result);
  Serial.print(F("MTX5: "));
  Serial.println(sresult);
 
  result = OV7670WriteReg(MTX6, MTX6_VALUE);
  sresult = ParseI2CResult(result);
  Serial.print(F("MTX6: "));
  Serial.println(sresult);
 
  result = OV7670WriteReg(CONTRAS, CONTRAS_VALUE);
  sresult = ParseI2CResult(result);
  Serial.print(F("CONTRAS: "));
  Serial.println(sresult);
 
  result = OV7670WriteReg(MTXS, MTXS_VALUE);
  sresult = ParseI2CResult(result);
  Serial.print(F("MTXS: "));
  Serial.println(sresult);  
}

void SetCameraFPSMode()
{
   // Set FPS for Camera
   if (FPSParam == "ThirtyFPS")
   {
     SetupCameraFor30FPS();
   }    
   else
   if (FPSParam == "NightMode")
   {
     SetupCameraNightMode();
   } 
}

void SetCameraAEC()
{
    // Process AEC
   if (AECParam == "AveAEC")
   {
     // Set Camera's Average AEC/AGC Parameters  
     SetupCameraAverageBasedAECAGC();  
   }
   else
   if (AECParam == "HistAEC")
   { 
     // Set Camera AEC algorithim to Histogram
     SetCameraHistogramBasedAECAGC();
   }
}

void SetupCameraAWB()
{
   // Set AWB Mode
   if (AWBParam == "SAWB")
   {
     // Set Simple Automatic White Balance
     SetupCameraSimpleAutomaticWhiteBalance(); // OK
      
     // Set Gain Config
     SetupCameraGain();
   }
   else
   if (AWBParam == "AAWB")
   {
     // Set Advanced Automatic White Balance
     SetupCameraAdvancedAutomaticWhiteBalance(); // ok
   
     // Set Camera Automatic White Balance Configuration
     SetupCameraAdvancedAutoWhiteBalanceConfig(); // ok
     
     // Set Gain Config
     SetupCameraGain();
   }
}


void SetupCameraDenoise()
{  
   int result = 0;
   String sresult = "";
   
   Serial.println(F("........... Setting Camera Denoise  ........"));
  
   result = OV7670WriteReg(DNSTH, DNSTH_VALUE);
   sresult = ParseI2CResult(result);
   Serial.print(F("DNSTH: "));
   Serial.println(sresult);
 
   result = OV7670WriteReg(REG77, REG77_VALUE);
   sresult = ParseI2CResult(result);
   Serial.print(F("REG77: "));
   Serial.println(sresult);
}


void SetupCameraEdgeEnhancement()
{
   int result = 0;
   String sresult = "";
   
   Serial.println(F("........... Setting Camera Edge Enhancement  ........"));
  
   result = OV7670WriteReg(EDGE, EDGE_VALUE);
   sresult = ParseI2CResult(result);
   Serial.print(F("EDGE: "));
   Serial.println(sresult);
 
   result = OV7670WriteReg(REG75, REG75_VALUE);
   sresult = ParseI2CResult(result);
   Serial.print(F("REG75: "));
   Serial.println(sresult);
 
   result = OV7670WriteReg(REG76, REG76_VALUE);
   sresult = ParseI2CResult(result);
   Serial.print(F("REG76: "));
   Serial.println(sresult);
}



void SetupCameraDenoiseEdgeEnhancement()
{
   int result = 0;
   String sresult = "";
   
   if ((DenoiseParam == "DenoiseYes")&& 
       (EdgeParam == "EdgeYes"))
      {
        SetupCameraDenoise();
        SetupCameraEdgeEnhancement();
        result = OV7670WriteReg(COM16, COM16_VALUE_DENOISE_ON__EDGE_ENHANCEMENT_ON__AWBGAIN_ON);
        sresult = ParseI2CResult(result);
        Serial.print(F("COM16: "));
        Serial.println(sresult);
      }
      else
      if ((DenoiseParam == "DenoiseYes")&& 
          (EdgeParam == "EdgeNo"))
       {
         SetupCameraDenoise();
         result = OV7670WriteReg(COM16, COM16_VALUE_DENOISE_ON__EDGE_ENHANCEMENT_OFF__AWBGAIN_ON);
         sresult = ParseI2CResult(result);
         Serial.print(F("COM16: "));
         Serial.println(sresult);
       }
       else
       if ((DenoiseParam == "DenoiseNo")&& 
          (EdgeParam == "EdgeYes"))
          {
            SetupCameraEdgeEnhancement();
            result = OV7670WriteReg(COM16, COM16_VALUE_DENOISE_OFF__EDGE_ENHANCEMENT_ON__AWBGAIN_ON);
            sresult = ParseI2CResult(result);
            Serial.print(F("COM16: "));
            Serial.println(sresult);
          }
}


/*

void SetCameraGamma()
{
   int result = 0;
   String sresult = "";
   
   Serial.println(F("........... Setting Camera Gamma  ........"));
  
   result = OV7670WriteReg(SLOP, SLOP_VALUE);
   sresult = ParseI2CResult(result);
   Serial.print(F("SLOP: "));
   Serial.println(sresult);

   result = OV7670WriteReg(GAM1, GAM1_VALUE);
   sresult = ParseI2CResult(result);
   Serial.print(F("GAM1: "));
   Serial.println(sresult);
  
   result = OV7670WriteReg(GAM2, GAM2_VALUE);
   sresult = ParseI2CResult(result);
   Serial.print(F("GAM2: "));
   Serial.println(sresult);
  
   result = OV7670WriteReg(GAM3, GAM3_VALUE);
   sresult = ParseI2CResult(result);
   Serial.print(F("GAM3: "));
   Serial.println(sresult);
 
   result = OV7670WriteReg(GAM4, GAM4_VALUE);
   sresult = ParseI2CResult(result);
   Serial.print(F("GAM4: "));
   Serial.println(sresult);
 
   result = OV7670WriteReg(GAM5, GAM5_VALUE);
   sresult = ParseI2CResult(result);
   Serial.print(F("GAM5: "));
   Serial.println(sresult);
 
   result = OV7670WriteReg(GAM6, GAM6_VALUE);
   sresult = ParseI2CResult(result);
   Serial.print(F("GAM6: "));
   Serial.println(sresult);
 
   result = OV7670WriteReg(GAM7, GAM7_VALUE);
   sresult = ParseI2CResult(result);
   Serial.print(F("GAM7: "));
   Serial.println(sresult);
 
   result = OV7670WriteReg(GAM8, GAM8_VALUE);
   sresult = ParseI2CResult(result);
   Serial.print(F("GAM8: "));
   Serial.println(sresult);
 
   result = OV7670WriteReg(GAM9, GAM9_VALUE);
   sresult = ParseI2CResult(result);
   Serial.print(F("GAM9: "));
   Serial.println(sresult);
 
   result = OV7670WriteReg(GAM10, GAM10_VALUE);
   sresult = ParseI2CResult(result);
   Serial.print(F("GAM10: "));
   Serial.println(sresult);
 
   result = OV7670WriteReg(GAM11, GAM11_VALUE);
   sresult = ParseI2CResult(result);
   Serial.print(F("GAM11: "));
   Serial.println(sresult);
 
   result = OV7670WriteReg(GAM12, GAM12_VALUE);
   sresult = ParseI2CResult(result);
   Serial.print(F("GAM12: "));
   Serial.println(sresult);
 
   result = OV7670WriteReg(GAM13, GAM13_VALUE);
   sresult = ParseI2CResult(result);
   Serial.print(F("GAM13: "));
   Serial.println(sresult);
 
   result = OV7670WriteReg(GAM14, GAM14_VALUE);
   sresult = ParseI2CResult(result);
   Serial.print(F("GAM14: "));
   Serial.println(sresult);
 
   result = OV7670WriteReg(GAM15, GAM15_VALUE);
   sresult = ParseI2CResult(result);
   Serial.print(F("GAM15: "));
   Serial.println(sresult);
}
*/



void SetupCameraArrayControl()
{
   int result = 0;
   String sresult = "";
   
   Serial.println(F("........... Setting Camera Array Control  ........"));
  
   result = OV7670WriteReg(CHLF, CHLF_VALUE);
   sresult = ParseI2CResult(result);
   Serial.print(F("CHLF: "));
   Serial.println(sresult);
 
   result = OV7670WriteReg(ARBLM, ARBLM_VALUE);
   sresult = ParseI2CResult(result);
   Serial.print(F("ARBLM: "));
   Serial.println(sresult);
}



void SetupCameraADCControl()
{
   int result = 0;
   String sresult = "";
   
   Serial.println(F("........... Setting Camera ADC Control  ........"));
  
   result = OV7670WriteReg(ADCCTR1, ADCCTR1_VALUE);
   sresult = ParseI2CResult(result);
   Serial.print(F("ADCCTR1: "));
   Serial.println(sresult);
 
   result = OV7670WriteReg(ADCCTR2, ADCCTR2_VALUE);
   sresult = ParseI2CResult(result);
   Serial.print(F("ADCCTR2: "));
   Serial.println(sresult);
 
   result = OV7670WriteReg(ADC, ADC_VALUE);
   sresult = ParseI2CResult(result);
   Serial.print(F("ADC: "));
   Serial.println(sresult);
  
   result = OV7670WriteReg(ACOM, ACOM_VALUE);
   sresult = ParseI2CResult(result);
   Serial.print(F("ACOM: "));
   Serial.println(sresult);

   result = OV7670WriteReg(OFON, OFON_VALUE);
   sresult = ParseI2CResult(result);
   Serial.print(F("OFON: "));
   Serial.println(sresult);  
}



void SetupOV7670ForQQVGAYUV()
{
   int result = 0;
   String sresult = "";
   
   Serial.println(F("--------------------------- Setting Camera for QQVGA YUV ---------------------------"));
  
   PHOTO_WIDTH  = 160;
   PHOTO_HEIGHT = 120; 
   PHOTO_BYTES_PER_PIXEL = 2;

   Serial.print(F("Photo Width = "));
   Serial.println(PHOTO_WIDTH);
   
   Serial.print(F("Photo Height = "));
   Serial.println(PHOTO_HEIGHT);
   
   Serial.print(F("Bytes Per Pixel = "));
   Serial.println(PHOTO_BYTES_PER_PIXEL);
   
    
   Serial.println(F("........... Setting Basic QQVGA Parameters  ........"));
 
   result = OV7670WriteReg(CLKRC, CLKRC_VALUE_QQVGA);
   sresult = ParseI2CResult(result);
   Serial.print(F("CLKRC: "));
   Serial.println(sresult);
 
   result = OV7670WriteReg(COM7, COM7_VALUE_QQVGA );
   //result = OV7670WriteReg(COM7, COM7_VALUE_QQVGA_COLOR_BAR );
   sresult = ParseI2CResult(result);
   Serial.print(F("COM7: "));
   Serial.println(sresult);

   result = OV7670WriteReg(COM3, COM3_VALUE_QQVGA); 
   sresult = ParseI2CResult(result);
   Serial.print(F("COM3: "));
   Serial.println(sresult);
   
   result = OV7670WriteReg(COM14, COM14_VALUE_QQVGA );
   sresult = ParseI2CResult(result);
   Serial.print(F("COM14: "));
   Serial.println(sresult);
  
   result = OV7670WriteReg(SCALING_XSC,SCALING_XSC_VALUE_QQVGA );
   sresult = ParseI2CResult(result);
   Serial.print(F("SCALING_XSC: "));
   Serial.println(sresult);
  
   result = OV7670WriteReg(SCALING_YSC,SCALING_YSC_VALUE_QQVGA );    
   sresult = ParseI2CResult(result);
   Serial.print(F("SCALING_YSC: "));
   Serial.println(sresult);
  
   result = OV7670WriteReg(SCALING_DCWCTR, SCALING_DCWCTR_VALUE_QQVGA );
   sresult = ParseI2CResult(result);
   Serial.print(F("SCALING_DCWCTR: "));
   Serial.println(sresult);
  
   result = OV7670WriteReg(SCALING_PCLK_DIV, SCALING_PCLK_DIV_VALUE_QQVGA);
   sresult = ParseI2CResult(result);
   Serial.print(F("SCALING_PCLK_DIV: "));
   Serial.println (sresult);
  
   result = OV7670WriteReg(SCALING_PCLK_DELAY,SCALING_PCLK_DELAY_VALUE_QQVGA );
   sresult = ParseI2CResult(result);
   Serial.print(F("SCALING_PCLK_DELAY: "));
   Serial.println(sresult);
   
   // YUV order control change from default use with COM13
   result = OV7670WriteReg(TSLB, TSLB_VALUE_YUYV_AUTO_OUTPUT_WINDOW_DISABLED); // Works
   sresult = ParseI2CResult(result);
   Serial.print(F("TSLB: "));
   Serial.println(sresult);
  
   //COM13
   //result = OV7670WriteReg(COM13, COM13_VALUE_GAMMA_YUYV);
   //result = OV7670WriteReg(COM13, COM13_VALUE_DEFAULT);
   //result = OV7670WriteReg(COM13, COM13_VALUE_GAMMA_YVYU);
   //result = OV7670WriteReg(COM13, COM13_VALUE_NOGAMMA_YUYV);
   //result = OV7670WriteReg(COM13, COM13_VALUE_YUYV_UVSATAUTOADJ_ON); 
   // { REG_COM13, /*0xc3*/0x48 } // error in datasheet bit 3 controls YUV order
   //result = OV7670WriteReg(COM13, 0x48);
   result = OV7670WriteReg(COM13, 0xC8);  // Gamma Enabled, UV Auto Adj On
   sresult = ParseI2CResult(result);
   Serial.print(F("COM13: "));
   Serial.println(sresult);
  
   // COM17 - DSP Color Bar Enable/Disable
   // 0000 1000 => to Enable
   // 0x08  
   // COM17_VALUE  0x08 // Activate Color Bar for DSP
   //result = OV7670WriteReg(COM17, COM17_VALUE_AEC_NORMAL_COLOR_BAR);
   result = OV7670WriteReg(COM17, COM17_VALUE_AEC_NORMAL_NO_COLOR_BAR);
   sresult = ParseI2CResult(result);
   Serial.print(F("COM17: "));
   Serial.println(sresult);
  
  
   // Set Additional Parameters
   // Set Camera Frames per second
   SetCameraFPSMode();
    
   // Set Camera Automatic Exposure Control
   SetCameraAEC();
   
   // Set Camera Automatic White Balance
   SetupCameraAWB();
   
   // Setup Undocumented Registers - Needed Minimum
   SetupCameraUndocumentedRegisters();
  
 
   // Set Color Matrix for YUV
   if (YUVMatrixParam == "YUVMatrixOn")
   {
     SetCameraColorMatrixYUV();
   }
  
   // Set Camera Saturation
   SetCameraSaturationControl();
   
   // Denoise and Edge Enhancement
   SetupCameraDenoiseEdgeEnhancement();
   
   // Set New Gamma Values
   //SetCameraGamma();
   
   // Set Array Control
   SetupCameraArrayControl();
   
   // Set ADC Control
   SetupCameraADCControl();

   // Set Automatic Black Level Calibration
   SetupCameraABLC();
    

   Serial.println(F("........... Setting Camera Window Output Parameters  ........"));
 
   // Change Window Output parameters after custom scaling
   result = OV7670WriteReg(HSTART, HSTART_VALUE_QQVGA );
   sresult = ParseI2CResult(result);
   Serial.print(F("HSTART: "));
   Serial.println(sresult);
  
   result = OV7670WriteReg(HSTOP, HSTOP_VALUE_QQVGA );
   sresult = ParseI2CResult(result);
   Serial.print(F("HSTOP: "));
   Serial.println(sresult);
  
   result = OV7670WriteReg(HREF, HREF_VALUE_QQVGA );
   sresult = ParseI2CResult(result);
   Serial.print(F("HREF: "));
   Serial.println(sresult);
  
   result = OV7670WriteReg(VSTRT, VSTRT_VALUE_QQVGA );
   sresult = ParseI2CResult(result);
   Serial.print(F("VSTRT: "));
   Serial.println(sresult);
  
   result = OV7670WriteReg(VSTOP, VSTOP_VALUE_QQVGA );
   sresult = ParseI2CResult(result);
   Serial.print(F("VSTOP: "));
   Serial.println(sresult);
  
   result = OV7670WriteReg(VREF, VREF_VALUE_QQVGA );
   sresult = ParseI2CResult(result);
   Serial.print(F("VREF: "));
   Serial.println(sresult);
}

void CaptureOV7670Frame()
{
   unsigned long DurationStart = 0;
   unsigned long DurationStop = 0;
   unsigned long TimeForCaptureStart = 0;
   unsigned long TimeForCaptureEnd = 0;
   unsigned long ElapsedTime = 0;
   
   //Capture one frame into FIFO memory
   
   // 0. Initialization. 
   Serial.println();
   Serial.println(F("Starting Capture of Photo ..."));
   TimeForCaptureStart = millis();
 
   // 1. Wait for VSync to pulse to indicate the start of the image
   DurationStart = pulseIn(VSYNC, LOW);
 
   // 2. Reset Write Pointer to 0. Which is the beginning of frame
   PulseLowEnabledPin(WRST, 6); // 3 microseconds + 3 microseconds for error factor on Arduino

   // 3. Set FIFO Write Enable to active (high) so that image can be written to ram
   digitalWrite(WEN, HIGH);
  
   // 4. Wait for VSync to pulse again to indicate the end of the frame capture
   DurationStop = pulseIn(VSYNC, LOW);
  
   // 5. Set FIFO Write Enable to nonactive (low) so that no more images can be written to the ram
   digitalWrite(WEN, LOW);
     
   // 6. Print out Stats
   TimeForCaptureEnd = millis();
   ElapsedTime = TimeForCaptureEnd - TimeForCaptureStart;
   
   Serial.print(F("Time for Frame Capture (milliseconds) = "));
   Serial.println(ElapsedTime);
   
   Serial.print(F("VSync beginning duration (microseconds) = "));
   Serial.println(DurationStart);
   
   Serial.print(F("VSync end duration (microseconds) = "));
   Serial.println(DurationStop);
   
   // 7. WAIT so that new data can appear on output pins Read new data.
   delay(2);
}

String CreatePhotoFilename()
{
  String Filename = "";
  String Ext = "";
  
  // Creates filename that the photo will be saved under
  
  // Create file extension
  // If Command = QQVGA or QVGA then extension is .yuv
  if ((Command == "QQVGA") || (Command == "QVGA"))
  {
     Ext = ".yuv";    
  }
  else
  if ((Command == "VGA") || (Command == "VGAP"))
  {
    Ext = ".raw"; 
  }
  
  // Create Filename from
  // Resolution + PhotoNumber + Extension
  Filename = Command + PhotoTakenCount + Ext; 
 
  return Filename;
}

String CreatePhotoInfoFilename()
{
  String Filename = "";
  String Ext = "";
  
  // Creates filename that the information about the photo will 
  // be saved under. 
  
  // Create file extension
  Ext = ".txt";
  
  // Create Filename from
  // Resolution + PhotoNumber + Extension
  Filename = Command + PhotoTakenCount + Ext; 
 
  return Filename;
}

String CreatePhotoInfo()
{
  String Info = "";
  
  Info = Command + " " + FPSParam + " " + AWBParam + " " + AECParam + " " + YUVMatrixParam + " " + 
         DenoiseParam + " " + EdgeParam + " " + ABLCParam;
         
  return Info;
}

void CreatePhotoInfoFile()
{
   // Creates the photo information file based on current settings
   
   // .txt information File for Photo
   File InfoFile;
   
   // Create name of Photo To save based on certain parameters
   String Filename = CreatePhotoInfoFilename();
   
   // Check if file already exists and remove it if it does.
   CheckRemoveFile(Filename);
  
   // Open File
   InfoFile = SD.open(Filename.c_str(), FILE_WRITE);
   
   // Test if file actually open
   if (!InfoFile)
   {
     Serial.println(F("\nCritical ERROR ... Can not open Photo Info File for output ... "));
     return;
   } 
  
   // Write Info to file
   String Data = CreatePhotoInfo();
   InfoFile.println(Data);
   
   // Close SD Card File
   InfoFile.close();
}

// Converts pin HIGH/LOW values on pins at positions 0-7 to a corresponding byte value
byte ConvertPinValueToByteValue(int PinValue, int PinPosition)
{
  byte ByteValue = 0;
  if (PinValue == HIGH)
  {
    ByteValue = 1 << PinPosition;
  }
  
  return ByteValue;
}

void ReadTransmitCapturedFrame()
{
   // Read captured frame from FIFO memory and send each byte as it is read to the Android controller
   // via bluetooth.
    
   // Set Output Enable OE to active (low).
   // * Make sure to connect the OE output to ground.
   
   // Reset the FIFO read pointer by setting RRST to active (low) then bringing it back to high.
   // *Done from previous CaptureOV7670Frame() assuming WRST and RRST are tied together.
   
   // Read in the QQVGA image that is captured in the camera buffer by reading in the 38400 bytes that make up the 
   //   YUV photo 
   
   byte PixelData = 0;
   byte PinVal7 = 0;
   byte PinVal6 = 0;
   byte PinVal5 = 0;
   byte PinVal4 = 0;
   byte PinVal3 = 0;
   byte PinVal2 = 0;
   byte PinVal1 = 0;
   byte PinVal0 = 0;
   
   Serial.println(F("Starting Transmission of Photo To SDCard ..."));
   
   
   //////////////////// Code for SD Card /////////////////////////////////
   
   // Image file to write to
   File ImageOutputFile;
   
   // Create name of Photo To save based on certain parameters
   String Filename = CreatePhotoFilename();
   
   // Check if file already exists and remove it if it does.
   CheckRemoveFile(Filename);
  
   ImageOutputFile = SD.open(Filename.c_str(), FILE_WRITE);
   
   // Test if file actually open
   if (!ImageOutputFile)
   {
     Serial.println(F("\nCritical ERROR ... Can not open Image Ouput File for output ... "));
     return;
   } 
   ////////////////////////////////////////////////////////////////////////
   
   // Set Read Buffer Pointer to start of frame
   digitalWrite(RRST, LOW);
   PulsePin(RCLK, 1); 
   PulsePin(RCLK, 1);
   PulsePin(RCLK, 1);
   digitalWrite(RRST, HIGH);
   
   unsigned long  ByteCounter = 0;
   for (int height = 0; height < PHOTO_HEIGHT; height++)
   {
     for (int width = 0; width < PHOTO_WIDTH; width++)
     {
       for (int bytenumber = 0; bytenumber < PHOTO_BYTES_PER_PIXEL; bytenumber++)
       {
         // Pulse the read clock RCLK to bring in new byte of data.
         // 7ns for RCLK High Pulse Width and Low Pulse Width .007 micro secs
         PulsePin(RCLK, 1); 
             
         // Convert Pin values to byte values for pins 0-7 of incoming pixel byte
         PinVal7 = ConvertPinValueToByteValue(digitalRead(DO7), 7);
         PinVal6 = ConvertPinValueToByteValue(digitalRead(DO6), 6);
         PinVal5 = ConvertPinValueToByteValue(digitalRead(DO5), 5);
         PinVal4 = ConvertPinValueToByteValue(digitalRead(DO4), 4);
         PinVal3 = ConvertPinValueToByteValue(digitalRead(DO3), 3);
         PinVal2 = ConvertPinValueToByteValue(digitalRead(DO2), 2);
         PinVal1 = ConvertPinValueToByteValue(digitalRead(DO1), 1);
         PinVal0 = ConvertPinValueToByteValue(digitalRead(DO0), 0);
     
         // Combine individual data from each pin into composite data in the form of a single byte
         PixelData = PinVal7 | PinVal6 | PinVal5 | PinVal4 | PinVal3 | PinVal2 | PinVal1 | PinVal0;
        
         /////////////////////////////  SD Card ////////////////////////////////
         ByteCounter = ByteCounter + ImageOutputFile.write(PixelData);       
         ///////////////////////////////////////////////////////////////////////
       }
     }
   }
   
   // Close SD Card File
   ImageOutputFile.close();
   
   Serial.print(F("Total Bytes Saved to SDCard = "));
   Serial.println(ByteCounter);
   
   // Write Photo's Info File to SDCard.
   Serial.println(F("Writing Photo's Info file (.txt file) to SD Card ..."));
   CreatePhotoInfoFile();
}

void TakePhoto()
{
  // Take Photo using the ov7670 camera and transmit the image to the Android controller via 
  // Bluetooth
  unsigned long StartTime   = 0;
  unsigned long EndTime     = 0;
  unsigned long ElapsedTime = 0;
  
  StartTime = millis();
  
  CaptureOV7670Frame(); 
  ReadTransmitCapturedFrame();
 
  EndTime = millis(); 
  ElapsedTime = (EndTime - StartTime)/1000; // Convert to seconds
  
  Serial.print(F("Elapsed Time for Taking and Sending Photo(secs) = "));
  Serial.println(ElapsedTime);
}

void PulseLowEnabledPin(int PinNumber, int DurationMicroSecs)
{
  // For Low Enabled Pins , 0 = on and 1 = off
  digitalWrite(PinNumber, LOW);            // Sets the pin on
  delayMicroseconds(DurationMicroSecs);    // Pauses for DurationMicroSecs microseconds      
  
  digitalWrite(PinNumber, HIGH);            // Sets the pin off
  delayMicroseconds(DurationMicroSecs);     // Pauses for DurationMicroSecs microseconds  
}

void PulsePin(int PinNumber, int DurationMicroSecs)
{
  digitalWrite(PinNumber, HIGH);           // Sets the pin on
  delayMicroseconds(DurationMicroSecs);    // Pauses for DurationMicroSecs microseconds      
  
  digitalWrite(PinNumber, LOW);            // Sets the pin off
  delayMicroseconds(DurationMicroSecs);    // Pauses for DurationMicroSecs microseconds  
}

String ParseI2CResult(int result)
{
  String sresult = "";
  switch(result)
  {
    case 0:
     sresult = "I2C Operation OK ...";
    break;
    
    case  I2C_ERROR_WRITING_START_ADDRESS:
     sresult = "I2C_ERROR_WRITING_START_ADDRESS";
    break;
    
    case I2C_ERROR_WRITING_DATA:
     sresult = "I2C_ERROR_WRITING_DATA";
    break;
      
    case DATA_TOO_LONG:
     sresult = "DATA_TOO_LONG";
    break;   
    
    case NACK_ON_TRANSMIT_OF_ADDRESS:
     sresult = "NACK_ON_TRANSMIT_OF_ADDRESS";
    break;
    
    case NACK_ON_TRANSMIT_OF_DATA:
     sresult = "NACK_ON_TRANSMIT_OF_DATA";
    break;
    
    case OTHER_ERROR:
     sresult = "OTHER_ERROR";
    break;
       
    default:
     sresult = "I2C ERROR TYPE NOT FOUND...";
    break;
  }
 
  return sresult;
}


// Parameters:
//   start : Start address, use a define for the register
//   pData : A pointer to the data to write.
//   size  : The number of bytes to write.
//
int OV7670Write(int start, const byte *pData, int size)
{
  int n, error;

  Wire.beginTransmission(OV7670_I2C_ADDRESS);
  n = Wire.write(start);        // write the start address
  if (n != 1)
  {
    return (I2C_ERROR_WRITING_START_ADDRESS);
  }
  
  n = Wire.write(pData, size);  // write data bytes
  if (n != size)
  {
    return (I2C_ERROR_WRITING_DATA);
  }
  
  error = Wire.endTransmission(true); // release the I2C-bus
  if (error != 0)
  {
    return (error);
  }
  
  return 0;         // return : no error
}


//
// A function to write a single register
//
int OV7670WriteReg(int reg, byte data)
{
  int error;

  error = OV7670Write(reg, &data, 1);

  return (error);
}

//
// This is a common function to read multiple bytes 
// from an I2C device.
//
// It uses the boolean parameter for Wire.endTransMission()
// to be able to hold or release the I2C-bus. 
// This is implemented in Arduino 1.0.1.
//
int OV7670Read(int start, byte *buffer, int size)
{
  int i, n, error;

  Wire.beginTransmission(OV7670_I2C_ADDRESS);
  n = Wire.write(start);
  if (n != 1)
  {
    return (I2C_READ_START_ADDRESS_ERROR);
  }
  
  n = Wire.endTransmission(false);    // hold the I2C-bus
  if (n != 0)
  {
    return (n);
  }
  
  // Third parameter is true: relase I2C-bus after data is read.
  Wire.requestFrom(OV7670_I2C_ADDRESS, size, true);
  i = 0;
  while(Wire.available() && i<size)
  {
    buffer[i++] = Wire.read();
  }
  if ( i != size)
  {
    return (I2C_READ_DATA_SIZE_MISMATCH_ERROR);
  }
  
  return (0);  // return no error
}

//
// A function to read a single register
//
int OV7670ReadReg(int reg, byte *data)
{
  int error;

  error = OV7670Read(reg, data, 1);

  return (error);
}

void WriteFileTest(String Filename)
{
   File TempFile;
  
   TempFile = SD.open(Filename.c_str(), FILE_WRITE);
   if (TempFile) 
   {
      Serial.print(F("Writing to testfile ..."));
      TempFile.print(F("TEST CAMERA SDCARD HOOKUP At Time... "));
      TempFile.print(millis()/1000);
      TempFile.println(F(" Seconds"));
      
      TempFile.print(F("Photo Info Filename: "));
      TempFile.println(CreatePhotoInfoFilename());
      TempFile.print(F("Photo Info:"));
      TempFile.println(CreatePhotoInfo());
    
      // close the file:
      TempFile.close();
      Serial.println(F("Writing File Done..."));
   } 
   else 
   {
      // if the file didn't open, print an error:
      Serial.print(F("Error opening "));
      Serial.println(Filename);
   }
}

void ReadPrintFile(String Filename)
{
  File TempFile;
  
  // Reads in file and prints it to screen via Serial
  TempFile = SD.open(Filename.c_str());
  if (TempFile) 
  {
    Serial.print(Filename);
    Serial.println(":");
    
    // read from the file until there's nothing else in it:
    while (TempFile.available()) 
    {
        Serial.write(TempFile.read());
    }
    // close the file:
    TempFile.close();
  } 
  else 
  {
    // Error opening file
    Serial.print("Error opening ");
    Serial.println(Filename);
  }
  
}



void setup()
{
     // Initialize Serial
     Serial.begin(9600); 
     Serial.println(F("Arduino SERIAL_MONITOR_CONTROLLED CAMERA ... Using ov7670 Camera"));
     Serial.println();
     
    
     // Setup the OV7670 Camera for use in taking still photos
     Wire.begin();
     Serial.println(F("----------------------------- Camera Registers ----------------------------"));
     ResetCameraRegisters();
     ReadRegisters();
     Serial.println(F("---------------------------------------------------------------------------"));
     SetupCamera();    
     Serial.println(F("FINISHED INITIALIZING CAMERA ..."));
     Serial.println();
     Serial.println();
    
                              
    // Initialize SD Card
    Serial.print(F("\nInitializing SD card..."));
    pinMode(HardwareSSPin, OUTPUT);     // change this to 53 on a mega

    if (!SD.begin(chipSelect))
    {
      Serial.println(F("Initialization failed ... /nThings to check:"));
      Serial.println(F("- Is a card is inserted?"));
      Serial.println(F("- Is your wiring correct?"));
      Serial.println(F("- Did you change the chipSelect pin to match your shield or module?"));
      return;
    } else {
       Serial.println(F("Wiring is correct and a card is present ..."));
    }

   Serial.println();
   Serial.println();
   Serial.println();
   Serial.println(F("============================================================================="));
   Serial.println();
   Serial.println(F("Omnivision ov7670 FIFO Camera Image Capture Software Version 1.0"));
   Serial.println(F("Copyright 2015 by Robert Chin.  All Rights Reserved."));
   Serial.println();
   Serial.println(F("This code to accompany the book entitled: "));
   Serial.println(F("Beginning Arduino ov7670 Camera Development"));
   Serial.println();
   Serial.println(F("Please refer to the book for detailed explainations"));
   Serial.println(F("of how to use this code, how to use the Omnivision "));
   Serial.println(F("ov7670 camera as well as how to set up the camera with"));
   Serial.println(F("the Arduino and an SD Card."));
   Serial.println();
   Serial.println(F("============================================================================="));
   Serial.println(F("Type h or help for main help menu ..."));
   Serial.println();
   

}


void ExecuteCommand(String Command)
{
  // Set up Camera for VGA, QVGA, or QQVGA Modes
  if (Command == "VGA")
  {
     Serial.println(F("Taking a VGA Photo...")); 
     if (Resolution != VGA)
     {
       // If current resolution is not QQVGA then set camera for QQVGA
       ResetCameraRegisters();
       
       Resolution = VGA;
       SetupOV7670ForVGARawRGB();
       Serial.println(F("----------------------------- Camera Registers ----------------------------"));
       ReadRegisters();
       Serial.println(F("---------------------------------------------------------------------------"));
     }
  }
  else
  if (Command == "VGAP")
  {
     Serial.println(F("Taking a VGAP Photo...")); 
     if (Resolution != VGAP)
     {
       // If current resolution is not VGAP then set camera for VGAP
       ResetCameraRegisters();
       
       Resolution = VGAP;
       SetupOV7670ForVGAProcessedBayerRGB();
       Serial.println(F("----------------------------- Camera Registers ----------------------------"));
       ReadRegisters();
       Serial.println(F("---------------------------------------------------------------------------"));
     }
  }
  else if (Command == "QVGA")
  {
     Serial.println(F("Taking a QVGA Photo...")); 
     if (Resolution != QVGA)
     {
       // If current resolution is not QQVGA then set camera for QQVGA
       Resolution = QVGA;
       SetupOV7670ForQVGAYUV();
       Serial.println(F("----------------------------- Camera Registers ----------------------------"));
       ReadRegisters();
       Serial.println(F("---------------------------------------------------------------------------"));
     }
  }
  else if (Command == "QQVGA")
  {
     Serial.println(F("Taking a QQVGA Photo...")); 
     if (Resolution != QQVGA)
     {
       // If current resolution is not QQVGA then set camera for QQVGA
       Resolution = QQVGA;
       SetupOV7670ForQQVGAYUV();
       Serial.println(F("----------------------------- Camera Registers ----------------------------"));
       ReadRegisters();
       Serial.println(F("---------------------------------------------------------------------------"));
     }
  }
  else
  {
     Serial.print(F("The command ")); 
     Serial.print(Command);
     Serial.println(F(" is not recognized ..."));
  }
  
  OV7670WriteReg(0x15, 0x02);
  // Delay for registers to settle
  delay(100);
  
  // Take Photo 
  TakePhoto();
}


boolean ProcessRawCommandElement(String Element)
{
  boolean result = false;
  
  Element.toLowerCase();
  
  if ((Element == "vga") || 
      (Element == "vgap") ||
      (Element == "qvga")||
      (Element == "qqvga"))
  {
    Element.toUpperCase(); 
    Command = Element;
    result = true;
  }
  else
  if (Element == "thirtyfps")
  { 
    FPSParam = "ThirtyFPS";
    result = true;
  }
  else
  if (Element == "nightmode")
  { 
    FPSParam = "NightMode";
    result = true;
  } 
  else
  if (Element == "sawb")
  { 
    AWBParam = "SAWB";
    result = true;
  }
  else
  if (Element == "aawb")
  { 
    AWBParam = "AAWB";
    result = true;
  }
  else
  if (Element == "aveaec")
  { 
    AECParam = "AveAEC";
    result = true;
  }
  else
  if (Element == "histaec")
  { 
    AECParam = "HistAEC";
    result = true;
  }
  else
  if (Element == "yuvmatrixon")
  { 
    YUVMatrixParam = "YUVMatrixOn";
    result = true;
  }
  else
  if (Element == "yuvmatrixoff")
  { 
    YUVMatrixParam = "YUVMatrixOff";
    result = true;
  }
  else
  if (Element == "denoiseyes")
  { 
    DenoiseParam = "DenoiseYes";
    result = true;
  }
  else
  if (Element == "denoiseno")
  { 
    DenoiseParam = "DenoiseNo";
    result = true;
  }
  else
  if (Element == "edgeyes")
  { 
    EdgeParam = "EdgeYes";
    result = true;
  }
  else
  if (Element == "edgeno")
  { 
    EdgeParam = "EdgeNo";
    result = true;
  }
  else
  if (Element == "ablcon")
  { 
    ABLCParam = "AblcON";
    result = true;
  }
  else
  if (Element == "ablcoff")
  { 
    ABLCParam = "AblcOFF";
    result = true;
  }
 
  return result;
}

void ParseRawCommand(String RawCommandLine)
{
   String Entries[10];
   boolean success = false;
     
   // Parse into command and parameters
   int NumberElements = ParseCommand(RawCommandLine.c_str(), ' ', Entries);
   
   for (int i = 0 ; i < NumberElements; i++)
   {
     boolean success = ProcessRawCommandElement(Entries[i]);
     if (!success)
     {
       Serial.print(F("Invalid Command or Parameter: "));
       Serial.println(Entries[i]);
     }
     else
     {
       Serial.print(F("Command or parameter "));
       Serial.print(Entries[i]);
       Serial.println(F(" sucessfully set ..."));
     }
   }
   
   
   // Assume parameter change since user is setting parameters on command line manually
   // Tells the camera to re-initialize and set up camera according to new parameters
   Resolution = None; // Reset and reload registers
   ResetCameraRegisters();
   
}


void DisplayHelpCommandsParams()
{
   Serial.println(F("....... Help Menu Camera Commands/Params .........."));
 
   Serial.println(F("Resolution Change Commands: VGA,VGAP,QVGA,QQVGA"));
   Serial.println(F("FPS Parameters: ThirtyFPS, NightMode"));
   Serial.println(F("AWB Parameters: SAWB, AAWB"));
   Serial.println(F("AEC Parameters: AveAEC, HistAEC"));	
   Serial.println(F("YUV Matrix Parameters: YUVMatrixOn, YUVMatrixOff"));
   Serial.println(F("Denoise Parameters: DenoiseYes, DenoiseNo"));
   Serial.println(F("Edge Enchancement: EdgeYes, EdgeNo"));
   Serial.println(F("Android Storage Parameters: StoreYes, StoreNo"));
   Serial.println(F("Automatic Black Level Calibration: AblcON, AblcOFF"));
   Serial.println();
   Serial.println();  
}

void  DisplayHelpMenu()
{
    Serial.println(F("................. Help Menu .................."));
    Serial.println(F("d - Display Current Camera Command"));
    Serial.println(F("t - Take Photograph using current Command and Parameters"));
    Serial.println(F("testread - Tests reading files from the SDCard by reading and printig the contents of test.txt"));
    Serial.println(F("testwrite - Tests writing files to SDCard"));
    
    //Serial.println(F("sdinfo - Display SD Card information including files present"));
    Serial.println(F("help camera - Displays Camera's Commands and Parameters"));
    Serial.println();
    Serial.println();
    
}

void DisplayCurrentCommand()
{
     // Print out Command and Parameters
     Serial.println(F("Current Command:"));
     Serial.print(F("Command: "));
     Serial.println(Command);
     
     Serial.print(F("FPSParam: "));
     Serial.println(FPSParam);
     
     Serial.print(F("AWBParam: "));
     Serial.println(AWBParam);
     
     Serial.print(F("AECParam: "));
     Serial.println(AECParam);   
     
     Serial.print(F("YUVMatrixParam: "));
     Serial.println(YUVMatrixParam);   
     
     Serial.print(F("DenoiseParam: "));
     Serial.println(DenoiseParam);   
    
     Serial.print(F("EdgeParam: "));
     Serial.println(EdgeParam);  
    
     Serial.print(F("ABLCParam: "));
     Serial.println(ABLCParam);  
    
     Serial.println();
}

void CheckRemoveFile(String Filename)
{
   // Check if file already exists and remove it if it does.
   char tempchar[50];
   strcpy(tempchar, Filename.c_str());
   
   if (SD.exists(tempchar))
   {
     Serial.print(F("Filename: "));
     Serial.print(tempchar);
     Serial.println(F(" Already Exists. Removing It..."));
     SD.remove(tempchar);
   }
   
   // If file still exists then new image file cannot be saved to SD Card. 
   if (SD.exists(tempchar))
   {
     Serial.println(F("Error.. Image output file cannot be created..."));
     return;
   }
}


void loop()
{   
     // Wait for Command
     Serial.println(F("Ready to Accept new Command => "));
     while (1)
     {
       if (Serial.available() > 0) 
       {
         int NumberCharsRead = Serial.readBytesUntil('\n', IncomingByte, BUFFERLENGTH);  
         for (int i = 0; i < NumberCharsRead; i++)
         {
           RawCommandLine += IncomingByte[i];
         } 
         break;
       }
     }  
        
     // Print out the command from Android
     Serial.print(F("Raw Command from Serial Monitor: "));
     Serial.println(RawCommandLine);
     
     if ((RawCommandLine == "h")||
         (RawCommandLine == "help"))
     {
       DisplayHelpMenu();
     }
     else
     if (RawCommandLine == "help camera")
     {       
       DisplayHelpCommandsParams();
     }
     else
     if (RawCommandLine == "d")
     {
        DisplayCurrentCommand();       
     }
     else
     if (RawCommandLine == "t")
     {
         // Take Photo
         Serial.println(F("\nGoing to take photo with current command:")); 
         DisplayCurrentCommand(); 
         
         // Take Photo
         ExecuteCommand(Command);
         Serial.println(F("Photo Taken and Saved to Arduino SD CARD ..."));
         
         String Testfile = CreatePhotoFilename();
         Serial.print(F("Image Output Filename :"));
         Serial.println(Testfile);
         PhotoTakenCount++;
         
     }
     else
     if (RawCommandLine == "testread")
     {
       ReadPrintFile("TEST.TXT");
     }
     else
     if (RawCommandLine == "testwrite")
     {
       CheckRemoveFile("TEST.TXT");
       WriteFileTest("TEST.TXT");
     }
     else
     {
        Serial.println(F("Changing command or parameters according to your input:"));
         // Parse Command Line and Set Command Line Elements
         // Parse Raw Command into Command and Parameters
         ParseRawCommand(RawCommandLine);
         
         // Display new changed camera command with parameters
         DisplayCurrentCommand();
     }
    
     // Reset Command Line
     RawCommandLine = "";
     
     Serial.println();
     Serial.println();
}


int ParseCommand(const char* commandline, char splitcharacter, String* Result)
{ 
  int ResultIndex = 0;
  int length = strlen(commandline);
  String temp = "";
  
  for (int i = 0; i < length ; i++)
  {
   char tempchar = commandline[i];
   if (tempchar == splitcharacter)
   {
       Result[ResultIndex] += temp;
       ResultIndex++;
       temp = "";
   }
   else
   {
     temp += tempchar;
   } 
  }
  
  // Put in end part of string
  Result[ResultIndex] = temp;
  
  return (ResultIndex + 1);
}

