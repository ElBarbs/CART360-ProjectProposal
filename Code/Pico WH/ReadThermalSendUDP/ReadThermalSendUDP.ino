/*
  Read the MLX90640 temperature readings as quickly as possible
  By: Nathan Seidle
  SparkFun Electronics
  Date: May 22nd, 2018
  License: MIT. See license file for more information but you can
  basically do whatever you want with this code.
*/

#include <Wire.h>
#include <WiFi.h>
#include <AsyncUDP_RP2040W.h>

#include "MLX90640_API.h"
#include "MLX90640_I2C_Driver.h"

#include <elapsedMillis.h>

elapsedMillis timeElapsed;

// Delay in milliseconds between UDP messages.
const int defaultInterval = 5000; // 5 seconds.
int sendInterval = defaultInterval;

#define UDP_PORT 51689
#define ORIGINAL_SIZE 768
#define DOWNSAMPLE_FACTOR 2
#define DOWNSAMPLED_SIZE ORIGINAL_SIZE / (DOWNSAMPLE_FACTOR * 2)

// Default 7-bit unshifted address of the MLX90640.
const byte MLX90640_address = 0x33;

// Default shift for MLX90640 in open air.
#define TA_SHIFT 8

float mlx90640To[768];
paramsMLX90640 mlx90640;

// Pin that goes high/low when calculations are complete.
// This makes the timing visible on the logic analyzer.
const byte calcStart = 33;

const char ssid[] = "SSID";
const char pass[] = "PASS";

AsyncUDP udp;

int status = WL_IDLE_STATUS;

void setup() {
  pinMode(calcStart, OUTPUT);

  Wire.begin();
  // Increase I2C clock speed to 400kHz.
  Wire.setClock(400000);

  // Fast serial as possible.
  Serial.begin(115200);

  // Check for the WiFi module.
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // Don't continue
    while (true)
      ;
  }

  Serial.print(F("Connecting to SSID: "));
  Serial.println(ssid);

  status = WiFi.begin(ssid, pass);

  delay(1000);

  // Attempt to connect to WiFi network.
  while (status != WL_CONNECTED) {
    delay(500);

    // Connect to WPA/WPA2 network.
    status = WiFi.status();
  }

  if (udp.connect(IPAddress(192, 168, 1, 100), UDP_PORT)) {
    Serial.println("UDP connected");
  }

  if (isConnected() == false) {
    Serial.println("MLX90640 not detected at default I2C address. Please check wiring. Freezing.");
    while (1)
      ;
  }

  // Get device parameters - We only have to do this once.
  int status;
  uint16_t eeMLX90640[832];

  status = MLX90640_DumpEE(MLX90640_address, eeMLX90640);
  if (status != 0) {
    Serial.println("Failed to load system parameters");
  }

  status = MLX90640_ExtractParameters(eeMLX90640, &mlx90640);
  if (status != 0) {
    Serial.println("Parameter extraction failed");
  }

  // Set refresh rate.
  // A rate of 0.5Hz takes 4Sec per reading because we have to read two frames to get complete picture.
  MLX90640_SetRefreshRate(MLX90640_address, 0x04);  //Set rate to 4Hz effective - Works

  // Once EEPROM has been read at 400kHz we can increase to 1MHz.
  Wire.setClock(1000000);  // Run I2C at 800kHz (because of clock division).
}

void loop() {
  if (timeElapsed > sendInterval) {
    for (byte x = 0; x < 2; x++) {
      uint16_t mlx90640Frame[834];
      int status = MLX90640_GetFrameData(MLX90640_address, mlx90640Frame);

      digitalWrite(calcStart, HIGH);
      float vdd = MLX90640_GetVdd(mlx90640Frame, &mlx90640);
      float Ta = MLX90640_GetTa(mlx90640Frame, &mlx90640);

      float tr = Ta - TA_SHIFT;  // Reflected temperature based on the sensor ambient temperature.
      float emissivity = 0.95;

      MLX90640_CalculateTo(mlx90640Frame, &mlx90640, emissivity, tr, mlx90640To);
      digitalWrite(calcStart, LOW);
    }

    // Downsample (x4) to be able to send all the data via UDP.
    char outputString[DOWNSAMPLED_SIZE * 7];
    outputString[0] = '\0';
    for (int row = 0; row < 24; row += DOWNSAMPLE_FACTOR) {
      for (int col = 0; col < 32; col += DOWNSAMPLE_FACTOR) {
        float sum = 0.0;

        // Calculate the average for the current 4x4 block
        for (int i = 0; i < DOWNSAMPLE_FACTOR; i++) {
          for (int j = 0; j < DOWNSAMPLE_FACTOR; j++) {
            int index = (row + i) * 32 + col + j;
            sum += mlx90640To[index];
          }
        }

        char buffer[10];
        sprintf(buffer, "%.2f", sum / (DOWNSAMPLE_FACTOR * DOWNSAMPLE_FACTOR));
        strcat(outputString, buffer);
        strcat(outputString, ",");
      }
    }

    outputString[strlen(outputString) - 1] = '\0';

    // Send UDP message.
    udp.broadcastTo(outputString, UDP_PORT);

    // Reset the time elapsed.
    timeElapsed = 0;
  }
}

// Returns true if the MLX90640 is detected on the I2C bus.
boolean isConnected() {
  Wire.beginTransmission((uint8_t)MLX90640_address);
  if (Wire.endTransmission() != 0)
    return (false);  // Sensor did not ACK.
  return (true);
}
