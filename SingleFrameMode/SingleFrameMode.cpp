
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fitsio.h>
#include "qhyccd.h"
#include <ctime>
#include <cmath>
#include <iostream>

using namespace std;

//=============================================
//=============================================
//=================|---------|=================
//=================|FUNCTIONS|=================
//=================|---------|=================
//=============================================
//=============================================

// // // // // INITIALIZE EVERYTHING // // // // //
qhyccd_handle *QuickInitialize(unsigned int retVal, int USB_TRAFFIC, unsigned int roiStartX, unsigned int roiStartY, unsigned int roiSizeX, unsigned int roiSizeY, int camBinX, int camBinY, int readMode)
{
  // Get Camera ID
  char camId[32];
  retVal = GetQHYCCDId(0, camId);

  // Open Camera
  qhyccd_handle *pCamHandle = OpenQHYCCD(camId);

  // Set Single Frame Mode (mode = 0)
  retVal = SetQHYCCDStreamMode(pCamHandle, 0);

  // Initialize Camera
  retVal = InitQHYCCD(pCamHandle);

    // Set USB Traffic Setting
  retVal = SetQHYCCDParam(pCamHandle, CONTROL_USBTRAFFIC, USB_TRAFFIC);

  // Set Image Resolution
  retVal = SetQHYCCDResolution(pCamHandle, roiStartX, roiStartY, roiSizeX, roiSizeY);

  // Set Binning mode
  retVal = SetQHYCCDBinMode(pCamHandle, camBinX, camBinY);

  // Set Bit Resolution
  retVal = SetQHYCCDBitsMode(pCamHandle, 16);

  // Set ReadMode
  retVal = SetQHYCCDReadMode(pCamHandle, readMode);

  return pCamHandle;
}

// // // // // CAMERA SETTINGS // // // // //
void QuickCamSettings(unsigned int retVal, qhyccd_handle *pCamHandle, int CHIP_GAIN, int CHIP_OFFSET, double tempSetting, double EXPOSURE_TIME)
{
  // Set Gain Setting
  retVal = SetQHYCCDParam(pCamHandle, CONTROL_GAIN, CHIP_GAIN);

  // Set Offset
  retVal = SetQHYCCDParam(pCamHandle, CONTROL_OFFSET, CHIP_OFFSET);

  // // // // // The Temperature Control System // // // // //

  //Get Current Temperature
  double currentTemp = GetQHYCCDParam(pCamHandle, CONTROL_CURTEMP);

  // Set Temperature to the temperature setting we want
  retVal = SetQHYCCDParam(pCamHandle, CONTROL_COOLER, tempSetting);

  // While the temperature difference is greater than 0 degrees
  
  for (int tempLoop = 0; tempLoop < 3; tempLoop++)
  {
    // Sleep for 5 seconds to get new temperature
    sleep(2);

    // Get current temperature again to loop
    currentTemp = GetQHYCCDParam(pCamHandle, CONTROL_CURTEMP);

    double tempPlay = 0.1;
    while (abs(currentTemp - tempSetting) > tempPlay)
    {
      // Report temperature progress to screen
      if ((currentTemp - tempSetting) > tempPlay)
      {
        printf("Current Temperature: %.2f || You Want: %.2f . Camera is cooling down. \n", currentTemp, tempSetting);
      }

      else
      {
        printf("Current Temperature: %.2f || You Want: %.2f . Camera is heating up. \n", currentTemp, tempSetting);
      }

      //Try again in 5 seconds
      sleep(2);

      // Get current temperature again to loop again
      currentTemp = GetQHYCCDParam(pCamHandle, CONTROL_CURTEMP);
    }

  }

  // Sleep for 5 seconds before running
  sleep(2);

  // // // // // End Temperature Control System // // // // //

  // Set Exposure Time
  retVal = SetQHYCCDParam(pCamHandle, CONTROL_EXPOSURE, EXPOSURE_TIME);
}

