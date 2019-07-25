#include <Arduino.h>

void SMSSerialFlush();
String readResponse();
void collectDebugInfo();
void collectSimInfo();
void configureModem();
String getModemSignal();
void sendSMSMessage(String txt);
String getValue(String data, String separator, int index);
