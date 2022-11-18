#include <Esp.h>

#include "Logger.h"
#include "SerialConsole.h"
#include "BMSModuleManager.h"
#include "SystemIO.h"
#include "esp32_can.h"


#include <EEPROM.h>

size_t EEPROM_SIZE = 512;

#define BMS_BAUD  612500


#define RX2 8
#define TX2 18


BMSModuleManager bms;
EEPROMSettings settings;
SerialConsole console;
uint32_t lastUpdate;



void loadSettings()
{
    EEPROM.get(EEPROM_PAGE, settings);
    
    if (settings.version != EEPROM_VERSION) //if settings are not the current version then erase them and set defaults
    {
        Logger::console("Resetting to factory defaults");
        settings.version = EEPROM_VERSION;
        settings.checksum = 0;
        settings.canSpeed = 500000;
        settings.batteryID = 0x01; //in the future should be 0xFF to force it to ask for an address
        settings.OverVSetpoint = 4.1f;
        settings.UnderVSetpoint = 2.3f;
        settings.OverTSetpoint = 65.0f;
        settings.UnderTSetpoint = -10.0f;
        settings.balanceVoltage = 3.95f;
        settings.balanceHyst = 0.02f;
        settings.logLevel = 2;
        //EEPROM.write(EEPROM_PAGE, settings);
        EEPROM.put(EEPROM_PAGE, settings); 
        EEPROM.commit();
    }
    else {
        Logger::console("Using stored values from EEPROM");
    }
        
    Logger::setLoglevel((Logger::LogLevel)settings.logLevel);
}

void initializeCAN()
{
    uint32_t id;
    Can0.begin(settings.canSpeed);
    if (settings.batteryID < 0xF)
    {
        //Setup filter for direct access to our registered battery ID
        id = (0xBAul << 20) + (((uint32_t)settings.batteryID & 0xF) << 16);
        Can0.setRXFilter(0, id, 0x1FFF0000ul, true);
        //Setup filter for request for all batteries to give summary data
        id = (0xBAul << 20) + (0xFul << 16);
        Can0.setRXFilter(1, id, 0x1FFF0000ul, true);
    }
}

void setup() 
{
    

    Serial0.begin(115200);
    Serial0.println("Starting up!");

    Serial2.begin(BMS_BAUD, SERIAL_8N1, RX2, TX2);

    Serial0.println("Started serial interface to BMS.");

    EEPROM.begin(EEPROM_SIZE);


    loadSettings();
    initializeCAN();
    
    

    bms.renumberBoardIDs();

    // Logger::setLoglevel(Logger::Debug);

    lastUpdate = 0;

    bms.clearFaults();
}

void loop() 
{
    CAN_FRAME incoming;

    console.loop();

    if (millis() > (lastUpdate + 1000))
    {    
        lastUpdate = millis();
        bms.balanceCells();
        bms.getAllVoltTemp();
    }


    /* if (Can0.available()) {
        Can0.read(incoming);
        bms.processCANMsg(incoming);
    } */
}
