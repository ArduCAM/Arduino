#ifndef _MEMORYSAVER_
#define _MEMORYSAVER_

// This file is used to turn certain drivers on or off in order to save memory.
//
// To select a particular driver, uncomment the line by removing the "//" at the
// start of the line. To turn off a particular driver, add the "//" back to the
// front of the line.
//
// Generally you will need only one or two lines in this file to be enabled.
//
// If you are using the ArduCAM shield series of products, you will need to enable
// one of the two shield versions (V2 or REVC) depending on which board you have,
// and then also enable the proper camera driver.
//
// If you are using the ArduCAM mini series, you only need to enable one line,
// the one for the particular mini you are using.

//Enable this line when using the Raspberry Pi platform
//#define RASPBERRY_PI

// Step 1: enable the correct ArduCAM hardware platform
//
// OV2640_MINI_2MP_PLUS is enabled by default, if you are using a different platform,
// disable that line and enable one of the others. The mini products are at the top
// of the list, and the shields at the bottom.

//#define OV2640_MINI_2MP
//#define OV3640_MINI_3MP
//#define OV5642_MINI_5MP
//#define OV5642_MINI_5MP_BIT_ROTATION_FIXED
#define OV2640_MINI_2MP_PLUS
//#define OV5642_MINI_5MP_PLUS
//#define OV5640_MINI_5MP_PLUS

//#define ARDUCAM_SHIELD_REVC	
//#define ARDUCAM_SHIELD_V2


// Step 2: enable the correct camera module
//
// All of these drivers are disabled by default, and are also
// disabled if you do not enable the shield drivers above.

#if (defined(ARDUCAM_SHIELD_REVC) || defined(ARDUCAM_SHIELD_V2))
	//#define OV7660_CAM
	//#define OV7725_CAM
	//#define OV7670_CAM
	//#define OV7675_CAM
  	//#define OV2640_CAM
	//#define OV3640_CAM
	//#define OV5642_CAM
	//#define OV5640_CAM 
	
	//#define MT9D111A_CAM
	//#define MT9D111B_CAM
	//#define MT9M112_CAM
	//#define MT9V111_CAM	
	//#define MT9M001_CAM	
	//#define MT9V034_CAM
	//#define MT9M034_CAM
	//#define MT9T112_CAM
	//#define MT9D112_CAM
#endif 

#endif	//_MEMORYSAVER_