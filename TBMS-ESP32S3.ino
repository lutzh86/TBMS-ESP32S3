
#include "BMSModuleManager.h"
#include <EEPROM.h>
#include <Arduino.h>
#include "driver/gpio.h"
#include "driver/twai.h"
#include "WiFi.h"
#include <WebServer.h>
#include <ArduinoOTA.h>
#include <WiFiUdp.h>


const char *ssid = "KOLOSSUS";
const char *password = "XXX";

IPAddress gateway(192, 168, 178, 1);
IPAddress subnet(255, 255, 255, 0);
WebServer server(80);

size_t EEPROM_SIZE = 512;

#define BMS_BAUD  612500


#define RX2 8
#define TX2 18


BMSModuleManager bms;
EEPROMSettings settings;
uint32_t lastUpdate;



void loadSettings()
{
    EEPROM.get(EEPROM_PAGE, settings);
    
    if (settings.version != EEPROM_VERSION) //if settings are not the current version then erase them and set defaults
    {
        Serial.println("Resetting to factory defaults");
        
        settings.version = EEPROM_VERSION;
        settings.checksum = 0;
        settings.canSpeed = 500000;
        settings.batteryID = 0x01; //in the future should be 0xFF to force it to ask for an address
        settings.OverVSetpoint = 4.1f;
        settings.UnderVSetpoint = 3.4f;
        settings.OverTSetpoint = 65.0f;
        settings.UnderTSetpoint = -10.0f;
        settings.balanceVoltage = 3.5f;
        settings.balanceHyst = 0.03f;
        settings.logLevel = 2;
        //EEPROM.write(EEPROM_PAGE, settings);
        EEPROM.put(EEPROM_PAGE, settings); 
        EEPROM.commit();
    }
    else {
        Serial.println("Using stored values from EEPROM");
    }

}


void handle_OnConnect() {
  server.send(200, "text/html", bms.htmlPackDetails());
}


void setup() 
{
     delay(4000);  //just for easy debugging. It takes a few seconds for USB to come up properly on most OS's

  WiFi.begin(ssid, password);
  server.on("/", handle_OnConnect);
  server.begin();
    
    Serial.begin(115200);
    Serial.println("Starting up!");

    SERIAL.begin(BMS_BAUD, SERIAL_8N1, RX2, TX2);

    Serial.println("Started serial interface to BMS.");

    EEPROM.begin(EEPROM_SIZE);


    loadSettings();
        
    

    bms.renumberBoardIDs();

 
    lastUpdate = 0;

    bms.clearFaults();


    
  ArduinoOTA
      .onStart([]() {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH)
          type = "sketch";
        else // U_SPIFFS
          type = "filesystem";

        // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS
        // using SPIFFS.end()
        Serial.println("Start updating " + type);
      })
      .onEnd([]() { Serial.println("\nEnd"); })
      .onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
      })
      .onError([](ota_error_t error) {
        Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR)
          Serial.println("Auth Failed");
        else if (error == OTA_BEGIN_ERROR)
          Serial.println("Begin Failed");
        else if (error == OTA_CONNECT_ERROR)
          Serial.println("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR)
          Serial.println("Receive Failed");
        else if (error == OTA_END_ERROR)
          Serial.println("End Failed");
      });

  ArduinoOTA.begin();
  

}

void loop() 
{
    

  server.handleClient();
  ArduinoOTA.handle();
    

    if (millis() > (lastUpdate + 1000))
    {    
        lastUpdate = millis();
        bms.balanceCells();
        
        bms.getAllVoltTemp();
        
    bms.printPackDetails();
    }


    
}
