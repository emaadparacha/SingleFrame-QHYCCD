/**
 * @file SingleFrameMode.cpp
 *
 * @brief C++ Program to take a single frame from a QHYCCD camera.
 * The program loops over different temperature, offset, gain, and exposure settings to take multiple pictures in one go.
 *
 * @author Emaad Paracha
 *
 */

// Dependencies
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

// !!Important!! -- These functions were tested with the QHY600 camera, some functions may not work properly with other QHYCCD cameras. 
// Please comment out relevant functions accordingly

//=============================================
//=============================================
//=================|---------|=================
//=================|FUNCTIONS|=================
//=================|---------|=================
//=============================================
//=============================================

/**
  @fn qhyccd_handle CamInitialize(unsigned int retVal, int USB_TRAFFIC, unsigned int roiStartX, unsigned int roiStartY, unsigned int roiSizeX, unsigned int roiSizeY, int camBinX, int camBinY, int readMode)
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
qhyccd_handle *CamInitialize(unsigned int retVal, int USB_TRAFFIC, unsigned int roiStartX, unsigned int roiStartY,
                               unsigned int roiSizeX, unsigned int roiSizeY, int camBinX, int camBinY, int readMode)
{
  // Check number of cameras connected
  int numCams = ScanQHYCCD();
  if (numCams > 0)
  {
    printf("Number of QHYCCD cameras found: %d. \n", numCams);
    printf("\n");
  }
  else
  {
    printf("No QHYCCD camera found. Please check USB or power.\n");
    exit(1);
  }

  // Get Camera ID
  char camId[32];
  retVal = GetQHYCCDId(0, camId);

  // Check if we could get camera ID
  if (retVal == QHYCCD_SUCCESS)
  {
    printf("Got Camera ID successfully. ID is %s .\n", camId);
    printf("\n");
  }
  else
  {
    printf("Could not get camera ID. Error: %d. Program will now exit. \n", retVal);
    exit(1);
  }

  // Open Camera
  qhyccd_handle *pCamHandle = OpenQHYCCD(camId);

  // Check if we could open the camera
  if (retVal == QHYCCD_SUCCESS)
  {
    printf("Camera opened successfully.\n");
    printf("\n");
  }
  else
  {
    printf("Could not open camera. Error: %d. Program will now exit. \n", retVal);
    exit(1);
  }

  // Set ReadMode
  retVal = SetQHYCCDReadMode(pCamHandle, readMode);
  if (retVal != QHYCCD_SUCCESS)
  {
    printf("Could not set read mode. Error: %d. Program will now exit. \n", retVal);
    exit(1);
  }

  // Set Single Frame Mode (mode = 0)
  retVal = SetQHYCCDStreamMode(pCamHandle, 0);
  if (retVal != QHYCCD_SUCCESS)
  {
    printf("Could not set stream mode. Error: %d. Program will now exit. \n", retVal);
    exit(1);
  }

  // Initialize Camera
  retVal = InitQHYCCD(pCamHandle);
  if (retVal != QHYCCD_SUCCESS)
  {
    printf("Could not initialize camera. Error: %d. Program will now exit. \n", retVal);
    exit(1);
  }

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
  if (retVal == QHYCCD_SUCCESS)
  {
    printf("USB traffic set to %d.\n", USB_TRAFFIC);
  }
  else
  {
    printf("Could not set USB traffic setting. Error: %d.\n", retVal);
  }

  // Set Image Resolution
  retVal = SetQHYCCDResolution(pCamHandle, roiStartX, roiStartY, roiSizeX, roiSizeY);
  if (retVal == QHYCCD_SUCCESS)
  {
    printf("Image resolution set to %dx%d.\n", roiSizeX, roiSizeY);
  }
  else
  {
    printf("Could not set the image resolution. Error: %d.\n", retVal);
  }

  // Set Binning mode
  retVal = SetQHYCCDBinMode(pCamHandle, camBinX, camBinY);
  if (retVal == QHYCCD_SUCCESS)
  {
    printf("Binning mode set to %dx%d.\n", camBinX, camBinY);
  }
  else
  {
    printf("Could not set the binning mode. Error: %d.\n", retVal);
  }

  // Set Bit Resolution
  retVal = SetQHYCCDBitsMode(pCamHandle, 16);
  if (retVal == QHYCCD_SUCCESS)
  {
    printf("Camera bit resolution set to %d.\n", 16);
  }
  else
  {
    printf("Could not set the bit resolution. Error: %d.\n", retVal);
  }

  printf(" \n");

  return pCamHandle;
}

/**
  @fn void CamSettings(unsigned int retVal, qhyccd_handle *pCamHandle, int gainSetting, int offsetSetting, double exposureTime)
    @brief Sets the gain, offset, and exposure time of the camera
    @param retVal Return value
    @param pCamHandle Camera handle
    @param gainSetting Gain setting to set camera to
    @param offsetSetting Offset setting to set camera to
    @param exposureTime Exposure time
*/
void CamSettings(unsigned int retVal, qhyccd_handle *pCamHandle, int gainSetting, int offsetSetting,
                      double exposureTime)
{
  // Set Gain Setting
  retVal = SetQHYCCDParam(pCamHandle, CONTROL_GAIN, gainSetting);
  if (retVal == QHYCCD_SUCCESS)
  {
    printf("Gain set to %d.\n", gainSetting);
  }
  else
  {
    printf("Could not set the gain setting. Error: %d.\n", retVal);
  }

  // Set Offset
  retVal = SetQHYCCDParam(pCamHandle, CONTROL_OFFSET, offsetSetting);
  if (retVal == QHYCCD_SUCCESS)
  {
    printf("Offset set to %d.\n", offsetSetting);
  }
  else
  {
    printf("Could not set the offset setting. Error: %d.\n", retVal);
  }

  // Set Exposure Time
  retVal = SetQHYCCDParam(pCamHandle, CONTROL_EXPOSURE, exposureTime);
  if (retVal == QHYCCD_SUCCESS)
  {
    printf("Exposure set to %.6f seconds. \n", exposureTime / 1000000);
  }
  else
  {
    printf("Could not set the exposure time. Error: %d.\n", retVal);
  }
}

