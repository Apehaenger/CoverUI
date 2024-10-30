/**
 * @file RM-EC3-V11/assembly.cpp
 * @author Apehaenger (joerg@ebeling.ws)
 * @brief YardForce RM-EC3-V1.1 CoverUI Assembly for OpenMower https://github.com/ClemensElflein/OpenMower
 * @version 0.1
 * @date 2024-10-02
 *
 * @copyright Copyright (c) 2024
 *
 */
#include "assembly.hpp"

#include "../../include/Emergency.hpp"
#include "Display.hpp"

// ----- LEDs -----

const uint32_t kPinByLedNum[] = {
    // Order derived from LowLevel "enum LED_id"
    //                       C500
    // Use LED         num  Original  Alternative
    LED_PIN_NC,        //  0
    LED_PIN_NC,        //  1
    LED_PIN_NC,        //  2
    LED_PIN_NC,        //  3
    LED_PIN_NC,        //  4    SUN
    LED_PIN_NC,        //  5    SAT
    LED_PIN_NC,        //  6    FRI
    LED_PIN_NC,        //  7    THU
    LED_PIN_NC,        //  8    WED
    LED_PIN_NC,        //  9    TUE
    LED_PIN_NC,        // 10    MON
    LED_PIN_NC,        // 11
    LED_PIN_S2,        // 12              digit 5
    LED_PIN_S1,        // 13              digit 4
    LED_PIN_NC,        // 14    8HR       digit 3
    LED_PIN_NC,        // 15    6HR       digit 2
    LED_PIN_NC,        // 16    4HR       digit 1
    LED_PIN_NC,        // 17    2HR       digit 0
    LED_PIN_NC,        // 18
    LED_PIN_SETUP,     // 19
    LED_PIN_BACKLIGHT  // 20
};

const unsigned int kLedAnimOrder[] = {13, 19, 12};
const unsigned int kNumFrontLeds = sizeof(kLedAnimOrder) / sizeof(unsigned int);

// Main LED controller object
LEDcontrolRMEC3V11 leds(kPinByLedNum, sizeof(kPinByLedNum) / sizeof(uint32_t), &LEDcontrol::set_base10_leds);

// ----- Buttons -----

// Map logic button number to pin. Use the same order as in OM FW so that they get scanned in a comparable order!
const std::map<uint8_t, Buttons::ButtonDef> kBtnDefByButtonNumMap = {
    {BTN_HOME_NUM, {BTN_HOME_PIN, -1}},
    {BTN_PLAY_NUM, {BTN_PLAY_PIN, -1}},
    {BTN_S1_NUM, {BTN_S1_PIN, LED_NUM_S1}},
    {BTN_S2_NUM, {BTN_S2_PIN, LED_NUM_S2}},
    {BTN_SETUP_NUM, {BTN_SETUP_PIN, 19}},
    {BTN_MENU_NUM, {BTN_MENU_PIN, -1}},
    {BTN_1_NUM, {BTN_1_PIN, -1}},
    {BTN_2_NUM, {BTN_2_PIN, -1}},
    {BTN_3_NUM, {BTN_3_PIN, -1}},
    {BTN_4_NUM, {BTN_4_PIN, -1}},
    {BTN_5_NUM, {BTN_5_PIN, -1}},
    {BTN_6_NUM, {BTN_6_PIN, -1}},
    {BTN_7_NUM, {BTN_7_PIN, -1}},
    {BTN_8_NUM, {BTN_8_PIN, -1}},
    {BTN_9_NUM, {BTN_9_PIN, -1}},
    {BTN_BACK_NUM, {BTN_BACK_PIN, -1}},
    {BTN_0_NUM, {BTN_0_PIN, -1}},
    {BTN_OK_NUM, {BTN_OK_PIN, -1}}};

/**
 * @brief Check if one of the internal button functionality got pressed and do it's function.
 * MENU + 0 = Display FW version
 * MENU + BACK = LED animation
 */
void ButtonsRMEC3V11::handle_sys_req() {
    if (!is_pressed(BTN_MENU_NUM))
        return;

    if (is_pressed(BTN_BACK_NUM))
        display.openmower_anim();
    else if (is_pressed(BTN_0_NUM))
        display.start_anncmnt(2000, yardforce::display::AbstractDisplay::AnncmntType::version);

    return;
};

ButtonsRMEC3V11 buttons(kBtnDefByButtonNumMap);

// ----- Emergency -----

const Emergency::PinStateDef kEmergencyPinStateDefs[] = {
#ifdef MOD_HALL
    {PIN_LIFT, INPUT_PULLUP, EmergencyState::LIFT},
    {PIN_LIFTX, INPUT_PULLUP, EmergencyState::LIFTX},
    {PIN_LBUMP, INPUT_PULLUP, EmergencyState::LBUMP},
    {PIN_RBUMP, INPUT_PULLUP, EmergencyState::RBUMP},
#endif
#ifdef MOD_STOP
    {PIN_STOP1, INPUT_PULLUP, EmergencyState::STOP1},
    {PIN_STOP2, INPUT_PULLUP, EmergencyState::STOP2},
#endif
};

Emergency emergency(kEmergencyPinStateDefs, sizeof(kEmergencyPinStateDefs) / sizeof(Emergency::PinStateDef));

// ----- Hatch -----

unsigned int HatchRMEC3V11::handle_button(unsigned int button_id, unsigned int press_cnt) {
    // If backlight is off, skip first button press
    if (display.check_backlight() == LED_off) {
        display.set_backlight();
        return 0;  // Skip handling of first button-press if backlight was off
    }
    display.set_backlight();

    // Clear emergency = Enter (OK) button
    if (button_id == BTN_OK_NUM) {
        display.start_anncmnt(4000, yardforce::display::AbstractDisplay::AnncmntType::close_hatch);
        queue_button(BTN_LOCK_NUM, 2, 4500);  // Send long-press Lock button in 4.5 sec
        return 0;                             // Don't return button now (hatch is still open)
    }

    // Play & Home button
    if (button_id == BTN_HOME_NUM || button_id == BTN_PLAY_NUM) {
        display.start_anncmnt(4000, yardforce::display::AbstractDisplay::AnncmntType::close_hatch);
        queue_button(BTN_LOCK_NUM, 2, 4500);  // Send long-press Lock button in 4.5 sec
        queue_button(button_id, 0, 5000);
        return 0;  // Don't return button now (hatch is still open)
    }

    if (button_id == BTN_1_NUM)
        return BTN_MON_NUM;  // Volume up

    if (button_id == BTN_2_NUM)
        return BTN_TUE_NUM;  // Volume down

    return button_id;
}

HatchRMEC3V11 hatch;
