#include <U8g2lib.h>
#include <Wire.h>

/*---------------------------------------------------------------
 * OLED hardware configuration
 * The software I2C bus uses the board's GPIO22 and GPIO21 pins.
 *--------------------------------------------------------------*/
#define I2C_SDA 22
#define I2C_SCL 21

// Controls the 128 x 64 SSD1306 OLED without a dedicated reset pin.
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /*clock=*/I2C_SCL, /*data=*/I2C_SDA, /*reset=*/U8X8_PIN_NONE);

/**
 * @brief Initialize the OLED and scroll the title across the screen.
 *
 * Arduino calls this function once after startup or reset. The page loop
 * is required by U8g2 so the complete frame is rendered before the next
 * horizontal text position is drawn.
 *
 * @param None.
 * @return Nothing.
 */
void setup() {
  Serial.begin(115200);

  /*---------------------------------------------------------------
   * Configure text rendering
   * Enable UTF-8 printing and select the font and drawing direction.
   *--------------------------------------------------------------*/
  u8g2.begin();
  u8g2.enableUTF8Print();
  u8g2.setFont(u8g2_font_ncenB14_tr);
  u8g2.setFontDirection(0);

  /*---------------------------------------------------------------
   * Animate the title
   * Move the text from the right edge to the left in 20-pixel steps.
   *--------------------------------------------------------------*/
  for (int i = 128; i > -78; i -= 20)
  {
    u8g2.firstPage();
    do {
      u8g2.drawStr(i, 25, "ELECROW");
      delay(2);
    } while (u8g2.nextPage());
  }
}

/**
 * @brief Keep the sketch idle after the one-time OLED animation.
 *
 * Arduino calls this function repeatedly after setup() finishes.
 *
 * @param None.
 * @return Nothing.
 */
void loop() {
}
