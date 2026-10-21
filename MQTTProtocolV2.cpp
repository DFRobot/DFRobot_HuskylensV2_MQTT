#include "MQTTProtocolV2.h"
#include "esp_idf_version.h" // check IDF version
#include <XiaozhiTextCheck.h>
#include <map>
String server = "mqtt://192.168.2.102:1883";

char *subscribeTopic = "usr/response";
char *publishTopic = "usr/cmd";

static String host = "192.168.2.102";
static uint16_t port = 1883;
static String username = "huskylens";
static String password = "dfrobot";

static volatile bool responseReady;
static String responsePayload;
static bool volatile connected = false;

ESP32MQTTClient mqttClient;
void subscribeCB(const std::string &topic, const std::string &payload) {
  log_i("Global callback: %s: %s", topic.c_str(), payload.c_str());
  DynamicJsonBuffer jsonBuffer;
  JsonObject &root = jsonBuffer.parseObject(payload.c_str());
  if (!root.success()) {
    log_e("deserializeJson() failed");
    return;
  }
  const char *cmd = root["cmd"];
  Serial.println(cmd);
}

MQTTProtocolV2::MQTTProtocolV2() {};
void gbegin(String _host, uint16_t _port, String _username, String _password) {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;
  host = _host;
  port = _port;
  username = _username;
  password = _password;
  server = "mqtt://" + host + ":" + String(port);
  log_i("%s(): %d host: %s, port: %d, username: %s, password: %s", __func__,
        __LINE__, host.c_str(), port, username.c_str(), password.c_str());
  mqttClient.enableDebuggingMessages();
  // agent 的 prompt/回答不分片，单条消息可能超过 4KB
  mqttClient.setMaxPacketSize(8192);
  mqttClient.setURI(server.c_str(), username.c_str(), password.c_str());
  mqttClient.enableLastWillMessage("lwt", "I am going offline");
  mqttClient.setKeepAlive(30);
  log_i("%s(): %d", __func__, __LINE__);
  mqttClient.loopStart();
  log_i("%s(): %d", __func__, __LINE__);
}
bool MQTTProtocolV2::begin(String &_host, uint16_t _port, String &_username,
                           String &_password) {
  gbegin(_host, _port, _username, _password);
  uint32_t start = millis();
  while (!connected) {
    if (millis() - start > 5000) {
      return false; // 超时
    }
    delay(1);
  }
  return true;
}

