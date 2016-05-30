 /**
 * IBM IoT Foundation managed Device
 * 
 * Author: Ant Elder
 * License: Apache License v2
 */
#include <ESP8266WiFi.h>
#include <Wire.h>
#include <SDL_Arduino_INA3221.h>

// the three channels of the INA3221 named for SunAirPlus Solar Power Controller channels (www.switchdoc.com)
#define LIPO_BATTERY_CHANNEL 1
#define SOLAR_CELL_CHANNEL 2
#define OUTPUT_CHANNEL 3
#include <PubSubClient.h> // https://github.com/knolleary/pubsubclient/releases/tag/v2.3
#include <ArduinoJson.h> // https://github.com/bblanchon/ArduinoJson/releases/tag/v5.0.7

//-------- Customise these values -----------
const char* ssid = "YOURSSID";
const char* password = "YOURPASSWORD";

#define ORG "YOURORG" 
#define DEVICE_TYPE "SmartSolarBattery" 
#define DEVICE_ID "SSB001"
#define TOKEN "YOURTOKEN"
//-------- Customise the above values --------


// Declare & Initialize sensor here 
SDL_Arduino_INA3221 ina3221;

char server[] = ORG ".messaging.internetofthings.ibmcloud.com";
char authMethod[] = "use-token-auth";
char token[] = TOKEN;
char clientId[] = "d:" ORG ":" DEVICE_TYPE ":" DEVICE_ID;

const char logDataTopic[] = "iot-2/evt/logData/fmt/json";
const char batteryChargeTopic[] = "iot-2/evt/batteryCharge/fmt/json";
const char getForecastTopic[] = "iot-2/evt/getForecast/fmt/json";
const char setForecastCmdTopic[] = "iot-2/cmd/setForecast/fmt/json";
const char setPowerModeCmdTopic[] = "iot-2/cmd/setPowerMode/fmt/json";
const char statusTopic[] = "iot-2/evt/status/fmt/json";
const char responseTopic[] = "iotdm-1/response";
const char manageTopic[] = "iotdevice-1/mgmt/manage";
const char updateTopic[] = "iotdm-1/device/update";
const char rebootTopic[] = "iotdm-1/mgmt/initiate/device/reboot";
const char locationTopic[] = "iotdevice-1/device/update/location";
const char logTopic[] = "iotdevice-1/add/diag/log";

void initManagedDevice();
void callback(char* topic, byte* payload, unsigned int payloadLength);
void publishData();
void publishBatteryCharge();
void publishStatus();
void handleUpdate(byte* payload);
void publishLog(String msg, int errorLevel);

WiFiClient wifiClient;
PubSubClient client(server, 1883, callback, wifiClient);

int publishInterval = 2000; // 2 seconds - this can be varied from the IoT platform
long lastPublishMillis;

void initSHSB() {
   // Construct the JSON string directly as a string
 const float lat = -33.8675;
 const float lng = 151.2079; // Always located in Sydney!  Why would you want to live anywhere else?!


 if (client.subscribe(setForecastCmdTopic)) {
 Serial.println("subscribe to setForecast OK");
 } else {
 Serial.println("subscribe to setForecast FAILED");
 }

 if (client.subscribe(setPowerModeCmdTopic)) {
 Serial.println("subscribe to setPowerMode OK");
 } else {
 Serial.println("subscribe to setPowerMode FAILED");
 }

 client.loop();
 if (client.publish(
              getForecastTopic, 
              String("{\"latitude\":" + String(lat) + ",\"longitude\":" + String(lng) + "}").c_str()
              )) {
  Serial.println("location publish ok");
 } else {
  Serial.println("location publish failed");
 }
 client.loop();
 
}

void wifiConnect() {
 Serial.print("Connecting to "); Serial.print(ssid);
 if (String(password) == "")
  WiFi.begin(ssid); // Use this form if no password required
 else
  WiFi.begin(ssid, password); // Use this form if password required
 while (WiFi.status() != WL_CONNECTED) {
   delay(500);
   Serial.print(".");
 } 
 Serial.print("\nWiFi connected, IP address: "); Serial.println(WiFi.localIP());
}

