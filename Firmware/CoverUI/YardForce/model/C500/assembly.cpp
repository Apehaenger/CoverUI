/**
 * @file C500/assembly.cpp
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
#include "LEDcontrol.hpp"

// ----- LEDs -----

const uint32_t kPinByLedNum[] = {
    // Order derived from LowLevel "enum LED_id"
    //                       C500
    // Use LED         num  Original  Alternative
    LED_PIN_CHARGE,  //  0
    LED_PIN_BAT,     //  1
    LED_PIN_WIRE,    //  2
    LED_PIN_LIFTED,  //  3
    LED_PIN_SUN,     //  4    SUN(6)
    LED_PIN_SAT,     //  5    SAT(5)
    LED_PIN_FRI,     //  6    FRI(4)
    LED_PIN_THU,     //  7    THU(3)
    LED_PIN_WED,     //  8    WED(2)
    LED_PIN_TUE,     //  9    TUE(1)
    LED_PIN_MON,     // 10    MON(0)
    LED_PIN_LOCK,    // 11
    LED_PIN_S2,      // 12              digit 5
    LED_PIN_S1,      // 13              digit 4
    LED_PIN_8HR,     // 14    8HR       digit 3
    LED_PIN_6HR,     // 15    6HR       digit 2
    LED_PIN_4HR,     // 16    4HR       digit 1
    LED_PIN_2HR,     // 17    2HR       digit 0
    LED_PIN_REAR     // 18
};

// Numeric (base10) representation of LEDs.
// I.e. Digit 0 = 4H(0), ... Then S2(5) + S1(4) = 9.
//
//    |------- Bit 6 = MON(0)
//    ||------ Bit 5 = TUE(1)
//    |||----- Bit 4 = WED(2)
//    ||||---- Bit 3 = THU(3)
//    |||||--- Bit 2 = FRI(4)
//    ||||||-- Bit 1 = SAT(5)
//    |||||||- Bit 0 = SUN(6)
const uint8_t kBase10Leds[] = {
    0b1000000,  // = 0
    0b0100000,
    0b0010000,
    0b0001000,
    0b0000100,
    0b0000010,
    0b0000001,
    0b0100001,
    0b0010001,
    0b0001001};  // = 9

// Main LED controller object
LEDcontrolC500 leds(kPinByLedNum, sizeof(kPinByLedNum) / sizeof(uint32_t), &LEDcontrol::set_base10_leds);

// ----- Buttons -----

// Map logic button number to pin. Use the same order as in OM FW so that they get scanned in a comparable order!
const std::map<uint8_t, Buttons::ButtonDef> kBtnDefByButtonNumMap = {
    {BTN_CLK_NUM, {BTN_CLK_PIN, -1}},
    {BTN_HOME_NUM, {BTN_HOME_PIN, -1}},
    {BTN_PLAY_NUM, {BTN_PLAY_PIN, -1}},
    {BTN_S1_NUM, {BTN_S1_PIN, 13}},
    {BTN_S2_NUM, {BTN_S2_PIN, 12}},
    {BTN_LOCK_NUM, {BTN_LOCK_PIN, 11}},
    {BTN_OK_NUM, {BTN_OK_PIN, -1}},
    {BTN_SUN_NUM, {BTN_SUN_PIN, 4}},
    {BTN_MON_NUM, {BTN_MON_PIN, 10}},
    {BTN_TUE_NUM, {BTN_TUE_PIN, 9}},
    {BTN_WED_NUM, {BTN_WED_PIN, 8}},
    {BTN_THU_NUM, {BTN_THU_PIN, 7}},
    {BTN_FRI_NUM, {BTN_FRI_PIN, 6}},
    {BTN_SAT_NUM, {BTN_SAT_PIN, 5}}};

ButtonsC500 buttons(kBtnDefByButtonNumMap);

// ----- Emergency -----
const Emergency::PinStateDef kEmergencyPinStateDefs[] = {
    {PIN_HALL_STOP_WHITE, INPUT, EmergencyState::STOP1},
    {PIN_HALL_STOP_YELLOW, INPUT, EmergencyState::STOP2},
    {PIN_HALL_WHEEL_RED, INPUT, EmergencyState::LIFT},
    {PIN_HALL_WHEEL_BLUE, INPUT, EmergencyState::LIFTX}};

Emergency emergency(kEmergencyPinStateDefs, sizeof(kEmergencyPinStateDefs) / sizeof(Emergency::PinStateDef));
