#include "server.h"
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <Arduino.h>

const char *ssid = "esptest";
const char *password = "password";

ESP8266WebServer server(80);

void handleRoot() {
    server.send(200, "text/html", "<h1>You are connected</h1>");
}

void startServer()
{
    WiFi.softAP(ssid, password);
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP address:");
    Serial.println(myIP);

    server.on("/", handleRoot);
    server.begin();

    Serial.println("HTTP server started.\n");
}
