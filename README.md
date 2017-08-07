![image](http://www.4dsystems.com.au/imagenes/header.png)

# ViSi-Genie-Arduino-Library - BETA - Updated 18-JULY-2017
==========================================================

Arduino Library for 4D Systems ViSi-Genie Environment

This library is an updated version of the original genieArduino library https://github.com/4dsystems/ViSi-Genie-Arduino-Library
and includes additional enhancements and funcationality.

## Information

This library provides high level functions for the Arduino, to ease communication with 4D Systems modules when using the module configured with ViSi-Genie.
Workshop4 PRO adds additional features to ViSi-Genie, allowing the User to write 4DGL code which can be executed at will from ViSi-Genie, enabling many advanced features not previously possible, as well as the Smart Widgets Editor for creating your own gauges, sliders, knobs etc.
Please refer to the 4D Systems website, namingly the Workshop 4 Product Page, for documentation regarding Workshop 4, and its environments.
[4D Systems website, Workshop4 Product Page](http://www.4dsystems.com.au/product/4D-Workshop-4-IDE)

## Installation

Library folder should be placed in the ```C:\Users\(User name)\My Documents\Arduino\Libraries\``` folder, or equivalent. (restart the IDE if already open).

PLEASE ensure that the old library (if installed) has been removed completely so it does not conflict.

For more information on the installation, please refer to [Installing Additional Arduino Libraries](http://arduino.cc/en/Guide/Libraries)

Open the ViSi-Genie project using Workshop4 and download to your display, connect the display to Arduino, reset the Arduino and it should work.

## Example Sketches

Inside the library are example sketches, to assist with getting started using this library. Inside is also a ViSi-Genie Workshop4 project, which can be used on a range of 4D Systems displays (designed for one specific display, however the target display can be changed via Workshop4 menu). It illustrates how to use some of the commands in the library include Read Object, Write Object, Reported Messages, Write Contrast, Write String, etc.

## Tested with

This library has been tested on the Duemilanove, Uno, Mega 1280, Mega 2560, Leonardo, Chipkit Max32, Due, Intel Galileo, Teensy and Yun (Software Serial only on Yun). 
Any problems discovered with this library, please contact technical support so fixes can be put in place, or seek support from our forum.

## Compatible 4D Systems Display Modules

This library will work with all 4D Systems Modules which are capable of using the ViSi-Genie environment. This is therefore all Picaso and Diablo16 Display Modules.
The demo included can easily be adapted to other size displays with a little editing, or simply changing the target display in Workshop4. Some minor positioning changes may be required, depending on the display you are changing to.

## Questions/Issues?

Please sign up for our Forum and ask a question there, or submit a Tech Support Ticket from our website.
Please do NOT post issues on Github, as this is not actively monitored. Please use one of the following:
http://forum.4dsystems.com.au or http://www.4dsystems.com.au/support
