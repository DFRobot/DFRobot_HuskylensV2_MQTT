/*
为ino提供接口
存储分析result
 */

#ifndef DFROBOT_HUSKYLENS_V2_H
#define DFROBOT_HUSKYLENS_V2_H
#include <Arduino.h>
#include <ESP32MQTTClient.h>
#include <MQTTProtocolV2.h>

#define COLOR_WHITE 0xFFFFFF   // 白色
#define COLOR_RED 0xFF0000     // 红色
#define COLOR_ORANGE 0xFFA500  // 橙色
#define COLOR_YELLOW 0xFFFF00  // 黄色
#define COLOR_GREEN 0x00FF00   // 绿色
#define COLOR_CYAN 0x00FFFF    // 青色
#define COLOR_BLUE 0x0000FF    // 蓝色
#define COLOR_PURPLE 0x800080  // 紫色
#define COLOR_PINK 0xFFC0CB    // 粉色
#define COLOR_GRAY 0x808080    // 灰色
#define COLOR_BLACK 0x000000   // 黑色
#define COLOR_BROWN 0xA52A2A   // 棕色
#define COLOR_OLIVE 0x808000   // 橄榄绿
#define COLOR_TEAL 0x008080    // 蓝绿色
#define COLOR_INDIGO 0x4B0082  // 靛蓝色
#define COLOR_MAGENTA 0xFF00FF // 洋红色

class HuskylensV2_MQTT : public MQTTProtocolV2 {

public:
  HuskylensV2_MQTT();
  ~HuskylensV2_MQTT();

  bool begin(String host, uint16_t port = 1883, String username = "",
             String password = "");

  int8_t getResult(eAlgorithm_t algo);
  bool available(eAlgorithm_t algo);
  bool switchAlgorithm(eAlgorithm_t algo);
  Result *popCachedResult(eAlgorithm_t algo);
  Result *getCachedCenterResult(eAlgorithm_t algo);
  Result *getCachedResultByIndex(eAlgorithm_t algo, int16_t index);
  Result *getCachedResultByID(eAlgorithm_t algo, int16_t ID);
  int16_t getCachedResultNum(eAlgorithm_t algo);
  int16_t getCachedResultLearnedNum(eAlgorithm_t algo);

  int16_t getCachedResultNumByID(eAlgorithm_t algo, uint8_t id);
  Result *getCachedIndexResultByID(eAlgorithm_t algo, uint8_t id,
                                   uint8_t index);
  int16_t getCachedResultMaxID(eAlgorithm_t algo);

  Result *getCurrentBranch(eAlgorithm_t algo);
  int8_t getUpcomingBranchCount(eAlgorithm_t algo);
  Result *getBranch(eAlgorithm_t algo, int16_t index);
  bool setMultiAlgorithm(eAlgorithm_t algo0, eAlgorithm_t algo1,
                         eAlgorithm_t algo2 = ALGORITHM_ANY);
};

#define RET_ITEM_NUM(func, type, item)                                         \
  [](type *rlt) { return rlt ? rlt->item : -1; }(static_cast<type *>(func))

#define RET_ITEM_STR(func, type, item)                                         \
  [](type *rlt) { return rlt ? rlt->item : ""; }(static_cast<type *>(func))

#endif // DFROBOT_HUSKYLENS_V2_H