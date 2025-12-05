/***************************************************
 HUSKYLENS V2 An Easy-to-use AI Machine Vision Sensor
 <https://www.dfrobot.com/product-1922.html>

 ***************************************************
 This example shows how to use HUSKYLENS V2's object classification algorithm
 via I2C to detect and classify objects, then print their names and class IDs.

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
  huskylens.switchAlgorithm(ALGORITHM_OBJECT_CLASSIFICATION);
  delay(5000);
}

void loop() {
  while (!huskylens.getResult(ALGORITHM_OBJECT_CLASSIFICATION)) {
    delay(100);
  }

  while (huskylens.available(ALGORITHM_OBJECT_CLASSIFICATION)) {
    Result *result = huskylens.popCachedResult(ALGORITHM_OBJECT_CLASSIFICATION);
    Serial.print("result->name=");
    Serial.println(result->name);
    Serial.print("result->classID=");
    Serial.println(result->classID);
    Serial.println();
  }
  delay(1000);
}