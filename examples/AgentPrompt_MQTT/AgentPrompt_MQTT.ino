/***************************************************
 HUSKYLENS V2 An Easy-to-use AI Machine Vision Sensor
 <https://www.dfrobot.com/product-1922.html>

 This example demonstrates the xiaozhi agent chat workflow of HUSKYLENS V2
 over MQTT: send a text prompt, poll the agent status
 (idle/processing/broadcasting/disabled) until the response is ready, then
 fetch and print the full response.

 Unlike the UART version, neither the prompt nor the response is fragmented:
 each is transferred in a single MQTT message (request on topic "usr/cmd",
 response on topic "usr/response").

 The MIT License (MIT).
 See <https://opensource.org/licenses/MIT> for details.
 All above must be included in any redistribution
 ****************************************************/

#include <DFRobot_HuskylensV2_MQTT.h>
#include <WiFi.h>

const char *ssid = "your_ssid";
const char *pass = "your_password";

HuskylensV2_MQTT huskylens;

// Total polling timeout: the cloud may take tens of seconds to generate a
// long answer; give up this round on timeout to avoid an endless loop
#define AGENT_POLL_TIMEOUT_MS 90000
#define AGENT_POLL_INTERVAL_MS 500

// Convert the enum status to a readable string for serial monitoring
static const char *agentStatusStr(eAgentStatus_t status) {
  switch (status) {
  case AGENT_PROCESSING:
    return "processing";   // Submitted, waiting for the cloud to generate
  case AGENT_BROADCASTING:
    return "broadcasting"; // Broadcasting the answer
  case AGENT_DISABLED:
    return "disabled";     // The agent switch is not turned on
  case AGENT_IDLE:
  default:
    return "idle";
  }
}

// Send one prompt and print the whole answer, return false on any failure
static bool runAgentRound(const String &prompt) {
  if (!huskylens.sendAgentPrompt(prompt)) {
    Serial.println("[AgentPrompt] Send prompt failed");
    return false;
  }

  // Wait for the device to start processing before checking the ready flag:
  // right after sending a prompt the agent has not entered processing yet,
  // so a stale ready state from the previous round may be read, which would
  // skip polling and fetch the answer immediately
  delay(2000);

  // Poll until the answer is ready or the total timeout expires
  uint32_t start = millis();
  while (!huskylens.isAgentResponseReady()) {
    eAgentStatus_t status = huskylens.getAgentStatus();
    if (status == AGENT_DISABLED) {
      Serial.print(F("agent status: "));
      Serial.println(agentStatusStr(status));
      return false;
    } else if (status != AGENT_QUERY_FAILED) {
      Serial.print(F("agent status: "));
      Serial.println(agentStatusStr(status));
    } else {
      Serial.println(F("agent status: <query failed>"));
    }
    if (millis() - start > AGENT_POLL_TIMEOUT_MS) {
      Serial.println("[AgentPrompt] Poll timeout, give up");
      return false;
    }
    delay(AGENT_POLL_INTERVAL_MS);
  }

  String response = huskylens.getAgentResponse();
  if (response.length() > 0) {
    Serial.println("[AgentPrompt] Response : " + response);
    return true;
  }
  Serial.println("[AgentPrompt] Response failed");
  return false;
}

void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  while (!huskylens.begin("192.168.2.102", 1883, "huskylens", "dfrobot")) {
    Serial.println("Begin failed! Please recheck the MQTT broker address.");
    delay(1000);
  }
}

void loop() {
  Serial.println();
  Serial.println("[AgentPrompt] Start ...");

  runAgentRound("tell me a long joke");
  delay(5000);

  runAgentRound("Turn on the flashlight");
  delay(5000);

  runAgentRound("Turn off the flashlight");
  delay(5000);
}
