
void sendData() {
  // Retrieve all parameters from the ATM90E32
  float V1 = batteryVoltages(1);
  float V2 = batteryVoltages(2);
  float V3 = batteryVoltages(3);
  float V4 = batteryVoltages(4);


  float Current = readCurrent();
  float Btemperature = measureTemperature(6, R13, A_10K, B_10K, C_10K);
  float Abtemperature = measureTemperature(2, R12, A_100K, B_100K, C_100K);

  float remainingBattery = 100 - ((energyConsumed / batteryCapacityWh) * 100) ; // State of Charge in percentage
 if ( remainingBattery > 100) {
  remainingBattery = 100;
 } 

  // Send all the collected data via MQTT to Home Assistant
  mqtt.publish("PbMonitor/VBatt1", String(V1).c_str());
  mqtt.publish("PbMonitor/VBatt2", String(V2).c_str());
  mqtt.publish("PbMonitor/VBatt3", String(V3).c_str());
  mqtt.publish("PbMonitor/VBatt4", String(V4).c_str());
  mqtt.publish("PbMonitor/Current", String(Current).c_str());

  mqtt.publish("PbMonitor/Abtemperature", String(Abtemperature).c_str());
  mqtt.publish("PbMonitor/Btemperature1", String(Btemperature).c_str());
  mqtt.publish("PbMonitor/RemainingBattery", String(remainingBattery).c_str());

  // Debugging: Print all energy data to the Serial Monitor if DEBUG is enabled
#if DEBUG == true
  Serial.println("Voltage Battery 1: " + String(V1) + "V");
  Serial.println("Voltage Battery 2: " + String(V2) + "V");
  Serial.println("Voltage Battery 3: " + String(V3) + "V");
  Serial.println("Voltage Battery 4: " + String(V4) + "V");

  Serial.println("Current A: " + String(Current) + "A");

  Serial.println("Power: " + String(readCurrent() * batteryVoltages(5)) + "W");
  Serial.println("Remianing Battery: " + String(remainingBattery) + "%");
#endif
}



void connect() {
  int i=0;
connect_to_wifi:
#if DEBUG == true
  Serial.print("Connecting to WiFi...");
#endif

  // Attempt to connect to WiFi
  while (WiFi.status() != WL_CONNECTED) {
  delay(1000);
  display.clearDisplay();
  display.setTextSize(2);               // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);  // Draw white text
  display.setCursor(1, 0);  // Start at top-left corner
  display.write("Connecting to WiFi...");
  if (i == 20){
    break;
  }

  display.setCursor(1, 18);
  display.print(i);
  i++;
  }
#if DEBUG == true
  Serial.println(" CONNECTED!");
#endif
connect_to_host:
#if DEBUG == true
  Serial.print("Connecting to Host...");
#endif
  // Disconnect and attempt to connect to the MQTT broker
  client.stop();
  while (!client.connect(HOMEASSISTANT_IP, 1883)) {
    delay(1000);
    if (WiFi.status() != WL_CONNECTED) {
#if DEBUG == true
      Serial.println("WiFi Disconnected");
#endif
      goto connect_to_wifi;
    }
  }
#if DEBUG == true
  Serial.println("  CONNECTED!");
  Serial.print("Connecting to MQTT Broker...");
#endif
  // Disconnect MQTT and attempt to reconnect
  mqtt.disconnect();
  while (!mqtt.connect(DEVICE_NAME, USER_ID, PASSWORD)) {
    delay(1000);
    if (WiFi.status() != WL_CONNECTED) {
#if DEBUG == true
      Serial.println("WiFi Disconnected");
#endif
      goto connect_to_wifi;  // If WiFi is disconnected, reconnect
    }
    if (client.connected() != 1) {
#if DEBUG == true
      Serial.println("WiFiClient disconnected");
#endif
      goto connect_to_host;  // If client is disconnected, reconnect to host
    }
  }
#if DEBUG == true
  Serial.println(" CONNECTED!");
#endif
}



float readCurrent() {
    float totalVoltage = 0.00;

    // Take 60 ADC readings and calculate the total voltage
    for (int i = 0; i < 60; i++) {
        totalVoltage += (adc.readADC(0) * 3.29) / 1023.0; // Convert ADC reading to voltage
    }

    // Calculate the average measured voltage
    float measuredVoltage = totalVoltage / 60.0;

    // Calculate the offset from the nominal zero point (1.65V)
    float offset = measuredVoltage - 1.67;

    // Calculate current using sensor sensitivity (e.g., 0.0264 V/A)
    float current = offset / 0.0264;

    // Eliminate small noise by setting very small currents to 0
    if (abs(current) < 0.5) current = 0; // Adjust the noise threshold as needed

    return current; // Returns positive for forward current, negative for reverse current
}

