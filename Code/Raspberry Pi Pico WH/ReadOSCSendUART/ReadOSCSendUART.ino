#include <WiFi.h>
#include <OSCMessage.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"

#define UART_ID uart0
#define BAUD_RATE 9600
#define TX_PIN 16
#define RX_PIN 17

// Network credentials.
const char SSID[] = "SSID";
const char PASS[] = "PASS";

// UDP settings.
const IPAddress IP(192, 168, 130, 180);
const unsigned int PORT_IN = 51688;
WiFiUDP udp;

void setup() {
  Serial.begin(BAUD_RATE);

   // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(SSID);
  WiFi.begin(SSID, PASS);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  
  Serial.println("Starting UDP");
  udp.begin(PORT_IN);
  Serial.print("Local port: ");
  Serial.println(udp.localPort());
  
  uart_init(UART_ID, BAUD_RATE);
  gpio_set_function(TX_PIN, GPIO_FUNC_UART);
  gpio_set_function(RX_PIN, GPIO_FUNC_UART);

  uart_set_hw_flow(UART_ID, false, false);
  uart_set_format(UART_ID, 8, 1, UART_PARITY_NONE);
}

void loop() {
  OSCMessage msg;
  int size = udp.parsePacket();

  if (size) {
    while (size--) {
      msg.fill(udp.read());
    }
    if (!msg.hasError()) {
      int data = msg.getInt(0);
      uart_putc(UART_ID, char(data));
    }
  }
}
