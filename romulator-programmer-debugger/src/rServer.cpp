#include "rServer.h"
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <Arduino.h>
#include <EEPROM.h>
#include <LittleFS.h>
#include "libRomulatorProgrammer.h"
#include "libRomulatorDebug.h"
#include "defines.h"
#include <ESP8266HTTPClient.h>

const char *ssid = "romulator";
const char *password = "bitfixer";

bool AP;
File fsUploadFile;
ESP8266WebServer server(80);
extern RomulatorProgrammer _programmer;
uint8_t _vram[2048];
uint8_t _led;

int _lastMillis;
extern void programFirmware();

WiFiSettings user_wifi;

void handlePortal() {
    if (AP) 
    {
        if (server.method() == HTTP_POST) {
            strncpy(user_wifi.ssid,     server.arg("ssid").c_str(),     sizeof(user_wifi.ssid) );
            strncpy(user_wifi.password, server.arg("password").c_str(), sizeof(user_wifi.password) );
            user_wifi.ssid[server.arg("ssid").length()] = user_wifi.password[server.arg("password").length()] = '\0';
            EEPROM.put(0, user_wifi);
            EEPROM.commit();
            server.send(200,   "text/html",  "<!doctype html><html lang='en'><head><meta charset='utf-8'><meta name='viewport' content='width=device-width, initial-scale=1'><title>Wifi Setup</title><style>*,::after,::before{box-sizing:border-box;}body{margin:0;font-family:'Segoe UI',Roboto,'Helvetica Neue',Arial,'Noto Sans','Liberation Sans';font-size:1rem;font-weight:400;line-height:1.5;color:#212529;background-color:#f5f5f5;}.form-control{display:block;width:100%;height:calc(1.5em + .75rem + 2px);border:1px solid #ced4da;}button{border:1px solid transparent;color:#fff;background-color:#007bff;border-color:#007bff;padding:.5rem 1rem;font-size:1.25rem;line-height:1.5;border-radius:.3rem;width:100%}.form-signin{width:100%;max-width:400px;padding:15px;margin:auto;}h1,p{text-align: center}</style> </head> <body><main class='form-signin'> <h1>Wifi Setup</h1> <br/> <p>Your settings have been saved successfully!<br />Please restart the device.</p></main></body></html>" );
        } else {
            File fp = LittleFS.open("/wifi.html", "r");
            server.send(200, "text/html", &fp);
        }
    }
    else
    {
        Serial.printf("request.\r\n");
        File fp = LittleFS.open("/romulator.html", "r");
        server.send(200, "text/html", &fp);
        fp.close();
        Serial.printf("done sending\r\n");
    }
}

void handleFileUpload()
{
    HTTPUpload& upload = server.upload();
    if(upload.status == UPLOAD_FILE_START)
    {
        String filename = upload.filename;
        Serial.printf("upload filename: %s\n", filename.c_str());
        if (!filename.startsWith("/")) 
        {
            filename = "/"+filename;
        }
        Serial.print("handleFileUpload Name: "); Serial.println(filename);
        fsUploadFile = LittleFS.open(filename, "w");            // Open the file for writing in SPIFFS (create if it doesn't exist)
        filename = String();
    } 
    else if (upload.status == UPLOAD_FILE_WRITE)
    {
        if(fsUploadFile)
        {
            fsUploadFile.write(upload.buf, upload.currentSize); // Write the received bytes to the file
        }
    } 
    else if(upload.status == UPLOAD_FILE_END) 
    {
        if(fsUploadFile) 
        {                                    // If the file was successfully created
            fsUploadFile.close();                               // Close the file again
            Serial.print("handleFileUpload Size: "); Serial.println(upload.totalSize);
            // start programming from uploaded file
            _programmer.beginProgrammingFromFile((char*)upload.filename.c_str());
            server.send(200, "text/html", "done uploading!");
        } 
        else 
        {
            server.send(500, "text/plain", "500: couldn't create file");
        }
    }
}

