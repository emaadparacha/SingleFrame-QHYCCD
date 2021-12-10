# SingleFrame-QHYCCD

C++ program to take a single frame from a QHYCCD camera. The program loops over different temperature, offset, gain, and exposure settings to take multiple pictures in one go.

## Dependencies
* [QHYCCD SDK](https://www.qhyccd.com/html/prepub/log_en.html)
* [CFITSIO](https://heasarc.gsfc.nasa.gov/fitsio/)
* A QHYCCD camera

## Description
This C++ program takes multiple single frame images from a QHYCCD camera by looping over different temperature, offset, gain, and exposure settings.
In the program, you can set these settings in the corresponding arrays:
* `sampleGains` for gain settings
* `sampleOffsets` for offset settings
* `sampleTemps` for temperature settings
* `sampleExps` for exposure time settings

The `howManyTimesToRun` parameter specifies how many times to take images at each unique combination of gain, offset, temperature, and exposure time.

The `tempError` parameter specifies the temperature error range (if the current CCD temperature is within the error range, the image will be taken.

The `fwPosition` parameter specifies the filter wheel position (if there is a filter wheel attached).

The `savePath` parameter specifies where to save the image and the prefix for the image.

## Operation
Simply run `make` and `./SingleFrameMode` to compile and run the program.

Any changes made to `SingleFrameMode.cpp` after compilation would require a `make clean` and subsequent `make` to recompile.

## Things to look out for
* This program was written for a QHY600M camera, therefore several variables are defaulted to that camera (such as `roiSizeX` and `roiSizeY`). If you are using a different QHYCCD camera, please change them accordingly.
* The QHY600M camera takes exposure time input in microseconds. Therefore, there is a constant declared `const int SECOND = 1000000` to have user input in seconds for ease of usage. This can be changed accordingly if needed.
* The `savePath` parameter should be changed to where you want to save the image. The last part of the `savePath` string is the image name prefix (currently set to `qhyImg`. Each image name starts with this.
* The image names are of the format `qhyImg_(Unix Time)_exp_(exposure time)_us_gain_(gain setting)_offset_(offset setting)_temp_(temperature)_(which iteration of the image).fits`. This is to allow for uniqueness and sorting ease.
* The program saves the image as a FITS file, this can be changed to anything else if needed.
