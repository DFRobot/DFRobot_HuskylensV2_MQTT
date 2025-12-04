/***************************************************
 HUSKYLENS V2 An Easy-to-use AI Machine Vision Sensor
 <https://www.dfrobot.com/product-1922.html>

 ***************************************************
 This example shows the basic function of library for HUSKYLENS V2 via I2c.

 Created 2025-07-04
 By [Ouki Wang](ouki.wang@dfrobot.com)

 GNU Lesser General Public License.
 See <http://www.gnu.org/licenses/> for details.
 All above must be included in any redistribution
 ****************************************************/

/***********Notice and Trouble shooting***************
 1.Connection and Diagram can be found here
 <https://wiki.dfrobot.com/HUSKYLENS_V1.0_SKU_SEN0305_SEN0336#target_23>
 2.This code is tested on Arduino Uno, Leonardo, Mega boards.
 ****************************************************/

#include <DFRobot_HuskylensV2.h>

// HUSKYLENS green line >> SDA; blue line >> SCL
HuskylensV2 huskylens;

void setup() {
    Serial.begin(115200);

    Wire.begin();
    while (!huskylens.begin(Wire)) {
        Serial.println(F("Begin failed!"));
        Serial.println(F("1.Please recheck the \"Protocol Type\" in HUSKYLENS (System Settings>>Protocol Type>> I2C/UART)"));
        Serial.println(F("2.Please recheck the connection."));
        Serial.println(F("\tgreen line >> SDA/TX; blue line >> SCL/RX"));
        delay(100);
    }
}

void loop() {
    while (!huskylens.getResult(ALGORITHM_LINE_TRACKING)) {
        delay(100);
    }
    Serial.println("\n\n\n\n");
    
    if (huskylens.available(ALGORITHM_LINE_TRACKING)) {
        //当前路径信息
        Serial.println("CurrentBranch  level=1");
        
        Serial.print("xTarget=");  
        Serial.println(RET_ITEM_NUM(huskylens.getCurrentBranch(ALGORITHM_LINE_TRACKING), Result, xTarget));
        Serial.print("yTarget=");  
        Serial.println(RET_ITEM_NUM(huskylens.getCurrentBranch(ALGORITHM_LINE_TRACKING), Result, yTarget));
        Serial.print("angle=");  
        Serial.println(RET_ITEM_NUM(huskylens.getCurrentBranch(ALGORITHM_LINE_TRACKING), Result, angle));
        Serial.print("length=");  
        Serial.println(RET_ITEM_NUM(huskylens.getCurrentBranch(ALGORITHM_LINE_TRACKING), Result, length));

        //前方路口数目
        uint8_t n = huskylens.getUpcomingBranchCount(ALGORITHM_LINE_TRACKING);
        Serial.print("UpcomingBranchCount      =     ");
        Serial.println(n);

        //逆时针取出前方路口的每条分支
        for (uint8_t i = 0; i < n; i++){
            Serial.println("\n\nlevel=2");
            Serial.print("xTarget=");
            Serial.println(RET_ITEM_NUM(huskylens.getBranch(ALGORITHM_LINE_TRACKING , i), Result, xTarget));
            Serial.print("yTarget=");
            Serial.println(RET_ITEM_NUM(huskylens.getBranch(ALGORITHM_LINE_TRACKING , i), Result, yTarget));
            Serial.print("angle=");
            Serial.println(RET_ITEM_NUM(huskylens.getBranch(ALGORITHM_LINE_TRACKING , i), Result, angle));
            Serial.print("length=");
            Serial.println(RET_ITEM_NUM(huskylens.getBranch(ALGORITHM_LINE_TRACKING , i), Result, length));
        }
    }
    delay(1000);
}

