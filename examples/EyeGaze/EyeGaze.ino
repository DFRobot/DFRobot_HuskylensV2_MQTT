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
  while (!huskylens.getResult(ALGORITHM_GAZE_RECOGNITION)) {
    delay(100);
  }

  while (huskylens.available(ALGORITHM_GAZE_RECOGNITION)) {
    Result *result = static_cast<Result *>(
        huskylens.popCachedResult(ALGORITHM_GAZE_RECOGNITION));

    Serial.print("result->ID=");
    Serial.println(result->ID);

    Serial.print("result->pitch=");
    Serial.println(result->pitch);
    Serial.print("result->yaw=");
    Serial.println(result->yaw);

    // Azimuth: the horizontal projection angle.
    // 0 degrees points to the right side of the screen, increasing
    // counterclockwise.
    Serial.print("result->azimuth=");
    Serial.println(result->azimuth);
    // The spatial unit length is 432. 'length' is the projected length.
    Serial.print("result->length=");
    Serial.println(result->length);

    Serial.print("result->name=");
    Serial.println(result->name);
    Serial.print("result->content=");
    Serial.println(result->content);
  }
  delay(1000);
}