bool MQTTProtocolV2::knock(void) { return true; }
int8_t MQTTProtocolV2::getResult(eAlgorithm_t algo) {
  log_i("%s(): %d", __func__, __LINE__);
  DynamicJsonBuffer jsonBuffer;
  JsonObject &root = jsonBuffer.createObject();
  root["cmd"] = "get_result";
  root["peer"] = "arduino";
  root["algorithm"] = (uint8_t)algo;
  root["correlation_id"] = "arduino-mqtt-" + String(correlation_id++);
  String jsonStr;
  root.printTo(jsonStr);
  bool ret = sendAndWait(jsonStr);
  log_i("%s(): %d ret: %d", __func__, __LINE__, (int)ret);
  if (ret) {
    log_i("%s(): %d response: %s", __func__, __LINE__, responsePayload.c_str());
    DynamicJsonBuffer jsonBuffer;
    JsonObject &resp = jsonBuffer.parseObject(responsePayload.c_str());
    if (!resp.success()) {
      log_e("deserializeJson() failed");
      return false;
    }
    const char *pret = resp["ret"];
    log_i("ret: %s", pret);
    const char *cmd = resp["cmd"];
    log_i("cmd: %s", cmd);
    if (strcmp(pret, "success") == 0) {
      maxID[(int)algo] = resp["max_id"];
      JsonArray &results = resp["results"];
      for (uint8_t i = 0; i < results.size(); i++) {
        JsonObject &item = results[i];
        if (algo == ALGORITHM_FACE_RECOGNITION) {
          result_tmp[i] = new FaceResult(item);
        } else if (algo == ALGORITHM_HAND_RECOGNITION) {
          result_tmp[i] = new HandResult(item);
        } else if (algo == ALGORITHM_POSE_RECOGNITION) {
          result_tmp[i] = new PoseResult(item);
        } else {
          result_tmp[i] = new Result(item);
        }
      }
      return true;
    }
  }
  return false;
}
uint8_t MQTTProtocolV2::learn(eAlgorithm_t algo) {
  bool ret = false;
  log_i("%s(): %d", __func__, __LINE__);
  {
    DynamicJsonBuffer jsonBuffer;
    JsonObject &root = jsonBuffer.createObject();
    root["cmd"] = "learn";
    root["peer"] = "arduino";
    root["algorithm"] = (uint8_t)algo;
    root["correlation_id"] = "arduino-mqtt-" + String(correlation_id++);
    String jsonStr;
    root.printTo(jsonStr);
    ret = sendAndWait(jsonStr);
  }
  log_i("%s(): %d ret: %d", __func__, __LINE__, (int)ret);
  if (!ret) {
    return 0;
  }

  log_i("%s(): %d response: %s", __func__, __LINE__, responsePayload.c_str());
  DynamicJsonBuffer jsonBuffer;
  JsonObject &resp = jsonBuffer.parseObject(responsePayload.c_str());

  if (!resp.success()) {
    log_e("deserializeJson() failed");
    return 0;
  }

  if (!resp["ret"].as<String>().equals("success")) {
    log_e("ret is not success , is %s", resp["ret"].as<String>().c_str());
    return 0;
  }
  return resp["id"].as<uint8_t>();
}
uint8_t MQTTProtocolV2::learnBlock(eAlgorithm_t algo, int16_t x, int16_t y,
                                   int16_t width, int16_t height) {
  bool ret = false;
  log_i("%s(): %d", __func__, __LINE__);
  {
    DynamicJsonBuffer jsonBuffer;
    JsonObject &root = jsonBuffer.createObject();
    root["cmd"] = "learn_block";
    root["peer"] = "arduino";
    root["algorithm"] = (uint8_t)algo;
    root["x"] = x;
    root["y"] = y;
    root["width"] = width;
    root["height"] = height;
    root["correlation_id"] = "arduino-mqtt-" + String(correlation_id++);
    String jsonStr;
    root.printTo(jsonStr);
    ret = sendAndWait(jsonStr);
  }

  log_i("%s(): %d ret: %d", __func__, __LINE__, (int)ret);
  if (!ret) {
    return false;
  }

  log_i("%s(): %d response: %s", __func__, __LINE__, responsePayload.c_str());
  DynamicJsonBuffer jsonBuffer;
  JsonObject &resp = jsonBuffer.parseObject(responsePayload.c_str());

  if (!resp.success()) {
    log_e("deserializeJson() failed");
    return 0;
  }

  if (!resp["ret"].as<String>().equals("success")) {
    log_e("ret is not success , is %s", resp["ret"].as<String>().c_str());
    return 0;
  }
  return resp["id"].as<uint8_t>();
}
bool MQTTProtocolV2::forget(eAlgorithm_t algo) {
  log_i("%s(): %d", __func__, __LINE__);
  DynamicJsonBuffer jsonBuffer;
  JsonObject &root = jsonBuffer.createObject();
  root["cmd"] = "forget";
  root["peer"] = "arduino";
  root["algorithm"] = (uint8_t)algo;
  root["correlation_id"] = "arduino-mqtt-" + String(correlation_id++);

  String jsonStr;
  root.printTo(jsonStr);
  bool ret = sendAndWait(jsonStr);
  log_i("%s(): %d ret: %d", __func__, __LINE__, (int)ret);

  if (!ret) {
    return false;
  }
  return isResponseSuccess(responsePayload);
}
bool MQTTProtocolV2::doSwitchAlgorithm(eAlgorithm_t algo) {
  log_i("%s(): %d", __func__, __LINE__);
  DynamicJsonBuffer jsonBuffer;
  JsonObject &root = jsonBuffer.createObject();
  root["cmd"] = "switch_algorithm";
  root["peer"] = "arduino";
  root["algorithm"] = (uint8_t)algo;
  root["correlation_id"] = "arduino-mqtt-" + String(correlation_id++);

  String jsonStr;
  root.printTo(jsonStr);
  bool ret = sendAndWait(jsonStr);
  log_i("%s(): %d ret: %d", __func__, __LINE__, (int)ret);

  if (!ret) {
    return false;
  }
  return isResponseSuccess(responsePayload);
}

