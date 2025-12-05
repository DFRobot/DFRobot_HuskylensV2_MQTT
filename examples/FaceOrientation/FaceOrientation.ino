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
  huskylens.switchAlgorithm(ALGORITHM_FACE_ORIENTATION);
  delay(5000);
}

void loop() {
  while (!huskylens.getResult(ALGORITHM_FACE_ORIENTATION)) {
    delay(100);
  }

  while (huskylens.available(ALGORITHM_FACE_ORIENTATION)) {
    Result *result = static_cast<Result *>(
        huskylens.popCachedResult(ALGORITHM_FACE_ORIENTATION));

    Serial.print("result->ID=");
    Serial.println(result->ID);

    Serial.print("result->pitch=");
    Serial.println(result->pitch);
    Serial.print("result->yaw=");
    Serial.println(result->yaw);
    Serial.print("result->roll=");
    Serial.println(result->roll);

    Serial.print("result->name=");
    Serial.println(result->name);
    Serial.print("result->content=");
    Serial.println(result->content);
  }
  delay(1000);
}