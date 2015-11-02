
/**
 * Copyright : www.ArduCAM.com @ 2015
 * Author: Lee Jackson
 * Display RAW Image
 * 
 * Copy photos to the same folder of the sketch, 
 * rename the file name in this sketch to your photo file name.
 * Run the sketch, your photo will be displayed.
 * The save() function allows you to save an image from the 
 * display window. In this example, save() is run when a mouse
 * button is pressed. The image "VGAP0.RAW" is saved to the 
 * same folder as the sketch's program file.
 */

//Define the image size
int IMG_HEIGHT = 480;
int IMG_WIDTH = 640;

//Global variables
int[][] pIn_RAW = new int[IMG_HEIGHT][IMG_WIDTH];
tagRGB[][] pOut = new tagRGB[IMG_HEIGHT][IMG_WIDTH];

PImage img;

//RGB Type definition
class tagRGB
{
  int R;  int G;  int B;
  tagRGB()
  {
    R = 0; G = 0; B = 0;
  }
}

//Color exchage constant
final static int Normal_Change = 1;
final static int RG_Change = 2;
final static int RB_Change = 3;
final static int GB_Change = 4;

//signed to unsigned conversion
int toUnsigned(byte b) {
    return (int) (b >= 0 ? b : 256 + b);
}

//Do color exchage
void RgbChangeProc( int i, int j, tagRGB OrgRgb )
{
  switch(RB_Change)
  {
  case RB_Change:
    pOut[i][j].R=OrgRgb.B;
    pOut[i][j].G=OrgRgb.G;
    pOut[i][j].B=OrgRgb.R;
    break;
  case RG_Change:
    pOut[i][j].R=OrgRgb.G;
    pOut[i][j].G=OrgRgb.R;
    pOut[i][j].B=OrgRgb.B;
    break;
  case GB_Change:
    pOut[i][j].R=OrgRgb.R;
    pOut[i][j].G=OrgRgb.B;
    pOut[i][j].B=OrgRgb.G;
    break;
  case Normal_Change:
  default:
    pOut[i][j].R=OrgRgb.R;
    pOut[i][j].G=OrgRgb.G;
    pOut[i][j].B=OrgRgb.B;
    break;
  }
}

//Do RAW to RGB color space conversion
int ByteToRGB( )
{
  tagRGB sTempRgb = new tagRGB();
  for (int i=0;i<IMG_HEIGHT;i+=2)
  {
    for (int j=0;j<IMG_WIDTH;j+=2)
    {
      sTempRgb.R= pIn_RAW[i][j] & 0xFF;
      sTempRgb.G= ((pIn_RAW[i][j+1]>>1 & 0x7F) + (pIn_RAW[i+1][j]>>1 & 0x7F)) & 0xFF;
      sTempRgb.B= pIn_RAW[i+1][j+1] & 0xFF;
      RgbChangeProc(i, j, sTempRgb);
    }
  }
  for (int i=0;i<IMG_HEIGHT;i+=2)
  {
    for (int j=1;j<IMG_WIDTH-1;j+=2)
    {
      sTempRgb.R= pIn_RAW[i][j+1] & 0xFF;
      sTempRgb.G= ((pIn_RAW[i][j]>>1 & 0x7F) + (pIn_RAW[i+1][j+1]>>1 & 0x7F)) & 0xFF;
      sTempRgb.B= pIn_RAW[i+1][j] & 0xFF;
      RgbChangeProc(i, j, sTempRgb);
    }
  }
  for (int i=1;i<IMG_HEIGHT-1;i+=2)
  {
    for (int j=0;j<IMG_WIDTH;j+=2)
    {
      sTempRgb.R= pIn_RAW[i+1][j] & 0xFF;
      sTempRgb.G= ((pIn_RAW[i][j]>>1 & 0x7F) + (pIn_RAW[i+1][j+1]>>1 & 0x7F)) & 0xFF;
      sTempRgb.B= pIn_RAW[i][j+1] & 0xFF;
      RgbChangeProc(i, j, sTempRgb);
    }
  }
  for (int i=1;i<IMG_HEIGHT-1;i+=2)
  {
    for (int j=1;j<IMG_WIDTH-1;j+=2)
    {
      sTempRgb.R= pIn_RAW[i+1][j+1] & 0xFF;
      sTempRgb.G= ((pIn_RAW[i][j+1]>>1 & 0x7F) + (pIn_RAW[i+1][j]>>1 & 0x7F)) & 0xFF;
      sTempRgb.B= pIn_RAW[i][j] & 0xFF;
      RgbChangeProc(i, j, sTempRgb);
    }
  }
  return 0;
}

void setup() 
{
  size(IMG_WIDTH, IMG_HEIGHT);  

  // open a file and read its binary data 
  byte pIn[] = loadBytes("VGAP0.RAW"); 

  //2D Image Array Conversion
  for (int i = 0; i < IMG_HEIGHT ; i++)
    for (int j = 0; j < IMG_WIDTH; j++)
    {
      pIn_RAW[i][j] = toUnsigned(pIn[i*IMG_WIDTH + j]);
      pOut[i][j] = new tagRGB();
    }
    
  //RAW to RGB covnersion
  ByteToRGB();
  img = createImage(IMG_WIDTH, IMG_HEIGHT, RGB);
  img.loadPixels();
  //Display Image
  for (int y=0; y < IMG_HEIGHT; y++) 
  {
    for (int x=0; x < IMG_WIDTH; x++)
      img.pixels[y*IMG_WIDTH + x] = color(pOut[y][x].R, pOut[y][x].G, pOut[y][x].G); 
  }
  img.updatePixels();
}

void draw() 
{
  image(img, 0, 0, IMG_WIDTH, IMG_HEIGHT);
}

//Save image when mouse pressed the dialog
void mousePressed() 
{
  save("VGAP0.jpg");
}