bool MQTTProtocolV2::drawUniqueRect(uint32_t color, uint8_t lineWidth,
                                    int16_t x, int16_t y, int16_t width,
                                    int16_t height) {
  log_i("%s(): %d", __func__, __LINE__);
  DynamicJsonBuffer jsonBuffer;
  JsonObject &root = jsonBuffer.createObject();
  root["cmd"] = "draw_unique_rect";
  root["peer"] = "arduino";
  JsonArray &arr = root.createNestedArray("color");
  arr.add((color >> 16) & 0xFF);
  arr.add((color >> 8) & 0xFF);
  arr.add(color & 0xFF);
  root["line_width"] = lineWidth;
  root["x"] = x;
  root["y"] = y;
  root["width"] = width;
  root["height"] = height;
  root["correlation_id"] = "arduino-mqtt-" + String(correlation_id++);

  String jsonStr;
  root.printTo(jsonStr);
  bool ret = sendAndWait(jsonStr);
  log_i("%s(): %d ret: %d", __func__, __LINE__, (int)ret);

  if (!ret) {
    return false;
  }
  return isResponseSuccess(responsePayload);
}
bool MQTTProtocolV2::drawRect(uint32_t color, uint8_t lineWidth, int16_t x,
                              int16_t y, int16_t width, int16_t height) {
  log_i("%s(): %d", __func__, __LINE__);
  DynamicJsonBuffer jsonBuffer;
  JsonObject &root = jsonBuffer.createObject();
  root["cmd"] = "draw_rect";
  root["peer"] = "arduino";
  JsonArray &arr = root.createNestedArray("color");
  arr.add((color >> 16) & 0xFF);
  arr.add((color >> 8) & 0xFF);
  arr.add(color & 0xFF);
  root["line_width"] = lineWidth;
  root["x"] = x;
  root["y"] = y;
  root["width"] = width;
  root["height"] = height;
  root["correlation_id"] = "arduino-mqtt-" + String(correlation_id++);

  String jsonStr;
  root.printTo(jsonStr);
  bool ret = sendAndWait(jsonStr);
  log_i("%s(): %d ret: %d", __func__, __LINE__, (int)ret);

  if (!ret) {
    return false;
  }
  return isResponseSuccess(responsePayload);
}
bool MQTTProtocolV2::clearRect() {
  log_i("%s(): %d", __func__, __LINE__);
  DynamicJsonBuffer jsonBuffer;
  JsonObject &root = jsonBuffer.createObject();
  root["cmd"] = "clear_rect";
  root["peer"] = "arduino";
  root["correlation_id"] = "arduino-mqtt-" + String(correlation_id++);

  String jsonStr;
  root.printTo(jsonStr);
  bool ret = sendAndWait(jsonStr);
  log_i("%s(): %d ret: %d", __func__, __LINE__, (int)ret);

  if (!ret) {
    return false;
  }
  return isResponseSuccess(responsePayload);
}
bool MQTTProtocolV2::drawText(uint32_t color, uint8_t fontSize, int16_t x,
                              int16_t y, String text) {
  log_i("%s(): %d", __func__, __LINE__);
  DynamicJsonBuffer jsonBuffer;
  JsonObject &root = jsonBuffer.createObject();
  root["cmd"] = "draw_text";
  root["peer"] = "arduino";
  root["font_size"] = fontSize;
  root["text"] = text;

  JsonArray &arr = root.createNestedArray("color");
  arr.add((color >> 16) & 0xFF);
  arr.add((color >> 8) & 0xFF);
  arr.add(color & 0xFF);

  root["x"] = x;
  root["y"] = y;
  root["correlation_id"] = "arduino-mqtt-" + String(correlation_id++);

  String jsonStr;
  root.printTo(jsonStr);
  bool ret = sendAndWait(jsonStr);
  log_i("%s(): %d ret: %d", __func__, __LINE__, (int)ret);

  if (!ret) {
    return false;
  }
  return isResponseSuccess(responsePayload);
}
bool MQTTProtocolV2::clearText() {
  log_i("%s(): %d", __func__, __LINE__);
  DynamicJsonBuffer jsonBuffer;
  JsonObject &root = jsonBuffer.createObject();
  root["cmd"] = "clear_text";
  root["peer"] = "arduino";
  root["correlation_id"] = "arduino-mqtt-" + String(correlation_id++);

  String jsonStr;
  root.printTo(jsonStr);
  bool ret = sendAndWait(jsonStr);
  log_i("%s(): %d ret: %d", __func__, __LINE__, (int)ret);

  if (!ret) {
    return false;
  }
  return isResponseSuccess(responsePayload);
}
bool MQTTProtocolV2::saveKnowledges(eAlgorithm_t algo, uint8_t knowledgeID) {
  log_i("%s(): %d", __func__, __LINE__);
  DynamicJsonBuffer jsonBuffer;
  JsonObject &root = jsonBuffer.createObject();
  root["cmd"] = "save_knowledges";
  root["peer"] = "arduino";
  root["algorithm"] = (uint8_t)algo;
  root["knowledges_id"] = knowledgeID;
  root["correlation_id"] = "arduino-mqtt-" + String(correlation_id++);

  String jsonStr;
  root.printTo(jsonStr);
  bool ret = sendAndWait(jsonStr);

  log_i("%s(): %d ret: %d", __func__, __LINE__, (int)ret);
  if (!ret) {
    return false;
  }
  return isResponseSuccess(responsePayload);
}
bool MQTTProtocolV2::loadKnowledges(eAlgorithm_t algo, uint8_t knowledgeID) {
  log_i("%s(): %d", __func__, __LINE__);
  DynamicJsonBuffer jsonBuffer;
  JsonObject &root = jsonBuffer.createObject();
  root["cmd"] = "load_knowledges";
  root["peer"] = "arduino";
  root["algorithm"] = (uint8_t)algo;
  root["knowledges_id"] = knowledgeID;
  root["correlation_id"] = "arduino-mqtt-" + String(correlation_id++);

  String jsonStr;
  root.printTo(jsonStr);
  bool ret = sendAndWait(jsonStr);

  log_i("%s(): %d ret: %d", __func__, __LINE__, (int)ret);
  if (!ret) {
    return false;
  }
  return isResponseSuccess(responsePayload);
}

