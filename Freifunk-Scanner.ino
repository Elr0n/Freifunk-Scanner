// LCD ST7735 
#include <Adafruit_GFX.h>   
#include <Adafruit_ST7735.h> 
#include <SPI.h>
#define TFT_CS     4
#define TFT_RST    5  
#define TFT_DC     16
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS,  TFT_DC, TFT_RST);
boolean refreshScreen = true;

// WIFI
#include "ESP8266WiFi.h"
#define RSSI_MAX   -35
#define RSSI_MIN   -95


// WIFI PROBE
#define WIFIPROBE_RA_SIZE 128
String  wifiProbeSSID                     = "trier.freifunk.net";
int     wifiProbeCount                    = 0;
int     wifiProbeBestRSSI                 = 0;
int     wifiProbeBestQuality              = 0;
String  wifiProbeEncryption               = "?";
int     wifiProbeRSSIArray[WIFIPROBE_RA_SIZE];
int     wifiProbeRSSIArrayNextValuePointer  = 0;


// FF WIFI PROBE
#define     FF_WIFIPROBE_RA_SIZE 102
int         ffWifiProbeCount                        = 0;
int         ffWifiProbeBestRSSI                     = 0;
int         ffWifiProbeBestQuality                  = 0;
int         ffWifiProbeRSSIArray[WIFIPROBE_RA_SIZE];
int         ffWifiProbeRSSIArrayNextValuePointer    = 0;
IPAddress   ffWifiLocalIP; 
IPAddress   ffWifiGatewayIP;
IPAddress   ffWifiSubnetMask;
WiFiClient  ffWifiClient;
const int   ffWifiHttpPort = 80;
String      ffRouterName;
String      ffRouterModel;
String      ffRouterFirmwareVersion;
String      ffRouterRuntime;
String      ffRouterCPUUtilization;
String      ffRouterRAMUtilization;
String      ffRouterMAC;
String      ffRouterIP6;
String      ffRouterContactPerson;

const char* FF_SSID     = "trier.freifunk.net";
const char* FF_PASSWORD = "";
const char* FF_HOST     = "router.fftr";

// MODE
#define START_SCREEN_DELAY    5000
#define MODE_START_SCREEN     1
#define MODE_MAIN_MENU        2
#define MODE_WIFI_SCAN        3
#define MODE_WIFI_PROBE       4
#define MODE_FF_CLIENT_PROBE  5
#define MODE_FF_MESH_PROBE    6
#define MODE_FF_ROUTER_CHECK  7
int modeCursor = 0;
boolean refreshModeCursor = true;

int MODE = MODE_START_SCREEN;

// BUTTONS
int oldAnalogValue       = 0;
const int BUTTON_EXIT    = 950;
const int BUTTON_UP      = 800;
const int BUTTON_DOWN    = 640;
const int BUTTON_ENTER   = 500;

// ANALOG INPUT
#define ANALOG_NOISE     200

///////////////////////////////////////////////////////////////////////////////////////

void setup(void) {
  // Serial
  Serial.begin(115200);
  
  // LCD
  tft.initR(INITR_BLACKTAB);   
  tft.fillScreen(ST7735_BLACK);
  
  // WIFI  
  // Set WiFi to station mode and disconnect from an AP if it was previously connected
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  resetWifiProbeRSSIArray();
  resetFFWifiProbeRSSIArray();

}

///////////////////////////////////////////////////////////////////////////////////////

void loop(){
    
  switch(MODE){
   case MODE_START_SCREEN    : loopModeStartScreen();     break; 
   case MODE_MAIN_MENU       : loopModeMainMenuScreen();  break; 
   case MODE_WIFI_SCAN       : loopModeWifiScan();        break;
   case MODE_WIFI_PROBE      : loopModeWifiProbe();       break;
   case MODE_FF_CLIENT_PROBE : loopModeFFClientProbe();   break;
   case MODE_FF_MESH_PROBE   : loopModeFFMeshProbe();     break;
   case MODE_FF_ROUTER_CHECK : loopModeFFRouterCheck();   break;
  }
  
  int analogValue = analogRead(A0);
  if(analogValue < ANALOG_NOISE){
    analogValue = 0;
  } else {
    // some value above analog noise detected
    delay(50); // wait 50ms for debounce
    analogValue = analogRead(A0); // and read again
    if(analogValue < ANALOG_NOISE){
      analogValue = 0;
    }
  }
  if(analogValue != oldAnalogValue){
    Serial.print("Analog Value: ");
    Serial.println(analogValue);
  }
  if(oldAnalogValue == 0 && analogValue > 0){
    if(analogValue >= BUTTON_EXIT){
      Serial.println("EXIT Button pressed!");
      buttonPressed(BUTTON_EXIT);
    } else if(analogValue >= BUTTON_UP){
      Serial.println("UP Button pressed!");
      buttonPressed(BUTTON_UP);
    } else if(analogValue >= BUTTON_DOWN){
      Serial.println("DOWN Button pressed!");
      buttonPressed(BUTTON_DOWN);
    } else if(analogValue >= BUTTON_ENTER){
      Serial.println("ENTER Button pressed!");
      buttonPressed(BUTTON_ENTER);
    }
  }
  oldAnalogValue = analogValue;
} 

///////////////////////////////////////////////////////////////////////////////////////

