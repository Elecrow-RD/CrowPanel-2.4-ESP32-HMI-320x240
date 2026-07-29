#include "BLEDevice.h"
#include "BLEServer.h"
#include "BLEUtils.h"
#include "BLE2902.h"
#include <BLECharacteristic.h>

/*---------------------------------------------------------------
 * BLE service configuration
 * The advertised service exposes one readable, writable, and notifiable
 * characteristic under fixed UUIDs.
 *--------------------------------------------------------------*/
#define bleServerName "ESP32SPI-BLE"
#define SERVICE_UUID "6479571c-2e6d-4b34-abe9-c35116712345"
#define CHARACTERISTIC_UUID "826f072d-f87c-4ae6-a416-6ffdcaa02d73"

// References the advertising controller created during setup().
BLEAdvertising* pAdvertising = NULL;

// References the BLE server that accepts central-device connections.
BLEServer* pServer = NULL;

// References the custom service published by this example.
BLEService *pService = NULL;

// References the data endpoint contained in the custom service.
BLECharacteristic* pCharacteristic = NULL;

// Records whether a BLE central is currently connected.
bool connected_state = false;

/**
 * @brief Track BLE connection state changes reported by the server.
 */
class MyServerCallbacks: public BLEServerCallbacks
{
    /**
     * @brief Record a successful central-device connection.
     *
     * The BLE stack invokes this callback whenever a central connects.
     *
     * @param pServer Server that accepted the connection.
     * @return Nothing.
     */
    void onConnect(BLEServer *pServer)
    {
      connected_state = true;
    }

    /**
     * @brief Record that the central device has disconnected.
     *
     * The BLE stack invokes this callback when the active link closes.
     *
     * @param pServer Server whose connection closed.
     * @return Nothing.
     */
    void onDisconnect(BLEServer *pServer)
    {
      connected_state = false;
    }
};

/**
 * @brief Create and advertise the BLE service and characteristic.
 *
 * Arduino calls this function once after startup or reset. Advertising
 * makes the board discoverable under bleServerName.
 *
 * @param None.
 * @return Nothing.
 */
void setup() {
  Serial.begin(115200);

  /*---------------------------------------------------------------
   * Build the GATT server
   * The characteristic supports the three operations demonstrated by
   * common BLE scanner applications.
   *--------------------------------------------------------------*/
  BLEDevice::init(bleServerName);
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY);
  pCharacteristic->setValue("ELECROW");

  /*---------------------------------------------------------------
   * Publish the service
   * Advertising includes the service UUID so a scanner can identify the
   * example before opening a connection.
   *--------------------------------------------------------------*/
  pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->start();
  pService->start();
  //  pAdvertising->stop();
  //  pService->stop();
}

/**
 * @brief Keep the sketch idle while the BLE stack handles events.
 *
 * Arduino calls this function repeatedly after setup() finishes.
 *
 * @param None.
 * @return Nothing.
 */
void loop() {
}