bool MQTTProtocolV2::playMusic(String name, int16_t volume) {
  log_i("%s(): %d", __func__, __LINE__);
  DynamicJsonBuffer jsonBuffer;
  JsonObject &root = jsonBuffer.createObject();
  root["cmd"] = "play_music";
  root["peer"] = "arduino";
  root["algorithm"] = 0;
  root["volume"] = volume;
  root["filename"] = name;
  root["correlation_id"] = "arduino-mqtt-" + String(correlation_id++);

  String jsonStr;
  root.printTo(jsonStr);
  bool ret = sendAndWait(jsonStr);
  log_i("%s(): %d ret: %d", __func__, __LINE__, (int)ret);

  if (!ret) {
    return false;
  }
  return isResponseSuccess(responsePayload);
}
bool MQTTProtocolV2::sendAndWait(String &command) {
  log_i("%s(): %d command: %s", __func__, __LINE__, command.c_str());
  responseReady = false;
  responsePayload = "";
  mqttClient.publish(publishTopic, command.c_str(), 0, false);
  uint32_t start = millis();
  while (!responseReady) {
    if (millis() - start > 5000) {
      return false; // 超时
    }
    delay(1);
  }
  return true;
}
bool MQTTProtocolV2::stopMusic() { return true; }
bool MQTTProtocolV2::setNameByID(eAlgorithm_t algo, uint8_t id, String name) {
  log_i("%s(): %d", __func__, __LINE__);
  DynamicJsonBuffer jsonBuffer;
  JsonObject &root = jsonBuffer.createObject();
  root["cmd"] = "set_name_by_id";
  root["peer"] = "arduino";
  root["algorithm"] = (uint8_t)algo;
  root["id"] = id;
  root["name"] = name;
  root["correlation_id"] = "arduino-mqtt-" + String(correlation_id++);

  String jsonStr;
  root.printTo(jsonStr);
  bool ret = sendAndWait(jsonStr);
  log_i("%s(): %d ret: %d", __func__, __LINE__, (int)ret);

  if (!ret) {
    return false;
  }
  return isResponseSuccess(responsePayload);
}

bool MQTTProtocolV2::doSetMultiAlgorithm(eAlgorithm_t algo0, eAlgorithm_t algo1,
                                         eAlgorithm_t algo2) {
  log_i("%s(): %d", __func__, __LINE__);
  DynamicJsonBuffer jsonBuffer;
  JsonObject &root = jsonBuffer.createObject();
  root["cmd"] = "set_multi_algorithm";
  root["peer"] = "arduino";
  JsonArray &arr = root.createNestedArray("algorithms");
  if (algo0 != ALGORITHM_ANY)
    arr.add((uint8_t)algo0);
  if (algo1 != ALGORITHM_ANY)
    arr.add((uint8_t)algo1);
  if (algo2 != ALGORITHM_ANY)
    arr.add((uint8_t)algo2);
  root["correlation_id"] = "arduino-mqtt-" + String(correlation_id++);

  String jsonStr;
  root.printTo(jsonStr);
  bool ret = sendAndWait(jsonStr);
  log_i("%s(): %d ret: %d", __func__, __LINE__, (int)ret);

  if (!ret) {
    return false;
  }
  return isResponseSuccess(responsePayload);
}
bool MQTTProtocolV2::setMultiAlgorithmRatio(int8_t ratio0, int8_t ratio1,
                                            int8_t ratio2) {
  log_i("%s(): %d", __func__, __LINE__);
  DynamicJsonBuffer jsonBuffer;
  JsonObject &root = jsonBuffer.createObject();
  root["cmd"] = "set_multi_ratio";
  root["peer"] = "arduino";
  JsonArray &arr = root.createNestedArray("ratios");
  if (ratio0 != -1)
    arr.add(ratio0);
  if (ratio1 != -1)
    arr.add(ratio1);
  if (ratio2 != -1)
    arr.add(ratio2);
  root["correlation_id"] = "arduino-mqtt-" + String(correlation_id++);

  String jsonStr;
  root.printTo(jsonStr);
  bool ret = sendAndWait(jsonStr);
  log_i("%s(): %d ret: %d", __func__, __LINE__, (int)ret);

  if (!ret) {
    return false;
  }
  return isResponseSuccess(responsePayload);
}

