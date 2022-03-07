#include "server.h"
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <Arduino.h>
#include <EEPROM.h>
#include <LittleFS.h>

const char *ssid = "romulator";
const char *password = "password";
const char* hostname = "romulator.local";

bool AP;

ESP8266WebServer server(80);

struct settings {
    char ssid[30];
    char password[30];
} user_wifi = {};

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
            String s = fp.readString();
            //server.send(200,   "text/html", "<!doctype html><html lang='en'><head><meta charset='utf-8'><meta name='viewport' content='width=device-width, initial-scale=1'><title>Wifi Setup</title> <style>*,::after,::before{box-sizing:border-box;}body{margin:0;font-family:'Segoe UI',Roboto,'Helvetica Neue',Arial,'Noto Sans','Liberation Sans';font-size:1rem;font-weight:400;line-height:1.5;color:#212529;background-color:#f5f5f5;}.form-control{display:block;width:100%;height:calc(1.5em + .75rem + 2px);border:1px solid #ced4da;}button{cursor: pointer;border:1px solid transparent;color:#fff;background-color:#007bff;border-color:#007bff;padding:.5rem 1rem;font-size:1.25rem;line-height:1.5;border-radius:.3rem;width:100%}.form-signin{width:100%;max-width:400px;padding:15px;margin:auto;}h1{text-align: center}</style> </head> <body><main class='form-signin'> <form action='/' method='post'> <h1 class=''>Wifi Setup</h1><br/><div class='form-floating'><label>SSID</label><input type='text' class='form-control' name='ssid'> </div><div class='form-floating'><br/><label>Password</label><input type='password' class='form-control' name='password'></div><br/><br/><button type='submit'>Save</button><p style='text-align: right'><a href='https://www.mrdiy.ca' style='color: #32C5FF'>mrdiy.ca</a></p></form></main> </body></html>" );
            server.send(200, "text/html", s);
        }
    }
    else
    {
        // handle incoming file upload
        if (server.method() == HTTP_POST) {
            server.send(200, "text/html", "is a post");
        } else {
            server.send(200, "text/html", "not a post!");
        }

    }
}

void handleRoot() {
    server.send(200, "text/html", "<h1>You are connected</h1>");
}

void startServer()
{
    EEPROM.begin(sizeof(struct settings));
    EEPROM.get(0, user_wifi);

    WiFi.mode(WIFI_STA);
    WiFi.hostname("romulator");
    WiFi.begin(user_wifi.ssid, user_wifi.password);
  
    byte tries = 0;
    AP = false;
    while (WiFi.status() != WL_CONNECTED) 
    {
        Serial.printf(".");
        delay(1000);
        if (tries++ > 10) 
        {
            Serial.printf("Starting access point.\n");
            AP = true;
            WiFi.mode(WIFI_AP);
            WiFi.softAP(ssid, "");
            break;
        }
    }

    if (AP) {
        Serial.printf("AP mode, ip address is %s\n", WiFi.softAPIP().toString().c_str());
    } else {
        Serial.printf("connected, ip address %s\n", WiFi.localIP().toString().c_str());
    }

    server.on("/",  handlePortal);
    server.begin();
    Serial.println("HTTP server started.\n");
}

void handleClient()
{
    server.handleClient();
}