// // // // // TAKE PICTURE // // // // //
void QuickCapture(unsigned int retVal, 
                  qhyccd_handle *pCamHandle, int runTimes, 
                  int runner, 
                  unsigned int roiStartX, 
                  unsigned int roiStartY, 
                  unsigned int roiSizeX, 
                  unsigned int roiSizeY, 
                  unsigned int bpp, 
                  int CHIP_GAIN, 
                  int CHIP_OFFSET, 
                  double EXPOSURE_TIME, 
                  double tempSetting,
                  int readMode, 
                  string savePath)
{

  // // // // // Take 2 Biases Before Taking Actual Image// // // // //

  // // Start Exposing
  retVal = ExpQHYCCDSingleFrame(pCamHandle);

  sleep(1);

  // Loop over however many images we want to take before taking the actual image
  for (int biasRun = 0; biasRun < 2; biasRun++)
  {
      // Set Exposure Time
     //retVal = SetQHYCCDParam(pCamHandle, CONTROL_EXPOSURE, 1100000); // 1.1 second
     retVal = SetQHYCCDParam(pCamHandle, CONTROL_EXPOSURE, 100); // 100 microseconds

     // Channel of Image
     unsigned int channels;

     // Image Data Variable
     unsigned char *pImgData = 0;

     // Get Requested Memory Length
     uint32_t length = GetQHYCCDMemLength(pCamHandle);
     pImgData = new unsigned char[length];
     memset(pImgData, 0, length);

     //Take Single Frame
     retVal = GetQHYCCDSingleFrame(pCamHandle, &roiSizeX, &roiSizeY, &bpp, &channels, pImgData);

     // Delete image data
     delete[] pImgData;
   }

   // Cancel Exposing and Readout
   retVal = CancelQHYCCDExposingAndReadout(pCamHandle);

   // Sleep for 3 seconds before taking the actual image
   sleep(3);

   // // // // // End Taking 2 Biases Before Taking Actual Image// // // // //

  // // // // // Take Actual Image // // // // //

  // Set Exposure Time
  retVal = SetQHYCCDParam(pCamHandle, CONTROL_EXPOSURE, EXPOSURE_TIME);

  // Channel of Image
  unsigned int channels;

  // Single Frame
  retVal = ExpQHYCCDSingleFrame(pCamHandle);

  sleep(1);

  // Image Data Variable
  unsigned char *pImgData = 0;

  // Get Requested Memory Length
  uint32_t length = GetQHYCCDMemLength(pCamHandle);
  pImgData = new unsigned char[length];
  memset(pImgData, 0, length);

  // Take Single Frame
  retVal = GetQHYCCDSingleFrame(pCamHandle, &roiSizeX, &roiSizeY, &bpp, &channels, pImgData);
  printf("GetQHYCCDSingleFrame: %d x %d, bpp: %d, channels: %d, success.\n", roiSizeX, roiSizeY, bpp, channels);

  // Image Processing to .fits file

  // Create File
  fitsfile *fptr;
  int status = 0;
  long naxes[2] = {roiSizeX, roiSizeY};
  long curUnixTime = time(0);

  // Naming:
  string fitname = savePath + "_" + to_string(curUnixTime) + "_exp_" + to_string((int)EXPOSURE_TIME) + "us_gain_" + to_string(CHIP_GAIN) + "_offset_" + to_string(CHIP_OFFSET) + "_temp_" + to_string((int)tempSetting) + "_" + to_string(runner) +  + ".fits";
  const char *fitsfilename = fitname.c_str();

  // Remove if exists already
  remove(fitsfilename);

  // Create File
  fits_create_file(&fptr, fitsfilename, &status);
  fits_create_img(fptr, USHORT_IMG, 2, naxes, &status);

  // Headers Information
  fits_update_key(fptr, TDOUBLE, "INTTEMP", &tempSetting, "Camera Temperature", &status);
  fits_update_key(fptr, TINT, "EXPTIME", &EXPOSURE_TIME, "Exposure time in microseconds", &status);
  fits_update_key(fptr, TINT, "OFFSET", &CHIP_OFFSET, "Offset Setting", &status);
  fits_update_key(fptr, TINT, "GAIN", &CHIP_GAIN, "Gain Setting", &status);
  fits_update_key(fptr, TINT, "QHREADMOE", &readMode, "ReadMode Setting", &status);
  fits_update_key(fptr, TLONG, "TIME", &curUnixTime, "UNIX Time", &status);

  // Write to File
  fits_write_img(fptr, TUSHORT, 1, roiSizeX * roiSizeY, pImgData, &status);

  // Close File
  fits_close_file(fptr, &status);

  // Delete Image Data
  delete[] pImgData;

  // Cancel Exposing and Readout
  retVal = CancelQHYCCDExposingAndReadout(pCamHandle);
}

