/***************************************************
 HUSKYLENS V2 An Easy-to-use AI Machine Vision Sensor
 <https://www.dfrobot.com/product-1922.html>

 ***************************************************
 This example demonstrates how to record video with HUSKYLENS
 V2 via I2C interface. The code initializes the device and shows how to use
 startRecording() and stopRecording() functions to record video files with
 different durations.

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
  huskylens.switchAlgorithm(ALGORITHM_RECORDING_VIDEO);
  delay(5000);
}

void loop() {
  bool ret = huskylens.startRecording(MEDIA_TYPE_VIDEO, 8, "test8s.mp4");
  Serial.print("startRecording test8s.mp4 ret=");
  Serial.println(ret);
  delay(12 * 1000);

  ret = huskylens.startRecording(MEDIA_TYPE_VIDEO, 0, "test5s.mp4");
  Serial.print("startRecording test5s.mp4 ret=");
  Serial.println(ret);
  delay(5000);

  ret = huskylens.stopRecording(MEDIA_TYPE_VIDEO);
  Serial.print("stopRecording ret=");
  Serial.println(ret);
  delay(5000);
  while (1) {
    delay(100);
  }
}