void mqttConnect() {
 if (!!!client.connected()) {
   Serial.print("Reconnecting MQTT client to "); Serial.println(server);
 while (!!!client.connect(clientId, authMethod, token)) {
    Serial.print(".");
    delay(500);
 }
 Serial.println();
 }
}

void setup() {
 Serial.begin(57600); 
 Serial.println("Starting...");
 ina3221.begin(); 
 wifiConnect();
 mqttConnect();
 initManagedDevice();
 initSHSB();
 publishLog("Up and running", 0);
}


//
// Main loop
//

void loop() {
 if (millis() - lastPublishMillis > publishInterval) {
 publishData(); 
 publishStatus();
 publishBatteryCharge();
 lastPublishMillis = millis();
 }
 if (!client.loop()) { // Invoke the MQTT client loop to check for published messages
   mqttConnect();
 }
 // delay(500); // Could go to sleep here for a while to save power
}


//
// Read data from our sensor
//

  float shuntvoltage1 = 0;
  float busvoltage1 = 0;
  float current_mA1 = 0;
  float loadvoltage1 = 0;
  float shuntvoltage2 = 0;
  float busvoltage2 = 0;
  float current_mA2 = 0;
  float loadvoltage2 = 0;
  float shuntvoltage3 = 0;
  float busvoltage3 = 0;
  float current_mA3 = 0;
  float loadvoltage3 = 0;

void readSensorData() {  
    busvoltage1 = ina3221.getBusVoltage_V(LIPO_BATTERY_CHANNEL);
  shuntvoltage1 = ina3221.getShuntVoltage_mV(LIPO_BATTERY_CHANNEL);
  current_mA1 = -ina3221.getCurrent_mA(LIPO_BATTERY_CHANNEL);  // minus is to get the "sense" right.   - means the battery is charging, + that it is discharging
  loadvoltage1 = busvoltage1 + (shuntvoltage1 / 1000);
  
  Serial.print("LIPO_Battery Bus Voltage:   "); Serial.print(busvoltage1); Serial.println(" V");
  Serial.print("LIPO_Battery Shunt Voltage: "); Serial.print(shuntvoltage1); Serial.println(" mV");
  Serial.print("LIPO_Battery Load Voltage:  "); Serial.print(loadvoltage1); Serial.println(" V");
  Serial.print("LIPO_Battery Current 1:       "); Serial.print(current_mA1); Serial.println(" mA");
  Serial.println("");



  busvoltage2 = ina3221.getBusVoltage_V(SOLAR_CELL_CHANNEL);
  shuntvoltage2 = ina3221.getShuntVoltage_mV(SOLAR_CELL_CHANNEL);
  current_mA2 = -ina3221.getCurrent_mA(SOLAR_CELL_CHANNEL);
  loadvoltage2 = busvoltage2 + (shuntvoltage2 / 1000);
  
  Serial.print("Solar Cell Bus Voltage 2:   "); Serial.print(busvoltage2); Serial.println(" V");
  Serial.print("Solar Cell Shunt Voltage 2: "); Serial.print(shuntvoltage2); Serial.println(" mV");
  Serial.print("Solar Cell Load Voltage 2:  "); Serial.print(loadvoltage2); Serial.println(" V");
  Serial.print("Solar Cell Current 2:       "); Serial.print(current_mA2); Serial.println(" mA");
  Serial.println("");



  busvoltage3 = ina3221.getBusVoltage_V(OUTPUT_CHANNEL);
  shuntvoltage3 = ina3221.getShuntVoltage_mV(OUTPUT_CHANNEL);
  current_mA3 = ina3221.getCurrent_mA(OUTPUT_CHANNEL);
  loadvoltage3 = busvoltage3 + (shuntvoltage3 / 1000);
  
  Serial.print("Output Bus Voltage 3:   "); Serial.print(busvoltage3); Serial.println(" V");
  Serial.print("Output Shunt Voltage 3: "); Serial.print(shuntvoltage3); Serial.println(" mV");
  Serial.print("Output Load Voltage 3:  "); Serial.print(loadvoltage3); Serial.println(" V");
  Serial.print("Output Current 3:       "); Serial.print(current_mA3); Serial.println(" mA");
  Serial.println(""); 
}

