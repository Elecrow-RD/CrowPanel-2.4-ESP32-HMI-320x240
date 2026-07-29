#include <TFT_eSPI.h>

// Controls both the LCD panel and its resistive touch interface.
TFT_eSPI lcd = TFT_eSPI();

// Stores the most recent calibrated touch position.
uint16_t touchX, touchY;

// Converts raw touch measurements into coordinates for this display.
uint16_t calData[5] = { 557, 3263, 369, 3493, 3 };

/**
 * @brief Initialize serial output, the LCD, and touch calibration data.
 *
 * Arduino calls this function once after startup or reset. The saved
 * calibration values are applied so touches can be reported immediately.
 *
 * @param None.
 * @return Nothing.
 */
void setup() {
  Serial.begin(9600);
  lcd.begin();
  lcd.setRotation(1);

  /*---------------------------------------------------------------
   * Select the touch calibration method
   * Run touch_calibrate() to measure a new panel, or apply the saved
   * values for normal use. Only the saved-value path is active here.
   *--------------------------------------------------------------*/
  //  touch_calibrate();
  lcd.setTouch(calData);
}

/**
 * @brief Report valid touch coordinates to the serial monitor.
 *
 * Arduino calls this function repeatedly after setup() finishes. A touch
 * is accepted only when TFT_eSPI passes the configured pressure threshold.
 *
 * @param None.
 * @return Nothing.
 */
void loop() {
  bool touched = lcd.getTouch(&touchX, &touchY, 600);
  if (touched)
  {
    Serial.print("Data x ");
    Serial.println(touchX);

    Serial.print("Data y ");
    Serial.println(touchY);
  }
}

/**
 * @brief Guide the user through touch calibration and print the result.
 *
 * Call this function from setup() when the panel requires new calibration
 * values. The printed array can then replace the saved calData values.
 *
 * @param None.
 * @return Nothing.
 */
void touch_calibrate()
{
  uint16_t calData[5];
  uint8_t calDataOK = 0;
  Serial.println("Touch-screen calibration");

  Serial.println("Please touch the corners as directed");

  //  lv_timer_handler();
  lcd.calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15);
  Serial.println("calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15)");
  Serial.println(); Serial.println();
  Serial.println("//Use this calibration code in setup():");
  Serial.print("uint16_t calData[5] = ");
  Serial.print("{ ");

  for (uint8_t i = 0; i < 5; i++)
  {
    Serial.print(calData[i]);
    if (i < 4) Serial.print(", ");
  }

  Serial.println(" };");
  Serial.print("  tft.setTouch(calData);");
  Serial.println(); Serial.println();
}
