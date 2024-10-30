/**
 * @file RM-ECOW-V100/assembly.cpp
 * @author Apehaenger (joerg@ebeling.ws)
 * @brief YardForce RM-ECOW-V1.0.0 CoverUI Assembly for OpenMower https://github.com/ClemensElflein/OpenMower
 * @version 0.2
 * @date 2024-10-30
 *
 * @copyright Copyright (c) 2024
 *
 */
#include "assembly.hpp"

#include "../../include/Emergency.hpp"

// ----- LEDs -----

const uint32_t kPinByLedNum[] = {
    // Order derived from LowLevel "enum LED_id"
    //                       C500
    // Use LED         num  Original  Alternative
    LED_PIN_CHARGE,  //  0
    LED_PIN_BAT,     //  1
    LED_PIN_WIRE,    //  2
    LED_PIN_LIFTED,  //  3
    LED_PIN_NC,      //  4    SUN
    LED_PIN_NC,      //  5    SAT
    LED_PIN_NC,      //  6    FRI
    LED_PIN_NC,      //  7    THU
    LED_PIN_NC,      //  8    WED
    LED_PIN_NC,      //  9    TUE
    LED_PIN_NC,      // 10    MON
    LED_PIN_LOCK,    // 11
    LED_PIN_S2,      // 12              digit 5
    LED_PIN_S1,      // 13              digit 4
    LED_PIN_10H,     // 14    8HR       digit 3
    LED_PIN_8H,      // 15    6HR       digit 2
    LED_PIN_6H,      // 16    4HR       digit 1
    LED_PIN_4H,      // 17    2HR       digit 0
    LED_PIN_REAR,    // 18
};

const unsigned int kLedAnimOrder[] = {3, 2, 1, 0, 17, 16, 15, 14, 13, 12, 11};
const unsigned int kNumFrontLeds = sizeof(kLedAnimOrder) / sizeof(unsigned int);

// Numeric (base10) representation of LEDs.
// I.e. Digit 0 = 4H(0), ... Then S2(5) + S1(4) = 9.
//
//    |------ Bit 5 =  4H(0)
//    ||----- Bit 4 =  6H(1)
//    |||---- Bit 3 =  8H(2)
//    ||||--- Bit 2 = 10H(3)
//    |||||-- Bit 1 =  S1(4)
//    ||||||- Bit 0 =  S2(5)
const uint8_t kBase10Leds[] = {
    0b100000,  // = 0
    0b010000,
    0b001000,
    0b000100,
    0b000010,
    0b000001,
    0b010001,
    0b001001,
    0b000101,
    0b000011};  // = 9

// Main LED controller object
LEDcontrolRMECOWV1x leds(kPinByLedNum, sizeof(kPinByLedNum) / sizeof(uint32_t), &LEDcontrol::set_base10_leds);

// ----- Buttons -----

// Map logic button number to pin. Use the same order as in OM FW so that they get scanned in a comparable order!
const std::map<uint8_t, Buttons::ButtonDef> kBtnDefByButtonNumMap = {
    {BTN_HOME_NUM, {BTN_HOME_PIN, -1}},
    {BTN_PLAY_NUM, {BTN_PLAY_PIN, -1}},
    {BTN_S1_NUM, {BTN_S1_PIN, LED_NUM_S1}},
    {BTN_S2_NUM, {BTN_S2_PIN, LED_NUM_S2}},
    {BTN_LOCK_NUM, {BTN_LOCK_PIN, 11}},
    {BTN_4H_NUM, {BTN_4H_PIN, 17}},
    {BTN_6H_NUM, {BTN_6H_PIN, 16}},
    {BTN_8H_NUM, {BTN_8H_PIN, 15}},
    {BTN_10H_NUM, {BTN_10H_PIN, 14}}
};

ButtonsRMECOWV110 buttons(kBtnDefByButtonNumMap);

// ----- Emergency -----
const Emergency::PinStateDef kEmergencyPinStateDefs[] = {
#ifdef MOD_HALL
    {PIN_LIFT, INPUT_PULLUP, EmergencyState::LIFT},
    {PIN_LIFTX, INPUT_PULLUP, EmergencyState::LIFTX},
    {PIN_LBUMP, INPUT_PULLUP, EmergencyState::LBUMP},
    {PIN_RBUMP, INPUT_PULLUP, EmergencyState::RBUMP},
#endif
#ifdef MOD_STOP
    {PIN_STOP_JP6, INPUT_PULLUP, EmergencyState::STOP1},
    {PIN_STOP_JP8, INPUT_PULLUP, EmergencyState::STOP2},
#endif
};

Emergency emergency(kEmergencyPinStateDefs, sizeof(kEmergencyPinStateDefs) / sizeof(Emergency::PinStateDef));

// ----- Hatch -----
HatchRMECOWV1x hatch;
