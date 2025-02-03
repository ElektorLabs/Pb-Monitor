#include <WiFi.h>
#include <SPI.h>
#include <Adafruit_MCP3008.h>
#include <MQTTPubSubClient.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <config.h>
#include <esp_task_wdt.h>  // Required for watchdog timer functions

#include <ESPmDNS.h>
#include <NetworkUdp.h>
#include <ArduinoOTA.h>


// WiFi Credentials
const char* ssid = WIFISSID;      // Your WiFi SSID
const char* pass = WIFIPASSWORD;  // Your WiFi Password

WiFiClient client;
MQTTPubSubClient mqtt;

Adafruit_MCP3008 adc;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Global Variables for Energy Calculation
unsigned long previousMillis = 0;  // Stores the last time the energy was updated
float energyConsumed = 0;          // Total energy consumed over time in Watt-hours (Wh)
unsigned long startTime;



void setup() {

/* Initialize the serial port for debugging if DEBUG is set to true */
#if DEBUG == true
  Serial.begin(115200);
  Serial.println("*** Pb Monitor ***");  // Debugging message to indicate the start of ATM90E32 initialization
#endif



  vTaskDelay(10 / portTICK_PERIOD_MS);  // Yields CPU for 10ms

  Wire.setPins(I2C_SDA, I2C_SCL);
  Wire.begin(I2C_SDA, I2C_SCL);

  // Software SPI (specify all, use any available digital)
  // (sck, mosi, miso, cs);
  adc.begin(D8, D10, D9, D3);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    //for(;;); // Don't proceed, loop forever
  }

  display.display();
  display.clearDisplay();

    WiFi.begin(ssid, pass);
  /* Initialize the MQTT client */
  mqtt.begin(client);
  vTaskDelay(10 / portTICK_PERIOD_MS);  // Yields CPU for 10ms
  /* Connect to WiFi, MQTT broker, and Home Assistant */
  
  connect();

  ArduinoOTA.setHostname("Pb-Monitor");



  /* Set up a subscription to handle incoming MQTT messages */
  mqtt.subscribe([](const String& topic, const String& payload, const size_t size) {
#if DEBUG == true
    Serial.println("mqtt received: " + topic + " - " + payload);  // Debugging message to show received MQTT messages
#endif
  });

  /* Subscribe to the /hello topic and set up a callback for incoming messages */
  mqtt.subscribe("/hello", [](const String& payload, const size_t size) {
#if DEBUG == true
    Serial.print("/hello ");
    Serial.println(payload);  // Debugging message to show the payload of the /hello topic
#endif
  });



  ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH) {
        type = "sketch";
      } else {  // U_SPIFFS
        type = "filesystem";
      }

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type);
    })
    .onEnd([]() {
      Serial.println("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) {
        Serial.println("Auth Failed");
      } else if (error == OTA_BEGIN_ERROR) {
        Serial.println("Begin Failed");
      } else if (error == OTA_CONNECT_ERROR) {
        Serial.println("Connect Failed");
      } else if (error == OTA_RECEIVE_ERROR) {
        Serial.println("Receive Failed");
      } else if (error == OTA_END_ERROR) {
        Serial.println("End Failed");
      }
    });

  ArduinoOTA.begin();

  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {

  ArduinoOTA.handle();
  //Periodic energy update (example: every 1 second)
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= 1000) {
    previousMillis = currentMillis;
    float current = readCurrent();       // Read current in Amps
    float voltage = batteryVoltages(5);  // Read voltage in Volts
    float power = voltage * current;     // Calculate power in Watts
    energyConsumed += power / 3600;      // Convert power to Wh and accumulate
  }
  vTaskDelay(10 / portTICK_PERIOD_MS);  // Yields CPU for 10ms
  updateDisplay();
  vTaskDelay(10 / portTICK_PERIOD_MS);  // Yields CPU for 10ms
  // /* Keep the MQTT client updated */
  mqtt.update();
  vTaskDelay(10 / portTICK_PERIOD_MS);  // Yields CPU for 10ms
  // /* Reconnect to the MQTT broker if the connection is lost */
  if (!mqtt.isConnected()) {
    connect();
  }

  /* Check and send energy data to Home Assistant every 3 seconds */
  static uint32_t prev_ms = millis();
  if (millis() > prev_ms + 3000) {
    prev_ms = millis();
    sendData();  // Retrieve energy data and send via MQTT
  }
}