//
// Read data from our sensor & publish it
//
void publishData() {
  readSensorData(); // Fetch current temperature and humidity  

    String payload = "{\"d\":{\"batteryBusVoltage\":" + String(busvoltage1) +
         ",\"batteryShuntVoltage\":" + String(shuntvoltage1) + 
         ",\"batteryLoadVoltage\":" + String(loadvoltage1) + 
         ",\"batteryCurrent\":" + String(current_mA1) + 
         ",\"solarBusVoltage\":" + String(busvoltage2) +
         ",\"solarSuntVoltage\":" + String(shuntvoltage2) +
         ",\"solarLoadVoltage\":" + String(loadvoltage2) + 
         ",\"solarCurrent\":" + String(current_mA2) + 
         /*
         ",\"OBV\":" + String(busvoltage3) +
         ",\"OSV\":" + String(shuntvoltage3) + 
         ",\"OLV\":" + String(loadvoltage3) +
         ",\"OmA\":" + String(current_mA3) + 
         */
         "}}";  
    Serial.print("Sending payload: "); Serial.println(payload);
    client.loop(); // Clear any received messages
   if (client.publish(logDataTopic, (char*) payload.c_str())) {
     Serial.println("Publish OK");
   } else {
     Serial.println("Publish FAILED");
   }
}

void publishBatteryCharge() {
    String payload = "{\"batteryCharge\":" +String((busvoltage1-3.6)/(4.12-3.6)*100) + "}";
    Serial.print("batteryCharge payload: "); Serial.println(payload);
   if (client.publish(batteryChargeTopic, payload.c_str())) {
     Serial.println("Publish OK");
   } else {
     Serial.println("Publish FAILED");
   }
}
   

char currentTemp[10];
char currentCond[100];
char forecast[100];
char powerMode[20];

void publishStatus() {
  String payload = "{\"currentTemp\": \"" + String(currentTemp) + "\",\"currentCond\": \"" + String(currentCond) + "\""
    + ",\"powerMode\":\"" + String(powerMode) + "\",\"forecast\": \"" + String(forecast) + "\"}";
  Serial.println("Publishing status: " + payload);
  if (client.publish(statusTopic, (char *)payload.c_str())) {
    Serial.println("Publish status OK");
  } else {
    Serial.println("Publish status FAILED");
  }
}

void handleForecast(byte* payload) {
 StaticJsonBuffer<300> jsonBuffer;
 JsonObject& root = jsonBuffer.parseObject((char*)payload);
 if (!root.success()) {
    Serial.println("handleForecast: payload parse FAILED");
    return;
 }
 Serial.println("handleForecast payload:"); root.prettyPrintTo(Serial); Serial.println();

// payload: {"deviceId":"SSB001","currentTemp":"22","currentCond":"Fair, Partly Cloudy","forecast":""}

 strcpy(currentTemp, root["currentTemp"]);
 strcpy(currentCond, root["currentCond"]);
 strcpy(forecast, root["forecast"]);
 Serial.println(String(currentTemp));
 Serial.println(String(currentCond));
 Serial.println(String(forecast));
}

void handleSetPowerMode(byte* payload) {
 StaticJsonBuffer<300> jsonBuffer;
 JsonObject& root = jsonBuffer.parseObject((char*)payload);
 if (!root.success()) {
    Serial.println("handleSetPowerMode: payload parse FAILED");
    return;
 }
 Serial.println("handleSetPowerMode payload:"); root.prettyPrintTo(Serial); Serial.println();

//payload: {
//    "deviceId": "SSB001",
//    "powerMode": "SOLAR"
//}

 strcpy(powerMode, root["powerMode"]);
 Serial.println(String(powerMode));
}

//
// Below here is code to support acting as a managed device & setting the location
// It illustrates 2 different ways of creating the required JSON objects
//

