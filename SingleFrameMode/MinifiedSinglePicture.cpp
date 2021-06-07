
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fitsio.h>
#include "qhyccd.h"
#include <ctime>

using namespace std;

int main(int argc, char *argv[])
{
  // From chip
  double chipWidthMM;
  double chipHeightMM;
  double pixelWidthUM;
  double pixelHeightUM;
  unsigned int maxImageSizeX;
  unsigned int maxImageSizeY;
  unsigned int bpp;
  unsigned int channels;

  unsigned int retVal;
  char camId[32];
  camId = QuickSetUp(retVal);

  qhyccd_handle *pCamHandle = QuickCameraInit(retVal, camId);

  // get chip info
  retVal = GetQHYCCDChipInfo(*pCamHandle, &chipWidthMM, &chipHeightMM, &maxImageSizeX, &maxImageSizeY, &pixelWidthUM, &pixelHeightUM, &bpp);

  //=================|----------|================
  //=================|EDIT THESE|================
  //=================|----------|================

  // Set gain, offset, exposure, binning
  int USB_TRAFFIC = 10;
  int CHIP_GAIN = 10;
  int CHIP_OFFSET = 140;
  int EXPOSURE_TIME = 20000;
  int camBinX = 1;
  int camBinY = 1;
  int bit_resolution = 16;

  //Set ROI
  unsigned int roiStartX = 0;
  unsigned int roiStartY = 0;
  unsigned int roiSizeX = maxImageSizeX;
  unsigned int roiSizeY = maxImageSizeY;

  //=================|----------|================
  //=================|EDIT THESE|================
  //=================|----------|================

  //Set Camera Settings
  QuickCameraSettings(retVal, *pCamHandle, USB_TRAFFIC, CHIP_GAIN, CHIP_OFFSET, EXPOSURE_TIME, camBinX, camBinY, bit_resolution, roiStartX, roiStartY, roiSizeX, roiSizeY);

  //Take Picture
  unsigned char *pImgData = 0;
  QuickCapture(retVal, *pCamHandle, *pImgData, roiSizeX, roiSizeY, bpp, channels);

  //Finish
  QuickEnd(retVal, *pCamHandle);

  return 0;
}

//=============================================
//=============================================
//=================|---------|=================
//=================|FUNCTIONS|=================
//=================|---------|=================
//=============================================
//=============================================

//Initialize SDK and get Camera ID
char QuickSetUp(unsigned int retVal)
{
  // Initialize SDK
  retVal = InitQHYCCDResource();

  // Scan for Camera and get Camera ID
  int camCount = ScanQHYCCD();
  char camId[32];
  retVal = GetQHYCCDId(0, camId);

  return camId;
}

//Open and Initialize Camera
qhyccd_handle QuickCameraInit(unsigned int retVal, char *camId)
{
  // Open Camera
  qhyccd_handle *pCamHandle = OpenQHYCCD(camId);

  // Set Single Frame Mode
  int mode = 0;
  retVal = SetQHYCCDStreamMode(pCamHandle, mode);

  // Initialize camera
  retVal = InitQHYCCD(pCamHandle);

  return pCamHandle;
}

//Set Camera Settings
void QuickCameraSettings(unsigned int retVal, qhyccd_handle *pCamHandle, int USB_TRAFFIC, int CHIP_GAIN, int CHIP_OFFSET, int EXPOSURE_TIME, int camBinX, int camBinY, int bit_resolution, unsigned int roiStartX, unsigned int roiStartY, unsigned int roiSizeX, unsigned int roiSizeY)
{
  // Set Traffic
  retVal = SetQHYCCDParam(pCamHandle, CONTROL_USBTRAFFIC, USB_TRAFFIC);

  // Set Gain
  retVal = SetQHYCCDParam(pCamHandle, CONTROL_GAIN, CHIP_GAIN);

  // Set Offset
  retVal = SetQHYCCDParam(pCamHandle, CONTROL_OFFSET, CHIP_OFFSET);

  // Set Exposure Time
  retVal = SetQHYCCDParam(pCamHandle, CONTROL_EXPOSURE, EXPOSURE_TIME);

  // Set Image Resolution
  retVal = SetQHYCCDResolution(pCamHandle, roiStartX, roiStartY, roiSizeX, roiSizeY);

  // Set Binning Mode
  retVal = SetQHYCCDBinMode(pCamHandle, camBinX, camBinY);

  // Set Bit Resolution
  retVal = SetQHYCCDBitsMode(pCamHandle, bit_resolution);
}

//Capture Picture
void QuickCapture(unsigned int retVal, qhyccd_handle *pCamHandle, unsigned char *pImgData, unsigned int roiSizeX, unsigned int roiSizeY, unsigned int bpp, unsigned int channels)
{

  // single frame
  retVal = ExpQHYCCDSingleFrame(pCamHandle);
  if (QHYCCD_READ_DIRECTLY != retVal)
  {
    sleep(1);
  }

  // get and allocated requested memory lenght
  uint32_t length = GetQHYCCDMemLength(pCamHandle);
  pImgData = new unsigned char[length];
  memset(pImgData, 0, length);

  // Get Pic
  retVal = GetQHYCCDSingleFrame(pCamHandle, &roiSizeX, &roiSizeY, &bpp, &channels, pImgData);

  printf("GetQHYCCDSingleFrame: %d x %d, bpp: %d, channels: %d, success.\n", roiSizeX, roiSizeY, bpp, channels);

  // create file
  fitsfile *fptr;
  int status = 0;
  long naxes[2] = {roiSizeX, roiSizeY};

  string fitname = "qhyImg_" + to_string(time(0)) + ".fits";

  const char *fitsfilename = fitname.c_str();

  remove(fitsfilename); //Remove if exists already

  fits_create_file(&fptr, fitsfilename, &status);
  fits_create_img(fptr, USHORT_IMG, 2, naxes, &status);

  fits_write_img(fptr, TUSHORT, 1, roiSizeX * roiSizeY, pImgData, &status);

  fits_close_file(fptr, &status);

  delete[] pImgData;
}

//Stop and Close Camera and Release SDK Resources
void QuickEnd(unsigned int retVal, qhyccd_handle *pCamHandle)
{
  //Stop Exposing and Readout
  retVal = CancelQHYCCDExposingAndReadout(pCamHandle);

  //Close Camera Handle
  retVal = CloseQHYCCD(pCamHandle);

  //Release SDK Resources
  retVal = ReleaseQHYCCDResource();
}