void loopModeFFClientProbe(){
  wifiProbeSSID = FF_SSID;
  loopModeWifiProbe();
}

///////////////////////////////////////////////////////////////////////////////////////

void resetModeFFClientProbe(){
  resetModeWifiProbe();
}

///////////////////////////////////////////////////////////////////////////////////////

void resetModeWifiProbe(){
  wifiProbeSSID         = FF_SSID;
  wifiProbeCount        = 0;
  wifiProbeBestRSSI     = 0;
  wifiProbeBestQuality  = 0;
  wifiProbeEncryption   = "?";
  for(int i = 0; i < WIFIPROBE_RA_SIZE; i++){
    wifiProbeRSSIArray[i] = 0;
  }
  wifiProbeRSSIArrayNextValuePointer  = 0;
  refreshScreen = true;
}

///////////////////////////////////////////////////////////////////////////////////////

  void resetModeFFMeshProbe(){
    resetModeFFClientProbe();
    ffWifiProbeCount        = 0;
    ffWifiProbeBestRSSI     = 0;
    ffWifiProbeBestQuality  = 0;
    for(int i = 0; i < WIFIPROBE_RA_SIZE; i++){
      ffWifiProbeRSSIArray[i] = 0;
    }
    ffWifiProbeRSSIArrayNextValuePointer    = 0;
    refreshScreen = true;
  }
  
///////////////////////////////////////////////////////////////////////////////////////

  void restModeFFRouterCheck(){
    ffWifiLocalIP            = IPAddress();
    ffWifiGatewayIP          = IPAddress();
    ffWifiSubnetMask         = IPAddress();
    ffWifiClient             = WiFiClient();
    ffRouterName             = "";
    ffRouterModel            = "";
    ffRouterFirmwareVersion  = "";
    ffRouterRuntime          = "";
    ffRouterCPUUtilization   = "";
    ffRouterRAMUtilization   = "";
    ffRouterMAC              = "";
    ffRouterIP6              = "";
    ffRouterContactPerson    = "";
    refreshScreen = true;
  }

///////////////////////////////////////////////////////////////////////////////////////

void loopModeMainMenuScreen(){
  if(refreshScreen){
    tft.fillScreen(ST7735_WHITE);
    tft.setTextColor(ST7735_BLACK);
    tft.setTextSize(2);
    tft.setCursor(11, 6);
    tft.print("WiFi SCAN");
    
    tft.drawLine(10, 25, 118, 25, ST7735_BLACK);
    tft.drawLine(10, 25, 10, 145, ST7735_BLACK);
    tft.drawLine(118, 25, 118, 145, ST7735_BLACK);
    tft.drawLine(10, 145, 118, 145, ST7735_BLACK);
    
    tft.setTextSize(1);
    tft.setCursor(14, 30);
    tft.print("WiFi Scanner");
    
    tft.setCursor(14, 42);
    tft.print("FF Client Netz");
    
    tft.setCursor(14, 54);
    tft.print("FF Mesh Netz");
    
    tft.setCursor(14, 66);
    tft.print("FF Router");
    
    tft.setTextSize(1);
    tft.setCursor(2, 150);
    tft.print("v1.0 (by S. Lang)");
    
    refreshScreen = false;
  }
  
  if(refreshModeCursor){
    tft.fillRect(0, 25, 10, 60, ST7735_WHITE);
    switch(modeCursor){
      case MODE_WIFI_SCAN       : tft.fillRect(1, 28, 8, 12, ST7735_RED);  break;
      case MODE_FF_CLIENT_PROBE : tft.fillRect(1, 40, 8, 12, ST7735_RED);  break;
      case MODE_FF_MESH_PROBE   : tft.fillRect(1, 52, 8, 12, ST7735_RED);  break;
      case MODE_FF_ROUTER_CHECK : tft.fillRect(1, 64, 8, 12, ST7735_RED);  break;
    }
    refreshModeCursor = false;
  }
}

///////////////////////////////////////////////////////////////////////////////////////

void buttonENTERPressed_MODE_MAIN_MENU(){
  switch(modeCursor){
    case 0                     :  /* do nothing */            break;
    case MODE_WIFI_SCAN        : MODE = MODE_WIFI_SCAN;       break;
    case MODE_FF_CLIENT_PROBE  : MODE = MODE_FF_CLIENT_PROBE; break;
    case MODE_FF_MESH_PROBE    : MODE = MODE_FF_MESH_PROBE;   break;
    case MODE_FF_ROUTER_CHECK  : MODE = MODE_FF_ROUTER_CHECK; break;
  }
}

///////////////////////////////////////////////////////////////////////////////////////

void buttonUPPressed_MODE_MAIN_MENU(){
  switch(modeCursor){
    case 0                     : modeCursor = MODE_WIFI_SCAN;         break;
    case MODE_WIFI_SCAN        : modeCursor = MODE_FF_ROUTER_CHECK;   break;
    case MODE_FF_CLIENT_PROBE  : modeCursor = MODE_WIFI_SCAN;         break;
    case MODE_FF_MESH_PROBE    : modeCursor = MODE_FF_CLIENT_PROBE;   break;
    case MODE_FF_ROUTER_CHECK  : modeCursor = MODE_FF_MESH_PROBE;     break;
  }
  refreshModeCursor = true;
}

///////////////////////////////////////////////////////////////////////////////////////

