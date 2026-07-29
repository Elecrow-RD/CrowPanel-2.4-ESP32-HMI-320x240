/*---------------------------------------------------------------
 * GPS UART configuration
 * The module uses ESP32 hardware UART2, leaving the USB serial port free
 * for monitoring and sending commands from the computer.
 *--------------------------------------------------------------*/
#define GPS_RX 16
#define GPS_TX 17

// Provides the dedicated hardware serial channel connected to the GPS.
HardwareSerial gpsSerial(2);

// Temporarily stores a block of bytes received from the GPS module.
unsigned char buffer[256];

// Tracks the number of valid bytes currently stored in buffer.
int count = 0;

/**
 * @brief Start the GPS and USB serial ports with matching settings.
 *
 * Arduino calls this function once after startup or reset. UART2 uses
 * 9600 baud, eight data bits, no parity, and one stop bit.
 *
 * @param None.
 * @return Nothing.
 */
void setup()
{
  gpsSerial.begin(9600, SERIAL_8N1, GPS_RX, GPS_TX);
  Serial.begin(9600);
}

/**
 * @brief Bridge data between the GPS module and the serial monitor.
 *
 * Arduino calls this function repeatedly after setup() finishes. GPS data
 * is forwarded in blocks, while bytes entered on the computer are sent
 * directly to the module.
 *
 * @param None.
 * @return Nothing.
 */
void loop()
{
  if (gpsSerial.available())
  {
    while (gpsSerial.available())
    {
      buffer[count++] = gpsSerial.read();

      // Stop before the next byte could exceed the fixed buffer capacity.
      if (count == 256) break;
    }
    Serial.write(buffer, count);
    clearBufferArray();
    count = 0;
  }

  if (Serial.available())
    gpsSerial.write(Serial.read());
}

/**
 * @brief Clear the portion of the receive buffer that was used.
 *
 * loop() calls this function after forwarding a GPS data block. Only the
 * valid range is cleared because the remaining bytes were not modified.
 *
 * @param None.
 * @return Nothing.
 */
void clearBufferArray()
{
  for (int i = 0; i < count; i++)
  {
    buffer[i] = 0;
  }
}
