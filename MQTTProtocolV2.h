/*
纯底层通信，包含了通用通信协议的处理
 */
#ifndef DFROBOT_HUSKEYLENS_V2_MQTT_PROTOCOL_H
#define DFROBOT_HUSKEYLENS_V2_MQTT_PROTOCOL_H
#include <ArduinoJson.h>
#include <ESP32MQTTClient.h>
#include <Result.h>

class MQTTProtocolV2 {
public:
  MQTTProtocolV2();

  bool begin(String &host, uint16_t port, String &username, String &password);

public:
  ~MQTTProtocolV2() {};

  bool knock(void);

  String takePhoto(eResolution_t resolution);
  String takeScreenshot();

  int8_t getResult(eAlgorithm_t algo);
  uint8_t learn(eAlgorithm_t algo);
  uint8_t learnBlock(eAlgorithm_t algo, int16_t x, int16_t y, int16_t width,
                     int16_t height);
  bool forget(eAlgorithm_t algo);

  bool drawUniqueRect(uint32_t color, uint8_t lineWidth, int16_t x, int16_t y,
                      int16_t width, int16_t height);
  bool drawRect(uint32_t color, uint8_t lineWidth, int16_t x, int16_t y,
                int16_t width, int16_t height);
  bool clearRect();
  bool drawText(uint32_t color, uint8_t fontSize, int16_t x, int16_t y,
                String text);
  bool clearText();

  bool saveKnowledges(eAlgorithm_t algo, uint8_t knowledgeID);
  bool loadKnowledges(eAlgorithm_t algo, uint8_t knowledgeID);

  bool playMusic(String name, int16_t volume);
  bool stopMusic();

  bool doSwitchAlgorithm(eAlgorithm_t algo);
  bool doSetMultiAlgorithm(eAlgorithm_t algo0, eAlgorithm_t algo1,
                           eAlgorithm_t algo2 = ALGORITHM_ANY);
  bool setMultiAlgorithmRatio(int8_t ratio0, int8_t ratio1, int8_t ratio2 = -1);

  bool setNameByID(eAlgorithm_t algo, uint8_t id, String name);
  bool getAlgoParamBool(eAlgorithm_t algo, String key);
  float getAlgoParamFloat(eAlgorithm_t algo, String key);
  String getAlgoParamString(eAlgorithm_t algo, String key);

  bool setAlgoParamBool(eAlgorithm_t algo, String key, bool value);
  bool setAlgoParamFloat(eAlgorithm_t algo, String key, float value);
  bool setAlgoParamString(eAlgorithm_t algo, String key, String value);

  bool updateAlgoParams(eAlgorithm_t algo);
  bool startRecording(eMediaType_t mediaType, int16_t duration = -1,
                      String filename = "",
                      eResolution_t resolution = RESOLUTION_DEFAULT);
  bool stopRecording(eMediaType_t mediaType);
  bool sendAndWait(String &command);

private:
  bool getAlgoParam(eAlgorithm_t algo, JsonObject *&outParams,
                    DynamicJsonBuffer &buffer);
  bool setAlgoParam(eAlgorithm_t algo, JsonObject &params);
  bool isResponseSuccess(String &respStr);

public:
  int16_t maxID[ALGORITHM_BUILTIN_COUNT];
  int16_t total_results;
  int16_t total_blocks;
  int16_t total_arrows;
  Result *result_tmp[MAX_RESULT_NUM];
  int16_t retry = 5;
  unsigned long timeOutDuration = 5000;
  unsigned long timeOutTimer;
  uint32_t correlation_id = 0;
};

#endif // DFROBOT_HUSKEYLENS_V2_RESULT_H