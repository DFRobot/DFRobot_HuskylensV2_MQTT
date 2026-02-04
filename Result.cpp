#include <Result.h>

Result::Result(JsonObject &obj) {
  if (obj.containsKey("id")) {
    ID = obj["id"];
  } else if (obj.containsKey("resolution")) {
    resolution = obj["resolution"];
  } else if (obj.containsKey("boardType")) {
    boardType = obj["boardType"];
  } else if (obj.containsKey("multiAlgoNum")) {
    multiAlgoNum = obj["multiAlgoNum"];
  }

  if (obj.containsKey("level")) {
    level = obj["level"];
  } else if (obj.containsKey("mediaType")) {
    mediaType = obj["mediaType"];
  } else if (obj.containsKey("retValue")) {
    retValue = obj["retValue"];
  } else if (obj.containsKey("retValue")) {
    lineWidth = obj["lineWidth"];
  } else if (obj.containsKey("confidence")) {
    confidence = obj["confidence"];
  }

  if (obj.containsKey("xCenter")) {
    xCenter = obj["xCenter"];
  } else if (obj.containsKey("xTarget")) {
    xTarget = obj["xTarget"];
  } else if (obj.containsKey("duration")) {
    duration = obj["duration"];
  } else if (obj.containsKey("duration")) {
    algorithmType = obj["algorithmType"];
  } else if (obj.containsKey("classID")) {
    classID = obj["classID"];
  } else if (obj.containsKey("total_results")) {
    total_results = obj["total_results"];
  } else if (obj.containsKey("pitch")) {
    pitch = obj["pitch"];
  }

  if (obj.containsKey("yCenter")) {
    yCenter = obj["yCenter"];
  } else if (obj.containsKey("yTarget")) {
    yTarget = obj["yTarget"];
  } else if (obj.containsKey("total_results_learned")) {
    total_results_learned = obj["total_results_learned"];
  } else if (obj.containsKey("yaw")) {
    yaw = obj["yaw"];
  }

  if (obj.containsKey("width")) {
    width = obj["width"];
  } else if (obj.containsKey("angle")) {
    angle = obj["angle"];
  } else if (obj.containsKey("azimuth")) {
    azimuth = obj["azimuth"];
  } else if (obj.containsKey("total_blocks")) {
    total_blocks = obj["total_blocks"];
  } else if (obj.containsKey("roll")) {
    roll = obj["roll"];
  }

  if (obj.containsKey("height")) {
    height = obj["height"];
  } else if (obj.containsKey("length")) {
    length = obj["length"];
  } else if (obj.containsKey("total_blocks_learned")) {
    total_blocks_learned = obj["total_blocks_learned"];
  }

  name = obj["name"].as<String>();
  content = obj["content"].as<String>();
  used = 0;

  DBG_KV("ID=", ID);
  DBG_KV("xCenter=", xCenter);
  DBG_KV("yCenter=", yCenter);
  DBG_KV("width=", width);

  DBG_KV("height=", height);
  DBG_KV("name=", name);
  DBG_KV("content=", content);
}
Result::~Result() {}

int16_t read16(JsonObject &obj, int &offset) {
  JsonArray &arr = obj["extended"].asArray();
  int16_t low = arr[offset++];
  int16_t high = arr[offset++];
  return low + (high << 8);
}

FaceResult::FaceResult(JsonObject &obj) : Result(obj) {
  int16_t *fields[] = {&leye_x, &leye_y,   &reye_x,   &reye_y,   &nose_x,
                       &nose_y, &lmouth_x, &lmouth_y, &rmouth_x, &rmouth_y};

  int offset = 0;
  const size_t count = sizeof(fields) / sizeof(fields[0]);
  for (int i = 0; i < count; i++) {
    *fields[i] = read16(obj, offset);
  }
}
HandResult::HandResult(JsonObject &obj) : Result(obj) {
  int16_t *fields[] = {&wrist_x,
                       &wrist_y,
                       &thumb_cmc_x,
                       &thumb_cmc_y,
                       &thumb_mcp_x,
                       &thumb_mcp_y,
                       &thumb_ip_x,
                       &thumb_ip_y,
                       &thumb_tip_x,
                       &thumb_tip_y,
                       &index_finger_mcp_x,
                       &index_finger_mcp_y,
                       &index_finger_pip_x,
                       &index_finger_pip_y,
                       &index_finger_dip_x,
                       &index_finger_dip_y,
                       &index_finger_tip_x,
                       &index_finger_tip_y,
                       &middle_finger_mcp_x,
                       &middle_finger_mcp_y,
                       &middle_finger_pip_x,
                       &middle_finger_pip_y,
                       &middle_finger_dip_x,
                       &middle_finger_dip_y,
                       &middle_finger_tip_x,
                       &middle_finger_tip_y,
                       &ring_finger_mcp_x,
                       &ring_finger_mcp_y,
                       &ring_finger_pip_x,
                       &ring_finger_pip_y,
                       &ring_finger_dip_x,
                       &ring_finger_dip_y,
                       &ring_finger_tip_x,
                       &ring_finger_tip_y,
                       &pinky_finger_mcp_x,
                       &pinky_finger_mcp_y,
                       &pinky_finger_pip_x,
                       &pinky_finger_pip_y,
                       &pinky_finger_dip_x,
                       &pinky_finger_dip_y,
                       &pinky_finger_tip_x,
                       &pinky_finger_tip_y};

  int offset = 0;
  const size_t count = sizeof(fields) / sizeof(fields[0]);

  for (size_t i = 0; i < count; i++) {
    *fields[i] = read16(obj, offset);
  }
}
PoseResult::PoseResult(JsonObject &obj) : Result(obj) {
  int16_t *fields[] = {
      &nose_x,      &nose_y,      &leye_x,      &leye_y,      &reye_x,
      &reye_y,      &lear_x,      &lear_y,      &rear_x,      &rear_y,
      &lshoulder_x, &lshoulder_y, &rshoulder_x, &rshoulder_y, &lelbow_x,
      &lelbow_y,    &relbow_x,    &relbow_y,    &lwrist_x,    &lwrist_y,
      &rwrist_x,    &rwrist_y,    &lhip_x,      &lhip_y,      &rhip_x,
      &rhip_y,      &lknee_x,     &lknee_y,     &rknee_x,     &rknee_y,
      &lankle_x,    &lankle_y,    &rankle_x,    &rankle_y};

  int offset = 0;
  const size_t count = sizeof(fields) / sizeof(fields[0]);

  for (size_t i = 0; i < count; i++) {
    *fields[i] = read16(obj, offset);
  }
}

Result *result[ALGORITHM_COUNT][MAX_RESULT_NUM];
uint8_t customId[3];
uint8_t customAlgoNum;
int16_t maxID[ALGORITHM_BUILTIN_COUNT];

eAlgorithm_t toRealID(uint8_t id) {
  eAlgorithm_t algo = (eAlgorithm_t)id;
  if (id >= ALGORITHM_CUSTOM_BEGIN) {
    for (uint8_t i = 0; i < CUSTOM_ALGORITHM_COUNT; i++)
      if (customId[i] == id) {
        algo = (eAlgorithm_t)((int)ALGORITHM_CUSTOM0 + i);
        break;
      }
  }
  return algo;
}