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

/**
  @fn qhyccd_handle QuickInitialize(unsigned int retVal, int USB_TRAFFIC, unsigned int roiStartX, unsigned int roiStartY, unsigned int roiSizeX, unsigned int roiSizeY, int camBinX, int camBinY, int readMode)
    @brief Initialize the camera, set the readmode, image resolution, binning mode, and bit resolution, and return the camera handle
    @param retVal Return value
    @param USB_TRAFFIC USB traffic value
    @param roiStartX Region of Interest starting X coordinate
    @param roiStartY Region of Interest starting Y coordinate
    @param roiSizeX Region of Interest size in X
    @param roiSizeY Region of Interest size in Y
    @param camBinX Camera binning size (X)
    @param camBinY Camera binning size (Y)
    @param readMode Camera readmode
  @return Return QHY camera handle and set readmode, image resolution, binning mode, and bit resolution
*/
qhyccd_handle *QuickInitialize(unsigned int retVal, int USB_TRAFFIC, unsigned int roiStartX, unsigned int roiStartY,
                               unsigned int roiSizeX, unsigned int roiSizeY, int camBinX, int camBinY, int readMode)
{
  // Get Camera ID
  char camId[32];
  retVal = GetQHYCCDId(0, camId);

  // Open Camera
  qhyccd_handle *pCamHandle = OpenQHYCCD(camId);

  // Set ReadMode
  retVal = SetQHYCCDReadMode(pCamHandle, readMode);

  // Set Single Frame Mode (mode = 0)
  retVal = SetQHYCCDStreamMode(pCamHandle, 0);

  // Initialize Camera
  retVal = InitQHYCCD(pCamHandle);
  printf(" \n");
  printf("Hello! Welcome to the QHY Imaging Centre.\n");
  printf(" \n");
  printf("Connecting to QHY Camera.\n");
  printf("QHY Camera initialized successfully. \n");
  printf("This is camera ID: %s \n", camId); // To print camera ID


  printf(" \n");

  printf("Camera readmode set to %d.\n", readMode);

  // Set USB Traffic Setting
  retVal = SetQHYCCDParam(pCamHandle, CONTROL_USBTRAFFIC, USB_TRAFFIC);
  printf("USB traffic set to %d.\n", USB_TRAFFIC);

  // Set Image Resolution
  retVal = SetQHYCCDResolution(pCamHandle, roiStartX, roiStartY, roiSizeX, roiSizeY);
  printf("Image resolution set to %dx%d.\n", roiSizeX, roiSizeY);

  // Set Binning mode
  retVal = SetQHYCCDBinMode(pCamHandle, camBinX, camBinY);
  printf("Binning mode set to %dx%d.\n", camBinX, camBinY);

  // Set Bit Resolution
  retVal = SetQHYCCDBitsMode(pCamHandle, 16);
  printf("Camera bit resolution set to %d.\n", 16);

  printf(" \n");

  return pCamHandle;
}

/**
  @fn void QuickCamSettings(unsigned int retVal, qhyccd_handle *pCamHandle, int gainSetting, int offsetSetting, double exposureTime)
    @brief Sets the gain, offset, and exposure time of the camera
    @param retVal Return value
    @param pCamHandle Camera handle
    @param gainSetting Gain setting to set camera to
    @param offsetSetting Offset setting to set camera to
    @param exposureTime Exposure time
*/
void QuickCamSettings(unsigned int retVal, qhyccd_handle *pCamHandle, int gainSetting, int offsetSetting,
                      double exposureTime)
{
  // Set Gain Setting
  retVal = SetQHYCCDParam(pCamHandle, CONTROL_GAIN, gainSetting);
  printf("Gain set to %d.\n", gainSetting);

  // Set Offset
  retVal = SetQHYCCDParam(pCamHandle, CONTROL_OFFSET, offsetSetting);
  printf("Offset set to %d.\n", offsetSetting);

  // Set Exposure Time
  retVal = SetQHYCCDParam(pCamHandle, CONTROL_EXPOSURE, exposureTime);
  printf("Exposure set to %.6f seconds. \n", exposureTime / 1000000);
}

