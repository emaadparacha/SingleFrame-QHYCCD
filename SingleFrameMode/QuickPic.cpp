
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
qhyccd_handle* QuickInitialize(unsigned int retVal)
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

  return pCamHandle;
}

// // // // // CAMERA SETTINGS // // // // //
void QuickCamSettings(unsigned int retVal, qhyccd_handle* pCamHandle, int USB_TRAFFIC, int CHIP_GAIN, int CHIP_OFFSET, double tempSetting, int EXPOSURE_TIME, unsigned int roiStartX, unsigned int roiStartY, unsigned int roiSizeX, unsigned int roiSizeY, int camBinX, int camBinY, int readMode)
{
  // Set USB Traffic Setting
  retVal = SetQHYCCDParam(pCamHandle, CONTROL_USBTRAFFIC, USB_TRAFFIC);

  // Set Gain Setting
  retVal = SetQHYCCDParam(pCamHandle, CONTROL_GAIN, CHIP_GAIN);

  // Set Offset
  retVal = SetQHYCCDParam(pCamHandle, CONTROL_OFFSET, CHIP_OFFSET);

  //Get Current Temperature
  double currentTemp = GetQHYCCDParam(pCamHandle, CONTROL_CURTEMP);

  //Set Temperature
  retVal = ControlQHYCCDTemp(pCamHandle,tempSetting);

  //Make Sure Temperature Is Correct
  while(abs(currentTemp - tempSetting) > 1)
  {
    sleep(1);
  }

  // Set Exposure Time
  retVal = SetQHYCCDParam(pCamHandle, CONTROL_EXPOSURE, EXPOSURE_TIME);

  // Set Image Resolution
  retVal = SetQHYCCDResolution(pCamHandle, roiStartX, roiStartY, roiSizeX, roiSizeY);

  // Set Binning mode
  retVal = SetQHYCCDBinMode(pCamHandle, camBinX, camBinY);

  // Set Bit Resolution
  retVal = SetQHYCCDBitsMode(pCamHandle, 16);

  // Set ReadMode
  retVal = SetQHYCCDReadMode(pCamHandle, readMode);

}

// // // // // TAKE PICTURE // // // // //
void QuickCapture(unsigned int retVal, qhyccd_handle* pCamHandle, int runTimes, int runner, unsigned int roiStartX, unsigned int roiStartY, unsigned int roiSizeX, unsigned int roiSizeY, unsigned int bpp, unsigned int channels, int CHIP_GAIN, int CHIP_OFFSET, int EXPOSURE_TIME)
{
    // Single Frame
    retVal = ExpQHYCCDSingleFrame(pCamHandle);

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

    // Naming:
    string fitname = "qhyImg_" + to_string(time(0)) + "_exp_" + to_string(EXPOSURE_TIME)+"us_gain_" + to_string(CHIP_GAIN) + "_offset_" + to_string(CHIP_OFFSET) + "_" + to_string(runner) + ".fits";
    const char *fitsfilename = fitname.c_str();

    // Remove if exists already
    remove(fitsfilename); 

    // Create File
    fits_create_file(&fptr, fitsfilename, &status);
    fits_create_img(fptr, USHORT_IMG, 2, naxes, &status);

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
void QuickExit(unsigned int retVal, qhyccd_handle* pCamHandle)
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
  // Variables We Set Through Arguments
  int EXPOSURE_TIME = atoi(argv[1]); // Exposure time (in us)
  int CHIP_GAIN = atoi(argv[2]); // Gain Setting
  int CHIP_OFFSET = atoi(argv[3]); // Offset Setting
  double tempSetting = atof(argv[4]); // Temperature of Camera
  int readMode = atoi(argv[5]); // ReadMode
  int runTimes = atoi(argv[6]); // How Many Pictures To Get

  //Variables Preset
  unsigned int roiStartX = 0; // ROI Start x
  unsigned int roiStartY = 0; // ROI Start y
  unsigned int roiSizeX = 9600; // Max x
  unsigned int roiSizeY = 6422; // Max y
  int camBinX = 1; // Binning
  int camBinY = 1; // Binning
  int USB_TRAFFIC = 10; // USB Traffic
  unsigned int bpp = 16; // Bit Depth of Image
  unsigned int channels; // Channel of Image

  // // // // // INITIALIZE SDK // // // // //
  unsigned int retVal = InitQHYCCDResource();

  // // // // // INITIALIZE EVERYTHING ELSE // // // // //
  qhyccd_handle *pCamHandle = QuickInitialize(retVal);

  // // // // // CAMERA SETTINGS // // // // //
  QuickCamSettings(retVal, pCamHandle, USB_TRAFFIC, CHIP_GAIN, CHIP_OFFSET, tempSetting, EXPOSURE_TIME, roiStartX, roiStartY, roiSizeX, roiSizeY, camBinX, camBinY, readMode);

  // // // // // TAKE PICTURE // // // // //
  for (int runner = 0; runner < runTimes; runner++)
  {
    QuickCapture(retVal, pCamHandle, runTimes, runner, roiStartX, roiStartY, roiSizeX, roiSizeY, bpp, channels, CHIP_GAIN, CHIP_OFFSET, EXPOSURE_TIME);
  }

  // // // // // THE END // // // // //
  QuickExit(retVal, pCamHandle);

  // Exit
  return 0;
}
