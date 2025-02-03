


#define DEBUG true  // Make this false if Serial Monitor is not required.

#define WIFISSID "xxxx"
#define WIFIPASSWORD "xxxx"

#define HOMEASSISTANT_IP "xxx"
#define DEVICE_NAME "Pb-Monitor"
#define USER_ID "xxx"
#define PASSWORD "xxx"


#define I2C_SDA D4
#define I2C_SCL D5

#define ZERO_ERROR_V 0
#define ZERO_ERROR_C 0.20

// Voltage Dividers
#define R1 97.8
#define R2 21.81
#define R3 98.1
#define R4 9.86
#define R5 106
#define R6 6.69
#define R7 98.5
#define R8 4.595

#define R12 98000
#define R13 9800


// ADC and Voltage Reference Constants
const float V_REF = 3.29; // ADC reference voltage
const int ADC_MAX = 1023; // 10-bit ADC resolution

// Steinhart-Hart Coefficients for 10K Thermistor (Adjust as per your thermistor datasheet)
const float A_10K = 0.001129148;
const float B_10K = 0.000234125;
const float C_10K = 0.0000000876741;

// Steinhart-Hart Coefficients for 100K Thermistor (Adjust as per your thermistor datasheet)
const float A_100K = 0.000827257;
const float B_100K = 0.000215191;
const float C_100K = 0.00000009304;




// Assuming a fixed battery capacity for SoC estimation (e.g., 100Wh for demonstration)
#define batteryCapacityWh  1920 // Replace with actual battery capacity

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C