/**
  @fn void QuickTempRegulation(unsigned int retVal, qhyccd_handle *pCamHandle, double tempSetting, double tempError)
    @brief Sets the temperature of the camera sensor within the specified temperature error range
    @param retVal Return value
    @param pCamHandle Camera handle
    @param tempSetting Temperature to set camera to
    @param tempError Temperature setting error range
*/
void QuickTempRegulation(unsigned int retVal, qhyccd_handle *pCamHandle, double tempSetting, double tempError)
{

  printf(" \n"); // Print new line

  // Get Current Temperature
  double currentTemp = GetQHYCCDParam(pCamHandle, CONTROL_CURTEMP);

  // Set Temperature to the temperature setting we want
  retVal = SetQHYCCDParam(pCamHandle, CONTROL_COOLER, tempSetting);

  // Get cooler PWM value
  double pwmValue = GetQHYCCDParam(pCamHandle, CONTROL_CURPWM);

  // If the temperature is not within the error range
  if (abs(currentTemp - tempSetting) > tempError)
  {
    // Then run the check temperature loop thrice to avoid overshooting
    for (int tempLoop = 0; tempLoop < 3; tempLoop++)
    {
      // Sleep for 1 second to get new temperature
      sleep(1);

      // Get current temperature again to loop
      currentTemp = GetQHYCCDParam(pCamHandle, CONTROL_CURTEMP);

      // While the temperature is outside of the error range
      while (abs(currentTemp - tempSetting) > tempError)
      {
        //Get cooler PWM again
        pwmValue = GetQHYCCDParam(pCamHandle, CONTROL_CURPWM);

        // Report temperature progress and cooler PWM to screen
        if ((currentTemp - tempSetting) > tempError)
        {
          printf("Current Temperature: %.2f || You Want: %.2f . Camera is cooling down. \n", currentTemp, tempSetting);
          printf("Cooler PWM is %.1f, running at %.1f%% of full power. \n", pwmValue, pwmValue / 255.0 * 100);
          printf(" \n");
        }

        else
        {
          printf("Current Temperature: %.2f || You Want: %.2f . Camera is heating up. \n", currentTemp, tempSetting);
          printf("Cooler PWM is %.1f, running at %.1f%% of full power. \n", pwmValue, pwmValue / 255.0 * 100);
          printf(" \n");
        }

        //Try again in 2 seconds
        sleep(2);

        // Get current temperature again to loop again
        currentTemp = GetQHYCCDParam(pCamHandle, CONTROL_CURTEMP);
      }
    }

    // Sleep for 1 second before running
    sleep(1);
  }

  printf("Camera temperature set to %.2f C. \n", tempSetting);
}

/**
  @fn void QuickFilterWheelControl(unsigned int retVal, qhyccd_handle *pCamHandle, char fwPosition)
    @brief Checks if a filter wheel is connected and if so, moves the filter wheel to the requested position
    @param retVal Return value
    @param pCamHandle Camera handle
    @param fwPosition Filter wheel position to move to
*/
void QuickFilterWheelControl(unsigned int retVal, qhyccd_handle *pCamHandle, char fwPosition)
{
  retVal = IsQHYCCDCFWPlugged(pCamHandle); // Check if filter wheel is plugged in

  // If filter wheel is plugged in
  if (retVal == QHYCCD_SUCCESS)
  {
    char status[64];
    retVal = GetQHYCCDCFWStatus(pCamHandle, status);                          // Get current position
    printf("Filter wheel is plugged in and is at position: %c. \n", *status); // Print current position

    // Compare if the filter wheel is at the position we want it to be
    if (*status != fwPosition)
    {
      retVal = SendOrder2QHYCCDCFW(pCamHandle, &fwPosition, 1);         // Send order to filter wheel to move to new position
      printf("Filter wheel is moving to position: %c. \n", fwPosition); // Print that the filter wheel is moving
    }

    // Check if filter wheel is moving
    retVal = GetQHYCCDCFWStatus(pCamHandle, status);

    // While camera is moving
    while (fwPosition != status[0])
    {
      retVal = GetQHYCCDCFWStatus(pCamHandle, status); // Check status again
      sleep(2);                                        // Sleep for 2 seconds
    }

    // Print final position of filter wheel
    printf("Filter wheel has been moved to position: %c. \n", fwPosition); // Print new position
  }

  // If filter wheel is not detected
  else
  {
    printf("No filter wheel detected. \n"); // Print that it is not detected
  }

  printf("\n");
}

