#ifndef _MEMORYSAVER_
#define _MEMORYSAVER_

//There are two steps you need to modify in this file before normal compilation
//Only ArduCAM Shield series platform need to select camera module, ArduCAM-Mini series platform doesn't

//Step 1: select the hardware platform, only one at a time
//#define OV2640_MINI_2MP
//#define OV5642_MINI_5MP
//#define OV5642_MINI_5MP_BIT_ROTATION_FIXED
//#define OV5642_MINI_5MP_PLUS
//#define OV5640_MINI_5MP_PLUS

//#define ARDUCAM_SHIELD_REVC	
#define ARDUCAM_SHIELD_V2


//Step 2: Select one of the camera module, only one at a time
#if (defined(ARDUCAM_SHIELD_REVC) || defined(ARDUCAM_SHIELD_V2))
	//#define OV7660_CAM
	//#define OV7725_CAM
	//#define OV7670_CAM
	//#define OV7675_CAM
    //#define OV2640_CAM
	//#define OV3640_CAM
	 #define OV5642_CAM
	//#define OV5640_CAM
	
	//#define MT9D111A_CAM
	//#define MT9D111B_CAM
	//#define MT9M112_CAM
	//#define MT9V111_CAM	
	//#define MT9M001_CAM	
	//#define MT9T112_CAM
	//#define MT9D112_CAM
#endif 





#endif	//_MEMORYSAVER_