bool MQTTProtocolV2::getAlgoParam(eAlgorithm_t algo, JsonObject *&outParams,
                                  DynamicJsonBuffer &buffer) {
  log_i("%s(): %d", __func__, __LINE__);

  JsonObject &root = buffer.createObject();
  root["cmd"] = "get_algorithm_params";
  root["peer"] = "arduino";
  root["algorithm"] = (uint8_t)algo;
  root["correlation_id"] = "arduino-mqtt-" + String(correlation_id++);

  String jsonStr;
  root.printTo(jsonStr);

  if (!sendAndWait(jsonStr))
    return false;

  log_i("%s(): %d response: %s", __func__, __LINE__, responsePayload.c_str());

  JsonObject &resp = buffer.parseObject(responsePayload.c_str());
  if (!resp.success()) {
    log_e("deserializeJson() failed");
    return false;
  }

  if (resp["ret"].as<String>() != "success") {
    log_e("params ret not success, ret: %s", resp["ret"].as<String>().c_str());
    return false;
  }
  JsonObject &resp_params =
      buffer.parseObject(resp["params"].as<String>().c_str());
  outParams = &resp_params;
  return true;
}

bool MQTTProtocolV2::getAlgoParamBool(eAlgorithm_t algo, String key) {
  DynamicJsonBuffer buffer;
  JsonObject *params = nullptr;
  if (getAlgoParam(algo, params, buffer)) {
    return (*params)[key.c_str()]["value"].as<bool>();
  }
  log_i("key: %s not found", key.c_str());
  return false;
}

float MQTTProtocolV2::getAlgoParamFloat(eAlgorithm_t algo, String key) {
  log_i("%s(): %d", __func__, __LINE__);
  DynamicJsonBuffer buffer;
  JsonObject *params = nullptr;
  if (getAlgoParam(algo, params, buffer)) {
    return (*params)[key.c_str()]["value"].as<float>();
  }
  log_i("key: %s not found", key.c_str());
  return 0.0f;
}

String MQTTProtocolV2::getAlgoParamString(eAlgorithm_t algo, String key) {
  log_i("%s(): %d", __func__, __LINE__);
  DynamicJsonBuffer buffer;
  JsonObject *params = nullptr;
  if (getAlgoParam(algo, params, buffer)) {
    return (*params)[key.c_str()]["value"].as<String>();
  }
  log_i("key: %s not found", key.c_str());
  return "";
}

bool MQTTProtocolV2::setAlgoParam(eAlgorithm_t algo, JsonObject &params) {
  bool ret;
  log_i("%s(): %d", __func__, __LINE__);
  {
    DynamicJsonBuffer jsonBuffer;
    JsonObject &root = jsonBuffer.createObject();

    root["cmd"] = "set_algorithm_params";
    root["peer"] = "arduino";
    root["algorithm"] = (uint8_t)algo;

    JsonArray &arr = root.createNestedArray("params");
    arr.add(params);

    root["correlation_id"] = "arduino-mqtt-" + String(correlation_id++);

    String jsonStr;
    root.printTo(jsonStr);
    ret = sendAndWait(jsonStr);
    log_i("%s(): %d ret: %d", __func__, __LINE__, (int)ret);
  }
  if (!ret) {
    return false;
  }
  return isResponseSuccess(responsePayload);
}

bool MQTTProtocolV2::setAlgoParamBool(eAlgorithm_t algo, String key,
                                      bool value) {
  DynamicJsonBuffer jsonBuffer;
  JsonObject &params = jsonBuffer.createObject();
  params[key] = value;
  return setAlgoParam(algo, params);
}