/**
  @fn void QuickCapture(unsigned int retVal, qhyccd_handle *pCamHandle, int runTimes, int runner, unsigned int roiStartX, unsigned int roiStartY, unsigned int roiSizeX, unsigned int roiSizeY, unsigned int bpp, int gainSetting, int offsetSetting, double exposureTime, double tempSetting, int readMode, string savePath)
    @brief Takes an image and saves a .fits file to disk with the settings in the filename
    @param retVal Return value
    @param pCamHandle Camera handle
    @param runTimes Number of times to take pictures at each specific setting
    @param runner The number of image being taken at that specific setting
    @param roiStartX Region of Interest starting X coordinate
    @param roiStartY Region of Interest starting Y coordinate
    @param roiSizeX Region of Interest size in X
    @param roiSizeY Region of Interest size in Y
    @param bpp Channel of image 
    @param gainSetting Gain setting to set camera to
    @param offsetSetting Offset setting to set camera to
    @param exposureTime Exposure time
    @param tempSetting Temperature to set camera to
    @param readMode Camera readmode
    @param savePath Path to save image to
*/
void QuickCapture(unsigned int retVal, qhyccd_handle *pCamHandle, int runTimes, int runner, unsigned int roiStartX,
                  unsigned int roiStartY, unsigned int roiSizeX, unsigned int roiSizeY, unsigned int bpp, int gainSetting,
                  int offsetSetting, double exposureTime, double tempSetting, int readMode, string savePath)
{
  // Channel of Image
  unsigned int channels;

  // Single Frame
  retVal = ExpQHYCCDSingleFrame(pCamHandle);

  // Image Data Variable
  unsigned char *pImgData = 0;

  // Get Requested Memory Length
  uint32_t length = GetQHYCCDMemLength(pCamHandle);
  pImgData = new unsigned char[length];
  memset(pImgData, 0, length);

  printf("Buffer length = %d.\n", length); 

  // Take Single Frame
  retVal = GetQHYCCDSingleFrame(pCamHandle, &roiSizeX, &roiSizeY, &bpp, &channels, pImgData);
  printf("Successfully got image of size: %dx%d.\n", roiSizeX, roiSizeY);

  // Image Processing to .fits file

  // Create File
  fitsfile *fptr;
  int status = 0;
  long naxes[2] = {roiSizeX, roiSizeY};
  long curUnixTime = time(0);

  // Naming:
  string fitname = savePath + "_" + to_string(curUnixTime) + "_exp_" + to_string((int)exposureTime) + "us_gain_" + to_string(gainSetting) + "_offset_" + to_string(offsetSetting) + "_temp_" + to_string((int)tempSetting) + "_" + to_string(runner) + +".fits";
  const char *fitsfilename = fitname.c_str();

  // Remove if exists already
  remove(fitsfilename);

  // Create File
  fits_create_file(&fptr, fitsfilename, &status);
  fits_create_img(fptr, USHORT_IMG, 2, naxes, &status);

  // Headers Information
  fits_update_key(fptr, TDOUBLE, "INTTEMP", &tempSetting, "Camera Temperature", &status);
  fits_update_key(fptr, TINT, "EXPTIME", &exposureTime, "Exposure time in microseconds", &status);
  fits_update_key(fptr, TINT, "OFFSET", &offsetSetting, "Offset Setting", &status);
  fits_update_key(fptr, TINT, "GAIN", &gainSetting, "Gain Setting", &status);
  fits_update_key(fptr, TINT, "QHREADMOE", &readMode, "ReadMode Setting", &status);
  fits_update_key(fptr, TLONG, "TIME", &curUnixTime, "UNIX Time", &status);

  // Write to File
  fits_write_img(fptr, TUSHORT, 1, roiSizeX * roiSizeY, pImgData, &status);

  // Close File
  fits_close_file(fptr, &status);

  // Report progress
  printf("Image with temp %.2fC, exp %.3fsec, offset %d, gain %d, saved successfully to disc.\n", tempSetting, exposureTime / 1000000, offsetSetting, gainSetting);
  printf(" \n");

  // Delete Image Data
  delete[] pImgData;

  // Cancel Exposing and Readout
  retVal = CancelQHYCCDExposingAndReadout(pCamHandle);
}

