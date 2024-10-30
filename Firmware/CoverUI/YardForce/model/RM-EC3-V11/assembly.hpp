/**
 * @file RM-EC3-V11/assembly.hpp
 * @author Apehaenger (joerg@ebeling.ws)
 * @brief YardForce RM-EC3-V1.1 CoverUI Assembly for OpenMower https://github.com/ClemensElflein/OpenMower
 * @version 0.2
 * @date 2024-10-30
 *
 * @copyright Copyright (c) 2024
 *
 */
#ifndef YARDFORCE_MDL_RMEC3V11_ASSEMBLY_HPP
#define YARDFORCE_MDL_RMEC3V11_ASSEMBLY_HPP

#include "../../include/Buttons.hpp"
#include "../../include/main.h"
#include "LEDcontrol.hpp"

// FIXME: AbstractDisplay depends on leds
extern LEDcontrolRMEC3V11 leds;
#include "Display.hpp"

#ifdef MOD_EMERGENCY
#include "../../include/Emergency.hpp"
#endif

// ***** LEDs *****

// 1st row: Lifted, Wire (WLAN), Battery empty, Charge
#define LED_PIN_S1 PC4
#define LED_PIN_SETUP PA1
#define LED_PIN_S2 PC5
#define LED_PIN_BACKLIGHT PA8
// Backside
#define LED_NUM_BACKLIGHT 20

// ***** Buttons *****

#define BTN_S1_PIN PC14
#define BTN_S2_PIN PC13
#define BTN_PLAY_PIN PC7
#define BTN_HOME_PIN PC2
#define BTN_SETUP_PIN PA0

#define BTN_MENU_PIN PB4

#define BTN_1_PIN PB8
#define BTN_2_PIN PB5
#define BTN_3_PIN PB3
#define BTN_4_PIN PD2
#define BTN_5_PIN PC12
#define BTN_6_PIN PC11
#define BTN_7_PIN PC10
#define BTN_8_PIN PB14
#define BTN_9_PIN PC6
#define BTN_BACK_PIN PA15
#define BTN_0_PIN PB9
#define BTN_OK_PIN PB13

// ***** Emergency *****

#define PIN_STOP1 PC15
#define PIN_STOP2 PB6

#define PIN_LIFT PC0
#define PIN_LIFTX PC9
#define PIN_LBUMP PC8
#define PIN_RBUMP PB15

class ButtonsRMEC3V11 : public Buttons {
   public:
    ButtonsRMEC3V11(const std::map<uint8_t, ButtonDef> &t_kBtnDefByNumMap) : Buttons(t_kBtnDefByNumMap) {};

    void handle_sys_req() override;
};
extern ButtonsRMEC3V11 buttons;

// ----- Hatch -----

#include "../../include/ABC_Hatch.hpp"

class HatchRMEC3V11 : public AbstractHatch {
   public:
    unsigned int handle_button(unsigned int button_id, unsigned int press_cnt);
};

extern HatchRMEC3V11 hatch;

// ----- LowLevel Serial -----

#define UART_LL_RX PA3
#define UART_LL_TX PA2

#endif  // YARDFORCE_MDL_RMEC3V11_ASSEMBLY_HPP
