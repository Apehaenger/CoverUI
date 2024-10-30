/**
 * @file Display_RM-EC3-V11.cpp
 * @author Apehaenger (joerg@ebeling.ws)
 * @brief YardForce CoverUI RM-EC3-V1.1 display driver class for OpenMower https://github.com/ClemensElflein/OpenMower
 * This JLX25664 dispay is 4-level (2 Bit) gray scale capable, but for ease of development, it's driven as monochrome display.
 * @version 0.1
 * @date 2023-11-27
 *
 * @copyright Copyright (c) 2023
 */
#include "Display.hpp"

#include <lvgl.h>

#include "../../include/main.h"
#include "../../include/subscription.h"

// C images
LV_IMG_DECLARE(OM_Logo_120x54x1);
LV_IMG_DECLARE(OM_Wordmark_240x35x1);

namespace yardforce {
namespace display {

static controller::ST75256 st75256;  // Controller driver

// LVGL buffers
static lv_disp_draw_buf_t lv_disp_buf;
static lv_color_t lv_buf_1[ST75256_DISPLAY_WIDTH * LVGL_BUFFER_MULTIPLIER];
static lv_color_t lv_buf_2[ST75256_DISPLAY_WIDTH * LVGL_BUFFER_MULTIPLIER];

// Status Screen Widgets
lvgl::WidgetLedSymbol *v_led_heartbeat, *v_led_ros,
    *v_led_emergency_wheel, *v_led_emergency, *v_led_emergency_stop,
    *v_led_bat, *v_led_gps, *v_led_charge, *v_led_power;
lvgl::WidgetBar *bar_gps, *bar_bat;
lvgl::WidgetTextTicker *text_ticker_status;

bool main_screen_active = false;  // Initialized and active
bool last_docked_state_;          // true = docked, false = undocked

bool DisplayRMEC3V11::init() {
    // Init LCD display controller
    st75256.init();

    // Init LVGL
    lv_init();
    lv_disp_drv_init(&lv_disp_drv);                                                                           // Basic LVGL display driver initialization
    lv_disp_drv.draw_buf = &lv_disp_buf;                                                                      // Set an initialized buffer
    lv_disp_drv.rounder_cb = rounder_cb;                                                                      // Round x coordinated so that it fit for our 3 RGB pixel/per dot display
    lv_disp_drv.flush_cb = flush_cb;                                                                          // Set a flush callback to draw to the display
    lv_disp_drv.hor_res = ST75256_DISPLAY_WIDTH;                                                              // Set the horizontal resolution in pixels
    lv_disp_drv.ver_res = ST75256_DISPLAY_HEIGHT;                                                             // Set the vertical resolution in pixels
    lv_disp_draw_buf_init(&lv_disp_buf, lv_buf_1, lv_buf_2, ST75256_DISPLAY_WIDTH * LVGL_BUFFER_MULTIPLIER);  // Initialize `disp_buf` with the buffer(s)
    lv_disp_t *disp;
    disp = lv_disp_drv_register(&lv_disp_drv);                                // Register the driver and save the created display objects
    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_white(), LV_PART_MAIN);  // No background color

    openmower_anim();
    // mainScreen();