/**
  @fn void QuickExit(unsigned int retVal, qhyccd_handle *pCamHandle)
    @brief Closes camera and releases SDK resource
    @param retVal Return value
    @param pCamHandle Camera handle
*/
void QuickExit(unsigned int retVal, qhyccd_handle *pCamHandle)
{
  // Close Camera Handle
  retVal = CloseQHYCCD(pCamHandle);

  // Release SDK Resources
  retVal = ReleaseQHYCCDResource();
  printf("Goodbye! Please visit us again.\n");
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
  const int SECOND = 1000000;   // Constant to multiply exposure time with

  // Initialize SDK
  unsigned int retVal = InitQHYCCDResource();

  // Initialize the camera and set initial settings
  qhyccd_handle *pCamHandle = QuickInitialize(retVal, USB_TRAFFIC, roiStartX, roiStartY, roiSizeX, roiSizeY, camBinX, camBinY, readMode);

  retVal = SetQHYCCDParam(pCamHandle,CONTROL_MANULPWM,0);


  // The List of All Variables -- SET THESE TO TAKE IMAGES
  int sampleGains[] = {56};    // List of gain settings to loop over
  int sampleOffsets[] = {20};  // List of offset setings to loop over
  double sampleTemps[] = {18}; // List of temperatures to loop over (in Celsius)
  double sampleExps[] = {10};   // List of exposure times to loop over (in seconds)
  int howManyTimesToRun = 1;   // How many times to take pictures at each unique setting
  double tempError = 0.3;      // Temperature regulation error range
  char fwPosition = '0';       // Set this to the filter wheel position you want (between 0 and 6)

  int totalNumberOfFiles = sizeof(sampleTemps) / sizeof(sampleTemps[0]) * sizeof(sampleOffsets) / sizeof(sampleOffsets[0]) * sizeof(sampleGains) / sizeof(sampleGains[0]) * sizeof(sampleExps) / sizeof(sampleExps[0]) * howManyTimesToRun; // How many images will be taken

  int takingImage = 1; // Which image is being taken

  // LoOp ThE lOoPs and take the pictures
  for (unsigned int t = 0; t < sizeof(sampleTemps) / sizeof(sampleTemps[0]); t++)
  {
    for (unsigned int o = 0; o < sizeof(sampleOffsets) / sizeof(sampleOffsets[0]); o++)
    {
      for (unsigned int g = 0; g < sizeof(sampleGains) / sizeof(sampleGains[0]); g++)
      {
        for (unsigned int e = 0; e < sizeof(sampleExps) / sizeof(sampleExps[0]); e++)
        {
          double exposureTime = sampleExps[e] * SECOND;               // Exposure time (in us)
          int gainSetting = sampleGains[g];                           // Gain Setting
          int offsetSetting = sampleOffsets[o];                       // Offset Setting
          double tempSetting = sampleTemps[t];                        // Temperature of Camera
          int runTimes = howManyTimesToRun;                           // How Many Pictures To Get
          string savePath = "/home/emaad/Documents/Images/qhyImg"; // Path to save image with first part of image name at the end

          // Operate filter wheel
          QuickFilterWheelControl(retVal, pCamHandle, fwPosition);

          // Set camera settings
          QuickCamSettings(retVal, pCamHandle, gainSetting, offsetSetting, exposureTime);

          // Set and regulate temperature
          QuickTempRegulation(retVal, pCamHandle, tempSetting, tempError);

          // Loop to take multiple pictures
          for (int runner = 0; runner < runTimes; runner++)
          {
            // Set and regulate temperature again
            QuickTempRegulation(retVal, pCamHandle, tempSetting, tempError);

            // Print which image is being taken
            printf("Taking image %d of %d images... \n", takingImage, totalNumberOfFiles);

            // Take the picture and save it
            QuickCapture(retVal, pCamHandle, runTimes, runner, roiStartX, roiStartY, roiSizeX, roiSizeY, bpp, gainSetting, offsetSetting, exposureTime, tempSetting, readMode, savePath);

            // Increment takingImage
            takingImage++;
          }
        }
      }
    }
  }

  //Turn off cooler after image capturing is complete
  //retVal = SetQHYCCDParam(pCamHandle,CONTROL_MANULPWM,0);

  // Close camera and release SDK resources
  QuickExit(retVal, pCamHandle);

  // Exit
  return 0;
}