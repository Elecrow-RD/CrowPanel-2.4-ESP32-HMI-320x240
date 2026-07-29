/*---------------------------------------------------------------
 * LED hardware configuration
 * GPIO25 drives the on-board LED used by this example.
 *--------------------------------------------------------------*/
#define D_PIN 25

/**
 * @brief Prepare the serial port and LED output.
 *
 * Arduino calls this function once after the board starts or resets.
 *
 * @param None.
 * @return Nothing.
 */
void setup() {
  Serial.begin(115200);
  pinMode(D_PIN, OUTPUT);
}

/**
 * @brief Blink the LED with equal on and off intervals.
 *
 * Arduino calls this function repeatedly after setup() finishes. The
 * two 500 ms delays produce a complete one-second blink cycle.
 *
 * @param None.
 * @return Nothing.
 */
void loop() {
  digitalWrite(D_PIN, HIGH);
  delay(500);
  digitalWrite(D_PIN, LOW);
  delay(500);
}
