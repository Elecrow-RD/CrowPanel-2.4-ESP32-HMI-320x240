#include <TFT_eSPI.h>
TFT_eSPI lcd = TFT_eSPI(); /* TFT实例 */
uint16_t touchX, touchY;
uint16_t calData[5] = { 557, 3263, 369, 3493, 3  };

void setup() {
  // put your setup code here, to run once:
  Serial.begin( 9600 ); /*初始化串口*/
  //LCD初始化
  lcd.begin(); 
  lcd.setRotation(1); /* 旋转 */
  //校准模式。一是四角定位、二是直接输入模拟数值直接定位
  //屏幕校准
//  touch_calibrate();
  lcd.setTouch(calData);

  /*初始化*/
}

void loop() {
  // put your main code here, to run repeatedly:
  bool touched = lcd.getTouch( &touchX, &touchY, 600);
  if ( touched )
  {
    Serial.print( "Data x " );
    Serial.println( touchX );

    Serial.print( "Data y " );
    Serial.println( touchY );
  }
}

void touch_calibrate()//屏幕校准
{
  uint16_t calData[5];
  uint8_t calDataOK = 0;
  Serial.println("屏幕校准");

  //校准
  Serial.println("按指示触摸角落");

 // lv_timer_handler();
  lcd.calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15);
  Serial.println("calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15)");
  Serial.println(); Serial.println();
  Serial.println("//在setup()中使用此校准代码:");
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
