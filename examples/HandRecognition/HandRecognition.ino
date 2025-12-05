#include <DFRobot_HuskylensV2_MQTT.h>
#include <WiFi.h>

#define PRINT_ITEM(item)                                                       \
  Serial.print("result->" #item "=(");                                         \
  Serial.print(result->item##_x);                                              \
  Serial.print(",");                                                           \
  Serial.print(result->item##_y);                                              \
  Serial.println(")");

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
  huskylens.switchAlgorithm(ALGORITHM_HAND_RECOGNITION);
  delay(5000);
}

void loop() {
  while (!huskylens.getResult(ALGORITHM_HAND_RECOGNITION)) {
    delay(100);
  }

  while (huskylens.available(ALGORITHM_HAND_RECOGNITION)) {
    HandResult *result = static_cast<HandResult *>(
        huskylens.popCachedResult(ALGORITHM_HAND_RECOGNITION));

    Serial.print("result->ID=");
    Serial.println(result->ID);
    Serial.print("result->xCenter=");
    Serial.println(result->xCenter);
    Serial.print("result->yCenter=");
    Serial.println(result->yCenter);
    Serial.print("result->width=");
    Serial.println(result->width);
    Serial.print("result->height=");
    Serial.println(result->height);
    Serial.print("result->name=");
    Serial.println(result->name);
    Serial.print("result->content=");
    Serial.println(result->content);

    PRINT_ITEM(wrist);

    PRINT_ITEM(thumb_cmc);
    PRINT_ITEM(thumb_mcp);
    PRINT_ITEM(thumb_ip);
    PRINT_ITEM(thumb_tip);

    PRINT_ITEM(index_finger_mcp);
    PRINT_ITEM(index_finger_pip);
    PRINT_ITEM(index_finger_dip);
    PRINT_ITEM(index_finger_tip);

    PRINT_ITEM(middle_finger_mcp);
    PRINT_ITEM(middle_finger_pip);
    PRINT_ITEM(middle_finger_dip);
    PRINT_ITEM(middle_finger_tip);

    PRINT_ITEM(ring_finger_mcp);
    PRINT_ITEM(ring_finger_pip);
    PRINT_ITEM(ring_finger_dip);
    PRINT_ITEM(ring_finger_tip);

    PRINT_ITEM(pinky_finger_mcp);
    PRINT_ITEM(pinky_finger_pip);
    PRINT_ITEM(pinky_finger_dip);
    PRINT_ITEM(pinky_finger_tip);
  }
  delay(5000);
  delay(5000);
}