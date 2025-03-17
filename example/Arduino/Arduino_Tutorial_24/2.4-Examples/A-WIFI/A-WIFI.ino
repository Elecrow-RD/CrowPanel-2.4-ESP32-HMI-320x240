#include <WiFi.h>

const char *ssid = "elecrow888"; //你的网络名称
const char *password = "elecrow2014"; //你的网络密码
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  WiFi.setAutoReconnect(true);//设置自动重连
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.println("connecting");
  }
  Serial.println("WiFi is connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
//  WiFi.disconnect();//断开WIFI
}

void loop() {

}
