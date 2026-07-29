#include <WiFi.h>

/*---------------------------------------------------------------
 * Wi-Fi credentials
 * Replace these example values with the local 2.4 GHz network details.
 *--------------------------------------------------------------*/
const char *ssid = "elecrow888";
const char *password = "elecrow2014";

/**
 * @brief Connect the board to Wi-Fi and print its assigned address.
 *
 * Arduino calls this function once after startup or reset. Execution waits
 * here until the access point accepts the connection.
 *
 * @param None.
 * @return Nothing.
 */
void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  WiFi.setAutoReconnect(true);

  // Do not continue until network-dependent code can use a valid link.
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.println("connecting");
  }
  Serial.println("WiFi is connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  //  WiFi.disconnect();
}

/**
 * @brief Keep the sketch idle after the one-time connection test.
 *
 * Arduino calls this function repeatedly after setup() finishes. Automatic
 * reconnection remains active in the Wi-Fi stack.
 *
 * @param None.
 * @return Nothing.
 */
void loop() {
}