float readVoltage(int channel) {
  int8_t val_0 = adc.readADC(5);
  float f0 = (adc.readADC(5) * 3.29) / 1023.0;

  int8_t val_1 = adc.readADC(4);
  float f1 = (adc.readADC(4) * 3.29) / 1023.0;

  int8_t val_2 = adc.readADC(3);
  float f2 = (adc.readADC(3) * 3.29) / 1023.0;

  int8_t val_3 = adc.readADC(1);
  float f3 = (adc.readADC(1) * 3.29) / 1023.0;


  float voltage = 0;

  switch (channel) {
    case 1:
      voltage = fabs((f0) * ((R1 + R2) / R2) + ZERO_ERROR_V);
      break;
    case 2:
      voltage = fabs((f1) * ((R3 + R4) / R4) + ZERO_ERROR_V);
      break;
    case 3:
      voltage = fabs((f2) * ((R5 + R6) / R6) + ZERO_ERROR_V);
      break;
    case 4:
      voltage = fabs((f3) * ((R7 + R8) / R8) + ZERO_ERROR_V);
      break;
  }
  return voltage;
}
float batteryVoltages(int battery) {
  float battery1 = readVoltage(1);
  if (battery1 < 3) battery1 = 0;
  float battery2 = readVoltage(2) - readVoltage(1);
  float battery3 = readVoltage(3) - readVoltage(2);
  float battery4 = readVoltage(4) - readVoltage(3);
  float totalbattery = readVoltage(4);

    // Handle invalid or low battery voltage (optional for lead-acid)
    
  float batteryvoltage = 0.00;

  switch (battery) {
    case 1:
      batteryvoltage = battery1;
      break;
    case 2:
      if (battery2 < 3) battery2 = 0;
      batteryvoltage = battery2;
      break;
    case 3:
      if (battery3 < 3) battery3 = 0;
      batteryvoltage = battery3;
      break;
    case 4:
      if (battery4 < 3) battery4 = 0.00;
      batteryvoltage = battery4;
      break;
    case 5:
      if (totalbattery < 3) totalbattery = 0.00;
      batteryvoltage = totalbattery;
      break;
  }

  return batteryvoltage;
}


float calculateSoC(float batteryVoltage, float V_min, float V_max) {
    // Ensure the battery voltage is within the valid range
    if (batteryVoltage <= V_min) return 0.0;  // Fully discharged
    if (batteryVoltage >= V_max) return 100.0; // Fully charged

    // Calculate SoC
    float SoC = ((batteryVoltage - V_min) / (V_max - V_min)) * 100.0;
    return SoC;
}

// Function to calculate temperature from resistance
float calculateTemperature(float resistance, float A, float B, float C) {
    float logR = log(resistance);
    float temperature = 1.0 / (A + (B * logR) + (C * logR * logR * logR)); // Kelvin
    return temperature - 273.15; // Convert to Celsius
}

// Function to measure temperature
float measureTemperature(int adcChannel, float R_FIXED, float A, float B, float C) {
    int adcValue = adc.readADC(adcChannel);
    float V_measured = (adcValue * V_REF) / ADC_MAX;

    // Fix: Correct resistance calculation
    float R_thermistor = R_FIXED * ((V_measured) / (V_REF - V_measured));

    // Compute temperature
    return calculateTemperature(R_thermistor, A, B, C);
}


void updateDisplay() {

  display.clearDisplay();

  display.setTextSize(2);               // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);  // Draw white text

  display.setCursor(1, 0);  // Start at top-left corner
  display.print(batteryVoltages(5));
  display.write("V");

  display.setCursor(1, 18);
  display.print(readCurrent());
  display.write("A");

  display.setCursor(1, 37);
  display.print(readCurrent() * batteryVoltages(5));
  //display.print(readCurrent());
  display.write("W");


  display.setTextSize(1);
  display.setCursor(80, 0);
  display.print(batteryVoltages(1),2);
  display.write("V");
  display.setCursor(80, 10);
  display.print(batteryVoltages(2),2);
  display.write("V");
  display.setCursor(80, 20);
  display.print(batteryVoltages(3),2);
  display.write("V");
  display.setCursor(80, 30);
  display.print(batteryVoltages(4),2);
  display.write("V");


  // enter the batteryCapacityWh in config.h
  float remainingBattery = 100 - ((energyConsumed / batteryCapacityWh) * 100) ; // State of Charge in percentage

  display.setCursor(80, 40);
  display.print(remainingBattery);
  display.write("%");

  
  display.setCursor(66, 55);
  display.write("T2:");
  display.print(measureTemperature(6, R13, A_10K, B_10K, C_10K),1);
  display.write(char(247));
  display.write("C");

  display.setCursor(1, 55);
  display.write("T1:");
  display.print(measureTemperature(2, R12, A_100K, B_100K, C_100K),1);
  display.write(char(247));
  display.write("C");


  display.display();
}