void buttonDOWNPressed_MODE_MAIN_MENU(){
  switch(modeCursor){
    case 0                     : modeCursor = MODE_WIFI_SCAN;         break;
    case MODE_WIFI_SCAN        : modeCursor = MODE_FF_CLIENT_PROBE;   break;
    case MODE_FF_CLIENT_PROBE  : modeCursor = MODE_FF_MESH_PROBE;     break;
    case MODE_FF_MESH_PROBE    : modeCursor = MODE_FF_ROUTER_CHECK;   break;
    case MODE_FF_ROUTER_CHECK  : modeCursor = MODE_WIFI_SCAN;         break;
  }
  refreshModeCursor = true;
}

///////////////////////////////////////////////////////////////////////////////////////

void buttonPressed(int button){
  switch(button){
    case BUTTON_EXIT:  buttonEXITPressed();   break;
    case BUTTON_UP:    buttonUPPressed();     break;
    case BUTTON_DOWN:  buttonDOWNPressed();   break;
    case BUTTON_ENTER: buttonENTERPressed();  break;
  }
}

///////////////////////////////////////////////////////////////////////////////////////

void buttonEXITPressed(){
  switch(MODE){
   case MODE_START_SCREEN    : /* do nothing */  break; 
   case MODE_MAIN_MENU       : /* do nothing */  break; 
   case MODE_WIFI_SCAN       : MODE = MODE_MAIN_MENU; resetModeWifiScan(); break;
   case MODE_WIFI_PROBE      : /* ToDo */    break;
   case MODE_FF_CLIENT_PROBE : MODE = MODE_MAIN_MENU; resetModeFFClientProbe(); break;
   case MODE_FF_MESH_PROBE   : MODE = MODE_MAIN_MENU; resetModeFFMeshProbe(); break;
   case MODE_FF_ROUTER_CHECK : MODE = MODE_MAIN_MENU; restModeFFRouterCheck(); break;
  }
}

///////////////////////////////////////////////////////////////////////////////////////

void buttonUPPressed(){
  switch(MODE){
   case MODE_START_SCREEN    : /* do nothing */  break; 
   case MODE_MAIN_MENU       : buttonUPPressed_MODE_MAIN_MENU();  break; 
   case MODE_WIFI_SCAN       : /* ToDo */    break;
   case MODE_WIFI_PROBE      : /* ToDo */    break;
   case MODE_FF_CLIENT_PROBE : /* ToDo */    break;
   case MODE_FF_MESH_PROBE   : /* ToDo */    break;
   case MODE_FF_ROUTER_CHECK : /* ToDo */    break;
  }
}

///////////////////////////////////////////////////////////////////////////////////////

void buttonDOWNPressed(){
  switch(MODE){
   case MODE_START_SCREEN    : /* do nothing */  break; 
   case MODE_MAIN_MENU       : buttonDOWNPressed_MODE_MAIN_MENU();  break; 
   case MODE_WIFI_SCAN       : /* ToDo */    break;
   case MODE_WIFI_PROBE      : /* ToDo */    break;
   case MODE_FF_CLIENT_PROBE : /* ToDo */    break;
   case MODE_FF_MESH_PROBE   : /* ToDo */    break;
   case MODE_FF_ROUTER_CHECK : /* ToDo */    break;
  }
}

///////////////////////////////////////////////////////////////////////////////////////

void buttonENTERPressed(){
  switch(MODE){
   case MODE_START_SCREEN    : /* do nothing */  break; 
   case MODE_MAIN_MENU       : buttonENTERPressed_MODE_MAIN_MENU();  break; 
   case MODE_WIFI_SCAN       : /* ToDo */    break;
   case MODE_WIFI_PROBE      : /* ToDo */    break;
   case MODE_FF_CLIENT_PROBE : /* ToDo */    break;
   case MODE_FF_MESH_PROBE   : /* ToDo */    break;
   case MODE_FF_ROUTER_CHECK : /* ToDo */    break;
  }
}

///////////////////////////////////////////////////////////////////////////////////////