// // // // // THE END // // // // //
void QuickExit(unsigned int retVal, qhyccd_handle *pCamHandle)
{
  // Close Camera Handle
  retVal = CloseQHYCCD(pCamHandle);

  // Release SDK Resources
  retVal = ReleaseQHYCCDResource();
}

//===============================================
//===============================================
//=================|-----------|=================
//=================|THE PROGRAM|=================
//=================|-----------|=================
//===============================================
//===============================================

int main(int argc, char *argv[])
{

  //Variables Preset
  unsigned int roiStartX = 0;   // ROI Start x
  unsigned int roiStartY = 0;   // ROI Start y
  unsigned int roiSizeX = 9600; // Max x
  unsigned int roiSizeY = 6422; // Max y
  int camBinX = 1;              // Binning
  int camBinY = 1;              // Binning
  int USB_TRAFFIC = 10;         // USB Traffic
  unsigned int bpp = 16;        // Bit Depth of Image
  int readMode = 1;             // ReadMode

  // // // // // INITIALIZE SDK // // // // //
  unsigned int retVal = InitQHYCCDResource();

  // // // // // INITIALIZE EVERYTHING ELSE // // // // //
  qhyccd_handle *pCamHandle = QuickInitialize(retVal, USB_TRAFFIC, roiStartX, roiStartY, roiSizeX, roiSizeY, camBinX, camBinY, readMode);

  // The List of All Variables -- SET THESE TO TAKE IMAGES

  //int sampleGains[1] = {56};            // List of gain settings to loop over
  //int sampleOffsets[3] = {10, 20, 40};          // List of offset setings to loop over
  //double sampleTemps[17] = {-8.00, -7.00, -6.00, -5.00, -4.00, -3.00, -2.00, -1.00, 0.00, 1.00, 2.00, 3.00, 4.00, 5.00, 6.00, 7.00, 8.00};    // List of temperatures to loop over (in Celsius)
  //int sampleExps[5] = {1000000, 10000000, 30000000, 100000000, 300000000}; // List of exposure times to loop over (in us)
  //int howManyTimesToRun = 4;               // How many times to take pictures at each unique setting

  const int MICROSECOND = 1000000;

  int sampleGains[1] = {56};            // List of gain settings to loop over
  int sampleOffsets[1] = {10};          // List of offset setings to loop over
  double sampleTemps[1] = {15};    // List of temperatures to loop over (in Celsius)
  double sampleExps[3] = {123, 123, 123}; // List of exposure times to loop over (in us)
  int howManyTimesToRun = 1;               // How many times to take pictures at each unique setting

  // LOOP THE LOOPS (MAKE SURE to change the values for t, o, g, and e to correspond to the arrays above)
  for (int t = 0; t < 1; t++)
  {
    for (int o = 0; o < 1; o++)
    {
      for (int g = 0; g < 1; g++)
      {
        for (unsigned int e = 0; e < sizeof(sampleExps)/sizeof(sampleExps[0]); e++)
        {
          double EXPOSURE_TIME = sampleExps[e] * MICROSECOND;   // Exposure time (in us)
          int CHIP_GAIN = sampleGains[g];      // Gain Setting
          int CHIP_OFFSET = sampleOffsets[o];  // Offset Setting
          double tempSetting = sampleTemps[t]; // Temperature of Camera
          int runTimes = howManyTimesToRun;    // How Many Pictures To Get
          string savePath = "/home/luvs/gill/qhy600m/july16/dark/dark";

          // // // // // CAMERA SETTINGS // // // // //
          QuickCamSettings(retVal, pCamHandle, CHIP_GAIN, CHIP_OFFSET, tempSetting, EXPOSURE_TIME);

          for (int runner = 0; runner < runTimes; runner++)
          {
            // // // // // TAKE PICTURE // // // // //
            QuickCapture(retVal, pCamHandle, runTimes, runner, roiStartX, roiStartY, roiSizeX, roiSizeY, bpp, CHIP_GAIN, CHIP_OFFSET, EXPOSURE_TIME, tempSetting, readMode, savePath);
          }
        }
      }
    }
  }

  // // // // // THE END // // // // //
  QuickExit(retVal, pCamHandle);

  // Exit
  return 0;

  ////////////////////////////////////////////////////////////////////////////////////
  // // // // // // //  // // USEFUL BUT UNNEEDED CODE // // // // // // // // // //

  // // // Code to set variables through arguments -- commented out for now

  // int EXPOSURE_TIME = atoi(argv[1]); // Exposure time (in us)
  // int CHIP_GAIN = atoi(argv[2]); // Gain Setting
  // int CHIP_OFFSET = atoi(argv[3]); // Offset Setting
  // double tempSetting = atof(argv[4]); // Temperature of Camera
  // int readMode = atoi(argv[5]); // ReadMode
  // int runTimes = atoi(argv[6]); // How Many Pictures To Get

  // // // Code without looping over variables -- commented out for now

  // int EXPOSURE_TIME = 1; // Exposure time (in us)
  // int CHIP_GAIN = 10; // Gain Setting
  // int CHIP_OFFSET = 0; // Offset Setting
  // double tempSetting = 0.00; // Temperature of Camera
  // int readMode = 1; // ReadMode
  // int runTimes = 10; // How Many Pictures To Get

  // //Variables Preset
  // unsigned int roiStartX = 0; // ROI Start x
  // unsigned int roiStartY = 0; // ROI Start y
  // unsigned int roiSizeX = 9600; // Max x
  // unsigned int roiSizeY = 6422; // Max y
  // int camBinX = 1; // Binning
  // int camBinY = 1; // Binning
  // int USB_TRAFFIC = 10; // USB Traffic
  // unsigned int bpp = 16; // Bit Depth of Image

  // // // // // // INITIALIZE SDK // // // // //
  // unsigned int retVal = InitQHYCCDResource();

  // // // // // // INITIALIZE EVERYTHING ELSE // // // // //
  // qhyccd_handle *pCamHandle = QuickInitialize(retVal);

  // // // // // // CAMERA SETTINGS // // // // //
  // QuickCamSettings(retVal, pCamHandle, USB_TRAFFIC, CHIP_GAIN, CHIP_OFFSET, tempSetting, EXPOSURE_TIME, roiStartX, roiStartY, roiSizeX, roiSizeY, camBinX, camBinY, readMode);

  // // // // // // TAKE PICTURE // // // // //
  // for (int runner = 0; runner < runTimes; runner++)
  // {
  //   QuickCapture(retVal, pCamHandle, runTimes, runner, roiStartX, roiStartY, roiSizeX, roiSizeY, bpp, CHIP_GAIN, CHIP_OFFSET, EXPOSURE_TIME, tempSetting, readMode);
  // }

  // // // // // // THE END // // // // //
  // QuickExit(retVal, pCamHandle);

  // // Exit
  // return 0;
}