/**
  @fn void TempRegulation(unsigned int retVal, qhyccd_handle *pCamHandle, double tempSetting, double tempError)
    @brief Sets the temperature of the camera sensor within the specified temperature error range
    @param retVal Return value
    @param pCamHandle Camera handle
    @param tempSetting Temperature to set camera to
    @param tempError Temperature setting error range
*/
void TempRegulation(unsigned int retVal, qhyccd_handle *pCamHandle, double tempSetting, double tempError)
{

  printf(" \n"); // Print new line

  // Get Current Temperature
  double currentTemp = GetQHYCCDParam(pCamHandle, CONTROL_CURTEMP);

  // Set Temperature to the temperature setting we want
  retVal = SetQHYCCDParam(pCamHandle, CONTROL_COOLER, tempSetting);
  if (retVal != QHYCCD_SUCCESS)
  {
    printf("Could not set the temperature. Error: %d.\n", retVal);
    return; // Return if we could not set the temperature
  }

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
        // Get cooler PWM again
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

        // Try again in 2 seconds
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
  @fn void FilterWheelControl(unsigned int retVal, qhyccd_handle *pCamHandle, char fwPosition)
    @brief Checks if a filter wheel is connected and if so, moves the filter wheel to the requested position
    @param retVal Return value
    @param pCamHandle Camera handle
    @param fwPosition Filter wheel position to move to
*/
void FilterWheelControl(unsigned int retVal, qhyccd_handle *pCamHandle, int fwPos)
{
  retVal = IsQHYCCDCFWPlugged(pCamHandle); // Check if filter wheel is plugged in

  char fwPosition = '0' + fwPos;

  // If filter wheel is plugged in
  if (retVal == QHYCCD_SUCCESS)
  {
    char status[64] = {0};
    retVal = GetQHYCCDCFWStatus(pCamHandle, status); // Get current position
    if (retVal == QHYCCD_SUCCESS)
    {
      printf("Filter wheel is plugged in and is at position: %s. \n", status); // Print current position
    }
    else
    {
      printf("Could not get filter wheel status. Error: %d.\n", retVal); // Print error
    }

    // Compare if the filter wheel is at the position we want it to be
    if (status[0] != fwPosition)
    {
      retVal = SendOrder2QHYCCDCFW(pCamHandle, &fwPosition, 1); // Send order to filter wheel to move to new position
      if (retVal == QHYCCD_SUCCESS)
      {
        printf("Filter wheel is moving to position: %c. \n", fwPosition); // Print that the filter wheel is moving
      }
      else
      {
        printf("Could not move filter wheel. Error: %d.\n", retVal); // Print error
      }

      // Check if filter wheel is moving
      retVal = GetQHYCCDCFWStatus(pCamHandle, status);
      if (retVal != QHYCCD_SUCCESS)
      {
        printf("Could not get filter wheel status. Error: %d.\n", retVal); // Print error
      }

      // If filter wheel needs to go to position 0 (slot 1)
      if (status[0] == fwPosition)
      {
        int sleeper = 0; // To print progress on screen

        while (sleeper < 11)
        {
          sleep(1); // Sleep for 1 second
          printf("Filter wheel is still moving.\n");
          sleeper++; // Add onto sleeper
        }
      }
      else // If filter wheel needs to go to any other position
      {
        // While camera is moving
        while (fwPosition != status[0])
        {
          sleep(1);                                        // Sleep for 1 second
          retVal = GetQHYCCDCFWStatus(pCamHandle, status); // Check status again
          if (retVal != QHYCCD_SUCCESS)
          {
            printf("Could not get filter wheel status. Error: %d.\n", retVal); // Print error
          }
          else
          {
            printf("Filter wheel is still moving.\n"); // Print that the filter wheel is still moving
          }
        }
      }

      // Print final position of filter wheel
      printf("Filter wheel has been moved to position: %c. \n", fwPosition); // Print new position
    }
  }

  // If filter wheel is not detected
  else
  {
    printf("No filter wheel detected. \n"); // Print that it is not detected
  }

  printf("\n");
}

/**
  @fn void CamCapture(unsigned int retVal, qhyccd_handle *pCamHandle, int runTimes, int runner, unsigned int roiStartX, unsigned int roiStartY, unsigned int roiSizeX, unsigned int roiSizeY, unsigned int bpp, int gainSetting, int offsetSetting, double exposureTime, double tempSetting, int readMode, string savePath)
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
void CamCapture(unsigned int retVal, qhyccd_handle *pCamHandle, int runTimes, int runner, unsigned int roiStartX,
                  unsigned int roiStartY, unsigned int roiSizeX, unsigned int roiSizeY, unsigned int bpp, int gainSetting,
                  int offsetSetting, double exposureTime, double tempSetting, int readMode, string savePath)
{
  // Channel of Image
  unsigned int channels;

  // Single Frame
  retVal = ExpQHYCCDSingleFrame(pCamHandle);
  if (retVal != QHYCCD_SUCCESS)
  {
    printf("Could not start exposure. Error: %d. \n", retVal);
  }

  // Image Data Variable
  unsigned char *pImgData = 0;

  // Get Requested Memory Length
  uint32_t length = GetQHYCCDMemLength(pCamHandle);
  pImgData = new unsigned char[length];
  memset(pImgData, 0, length);

  printf("Buffer length = %d.\n", length);

  // Take Single Frame
  retVal = GetQHYCCDSingleFrame(pCamHandle, &roiSizeX, &roiSizeY, &bpp, &channels, pImgData);
  if (retVal == QHYCCD_SUCCESS)
  {
    printf("Successfully got image of size: %dx%d.\n", roiSizeX, roiSizeY);
  }
  else
  {
    printf("Could not grab image data from camera. Error: %d. \n", retVal);
  }

  // Image Processing to .fits file

  // Create File
  fitsfile *fptr;
  int status = 0;
  long naxes[2] = {roiSizeX, roiSizeY};
  long curUnixTime = time(0);

  // Naming:
  string fitname = savePath + "_" + to_string(curUnixTime) + "_exp_" + to_string((int)exposureTime) + "us_gain_" + to_string(gainSetting) + "_offset_" + to_string(offsetSetting) + "_temp_" + to_string((int)tempSetting) + "_" + to_string(runner) + ".fits";
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
  if (retVal == QHYCCD_SUCCESS)
  {
    printf("Exposure and readout cancelled successfully.\n");
  }
  else
  {
    printf("Could not cancel exposure and readout. Error: %d. \n", retVal);
  }
}

/**
  @fn void CamExit(unsigned int retVal, qhyccd_handle *pCamHandle)
    @brief Closes camera and releases SDK resource
    @param retVal Return value
    @param pCamHandle Camera handle
*/
void CamExit(unsigned int retVal, qhyccd_handle *pCamHandle)
{
  // Close Camera Handle
  retVal = CloseQHYCCD(pCamHandle);
  if (retVal == QHYCCD_SUCCESS)
  {
    printf("Camera handle closed successfully. \n");
  }
  else
  {
    printf("Could not close camera handle. Error: %d. \n", retVal);
  }

  // Release SDK Resources
  retVal = ReleaseQHYCCDResource();
  if (retVal == QHYCCD_SUCCESS)
  {
    printf("SDK resources released successfully. \n");
  }
  else
  {
    printf("Could not release SDK resources. Error: %d. Program will now exit.\n", retVal);
    exit(1);
  }

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

  // Variables Preset
  unsigned int roiStartX = 0;   // ROI Start x
  unsigned int roiStartY = 0;   // ROI Start y
  unsigned int roiSizeX = 9600; // Max x
  unsigned int roiSizeY = 6422; // Max y
  int camBinX = 1;              // Binning
  int camBinY = 1;              // Binning
  int USB_TRAFFIC = 10;         // USB Traffic
  unsigned int bpp = 16;        // Bit Depth of Image
  int readMode = 1;             // ReadMode
  const int SECOND = 1000000;   // Constant to multiply exposure time with, since QHY600M takes microseconds

  // Initialize SDK
  unsigned int retVal = InitQHYCCDResource();

  // Check if SDK resources were initialized
  if (retVal == QHYCCD_SUCCESS)
  {
    printf("SDK resources initialized successfully .\n");
    printf("\n");
  }
  else
  {
    printf("SDK resources could not be initialized. Error: %d. Program will now exit. \n", retVal);
    exit(1);
  }

  // Initialize the camera and set initial settings
  qhyccd_handle *pCamHandle = CamInitialize(retVal, USB_TRAFFIC, roiStartX, roiStartY, roiSizeX, roiSizeY, camBinX, camBinY, readMode);

  retVal = SetQHYCCDParam(pCamHandle, CONTROL_MANULPWM, 0);

  // The List of All Variables -- SET THESE TO TAKE IMAGES
  int sampleGains[] = {56,60};    // List of gain settings to loop over
  int sampleOffsets[] = {20,40};  // List of offset setings to loop over
  double sampleTemps[] = {18,20}; // List of temperatures to loop over (in Celsius)
  double sampleExps[] = {5,10};  // List of exposure times to loop over (in seconds)
  int howManyTimesToRun = 2;   // How many times to take pictures at each unique setting
  double tempError = 0.3;      // Temperature regulation error range
  int fwPosition = 2;          // Set this to the filter wheel position you want (between 0 and 6)

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
          double exposureTime = sampleExps[e] * SECOND;           // Exposure time (in us)
          int gainSetting = sampleGains[g];                       // Gain Setting
          int offsetSetting = sampleOffsets[o];                   // Offset Setting
          double tempSetting = sampleTemps[t];                    // Temperature of Camera
          int runTimes = howManyTimesToRun;                       // How Many Pictures To Get
          string savePath = "/home/user/Documents/Images/qhyImg"; // Path to save image with first part of image name at the end

          // Operate filter wheel
          FilterWheelControl(retVal, pCamHandle, fwPosition);

          // Set camera settings
          CamSettings(retVal, pCamHandle, gainSetting, offsetSetting, exposureTime);

          // Set and regulate temperature
          TempRegulation(retVal, pCamHandle, tempSetting, tempError);

          // Loop to take multiple pictures
          for (int runner = 0; runner < runTimes; runner++)
          {
            // Set and regulate temperature again
            TempRegulation(retVal, pCamHandle, tempSetting, tempError);

            // Print which image is being taken
            printf("Taking image %d of %d images... \n", takingImage, totalNumberOfFiles);

            // Take the picture and save it
            CamCapture(retVal, pCamHandle, runTimes, runner, roiStartX, roiStartY, roiSizeX, roiSizeY, bpp, gainSetting, offsetSetting, exposureTime, tempSetting, readMode, savePath);

            // Increment takingImage
            takingImage++;
          }
        }
      }
    }
  }

  // Close camera and release SDK resources
  CamExit(retVal, pCamHandle);

  // Exit
  return 0;
}