bool MQTTProtocolV2::setAlgoParamFloat(eAlgorithm_t algo, String key,
                                       float value) {
  DynamicJsonBuffer jsonBuffer;
  JsonObject &params = jsonBuffer.createObject();
  params[key] = value;
  return setAlgoParam(algo, params);
}
bool MQTTProtocolV2::setAlgoParamString(eAlgorithm_t algo, String key,
                                        String value) {
  DynamicJsonBuffer jsonBuffer;
  JsonObject &params = jsonBuffer.createObject();
  params[key] = value;
  return setAlgoParam(algo, params);
}
bool MQTTProtocolV2::updateAlgoParams(eAlgorithm_t algo) { return true; }
bool MQTTProtocolV2::startRecording(eMediaType_t mediaType, int16_t duration,
                                    String filename, eResolution_t resolution) {
  log_i("%s(): %d", __func__, __LINE__);
  DynamicJsonBuffer jsonBuffer;
  JsonObject &root = jsonBuffer.createObject();
  root["filename"] = filename;
  root["peer"] = "arduino";
  if (mediaType == MEDIA_TYPE_AUDIO) {
    root["cmd"] = "start_recording_audio";
  } else if (mediaType == MEDIA_TYPE_VIDEO) {
    root["cmd"] = "start_recording_video";
    root["resolution"] = "default";
  }
  root["duration"] = duration;
  root["correlation_id"] = "arduino-mqtt-" + String(correlation_id++);

  String jsonStr;
  root.printTo(jsonStr);
  bool ret = sendAndWait(jsonStr);
  log_i("%s(): %d ret: %d", __func__, __LINE__, (int)ret);
  if (!ret) {
    return false;
  }
  return isResponseSuccess(responsePayload);
}
bool MQTTProtocolV2::stopRecording(eMediaType_t mediaType) {
  log_i("%s(): %d", __func__, __LINE__);
  DynamicJsonBuffer jsonBuffer;
  JsonObject &root = jsonBuffer.createObject();
  if (mediaType == MEDIA_TYPE_AUDIO) {
    root["cmd"] = "stop_recording_audio";
  } else if (mediaType == MEDIA_TYPE_VIDEO) {
    root["cmd"] = "stop_recording_video";
  }
  root["peer"] = "arduino";
  root["correlation_id"] = "arduino-mqtt-" + String(correlation_id++);

  String jsonStr;
  root.printTo(jsonStr);
  bool ret = sendAndWait(jsonStr);
  log_i("%s(): %d ret: %d", __func__, __LINE__, (int)ret);
  if (!ret) {
    return false;
  }
  return isResponseSuccess(responsePayload);
}

// 小智平台 listen/detect 文本长度校验（2026-10-20 约 300 组实测标定的
// v5 模型，与 DFRobot_HuskylensV2 库行为一致）：先按 UTF-8 码点分类
// （汉字类/重型非ASCII/字母空格/数字/句号/其他标点），再按加权等效数
// （zh_eff = ceil(汉字类 + 1.5×重型), en_eff = 字母 + ceil(数字×1.125)
// + 3×句号 + 5×标点）和混合预算表判定。宁可误拒，绝不误放。
AgentPromptLengthInfo_t MQTTProtocolV2::checkAgentPromptLength(
    const String &prompt) {
  xiaozhi::MeasureResult m =
      xiaozhi::measure(prompt.c_str(), prompt.length());
  AgentPromptLengthInfo_t info = {xiaozhi::can_send(m) ? AGENT_PROMPT_OK
                                                       : AGENT_PROMPT_TOO_LONG,
                                  m.zh_eff, m.en_eff, m.hanzi,  m.heavy,
                                  m.letter, m.digit,  m.period, m.punct,
                                  m.cjk_punct};
  return info;
}

bool MQTTProtocolV2::canSendAgentPrompt(const String &prompt) {
  return xiaozhi::can_send(prompt.c_str(), prompt.length());
}

bool MQTTProtocolV2::sendAgentPrompt(const String &prompt,
                                     eAgentType_t agentType) {
  log_i("%s(): %d", __func__, __LINE__);
  if (prompt.length() == 0) {
    log_e("missing prompt");
    return false;
  }
  if (prompt.length() > 4096) {
    log_e("prompt too large: %d bytes", prompt.length());
    return false;
  }
  // 仅小智平台有 listen/detect 长度限制, 发送前校验文本,
  // 超限直接拒绝, 避免平台告警或静默丢弃(宁可误拒, 绝不误放)
  if (eXIAOZHI_AGENT == agentType &&
      !xiaozhi::can_send(prompt.c_str(), prompt.length())) {
    log_e("prompt exceeds xiaozhi listen/detect length limit, rejected");
    return false;
  }
  DynamicJsonBuffer jsonBuffer;
  JsonObject &root = jsonBuffer.createObject();
  root["cmd"] = "send_agent_prompt";
  root["peer"] = "arduino";
  root["agent_type"] = (uint8_t)agentType;
  root["prompt"] = prompt;
  root["correlation_id"] = "arduino-mqtt-" + String(correlation_id++);

  String jsonStr;
  root.printTo(jsonStr);
  bool ret = sendAndWait(jsonStr);
  log_i("%s(): %d ret: %d", __func__, __LINE__, (int)ret);

  if (!ret) {
    return false;
  }
  return isResponseSuccess(responsePayload);
}