void loopModeFFRouterCheck(){
  
  if(WiFi.status() != WL_CONNECTED){
    ffWifiLocalIP;
    ffWifiGatewayIP;
    ffWifiSubnetMask;
    Serial.print("Connecting to ");
    Serial.println(FF_SSID);
    WiFi.begin(FF_SSID, FF_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");  
    ffWifiLocalIP       = WiFi.localIP();
    ffWifiGatewayIP      = WiFi.gatewayIP();
    ffWifiSubnetMask     = WiFi.subnetMask();
    Serial.print("Local IP address: ");
    Serial.println(ffWifiLocalIP);
    Serial.print("Gateway IP address: ");
    Serial.println(ffWifiGatewayIP);
    Serial.print("Subnet Mask: ");
    Serial.println(ffWifiSubnetMask);
  }
  
  Serial.print("connecting to ");
  Serial.println(FF_HOST);
  
  // Use WiFiClient class to create TCP connections

  if (!ffWifiClient.connect(FF_HOST, ffWifiHttpPort)) {
    Serial.println("connection failed");
  } else {
  
    // We now create a URI for the request
    String url = "/cgi-bin/status";
    
    Serial.print("Requesting URL: ");
    Serial.println(url);
    
    // This will send the request to the server
    ffWifiClient.print(String("GET ") + url + " HTTP/1.1\r\n" +
                 "Host: " + FF_HOST + "\r\n" + 
                 "Connection: close\r\n\r\n");
    delay(1000);
    
    // Read all the lines of the reply from server and print them to Serial
    while(ffWifiClient.available()){
      String line = ffWifiClient.readStringUntil('\n');
      parseFFRouterData(line);
      //Serial.print(line);
    }
    Serial.println();
    Serial.println("closing connection");
  }
  
  tft.fillScreen(ST7735_WHITE);
  tft.setTextWrap(false);
  //tft.fillRect(0, 0, 128, 12, ST7735_BLACK);
  //tft.setTextColor(ST7735_WHITE);
  //tft.setTextSize(1);
  //tft.setCursor(10, 2);
  //tft.print(FF_SSID);
  
  tft.drawLine(21, 0, 21, 160, ST7735_BLACK);
  
  
  tft.setTextColor(ST7735_BLACK);
  tft.setTextSize(1);
  
  const int x1 = 2;
  const int x2 = 24;
  int y = 0;
  const int textOffset = 2;
  const int lineOffset = 10;
  
  tft.setCursor(x1, y+textOffset);
  tft.print("IP");
  tft.setCursor(x2, y+textOffset);
  tft.print(ffWifiLocalIP);
  
  y+=lineOffset;
  
  tft.drawLine(0, y, 128, y, ST7735_BLACK);
  tft.setCursor(x1, y+textOffset);
  tft.print("Sub");
  tft.setCursor(x2, y+textOffset);
  tft.print(ffWifiSubnetMask);
  
  y+=lineOffset;
  
  tft.drawLine(0, y, 128, y, ST7735_BLACK);
  tft.setCursor(x1, y+textOffset);
  tft.print("GW");
  tft.setCursor(x2, y+textOffset);
  tft.print(ffWifiGatewayIP);
  
  y+=lineOffset;
  
  tft.drawLine(0, y, 128, y, ST7735_BLACK);
  tft.setCursor(x1, y+textOffset);
  tft.print("R/Q");
  tft.setCursor(x2, y+textOffset);
  int rssi = WiFi.RSSI();
  tft.print(rssi);
  tft.print("dBm  ");
  int quality = convertRSSI2Quality(rssi);
  tft.print(quality);
  tft.print("%");
  
  y+=lineOffset;
  
  tft.drawLine(0, y, 128, y, ST7735_BLACK);
  tft.setCursor(x1, y+textOffset);
  tft.print("Nam");
  tft.setCursor(x2, y+textOffset);
  if(ffRouterName.length() < 17){
    tft.print(ffRouterName);
  } else {
    String n1 = ffRouterName.substring(0,17);
    String n2 = ffRouterName.substring(17,ffRouterName.length());
    tft.print(n1);
    y+=9;
    tft.setCursor(x2, y+textOffset);
    tft.print(n2);
  }
  
  y+=lineOffset;
 
  tft.drawLine(0, y, 128, y, ST7735_BLACK);
  tft.setCursor(x1, y+textOffset);
  tft.print("HW");
  tft.setCursor(x2, y+textOffset);
  String hw = ffRouterModel;
  hw += " (";
  hw += ffRouterFirmwareVersion;
  hw += ")";
  if(hw.length() < 17){
    tft.print(hw);
  } else {
    String n1 = hw.substring(0,17);
    String n2 = hw.substring(17,hw.length());
    tft.print(n1);
    y+=9;
    tft.setCursor(x2, y+textOffset);
    tft.print(n2);
  }
  
  //y+=lineOffset;
  
  //tft.drawLine(0, y, 128, y, ST7735_BLACK);
  //tft.setCursor(x1, y+textOffset);
  //tft.print("SW");
  //tft.setCursor(x2, y+textOffset);
  //tft.print(ffRouterFirmwareVersion);
  
  y+=lineOffset;
  
  tft.drawLine(0, y, 128, y, ST7735_BLACK);
  tft.setCursor(x1, y+textOffset);
  tft.print("Run");
  tft.setCursor(x2, y+textOffset);
  ffRouterRuntime.replace("Stunden","h");
  tft.print(ffRouterRuntime);
  
  y+=lineOffset;
  
  tft.drawLine(0, y, 128, y, ST7735_BLACK);
  tft.setCursor(x1, y+textOffset);
  tft.print("Utl");
  tft.setCursor(x2, y+textOffset);
  tft.print("CPU:");
  tft.print(ffRouterCPUUtilization);
  tft.print(" RAM:");
  tft.print(ffRouterRAMUtilization);
  
  y+=lineOffset;
  
  tft.drawLine(0, y, 128, y, ST7735_BLACK);
  tft.setCursor(x1, y+textOffset);
  tft.print("MAC");
  tft.setCursor(x2, y+textOffset);
  tft.print(ffRouterMAC);
  
  y+=lineOffset;
  
  tft.drawLine(0, y, 128, y, ST7735_BLACK);
  tft.setCursor(x1, y+textOffset);
  tft.print("IP6");
  tft.setCursor(x2, y+textOffset);
  if(ffRouterIP6.length() < 17){
    tft.print(ffRouterIP6);
  } else {
    String n1 = ffRouterIP6.substring(0,17);
    String n2 = ffRouterIP6.substring(17,ffRouterIP6.length());
    tft.print(n1);
    y+=9;
    tft.setCursor(x2, y+textOffset);
    if(n2.length() < 17){
      tft.print(n2);
    } else {
      String n21 = n2.substring(0,17);
      String n22 = n2.substring(17,n2.length());
      tft.print(n21);
      y+=9;
      tft.setCursor(x2, y+textOffset);
      tft.print(n22);
    }
  }
  
  y+=lineOffset;
  
  tft.drawLine(0, y, 128, y, ST7735_BLACK);
  tft.setCursor(x1, y+textOffset);
  tft.print("Inf");
  tft.setCursor(x2, y+textOffset);
  if(ffRouterContactPerson.length() < 17){
    tft.print(ffRouterContactPerson);
  } else {
    String n1 = ffRouterContactPerson.substring(0,17);
    String n2 = ffRouterContactPerson.substring(17,ffRouterContactPerson.length());
    tft.print(n1);
    y+=9;
    tft.setCursor(x2, y+textOffset);
    tft.print(n2);
  }
}

///////////////////////////////////////////////////////////////////////////////////////

/*
<html><head><title>54309-Freifunk-Besslich-02</title>
<tr><th>Modell: </th><td>TP-Link TL-WR1043N/ND v1</td></tr>
<tr><th>Firmware Version: </th><td>0.7.3</td></tr>
<tr><th>Laufzeit:</th><td>75.48 Stunden</td></tr>
<tr><th>CPU Auslastung:</th><td>85%</td></tr>
<tr><th>RAM Auslastung:</th><td>83%</td></tr>
<tr><th>MAC Adresse</th><td>74:ea:3a:e4:d1:da</td></tr>
<tr><th>IPv6 Adresse</th><td>2001:bf7:fc0f:0:76ea:3aff:fee4:d1da</td></tr>
<div class="frame"><h2>VPN</h2><div><table>fastd not running</table></div>
<table><tr><th>Ansprechpartner:</th><td>freifunk@lang-it.net</td></tr>

String      ffRouterName;
String      ffRouterModel;
String      ffRouterFirmwareVersion;
String      ffRouterRuntime;
String      ffRouterCPUUtilization;
String      ffRouterRAMUtilization;
String      ffRouterMAC;
String      ffRouterIP6;
String      ffRouterContactPerson;

*/
void parseFFRouterData(String s){
  if(s != ""){
    String v1 = getValueFromString(s, "<head><title>");
    if(v1 != ""){
      ffRouterName = v1;
    }
    
    String v2 = getValueFromString(s, "Firmware Version: </th><td>");
    if(v2 != ""){
      ffRouterFirmwareVersion = v2;
    }
    
    String v3 = getValueFromString(s, "Laufzeit:</th><td>");
    if(v3 != ""){
      ffRouterRuntime = v3;
    }
    
    String v4 = getValueFromString(s, "CPU Auslastung:</th><td>");
    if(v4 != ""){
      ffRouterCPUUtilization =  v4;
    }
    
    String v5 = getValueFromString(s, "RAM Auslastung:</th><td>");
    if(v5 != ""){
      ffRouterRAMUtilization = v5;
    } 
      
    String v6 = getValueFromString(s, "MAC Adresse</th><td>");
    if(v6 != ""){
      ffRouterMAC = v6;
    }
    String v7 = getValueFromString(s, "IPv6 Adresse</th><td>");
    if(v7 != ""){
      ffRouterIP6 = v7;
    }
    
    String v8 = getValueFromString(s, "Ansprechpartner:</th><td>");
    if(v8 != ""){
      ffRouterContactPerson = v8;
    }
    
    String v9 = getValueFromString(s, "Modell: </th><td>");
    if(v9 != ""){
      ffRouterModel = v9;
    }
  }
}

///////////////////////////////////////////////////////////////////////////////////////

String getValueFromString(String s, String key){
  int iStart   = s.indexOf(key);
  if(iStart < 0){
    return "";
  }
  iStart = iStart + key.length();
  int iEnd     = s.indexOf("<", iStart); 
  String value = s.substring(iStart, iEnd);
  return value;
}

///////////////////////////////////////////////////////////////////////////////////////

void loopModeStartScreen(){
  tft.fillScreen(ST7735_WHITE);
  
  tft.fillCircle(64, 110, 80, ST7735_RED);    
  tft.fillCircle(64, 110, 68, ST7735_WHITE);  
  tft.fillCircle(64, 110, 60, ST7735_RED);    
  tft.fillCircle(64, 110, 48, ST7735_WHITE);  
  tft.fillCircle(64, 110, 40, ST7735_RED);    
  tft.fillCircle(64, 110, 28, ST7735_WHITE);  
  
  tft.fillTriangle(0 , 50, 0, 160, 128, 160, ST7735_WHITE);
  tft.fillTriangle(128 , 50, 0, 160, 128, 160, ST7735_WHITE);
  
  tft.fillCircle(64, 110, 20, ST7735_RED);
  tft.fillCircle(64, 110, 17, ST7735_WHITE);
  tft.drawCircle(64, 110, 14, ST7735_RED);
  
  tft.drawCircle(43, 117, 13, ST7735_RED);
  
  tft.setTextColor(ST7735_BLACK);
  tft.setTextSize(2);
  tft.setCursor(11, 6);
  tft.print("WiFi SCAN");
  
  tft.setTextSize(2);
  tft.setCursor(17, 140);
  tft.print("Freifunk");
  
  delay(START_SCREEN_DELAY);
  MODE = MODE_MAIN_MENU;
}


///////////////////////////////////////////////////////////////////////////////////////

void loopModeWifiScan(){  
    
  int n = WiFi.scanNetworks();
  
  tft.fillScreen(ST7735_WHITE);
  tft.setTextWrap(false);
  tft.setTextColor(ST7735_BLACK);
  tft.setTextSize(2);
  tft.setCursor(11, 2);
  tft.print("WiFi SCAN");
  tft.drawLine(0, 18, 128, 18, ST7735_BLACK);
  tft.drawLine(0, 19, 128, 19, ST7735_BLACK);
    
  if (n > 0){
    int line = 20; // line
    tft.setTextSize(1);
    for (int i = 0; i < n; ++i){
      String   ssid       = WiFi.SSID(i);
      int      rssi       = WiFi.RSSI(i);
      int      quality    = convertRSSI2Quality(rssi);
      String   encryption = convertEncryptionType2String(WiFi.encryptionType(i));
      uint16_t color      = convertRSSI2Color(rssi);
     
      tft.fillRect(0, line, 18, line + 10, color);
      tft.fillRect(0, line + 10, 18, 160, ST7735_WHITE);
      tft.setCursor(0, line + 1);
      if(quality < 10){
        tft.print("  ");
      } else if(quality < 100){
        tft.print(" ");
      }
      tft.print(quality);
      tft.setCursor(20, line + 1);
      tft.print(ssid);
      line += 10;
    }
    
    tft.drawLine(18, 19, 18, 160, ST7735_BLACK);
  } 
}

///////////////////////////////////////////////////////////////////////////////////////
  
void resetModeWifiScan(){
  refreshScreen = true;
}

///////////////////////////////////////////////////////////////////////////////////////
   
void loopModeFFMeshProbe(){
  
  probeFFWiFi(wifiProbeSSID);
  
  tft.fillScreen(ST7735_WHITE);
  tft.setTextWrap(false);
  tft.setTextColor(ST7735_BLACK);
  tft.setTextSize(2);
  tft.setCursor(5, 2);
  tft.print("FFTR PROBE");
  tft.fillRect(0, 18, 128, 15, ST7735_BLACK);
  tft.setTextColor(ST7735_WHITE);
  tft.setTextSize(1);
  tft.setCursor(10, 22);
  tft.print(wifiProbeSSID);
  tft.setTextColor(ST7735_BLACK);
  tft.setCursor(1, 36);
  tft.print("Client: ");
  tft.print(wifiProbeCount);
  tft.setCursor(66, 36);
  tft.print("Mesh: ");
  tft.print(ffWifiProbeCount);
  tft.drawLine(0, 45, 128, 45, ST7735_BLACK);
  tft.setCursor(1, 49);
  tft.print(wifiProbeBestQuality);
  tft.print("% ");
  tft.print(wifiProbeBestRSSI);
  tft.print("dB");
  tft.setCursor(66, 49);
  tft.print(ffWifiProbeBestQuality);
  tft.print("% ");
  tft.print(ffWifiProbeBestRSSI);
  tft.print("dB");
  tft.drawLine(64, 33, 64, 59, ST7735_BLACK);
  tft.drawLine(0, 58, 128, 58, ST7735_BLACK);
  tft.drawLine(0, 59, 128, 59, ST7735_BLACK);
  
  //uint16_t color = convertRSSI2Color(ffWifiProbeBestRSSI);
  //tft.fillRect(0, 160 - ffWifiProbeBestQuality, 25, 160 , color);
  //if(ffWifiProbeBestQuality >= 50){
  //  tft.setCursor(1, (160 - ffWifiProbeBestQuality + 2));
  //} else {
  //  tft.setCursor(1, (160 - ffWifiProbeBestQuality - 9));
  //}
  //tft.print(ffWifiProbeBestQuality);
  //tft.print("%");
 
  addRSSI2FFWifiProbeRSSIArray(ffWifiProbeBestRSSI);
  
  tft.fillRect(0, 60, 128, 160, ST7735_BLACK);

  for(int i = 0; i < FF_WIFIPROBE_RA_SIZE; i++){
    int      r = getRSSIFromFFWifiProbeRSSIArray(i);
    if(r < 0){
      int      q = convertRSSI2Quality(r);
      uint16_t c = convertRSSI2Color(r);
      tft.drawLine(128 - i, 160, 128 - i, 160 - q, c);

    } 
  }
  
  tft.drawLine(0, 160 - convertRSSI2Quality(-49), 128, 160 - convertRSSI2Quality(-49) , ST7735_GREEN);
  tft.drawLine(0, 160 - convertRSSI2Quality(-70), 128, 160 - convertRSSI2Quality(-70) , ST7735_YELLOW);
  
  //int averageRSSI = getAverageRSSIFromWifiProbeRSSIArray();
  //if(averageRSSI < 0){
  //  int averageQuality = convertRSSI2Quality(averageRSSI);
  //  tft.setTextColor(ST7735_WHITE);
  //  tft.setCursor(60, 60);
  //  tft.print(averageQuality);
  //  tft.print(" %");
  //}
  
}

///////////////////////////////////////////////////////////////////////////////////////


void loopModeWifiProbe(){
  
  probeWiFi(wifiProbeSSID);
  
  tft.fillScreen(ST7735_WHITE);
  tft.setTextWrap(false);
  tft.setTextColor(ST7735_BLACK);
  tft.setTextSize(2);
  tft.setCursor(5, 2);
  tft.print("WiFi PROBE");
  tft.fillRect(0, 18, 128, 15, ST7735_BLACK);
  tft.setTextColor(ST7735_WHITE);
  tft.setTextSize(1);
  tft.setCursor(10, 22);
  tft.print(wifiProbeSSID);
  tft.setTextColor(ST7735_BLACK);
  tft.setCursor(1, 36);
  tft.print("Encr: ");
  tft.print(wifiProbeEncryption);
  tft.setCursor(66, 36);
  tft.print("Count: ");
  tft.print(wifiProbeCount);
  tft.drawLine(0, 45, 128, 45, ST7735_BLACK);
  tft.setCursor(1, 49);
  tft.print("Q: ");
  tft.print(wifiProbeBestQuality);
  tft.print(" %");
  tft.setCursor(66, 49);
  tft.print("S: ");
  tft.print(wifiProbeBestRSSI);
  tft.print(" dBm");
  tft.drawLine(64, 33, 64, 59, ST7735_BLACK);
  tft.drawLine(0, 58, 128, 58, ST7735_BLACK);
  tft.drawLine(0, 59, 128, 59, ST7735_BLACK);
  
  //uint16_t color = convertRSSI2Color(wifiProbeBestRSSI);
  //tft.fillRect(0, 160 - wifiProbeBestQuality, 25, 160 , color);
  //tft.drawLine(0, 160 - wifiProbeBestQuality, 25, 160 - wifiProbeBestQuality, ST7735_BLACK);
  //if(wifiProbeBestQuality >= 50){
  //  tft.setCursor(1, (160 - wifiProbeBestQuality + 2));
  //} else {
  //  tft.setCursor(1, (160 - wifiProbeBestQuality - 9));
  //}
  //tft.print(wifiProbeBestQuality);
  //tft.print("%");
 
  addRSSI2WifiProbeRSSIArray(wifiProbeBestRSSI);
  
  //tft.fillRect(25, 60, 128, 160, ST7735_BLACK);
  tft.fillRect(0, 60, 128, 160, ST7735_BLACK);

  for(int i = 0; i < WIFIPROBE_RA_SIZE; i++){
    int      r = getRSSIFromWifiProbeRSSIArray(i);
    if(r < 0){
      int      q = convertRSSI2Quality(r);
      uint16_t c = convertRSSI2Color(r);
      tft.drawLine(128 - i, 160, 128 - i, 160 - q, c);

    } 
  }
  
  
  tft.drawLine(0, 160 - convertRSSI2Quality(-49), 128, 160 - convertRSSI2Quality(-49) , ST7735_GREEN);
  tft.drawLine(0, 160 - convertRSSI2Quality(-70), 128, 160 - convertRSSI2Quality(-70) , ST7735_YELLOW);
  
  //int averageRSSI = getAverageRSSIFromWifiProbeRSSIArray();
  //if(averageRSSI < 0){
  //  int averageQuality = convertRSSI2Quality(averageRSSI);
  //  tft.setTextColor(ST7735_WHITE);
  //  tft.setCursor(60, 60);
  //  tft.print(averageQuality);
  //  tft.print(" %");
  //}
  
}

///////////////////////////////////////////////////////////////////////////////////////

int getAverageRSSIFromWifiProbeRSSIArray(){
  int counter = 0;
  int sum = 0;
  for(int i = 0; i < WIFIPROBE_RA_SIZE; i++){
    int rssi = wifiProbeRSSIArray[i];
    if(rssi < 0){
      counter++;
      sum = sum + rssi;
    }
  }
  if(counter > 0){
    return sum / counter;
  } else {
    return 0;
  }
}

///////////////////////////////////////////////////////////////////////////////////////

int getRSSIFromWifiProbeRSSIArray(int t_minus_x){
  if(t_minus_x < 0 || t_minus_x >= WIFIPROBE_RA_SIZE){
    return 0;
  } else {
    int index = wifiProbeRSSIArrayNextValuePointer - 1 - t_minus_x;
    if(index < 0){
      index = index + WIFIPROBE_RA_SIZE;
    }
    return wifiProbeRSSIArray[index];
  }
}

///////////////////////////////////////////////////////////////////////////////////////

void addRSSI2WifiProbeRSSIArray(int rssi){
  
  wifiProbeRSSIArray[wifiProbeRSSIArrayNextValuePointer] = rssi;
  
  if(wifiProbeRSSIArrayNextValuePointer + 1 < WIFIPROBE_RA_SIZE){
    wifiProbeRSSIArrayNextValuePointer++;
  } else {
    wifiProbeRSSIArrayNextValuePointer = 0;
  }  
}

///////////////////////////////////////////////////////////////////////////////////////

void resetWifiProbeRSSIArray(){
  for(int i = 0; i < WIFIPROBE_RA_SIZE; i++){
    wifiProbeRSSIArray[i] = 0;
  }
  wifiProbeRSSIArrayNextValuePointer = 0;
}

///////////////////////////////////////////////////////////////////////////////////////

int getAverageRSSIFromFFWifiProbeRSSIArray(){
  int counter = 0;
  int sum = 0;
  for(int i = 0; i < FF_WIFIPROBE_RA_SIZE; i++){
    int rssi = ffWifiProbeRSSIArray[i];
    if(rssi < 0){
      counter++;
      sum = sum + rssi;
    }
  }
  if(counter > 0){
    return sum / counter;
  } else {
    return 0;
  }
}

///////////////////////////////////////////////////////////////////////////////////////

int getRSSIFromFFWifiProbeRSSIArray(int t_minus_x){
  if(t_minus_x < 0 || t_minus_x >= FF_WIFIPROBE_RA_SIZE){
    return 0;
  } else {
    int index = ffWifiProbeRSSIArrayNextValuePointer - 1 - t_minus_x;
    if(index < 0){
      index = index + FF_WIFIPROBE_RA_SIZE;
    }
    return ffWifiProbeRSSIArray[index];
  }
}

///////////////////////////////////////////////////////////////////////////////////////

void addRSSI2FFWifiProbeRSSIArray(int rssi){
  
  ffWifiProbeRSSIArray[ffWifiProbeRSSIArrayNextValuePointer] = rssi;
  
  if(ffWifiProbeRSSIArrayNextValuePointer + 1 < FF_WIFIPROBE_RA_SIZE){
    ffWifiProbeRSSIArrayNextValuePointer++;
  } else {
    ffWifiProbeRSSIArrayNextValuePointer = 0;
  }  
}

///////////////////////////////////////////////////////////////////////////////////////

void resetFFWifiProbeRSSIArray(){
  for(int i = 0; i < FF_WIFIPROBE_RA_SIZE; i++){
    ffWifiProbeRSSIArray[i] = 0;
  }
  ffWifiProbeRSSIArrayNextValuePointer = 0;
}

///////////////////////////////////////////////////////////////////////////////////////

uint16_t convertRSSI2Color(int rssi){
 if(rssi > - 50){
   return ST7735_GREEN;
 } else if(rssi <= -50 && rssi > - 70){
   return ST7735_YELLOW;
 } else {
   return ST7735_RED;
 } 
}

///////////////////////////////////////////////////////////////////////////////////////

int convertRSSI2Quality(int rssi){
  float quality = (100.0/(RSSI_MAX - RSSI_MIN)) * ( rssi - RSSI_MIN);
  if(quality > 100){
    return 100;
  } else {
    return (int)quality;
  }
}

///////////////////////////////////////////////////////////////////////////////////////

void probeFFWiFi(String ssid){
  wifiProbeCount = 0;
  wifiProbeBestRSSI = -100;
  wifiProbeBestQuality= 0;
  ffWifiProbeCount = 0;
  ffWifiProbeBestRSSI = -100;
  ffWifiProbeBestQuality= 0;
  int n = WiFi.scanNetworks();
  if (n > 0){
    for (int i = 0; i < n; ++i) {
      String ssid2Test= WiFi.SSID(i);
      //Serial.println(ssid2Test);
      if(ssid.equals(ssid2Test)){
        wifiProbeCount++;
        int rssi          = WiFi.RSSI(i);
        int quality       = convertRSSI2Quality(rssi);
        wifiProbeEncryption = convertEncryptionType2String(WiFi.encryptionType(i));
        if(rssi > wifiProbeBestRSSI){
          wifiProbeBestRSSI = rssi;
          wifiProbeBestQuality= quality;
        }
      } else if(ssid2Test.indexOf(":") > 0){
        ffWifiProbeCount++;
        int ffRssi        = WiFi.RSSI(i);
        int ffQuality     = convertRSSI2Quality(ffRssi);
        if(ffRssi > ffWifiProbeBestRSSI){
          ffWifiProbeBestRSSI = ffRssi;
          ffWifiProbeBestQuality= ffQuality;
        }
      }
    }
  }
  //Serial.println("--------------");
}

///////////////////////////////////////////////////////////////////////////////////////

void probeWiFi(String ssid){
  wifiProbeCount = 0;
  wifiProbeBestRSSI = -100;
  wifiProbeBestQuality= 0;
  int n = WiFi.scanNetworks();
  if (n > 0){
    for (int i = 0; i < n; ++i) {
      String ssid2Test= WiFi.SSID(i);
      //Serial.println(ssid2Test);
      if(ssid.equals(ssid2Test)){
        wifiProbeCount++;
        int rssi          = WiFi.RSSI(i);
        int quality       = convertRSSI2Quality(rssi);
        //Serial.print(rssi);
        //Serial.println( "dBm");
        //Serial.print(quality);
        //Serial.println( "%");
        wifiProbeEncryption = convertEncryptionType2String(WiFi.encryptionType(i));
        if(rssi > wifiProbeBestRSSI){
          wifiProbeBestRSSI = rssi;
          wifiProbeBestQuality= quality;
        }
      }
    }
  }
  //Serial.println("--------------");
}


///////////////////////////////////////////////////////////////////////////////////////

String convertEncryptionType2String(int type) {
  switch (type) {
    case ENC_TYPE_WEP:
      return "WEP";
    case ENC_TYPE_TKIP:
      return "WPA";
    case ENC_TYPE_CCMP:
      return "WPA2";
    case ENC_TYPE_NONE:
      return "None";
    case ENC_TYPE_AUTO:
      return "Auto";
  }
  return "?";
}



