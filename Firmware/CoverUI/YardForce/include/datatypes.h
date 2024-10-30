// Created by Clemens Elflein on 3/07/22.
// Copyright (c) 2022 Clemens Elflein. All rights reserved.
//
// This work is licensed under a Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
//
// Feel free to use the design in your private/educational projects, but don't try to sell the design or products based on it without getting my consent first.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
//

#ifndef _DATATYPES_H
#define _DATATYPES_H

#include <stdint.h>

#define PACKET_ID_LL_STATUS 1
#define PACKET_ID_LL_IMU 2
#define PACKET_ID_LL_UI_EVENT 3
#define PACKET_ID_LL_HEARTBEAT 0x42
#define PACKET_ID_LL_HIGH_LEVEL_STATE 0x43

enum HighLevelMode {
    MODE_IDLE = 1, // ROS connected, idle mode
    MODE_AUTONOMOUS = 2, // ROS connected, Autonomous mode, either mowing or docking or undocking
    MODE_RECORDING = 3 // ROS connected, Manual mode during recording etc
};

// clang-format off
#define LL_EMERGENCY_BIT_LATCH (1 << 0)  // Any emergency latch
#define LL_EMERGENCY_BIT_STOP  (1 << 1)  // Stop
#define LL_EMERGENCY_BIT_LIFT  (1 << 2)  // Lift (or tilt)

#define LL_EMERGENCY_BIT_CU_LATCH (1 << 0)  // Any emergency latch
#define LL_EMERGENCY_BIT_CU_STOP1 (1 << 1)  // Stop1
#define LL_EMERGENCY_BIT_CU_STOP2 (1 << 2)  // Stop2
#define LL_EMERGENCY_BIT_CU_LIFT  (1 << 3)  // LIFT
#define LL_EMERGENCY_BIT_CU_LBUMP (1 << 4)  // LBUMP
#define LL_EMERGENCY_BIT_CU_LIFTX (1 << 5)  // LIFTX
#define LL_EMERGENCY_BIT_CU_RBUMP (1 << 6)  // RBUMP
// clang-format on

// Same bitmask as in ll_status.emergency_bitmask (LowLevel, datatypes.h)
enum class EmergencyState {
    LATCH = LL_EMERGENCY_BIT_CU_LATCH,
    STOP1 = LL_EMERGENCY_BIT_CU_STOP1,
    STOP2 = LL_EMERGENCY_BIT_CU_STOP2,
    LIFT = LL_EMERGENCY_BIT_CU_LIFT,
    LBUMP = LL_EMERGENCY_BIT_CU_LBUMP,
    LIFTX = LL_EMERGENCY_BIT_CU_LIFTX,
    RBUMP = LL_EMERGENCY_BIT_CU_RBUMP
};

#pragma pack(push, 1)
struct ll_status {
    // Type of this message. Has to be PACKET_ID_LL_STATUS.
    uint8_t type;
    // Bitmask for rain, sound, powers etc
    // Bit 0: Initialized (i.e. setup() was a success). If this is 0, all other bits are meaningless.
    // Bit 1: Raspberry Power
    // Bit 2: Charging enabled
    // Bit 3: don't care
    // Bit 4: Rain detected
    // Bit 5: Sound available
    // Bit 6: Sound busy
    // Bit 7: UI Board available
    uint8_t status_bitmask;
    // USS range in m
    float uss_ranges_m[5];
    // Emergency bitmask:
    // Bit 0: Emergency latch
    // Bit 1: Emergency/Stop
    // Bit 2: Emergency/Lift (or tilt)
    uint8_t emergency_bitmask;
    // Charge voltage
    float v_charge;
    // System voltage
    float v_battery;
    // Charge current
    float charging_current;
    uint8_t batt_percentage;
    uint16_t crc;
} __attribute__((packed));
#pragma pack(pop)

#pragma pack(push, 1)
struct ll_high_level_state {
    // Type of this message. Has to be PACKET_ID_LL_HIGH_LEVEL_STATE
    uint8_t type;
    uint8_t current_mode; // see HighLevelMode
    uint8_t gps_quality;   // GPS quality in percent (0-100)
    uint16_t crc;
} __attribute__((packed));
#pragma pack(pop)

#endif