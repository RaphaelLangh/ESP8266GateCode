#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>

// Parameters for gate credentials
int  address     = 0;
String pass_gate = "1a2b3c";
String id_gate   = "Gate_1234";
String readID    = "";
String readPass  = "";

// Parameters for ESP8266 wifi setup
const char *ssid      = "gate_project";
const char *password  = "12345";
String     returnOk[] = {"true"};  
const int  ledPin     = D7; // GPIO2
ESP8266WebServer server(80);

void setup() {
  EEPROM.begin(512);
  Serial.begin(9600); 
  delay(1000);

  pinMode(ledPin, OUTPUT);
  
  Serial.println();
  Serial.print("Configuring access point...");

  WiFi.softAP(ssid, password);

  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  
// Writing data to memory
  EEPROM.put(0, id_gate);
  EEPROM.put(16, pass_gate);
  Serial.println("gate id we just wrote is: "+getGateID());
  Serial.println("gate password we just wrote is: "+getGatePass());

  server.on( "/open", HTTP_GET, openGate); // Listen for command from andriod
  server.on( "/id", HTTP_POST, sendGateId);
  server.on( "/off", HTTP_GET, closeGate);    
  server.on( "/changePass", HTTP_GET, changePassword);
  server.begin();
  Serial.println("HTTP server started, send gate id ");
}

void sendGateId()
{
  String answer = getGateID();
  Serial.println(answer);   
  server.send( 200, "application/gson; charset=utf-8", answer);
}

String getGateID()
{
  //Read data from memory
  EEPROM.get(0, readID);
  return readID;
}

String getGatePass()
{
  //Read data from memory
  EEPROM.get(16, readPass);
  return readPass;
}

void openGate()
{
  if (server.hasArg("password"))
  { 
    Serial.println("Get Gate Pass: ");
    Serial.print(EEPROM.get(16, readPass));
    Serial.println("server.arg : ");
    Serial.print(server.arg("password"));
    
    if (server.arg("password") == getGatePass()) // Verify that the correct password is received 
    { 
      digitalWrite(ledPin, HIGH); // open gate
      server.send (200, "application/gson; charset=utf-8","true");
      Serial.println("Correct Password. Gate is now opening.");
    }
    else 
    {
      Serial.println("Incorrect password.");
      server.send(203, "text/plain", "false");
    }
  }
  else
  {
    server.send(203, "text/plain", "false");
   Serial.println("No password received.");
  }
}

void closeGate()
{
  digitalWrite(ledPin, LOW); // open gate
  server.send ( 200, "application/json; charset=utf-8", "true");
  Serial.println("Gate is now closing.");
}

void changePassword()
{
  if (server.hasArg("password") && server.hasArg("nPassword") && (getGatePass() != server.arg("nPassword")))
  {   
    if (server.arg("password") == getGatePass()) // Verify that the correct password is received 
    { 
      String newPassword = server.arg("nPassword");
      EEPROM.put(16, newPassword);
      Serial.println("gate password we just wrote is: "+getGatePass());
      server.send(200, "application/json; charset=utf-8", "true");
      Serial.println("You have successfully changed the password");
    }
    else 
    {
      Serial.println("You are not authorised to change the password.");
      server.send(203, "application/json; charset=utf-8", "false");
    }
  }
}

void loop()
{
  server.handleClient();
}
