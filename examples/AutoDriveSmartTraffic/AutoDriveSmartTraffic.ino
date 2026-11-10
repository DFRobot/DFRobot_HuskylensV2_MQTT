/***************************************************
 HUSKYLENS V2 An Easy-to-use AI Machine Vision Sensor
 <https://www.dfrobot.com/product-1922.html>

 This example demonstrates the auto drive (smart traffic) functionality of
 HUSKYLENS V2 via MQTT interface. It shows how to initialize the sensor, run
 the end-to-end driving model, and print the steering angle and throttle
 values for controlling a car.
 ***************************************************
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
  huskylens.switchAlgorithm(ALGORITHM_AUTO_DRIVE_SMART_TRAFFIC);
  delay(5000);
}

void loop() {
  while (!huskylens.getResult(ALGORITHM_AUTO_DRIVE_SMART_TRAFFIC)) {
    delay(100);
  }

  while (huskylens.available(ALGORITHM_AUTO_DRIVE_SMART_TRAFFIC)) {
    Result *result = huskylens.popCachedResult(ALGORITHM_AUTO_DRIVE_SMART_TRAFFIC);

    Serial.println();

    Serial.print("result->steering=");
    Serial.println(result->steering);
    Serial.print("result->throttle=");
    Serial.println(result->throttle);
  }
  delay(1000);
}