void initManagedDevice() {

 if (client.subscribe("iotdm-1/response")) {
 Serial.println("subscribe to responses OK");
 } else {
 Serial.println("subscribe to responses FAILED");
 }

 if (client.subscribe(rebootTopic)) {
 Serial.println("subscribe to reboot OK");
 } else {
 Serial.println("subscribe to reboot FAILED");
 }

 if (client.subscribe("iotdm-1/device/update")) {
 Serial.println("subscribe to update OK");
 } else {
 Serial.println("subscribe to update FAILED");
 }

// Use the JsonObject library to create JSON structure
 StaticJsonBuffer<300> jsonBuffer;
 JsonObject& root = jsonBuffer.createObject();
 JsonObject& d = root.createNestedObject("d");
 JsonObject& metadata = d.createNestedObject("metadata");
 metadata["publishInterval"] = publishInterval;
 JsonObject& supports = d.createNestedObject("supports");
 supports["deviceActions"] = true;

 char buff[300];
 root.printTo(buff, sizeof(buff));
 Serial.println("publishing device metadata:"); Serial.println(buff);
 if (client.publish(manageTopic, buff)) {
 Serial.println("device Publish ok");
 } else {
 Serial.print("device Publish failed:");
 }
 Serial.println("publishing location:");

 // Construct the JSON string directly as a string
 const float lat = -33.8675;
 const float lng = 151.2079; // Always located in Sydney!  Why would you want to live anywhere else?!
 
 if (client.publish(
              locationTopic, 
              String("{\"d\":{\"latitude\":" + String(lat) + ",\"longitude\":" + String(lng) + "}}").c_str()
              )) {
  Serial.println("location publish ok");
 } else {
  Serial.println("location publish failed");
 }
}

void publishLog(String msg, int errorLevel) {
 StaticJsonBuffer<300> jsonBuffer;
 JsonObject& root = jsonBuffer.createObject();
 JsonObject& d = root.createNestedObject("d");
 d["message"] = msg;
 d["severity"] = errorLevel;
 
 char buff[300];
 root.printTo(buff, sizeof(buff));
 Serial.println("publishing log:"); Serial.println(buff);
 if (client.publish(logTopic, buff)) {
 Serial.println("log Publish ok");
 } else {
 Serial.print("log Publish failed:");
 }  
}

void callback(char* topic, byte* payload, unsigned int payloadLength) {
 Serial.print("callback invoked for topic: "); Serial.println(topic);
 payload[payloadLength] = '\000';
 Serial.print("payload: "); Serial.println((char *)payload);
 Serial.print("length: "); Serial.println(payloadLength, DEC);
 if (strcmp (responseTopic, topic) == 0) {
    return; // just print of response for now 
 }

 if (strcmp(setForecastCmdTopic, topic) == 0) {
  handleForecast(payload);
 }

  if (strcmp(setPowerModeCmdTopic, topic) == 0) {
    handleSetPowerMode(payload);
  }
  
 if (strcmp (rebootTopic, topic) == 0) {
    Serial.println("Rebooting...");
    ESP.restart();
 }

 if (strcmp (updateTopic, topic) == 0) {
 handleUpdate(payload); 
 } 
}

void handleUpdate(byte* payload) {
 StaticJsonBuffer<300> jsonBuffer;
 JsonObject& root = jsonBuffer.parseObject((char*)payload);
 if (!root.success()) {
    Serial.println("handleUpdate: payload parse FAILED");
    return;
 }
 Serial.println("handleUpdate payload:"); root.prettyPrintTo(Serial); Serial.println();

 JsonObject& d = root["d"];
 JsonArray& fields = d["fields"];
 for(JsonArray::iterator it=fields.begin(); it!=fields.end(); ++it) {
    JsonObject& field = *it;
    const char* fieldName = field["field"];
    if (strcmp (fieldName, "metadata") == 0) {
        JsonObject& fieldValue = field["value"];
        if (fieldValue.containsKey("publishInterval")) {
            publishInterval = fieldValue["publishInterval"];
            Serial.print("publishInterval:"); Serial.println(publishInterval);
        }
    }
 }
}
