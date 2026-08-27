#include "DHT20.h"

/**
 * @brief Store the I2C bus and sensor address used by this driver.
 *
 * @param pWire I2C bus object used for sensor communication.
 * @param address 7-bit I2C address of the sensor.
 * @return Nothing.
 * @call The constructor runs when the global DHT20 object is created.
 */
DHT20::DHT20(TwoWire *pWire, uint8_t address)
  : _pWire(pWire), _address(address) {
}

/**
 * @brief Check that the DHT20 responds on the selected I2C bus.
 *
 * @param None.
 * @return 0 when the sensor responds; 1 when the probe fails.
 * @call Call once from setup() after the I2C bus has been started.
 */
int DHT20::begin() {
  uint8_t readCMD[1] = {0x71};
  uint8_t data;
  delay(100);
  writeCommand(readCMD, 1);
  readData(&data, 1);
  if ((data | 0x8) == 0 || data == 255) {
    return 1;
  }
  return 0;
}

/**
 * @brief Request and convert the current temperature measurement.
 *
 * The sensor reports a raw 20-bit value. This method waits for the sensor to
 * finish its conversion and maps that value to degrees Celsius.
 *
 * @param None.
 * @return Temperature in degrees Celsius as an integer.
 * @call Call from loop() when the display needs a new temperature value.
 */
int DHT20::getTemperature() {
  uint8_t readCMD[3] = {0xac, 0x33, 0x00};
  uint8_t data[6] = {0};
  int retries = 10;
  writeCommand(readCMD, 3);
  while (retries--) {
    delay(10);
    readData(data, 6);
    if ((data[0] >> 7) == 0) {
      break;
    }
  }
  uint32_t rawData = ((data[3] & 0xf) << 16) + ((data[4] & 0xff) << 8) + data[5];
  return (int)rawData / 5242 - 50;
}

/**
 * @brief Request and convert the current relative humidity measurement.
 *
 * @param None.
 * @return Relative humidity in percent as an integer.
 * @call Call from loop() when the display needs a new humidity value.
 */
int DHT20::getHumidity() {
  uint8_t readCMD[3] = {0xac, 0x33, 0x00};
  uint8_t data[6] = {0};
  int retries = 10;
  writeCommand(readCMD, 3);
  while (retries--) {
    delay(10);
    readData(data, 6);
    if ((data[0] >> 7) == 0) {
      break;
    }
  }
  uint32_t rawData = ((data[1] & 0xff) << 12) + ((data[2] & 0xff) << 4) + ((data[3] & 0xf0) >> 4);
  return (int)(((float)rawData / 0x100000) * 100);
}

/**
 * @brief Send a command to the DHT20 over I2C.
 *
 * @param pBuf Bytes to transmit.
 * @param size Number of bytes in pBuf.
 * @return Nothing.
 * @call Called internally by the public sensor methods.
 */
void DHT20::writeCommand(const void *pBuf, size_t size) {
  const uint8_t *data = static_cast<const uint8_t *>(pBuf);
  _pWire->beginTransmission(_address);
  for (size_t i = 0; i < size; i++) {
    _pWire->write(data[i]);
  }
  _pWire->endTransmission();
}

/**
 * @brief Read a response buffer from the DHT20 over I2C.
 *
 * @param pBuf Destination buffer for received bytes.
 * @param size Number of bytes to request.
 * @return 1 after the requested bytes have been copied.
 * @call Called internally by the public sensor methods.
 */
uint8_t DHT20::readData(void *pBuf, size_t size) {
  delay(10);
  uint8_t *data = static_cast<uint8_t *>(pBuf);
  _pWire->requestFrom(_address, size);
  for (size_t i = 0; i < size; i++) {
    data[i] = _pWire->read();
  }
  return 1;
}
