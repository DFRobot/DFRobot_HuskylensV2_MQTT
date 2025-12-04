/***************************************************
 HUSKYLENS V2 An Easy-to-use AI Machine Vision Sensor
 <https://www.dfrobot.com/product-1922.html>

 ***************************************************
 This example demonstrates the drawing and text display capabilities
 of HUSKYLENS V2 via I2C interface. It shows how to clear the screen,
 draw text, draw rectangles, and create animated rectangle displays.

 Created 2025-07-04
 By [Ouki Wang](ouki.wang@dfrobot.com)

 GNU Lesser General Public License.
 See <http://www.gnu.org/licenses/> for details.
 All above must be included in any redistribution
 ****************************************************/

/***********Notice and Trouble shooting***************
 1.Connection and Diagram can be found here
 <https://wiki.dfrobot.com/HUSKYLENS_V1.0_SKU_SEN0305_SEN0336#target_23>
 2.This code is tested on Arduino Uno, Leonardo, Mega, Microbit,ESP8266,ESP32
 boards.
 ****************************************************/
#include <DFRobot_HuskylensV2_MQTT.h>
#include <WiFi.h>

const char *ssid = "wanghui";
const char *pass = "dfrobot2011";

HuskylensV2_MQTT huskylens;

void setup() {
  Serial.begin(115200);
  delay(5000);
  Serial.println("setup, ESP.getSdkVersion(): ");
  Serial.println(ESP.getSdkVersion());
  log_i();
  log_i("setup, ESP.getSdkVersion(): ");
  log_i("%s", ESP.getSdkVersion());

  WiFi.begin(ssid, pass);
  WiFi.setHostname("c3test");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  huskylens.begin("192.168.2.102", 1883, "huskylens", "dfrobot");
}

void loop() {
  huskylens.clearText();
  delay(2000);
  huskylens.drawText(COLOR_RED, 20, 10, 10, "DFRobot Test");
  delay(2000);
  huskylens.clearRect();
  delay(2000);
  huskylens.drawRect(COLOR_GREEN, 4, 10, 10, 200, 30);
  delay(2000);
  for (int i = 0; i < 150; i++) {
    huskylens.drawUniqueRect(COLOR_GREEN, 4, 50, 50, 102 + i * 2, 102 + i * 2);
    delay(5);
  }
}