eAgentStatus_t MQTTProtocolV2::getAgentStatus(eAgentType_t agentType) {
  bool ret = false;
  log_i("%s(): %d", __func__, __LINE__);
  {
    DynamicJsonBuffer jsonBuffer;
    JsonObject &root = jsonBuffer.createObject();
    root["cmd"] = "get_agent_status";
    root["peer"] = "arduino";
    root["agent_type"] = (uint8_t)agentType;
    root["correlation_id"] = "arduino-mqtt-" + String(correlation_id++);

    String jsonStr;
    root.printTo(jsonStr);
    ret = sendAndWait(jsonStr);
    log_i("%s(): %d ret: %d", __func__, __LINE__, (int)ret);
  }
  if (!ret) {
    return AGENT_QUERY_FAILED;
  }

  log_i("%s(): %d response: %s", __func__, __LINE__, responsePayload.c_str());
  DynamicJsonBuffer jsonBuffer;
  JsonObject &resp = jsonBuffer.parseObject(responsePayload.c_str());

  if (!resp.success()) {
    log_e("deserializeJson() failed");
    return AGENT_QUERY_FAILED;
  }

  if (!resp["ret"].as<String>().equals("success")) {
    log_e("ret is not success , is %s", resp["ret"].as<String>().c_str());
    return AGENT_QUERY_FAILED;
  }

  String state = resp["state"].as<String>();
  if (state.equals("processing")) {
    return AGENT_PROCESSING;
  } else if (state.equals("broadcasting")) {
    return AGENT_BROADCASTING;
  } else if (state.equals("disabled")) {
    return AGENT_DISABLED;
  } else if (!state.equals("idle")) {
    log_w("unknown agent state: %s", state.c_str());
  }
  return AGENT_IDLE;
}

bool MQTTProtocolV2::isAgentResponseReady(eAgentType_t agentType) {
  bool ret = false;
  log_i("%s(): %d", __func__, __LINE__);
  {
    DynamicJsonBuffer jsonBuffer;
    JsonObject &root = jsonBuffer.createObject();
    root["cmd"] = "is_agent_response_ready";
    root["peer"] = "arduino";
    root["agent_type"] = (uint8_t)agentType;
    root["correlation_id"] = "arduino-mqtt-" + String(correlation_id++);

    String jsonStr;
    root.printTo(jsonStr);
    ret = sendAndWait(jsonStr);
    log_i("%s(): %d ret: %d", __func__, __LINE__, (int)ret);
  }
  if (!ret) {
    return false;
  }

  log_i("%s(): %d response: %s", __func__, __LINE__, responsePayload.c_str());
  DynamicJsonBuffer jsonBuffer;
  JsonObject &resp = jsonBuffer.parseObject(responsePayload.c_str());

  if (!resp.success()) {
    log_e("deserializeJson() failed");
    return false;
  }

  if (!resp["ret"].as<String>().equals("success")) {
    log_e("ret is not success , is %s", resp["ret"].as<String>().c_str());
    return false;
  }
  return resp["ready"].as<bool>();
}

String MQTTProtocolV2::getAgentResponse(eAgentType_t agentType) {
  bool ret = false;
  log_i("%s(): %d", __func__, __LINE__);
  {
    DynamicJsonBuffer jsonBuffer;
    JsonObject &root = jsonBuffer.createObject();
    root["cmd"] = "get_agent_response";
    root["peer"] = "arduino";
    root["agent_type"] = (uint8_t)agentType;
    root["correlation_id"] = "arduino-mqtt-" + String(correlation_id++);

    String jsonStr;
    root.printTo(jsonStr);
    ret = sendAndWait(jsonStr);
    log_i("%s(): %d ret: %d", __func__, __LINE__, (int)ret);
  }
  if (!ret) {
    return "";
  }

  log_i("%s(): %d response: %s", __func__, __LINE__, responsePayload.c_str());
  DynamicJsonBuffer jsonBuffer;
  JsonObject &resp = jsonBuffer.parseObject(responsePayload.c_str());

  if (!resp.success()) {
    log_e("deserializeJson() failed");
    return "";
  }

  if (!resp["ret"].as<String>().equals("success")) {
    log_e("ret is not success , is %s", resp["ret"].as<String>().c_str());
    return "";
  }
  return resp["response"].as<String>();
}

