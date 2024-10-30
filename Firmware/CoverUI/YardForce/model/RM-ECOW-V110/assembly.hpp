/**
 * @file RM-ECOW-V110/assembly.hpp
 * @author Apehaenger (joerg@ebeling.ws)
 * @brief YardForce RM-ECOW-V1.0.0 CoverUI Assembly for OpenMower https://github.com/ClemensElflein/OpenMower
 * @version 0.1
 * @date 2024-10-02
 *
 * @copyright Copyright (c) 2024
 *
 */
#ifndef YARDFORCE_MDL_RMECOWV100_ASSEMBLY_HPP
#define YARDFORCE_MDL_RMECOWV100_ASSEMBLY_HPP

#include "../../include/Buttons.hpp"
#include "../../include/main.h"
#include "../RM-ECOW-V1x/LEDcontrol.hpp"

#ifdef MOD_EMERGENCY
#include "../../include/Emergency.hpp"
#endif

// ***** LEDs *****
// 1st row: Lifted, Wire (WLAN), Battery empty, Charge
#define LED_PIN_LIFTED PA0
#define LED_PIN_WIRE PA4
#define LED_PIN_BAT PC4
#define LED_PIN_CHARGE PA6
// 2nd row: 4, 6, 8, 10HR
#define LED_PIN_4H PA7
#define LED_PIN_6H PC5
#define LED_PIN_8H PB1
#define LED_PIN_10H PB10
// 3rd row: S1, Reserved for Setup (WLAN), S2
#define LED_PIN_S1 PA5
#define LED_PIN_S2 PC7
// 4th row: Lock
#define LED_PIN_LOCK PA1
// Backside
#define LED_PIN_REAR PB0

// ***** Buttons *****
#define BTN_S1_PIN PB11
#define BTN_S2_PIN PC6
#define BTN_LOCK_PIN PB12
#define BTN_4H_PIN PB14
#define BTN_6H_PIN PB15
#define BTN_8H_PIN PB13
#define BTN_10H_PIN PB2
#define BTN_PLAY_PIN PC11
#define BTN_HOME_PIN PC12

// ***** Emergency *****
#define PIN_STOP_JP6 PC10
#define PIN_STOP_JP8 PA15

#define PIN_LIFT PA8  // LIFT
#define PIN_LIFTX PA9  // LIFTX
#define PIN_LBUMP PC8  // LBUMP
#define PIN_RBUMP PC9  // RBUMP

extern LEDcontrolRMECOWV1x leds;

class ButtonsRMECOWV110 : public Buttons {
   public:
    ButtonsRMECOWV110(const std::map<uint8_t, ButtonDef> &t_kBtnDefByNumMap) : Buttons(t_kBtnDefByNumMap) {};

    /**
     * @brief Check if one of the "magic buttons" got pressed and do his function.
     * LOCK + 4H = Display FW version
     * LOCK + 10H = LED animation
     */
    void handle_sys_req() override {
        if (!is_pressed(BTN_LOCK_NUM))
            return;

        if (is_pressed(BTN_10H_NUM))
            leds.sequence_start(&LEDcontrol::sequence_animate_handler);
        else if (is_pressed(BTN_4H_NUM))
            leds.show_num(FIRMWARE_VERSION);
        return;
    };
};
extern ButtonsRMECOWV110 buttons;

#include "../RM-ECOW-V1x/Hatch.hpp"

// LowLevel Serial Pins
#define UART_LL_RX PA3
#define UART_LL_TX PA2

#endif  // YARDFORCE_MDL_RMECOWV100_ASSEMBLY_HPP