void handleWriteMemory()
{
    Serial.printf("handleWriteMemory\n");
    HTTPUpload& upload = server.upload();
    if(upload.status == UPLOAD_FILE_START)
    {
        Serial.printf("start memory upload.\n");
        fsUploadFile = LittleFS.open("/memory.bin", "w");
    } 
    else if (upload.status == UPLOAD_FILE_WRITE)
    {
        if(fsUploadFile)
        {
            fsUploadFile.write(upload.buf, upload.currentSize); // Write the received bytes to the file
        }
    } 
    else if(upload.status == UPLOAD_FILE_END) 
    {
        if(fsUploadFile) 
        {                                    // If the file was successfully created
            fsUploadFile.close();                               // Close the file again
            Serial.print("handleFileUpload Size: "); Serial.println(upload.totalSize);
            // start programming from uploaded file
            romulatorWriteMemoryFromFile();
            server.send(200, "text/html", "done writing!");
        } 
        else 
        {
            server.send(500, "text/plain", "500: couldn't create file");
        }
    }
}

void handleRoot() {
    server.send(200, "text/html", "<h1>You are connected</h1>");
}

void handleProgress() {
    char progressStr[32];
    int progressPct = _programmer.getProgrammingPercentage();
    sprintf(progressStr, "%d", progressPct);
    server.send(200, "text/html", progressStr);
}

void handleHalt() {
    Serial.printf("halt\n");
    romulatorInitDebug();
    romulatorHaltCpu();
    server.send(200, "text/html", "halted");
}

void handleRun() {
    Serial.printf("run\n");
    romulatorStartCpu();
    romulatorSetInput();
    server.send(200, "text/html", "cpu running");
}

void handleReadMemory() {
    romulatorInitDebug();
    //romulatorHaltCpu();
    romulatorReadMemoryToFile();
    //romulatorStartCpu();

    File fp = LittleFS.open("/memory.bin", "r");
    server.sendHeader("content-disposition", "attachment; filename=\"memory.bin\"");
    server.send(200, "application/octet-stream", &fp, fp.size());
    fp.close();
}

void handleVram() {
    int startMillis = millis();
    romulatorInitDebug();
    romulatorReadVram(_vram, 2048, 2000, 1);
    int endMillis = millis();
    //Serial.printf("vram %d\r\n", endMillis-startMillis);
    server.send(200, "application/octet-stream", _vram, 2048);
}

void handleReset() {
    romulatorResetDevice();
    server.send(200, "text/html", "device reset.");
}



void handleCharacterRom()
{
    // determine what character rom to use based on configuration
    // first get configuration byte
    romulatorInitDebug();
    uint8_t configByte = romulatorReadConfig();

    Serial.printf("configbyte is: %X\r\n", configByte);

    // get name of character rom indicated by this config byte
    File fp = LittleFS.open("/enable_table.csv", "r");
    if (!fp)
    {
        Serial.printf("no enable table.\r\n");
        return;
    }

    String enableTable = fp.readString();
    fp.close();
    int enableTableLen = enableTable.length();
    
    bool found = false;
    char* start; 
    char* end;
    char line[128];
    char* tableBegin = (char*)enableTable.c_str();
    start = tableBegin;
    end = start;

    int configIndex;
    unsigned int startAddr;
    unsigned int endAddr;
    char romName[64];
    romName[0] = '/';

    while (!found)
    {
        while (*end != '\n' && *end != 0 && end - tableBegin < enableTableLen) {
            end++;
        }
    
        int numchars = end-start;
        strncpy(line, start, numchars);
        line[numchars] = 0;

        // try to read in vram line for this config
        if (sscanf(line, "%d,0x%X,0x%X,\"vram\",%s", &configIndex, &startAddr, &endAddr, &romName[1]) == 4)
        {
            if (configIndex == configByte)
            {
                // found the name of the character rom
                found = true;
                break;
            }
        }

        if (end == 0 || end-tableBegin >= enableTableLen)
        {
            break;
        }

        start = end+1;
        end = start;
    }

    if (found)
    {
        fp = LittleFS.open(romName, "r");
        if (!fp)
        {
            Serial.printf("could not open: %s\r\n", romName);
            return;
        }

        Serial.printf("opened %s\r\n", romName);
        server.send(200, "application/octet-stream", &fp);
        fp.close();
    }
    else
    {
        Serial.printf("could not find vram entry for config %d\r\n", configByte);
    }
}