String MQTTProtocolV2::takePhoto(eResolution_t resolution) {
  bool ret = false;
  log_i("%s(): %d", __func__, __LINE__);
  {
    DynamicJsonBuffer jsonBuffer;
    JsonObject &root = jsonBuffer.createObject();

    std::map<eResolution_t, String> resolutionMap = {
        {RESOLUTION_DEFAULT, "640x480"},
        {RESOLUTION_640x480, "640x480"},
        {RESOLUTION_1280x720, "1280x720"},
        {RESOLUTION_1920x1080, "1920x1080"},
    };
    root["cmd"] = "take_photo";
    root["peer"] = "arduino";
    root["resolution"] = resolutionMap[resolution];
    root["correlation_id"] = "arduino-mqtt-" + String(correlation_id++);

    String jsonStr;
    root.printTo(jsonStr);
    ret = sendAndWait(jsonStr);
    log_i("%s(): %d ret: %d", __func__, __LINE__, (int)ret);
  }
  if (!ret) {
    return "";
  }

  log_i("%s(): %d response: %s", __func__, __LINE__, responsePayload.c_str());
  DynamicJsonBuffer jsonBuffer;
  JsonObject &resp = jsonBuffer.parseObject(responsePayload.c_str());

  if (!resp.success()) {
    log_e("deserializeJson() failed");
    return "";
  }

  if (!resp["ret"].as<String>().equals("success")) {
    log_e("ret is not success , is %s", resp["ret"].as<String>().c_str());
    return "";
  }
  return resp["filename"].as<String>();
}

String MQTTProtocolV2::takeScreenshot() {
  bool ret = false;
  log_i("%s(): %d", __func__, __LINE__);
  {
    DynamicJsonBuffer jsonBuffer;
    JsonObject &root = jsonBuffer.createObject();
    root["cmd"] = "take_screenshot";
    root["correlation_id"] = "arduino-mqtt-" + String(correlation_id++);

    String jsonStr;
    root.printTo(jsonStr);
    ret = sendAndWait(jsonStr);
    log_i("%s(): %d ret: %d", __func__, __LINE__, (int)ret);
  }
  if (!ret) {
    return "";
  }

  log_i("%s(): %d response: %s", __func__, __LINE__, responsePayload.c_str());
  DynamicJsonBuffer jsonBuffer;
  JsonObject &resp = jsonBuffer.parseObject(responsePayload.c_str());

  if (!resp.success()) {
    log_e("deserializeJson() failed");
    return "";
  }

  if (!resp["ret"].as<String>().equals("success")) {
    log_e("ret is not success , is %s", resp["ret"].as<String>().c_str());
    return "";
  }
  return resp["filename"].as<String>();
}

bool MQTTProtocolV2::isResponseSuccess(String &respStr) {
  log_i("%s(): %d response: %s", __func__, __LINE__, respStr.c_str());
  DynamicJsonBuffer jsonBuffer;
  JsonObject &resp = jsonBuffer.parseObject(respStr.c_str());

  if (!resp.success()) {
    log_e("deserializeJson() failed");
    return false;
  }
  if (!resp["ret"].as<String>().equals("success")) {
    log_e("ret is not success , is %s", resp["ret"].as<String>().c_str());
    return false;
  }
  return true;
}

void onMqttConnect(esp_mqtt_client_handle_t client) {
  log_i("%s(): %d", __func__, __LINE__);
  if (mqttClient.isMyTurn(client)) // can be omitted if only one client
  {
    connected = true;
    log_i("%s(): %d", __func__, __LINE__);
    mqttClient.subscribe(subscribeTopic, [](const std::string &payload) {
      log_i("payload length = %d", payload.length());
      responsePayload = String(payload.c_str());
      responseReady = true;
    });
  }
}

#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 0, 0)
esp_err_t handleMQTT(esp_mqtt_event_handle_t event) {
  mqttClient.onEventCallback(event);
  return ESP_OK;
}
#else  // IDF CHECK
void handleMQTT(void *handler_args, esp_event_base_t base, int32_t event_id,
                void *event_data) {
  auto *event = static_cast<esp_mqtt_event_handle_t>(event_data);
  mqttClient.onEventCallback(event);
  if (event->event_id == MQTT_EVENT_CONNECTED) {
    connected = true;
  } else if (event->event_id == MQTT_EVENT_DISCONNECTED) {
    connected = false;
  }
}
#endif // // IDF CHECK