    return true;
}

/**
 * @brief Rounder callback will round the display area to a multiple of 8, on y axis (because one DDRAM byte are 8 y-pixels (page), per column)
 *
 * @param disp_drv
 * @param area
 */
void DisplayRMEC3V11::rounder_cb(lv_disp_drv_t *disp_drv, lv_area_t *area) {
    area->y1 = area->y1 - (area->y1 % 8);            // Round down to neares multiple of 8
    area->y2 = (area->y2 + 8) - (area->y2 % 8) - 1;  // Round up to nearest multiple of 8, minus -1
}

/**
 * @brief Flush display buffer to display controller.
 *
 * @param disp_drv
 * @param area
 * @param t_color_p
 */
void DisplayRMEC3V11::flush_cb(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *t_color_p) {
    size_t cols = (area->x2 - area->x1) + 1;  // Num of columns for this flush area
    uint8_t page_pixel_buffer[cols];          // Store 8 y pixel (LSB = top) for area of x
    uint8_t y_shift = 0;

    st75256.send_ctrl(0x30);  // EXT1=0, EXT0=0
    st75256.set_column_address(area->x1, area->x2);
    st75256.set_page_address(area->y1 / 8, (area->y2 - 7) / 8);  // ATTENTION: Will only work with a correct rounder_cb()
    st75256.send_ctrl(0b01011100);                               // [10] Write Data

    for (size_t y = area->y1; y <= area->y2; y++) {
        for (size_t x = 0; x < cols; x++) {
            t_color_p->full ? page_pixel_buffer[x] &= ~(1 << y_shift) : page_pixel_buffer[x] |= (1 << y_shift);
            t_color_p++;
        }
        if (y_shift < 7) {
            y_shift++;
            continue;
        }
        st75256.send_data(page_pixel_buffer, cols);
        y_shift = 0;
    }
    lv_disp_flush_ready(disp_drv);
}

void DisplayRMEC3V11::set_undocked() {
    v_led_power->set(LED_off);
    v_led_charge->set(LED_off);
    bar_bat->set_range(BATT_ABS_MIN, BATT_ABS_MAX);
    bar_bat->bar_label = FA_SYMBOL_BATTERY " %d V";
    last_docked_state_ = false;
}

void DisplayRMEC3V11::mainScreen() {
    lv_obj_clean(lv_scr_act());

    // On the left side of the status bar we do have functional status symbols like heartbeat and ROS
    v_led_ros = new lvgl::WidgetLedSymbol(FA_SYMBOL_ROS, LV_ALIGN_TOP_LEFT, 0, -1);  // Leftmost

    // In the middle, we do have emergencies
    v_led_emergency = new lvgl::WidgetLedSymbol(FA_SYMBOL_EMERGENCY, LV_ALIGN_TOP_MID, 0, -1);                                            // Centered
    v_led_emergency_wheel = new lvgl::WidgetLedSymbol(FA_SYMBOL_EMERGENCY_WHEEL, LV_ALIGN_TOP_MID, -14 - TOP_STATUS_BAR_GAP_PX - 2, -1);  // Left of centered
    // TODO: if next level LL proto
    // v_led_heartbeat = new WidgetLedSymbol(FA_SYMBOL_HEARTBEAT, LV_ALIGN_TOP_MID, -(2 * 14) - (2 * TOP_STATUS_BAR_GAP_PX) - 2, -1); // 2nd left of centered
    v_led_emergency_stop = new lvgl::WidgetLedSymbol(FA_SYMBOL_EMERGENCY_STOP, LV_ALIGN_TOP_MID, 14 + TOP_STATUS_BAR_GAP_PX, -1);  // Right of centered

    // On the right side, mowing status like, charging, docking, ...
    v_led_power = new lvgl::WidgetLedSymbol(FA_SYMBOL_PLUG, LV_ALIGN_OUT_TOP_RIGHT, (ST75256_DISPLAY_WIDTH - (1 * 14)), -1);  // Rightmost
    v_led_charge = new lvgl::WidgetLedSymbol(FA_SYMBOL_CHARGE, LV_ALIGN_OUT_TOP_RIGHT, (ST75256_DISPLAY_WIDTH - (2 * 14) - TOP_STATUS_BAR_GAP_PX), -1);
    v_led_gps = new lvgl::WidgetLedSymbol(FA_SYMBOL_GPS1, LV_ALIGN_OUT_TOP_RIGHT, (ST75256_DISPLAY_WIDTH - (3 * 14) - (2 * TOP_STATUS_BAR_GAP_PX)), -1);
    v_led_bat = new lvgl::WidgetLedSymbol(FA_SYMBOL_BATTERY, LV_ALIGN_OUT_TOP_RIGHT, (ST75256_DISPLAY_WIDTH - (4 * 14) - (3 * TOP_STATUS_BAR_GAP_PX)), -1);

    // GPS & Battery bars
    bar_gps = new lvgl::WidgetBar(FA_SYMBOL_GPS2 " %d %%", LV_ALIGN_TOP_LEFT, 0, 19, (ST75256_DISPLAY_WIDTH / 2) - 1, 21);
    bar_bat = new lvgl::WidgetBar(FA_SYMBOL_BATTERY " %d %%", LV_ALIGN_TOP_RIGHT, 0, 19, (ST75256_DISPLAY_WIDTH / 2) - 1, 21);

    // Mower status text (ticker)
    text_ticker_status = new lvgl::WidgetTextTicker(LV_ALIGN_TOP_MID, 0, 42, ST75256_DISPLAY_WIDTH, 40);

    // Set defined state
    set_undocked();
    bar_bat->set_value(BATT_ABS_MIN);

    main_screen_active = true;
}

void DisplayRMEC3V11::anim_x_cb(void *var, int32_t v) {
    lv_obj_set_x((lv_obj_t *)var, v);
}

void DisplayRMEC3V11::openmower_anim_delay() {
    delay(1000);
    mainScreen();
}

// Probably static void
void DisplayRMEC3V11::openmower_anim() {
    const uint8_t wm_start_offset = 20;
    main_screen_active = false;

    lv_obj_clean(lv_scr_act());

    // Mower Logo - img_logo
    lv_obj_t *img_logo = lv_img_create(lv_scr_act());
    lv_img_set_src(img_logo, &OM_Logo_120x54x1);
    lv_obj_align(img_logo, LV_ALIGN_RIGHT_MID, 0, 0);

    // OpenMower Wordmark - img_wordmark
    lv_obj_t *img_wordmark = lv_img_create(lv_scr_act());
    lv_img_set_src(img_wordmark, &OM_Wordmark_240x35x1);
    lv_obj_align(img_wordmark, LV_ALIGN_RIGHT_MID, OM_Wordmark_240x35x1.header.w + wm_start_offset, 0);

    // Anim of logo
    lv_anim_t al;
    lv_anim_init(&al);
    lv_anim_set_var(&al, img_logo);
    lv_anim_set_values(&al, 0, -ST75256_DISPLAY_WIDTH);
    lv_anim_set_time(&al, 1800);
    lv_anim_set_delay(&al, 1200);
    lv_anim_set_exec_cb(&al, (lv_anim_exec_xcb_t)anim_x_cb);
    lv_anim_set_path_cb(&al, lv_anim_path_ease_in);
    // lv_anim_set_repeat_count(&al, LV_ANIM_REPEAT_INFINITE);
    lv_anim_start(&al);

    // Anim of Wordmark
    lv_anim_t aw;
    lv_anim_init(&aw);
    lv_anim_set_var(&aw, img_wordmark);
    lv_anim_set_values(&aw, OM_Wordmark_240x35x1.header.w + wm_start_offset, -((ST75256_DISPLAY_WIDTH - OM_Wordmark_240x35x1.header.w) / 2));
    lv_anim_set_time(&aw, 1800);
    lv_anim_set_delay(&aw, 1900);
    lv_anim_set_exec_cb(&aw, (lv_anim_exec_xcb_t)anim_x_cb);
    lv_anim_set_path_cb(&aw, lv_anim_path_ease_in_out);
    lv_anim_set_deleted_cb(&aw, (lv_anim_deleted_cb_t)openmower_anim_delay);  // Set a callback to indicate when the animation is deleted (idle)
    // lv_anim_set_repeat_count(&aw, LV_ANIM_REPEAT_INFINITE);
    lv_anim_start(&aw);
}

void DisplayRMEC3V11::loop_low_prio() {
    AbstractDisplay::loop_low_prio();

    // LEDs & Buttons to main status screen conversion
    if (!main_screen_active)
        return;  // Still in OM anim

    char status_ticker[STATUS_TICKER_LENGTH] = "";

    // GPS
    if (subscription::recv_hl_state.gps_quality < 25)
        v_led_gps->set(LED_on);
    else if (subscription::recv_hl_state.gps_quality < 50)
        v_led_gps->set(LED_blink_fast);
    else if (subscription::recv_hl_state.gps_quality < 75)
        v_led_gps->set(LED_blink_slow);
    else
        v_led_gps->set(LED_off);
    bar_gps->set_value(subscription::recv_hl_state.gps_quality);

    // V-Battery vLED
    if (subscription::recv_ll_status.v_battery >= (BATT_EMPTY + 2.0f))
        v_led_bat->set(LED_off);
    else
        v_led_bat->set(LED_on);

    // Docked (Plug) & Charging (charge-station)
    if (subscription::recv_ll_status.v_charge > 20.0f)  // Docked
    {
        if (!last_docked_state_) {
            v_led_power->set(LED_on);
            bar_bat->set_range(100, 1100);
            bar_bat->bar_label = FA_SYMBOL_CHARGE " %d mA";
            set_backlight();
            last_docked_state_ = true;
        }
        bar_bat->set_value(subscription::recv_ll_status.charging_current * 1000);

        if (subscription::recv_ll_status.charging_current < 0.15f)
            v_led_charge->set(LED_off);
        else if (subscription::recv_ll_status.charging_current >= 0.15f && subscription::recv_ll_status.charging_current <= 0.8f)
            v_led_charge->set(LED_blink_slow);
        else if (subscription::recv_ll_status.charging_current > 0.8f)
            v_led_charge->set(LED_blink_fast);
    } else  // Undocked
    {
        if (last_docked_state_) {
            set_undocked();
        }
        bar_bat->set_value(subscription::recv_ll_status.v_battery);
    }

    // HL Mode & SubMode
    switch (subscription::recv_hl_state.current_mode & 0b111111) {
        case HighLevelMode::MODE_IDLE:
            v_led_ros->set(LED_on);
            strncpy(status_ticker, "Idle", STATUS_TICKER_LENGTH);
            break;
        case HighLevelMode::MODE_AUTONOMOUS:
            v_led_ros->set(LED_blink_slow);
            if ((subscription::recv_hl_state.current_mode >> 6) & 0b01)
                strncpy(status_ticker, "Docking", STATUS_TICKER_LENGTH);
            else if ((subscription::recv_hl_state.current_mode >> 6) & 0b10)
                strncpy(status_ticker, "Undocking", STATUS_TICKER_LENGTH);
            else
                strncpy(status_ticker, "Mowing", STATUS_TICKER_LENGTH);
            break;
        case HighLevelMode::MODE_RECORDING:
            v_led_ros->set(LED_blink_fast);
            if ((subscription::recv_hl_state.current_mode >> 6) & 0b01)
                strncpy(status_ticker, "Record area outline", STATUS_TICKER_LENGTH);
            else if ((subscription::recv_hl_state.current_mode >> 6) & 0b10)
                strncpy(status_ticker, "Record obstacle", STATUS_TICKER_LENGTH);
            break;
        default:  // We currently don't have a real "ROS Running" identifier. Let's use the current mode.
            v_led_ros->set(LED_off);
            strncpy(status_ticker, "Waiting for ROS...", STATUS_TICKER_LENGTH);
            break;
    }

    // ----- Most important text-states, last! -----

    // Emergencies
    static bool last_stop_button = false;
    if (subscription::recv_ll_status.emergency_bitmask & LL_EMERGENCY_BIT_STOP)  // Stop switch
    {
        v_led_emergency_stop->set(LED_blink_fast);
        strncpy(status_ticker, EMERGENCY_CLEAR_TEXT, STATUS_TICKER_LENGTH);

        if (!last_stop_button)  // Backlight on cover- open
            set_backlight();
        last_stop_button = true;
    } else {
        v_led_emergency_stop->set(LED_off);
        last_stop_button = false;
    }

    if (subscription::recv_ll_status.emergency_bitmask & LL_EMERGENCY_BIT_LIFT)  // Lifted or tilted
    {
        v_led_emergency_wheel->set(LED_blink_fast);
        strncpy(status_ticker, EMERGENCY_CLEAR_TEXT, STATUS_TICKER_LENGTH);
    } else
        v_led_emergency_wheel->set(LED_off);

    if (subscription::recv_ll_status.emergency_bitmask & LL_EMERGENCY_BIT_LATCH)  // Emergency latch (no LL heartbeat or emergency by ROS)
    {
        v_led_emergency->set(LED_blink_slow);
        strncpy(status_ticker, EMERGENCY_CLEAR_TEXT, STATUS_TICKER_LENGTH);
    } else
        v_led_emergency->set(LED_off);

    // ----- Announcement -----
    switch (auto ann = get_anncmnt()) {
        case AnncmntType::close_hatch:
            sprintf(status_ticker, "Close hatch in %i sec.", ((anncmnt.timeout - millis()) / 1000) + 1);
            break;
        case AnncmntType::version:
            sprintf(status_ticker, "Version %i", FIRMWARE_VERSION);
            break;
        default:
            break;
    }

    text_ticker_status->set_text(status_ticker);
}
}  // namespace display
}  // namespace yardforce

yardforce::display::DisplayRMEC3V11 display(yardforce::display::AbstractDisplay::Config{.backlight_led_num = LED_NUM_BACKLIGHT});