void handleCanvas()
{
    File fp = LittleFS.open("/canvas.html", "r");
    server.send(200, "text/html", &fp);
    fp.close();
}

void handleDraw()
{
    File fp = LittleFS.open("/draw.js", "r");
    server.send(200, "application/javascript", &fp);
    fp.close();
}

void handleScreenImage()
{
    File fp = LittleFS.open("/screenImage.js", "r");
    server.send(200, "application/javascript", &fp);
    fp.close();
}

void handleSetConfig()
{
    String confStr = server.arg(0);
    int c = atoi(confStr.c_str());
    Serial.printf("setting config %d\r\n", c);

    bool success = romulatorChangeConfiguration(c);
    if (success)
    {
        server.send(200, "text/html", "config set");
    }
    else
    {
        server.send(200, "text/html", "config could not be set");
    }
}

void startServer()
{
    EEPROM.get(0, user_wifi);

    delay(1000);
    WiFi.mode(WIFI_STA);
    //WiFi.setHostname("romulator");
    Serial.printf("connecting %s %s\n", user_wifi.ssid, user_wifi.password);
    WiFi.begin(user_wifi.ssid, user_wifi.password);

    byte tries = 0;
    AP = false;
    _led = LED_ON;
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, _led);
    while (WiFi.status() != WL_CONNECTED) 
    {
        Serial.printf(".");
        if (tries++ > 20 || user_wifi.ssid[0] == 0) 
        {
            Serial.printf("Starting access point.\n");
            AP = true;
            WiFi.mode(WIFI_AP);
            WiFi.softAP(ssid, password);
            break;
        }

        if (_led == LED_OFF)
        {
            _led = LED_ON;
        }
        else
        {
            _led = LED_OFF;
        }

        delay(500);
        digitalWrite(LED_PIN, _led);
    }

    if (AP) {
        Serial.printf("AP mode, ip address is %s\n", WiFi.softAPIP().toString().c_str());
    } else {
        digitalWrite(LED_PIN, LED_ON);
        Serial.printf("connected, ip address %s\n", WiFi.localIP().toString().c_str());

        // report local ip address to forwarding service
        // this allows you to find your romulator on your local network easily
        WiFiClient client;
        HTTPClient http;
        char url[256];
        sprintf(url, "http://bitfixer.com/rmltr/r.php?ip=%s", WiFi.localIP().toString().c_str());

        http.begin(client, url);
        int httpCode = http.GET();
        Serial.printf("recv %d\n", httpCode);
        String payload = http.getString();
        Serial.printf("payload: %s\n", payload.c_str());
    }

    server.on("/",  handlePortal);
    server.on("/upload", HTTP_POST, [](){server.send(200);}, handleFileUpload);
    server.on("/progress", handleProgress);
    server.on("/halt", handleHalt);
    server.on("/run", handleRun);
    server.on("/readmemory", handleReadMemory);
    server.on("/reset", handleReset);
    server.on("/vram", handleVram);
    server.on("/writememory", HTTP_POST, [](){server.send(200);}, handleWriteMemory);
    server.on("/setConfig", handleSetConfig);
    server.on("/romulator.rom", handleCharacterRom);
    server.on("/canvas.html", handleCanvas);
    server.on("/draw.js", handleDraw);
    server.on("/screenImage.js", handleScreenImage);
    server.begin();
    Serial.println("HTTP server started.\n");

    _lastMillis = millis();
}

void handleClient()
{
    if (AP)
    {
        int nowMillis = millis();
        if (nowMillis - _lastMillis >= 500)
        {
            if (_led == LED_OFF)
            {
                _led = LED_ON;
            }
            else
            {
                _led = LED_OFF;
            }

            digitalWrite(LED_PIN, _led);
            _lastMillis += 500;
        }
    }

    server.handleClient();
}