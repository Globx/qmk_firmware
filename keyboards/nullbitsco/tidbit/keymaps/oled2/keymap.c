/* Copyright 2021 Jay Greco
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include QMK_KEYBOARD_H
#include "action_layer.h"

__attribute__((weak)) RGB rgb_matrix_hsv_to_rgb(HSV hsv) { return hsv_to_rgb(hsv); }

enum layer_names {
  _BASE,
  _VIA1,
  _VIA2,
  _VIA3
};

//bool numlock_set = false;

enum custom_keycodes {
      PROG = SAFE_RANGE,
      ENCODER_MODE,
      DEBUG_MODE      
};

// globx
uint8_t encoder_mode = 0; // default vol, 1=RGB
uint8_t max_encoder_modes = 5;
static uint32_t key_timer;
static uint32_t key_timer_sleep;
int syncdata_state = 0;
char text_buf[4];

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
  [_BASE] = LAYOUT(
           KC_MUTE, ENCODER_MODE, RGB_M_P, 
    RM_9,   RM_10,    RM_11, RM_12, 
    RM_5,    RM_6,     RM_7,  RM_8, 
    RM_1,    RM_2,     RM_3,  RM_4, 
    KC_PSCR, KC_PGDN,  KC_PGUP, TO(_VIA1) 
  ),

  [_VIA1] = LAYOUT(
           KC_MUTE, ENCODER_MODE, RGB_TOG, 
    KC_F9,  KC_F10,   KC_F11, KC_F12, 
    KC_F5,   KC_F6,    KC_F7,  KC_F8, 
    KC_F1,   KC_F2,    KC_F3,  KC_F4, 
    KC_PSCR, KC_PGDN,  KC_PGUP, TO(_VIA3) 
  ),

  [_VIA2] = LAYOUT(
             KC_MUTE, KC_VOLD, KC_VOLU,
    KC_NO,   KC_NO,   KC_NO, KC_NO,
    KC_MSEL,   KC_NO,   KC_NO, KC_NO,
    KC_MSTP,   KC_MPLY,   KC_MPRV, KC_MNXT,
    KC_MUTE, KC_VOLD, KC_VOLU, TO(_VIA3)
  ),

  [_VIA3] = LAYOUT(
            DEBUG_MODE,   ENCODER_MODE,    PROG,
    RGB_SPD,RGB_SPI,   KC_NO,   KC_NO,
    RGB_HUI,RGB_SAI,RGB_VAI , RGB_MOD, 
    RGB_HUD,RGB_SAD,RGB_VAD, RGB_RMOD,
    RGB_TOG,RGB_MODE_PLAIN,RGB_MODE_BREATHE, TO(_BASE)
  ),
};


#ifdef OLED_ENABLE

/*
char hex_to_char(uint8_t hex)
{
    if (hex >= 10)
    {
        return 'A' + (hex - 10);
    }
    else
    {
        return '0' + (hex);
    }
}
*/

oled_rotation_t oled_init_user(oled_rotation_t rotation) { return OLED_ROTATION_270; }

static void num_to_buf(uint8_t num, bool three_digits)
{
    uint8_t pos = 2 + (three_digits == 1);

    text_buf[pos] = '\0';    
    text_buf[pos-1] = '0' + num % 10;
    text_buf[pos-2] = '0' + (num /= 10) % 10;
    if (three_digits)
    { 
      text_buf[pos-3] = '0' + num / 10;
    }
}

static void print_status_narrow(void) {
    oled_set_cursor(0,0);

    if (get_is_host())
    {
      oled_write_P(PSTR("HOST "), false);
    }
    else
    {
      oled_write_P(PSTR("REMOT"), false);
    }

    switch(encoder_mode)
    {
      case 0:
        oled_write_P(PSTR("D VOL"), false);
        break;
      case 1:
        oled_write_P(PSTR("D RGB"), false);
        break;
      case 2:
        oled_write_P(PSTR("D HUE"), false);
        break;
      case 3:
        oled_write_P(PSTR("D SAT"), false);
        break;
      case 4:
        oled_write_P(PSTR("D VAL"), false);
        break;
    }
    
    oled_write_P(PSTR("M "), false);
    num_to_buf(rgblight_get_mode(), false);
    oled_write_ln(text_buf, false);
    
    oled_set_cursor(0,4);
    HSV hsv = (HSV) { rgblight_get_hue(), rgblight_get_sat(), rgblight_get_val() };
    
    oled_write_P(PSTR("H "), false);
    num_to_buf(hsv.h, true);
    oled_write(text_buf, false);

    oled_write_P(PSTR("S "), false);
    num_to_buf(hsv.s, true);
    oled_write(text_buf, false);

    oled_write_P(PSTR("V "), false);
    num_to_buf(hsv.v, true);
    oled_write(text_buf, false);    

    oled_set_cursor(0,8);
    switch (get_highest_layer(layer_state)) {
        case _BASE:
            oled_write_P(PSTR("MACRO"), false);            
            oled_write_P(PSTR("     "), false);
            oled_write_P(PSTR("     "), false);
            oled_write_P(PSTR("     "), false);
            oled_write_P(PSTR("     "), false);
            break;
        case _VIA1:
            oled_write_P(PSTR("FUNC "), false);
            oled_write_P(PSTR("     "), false);
            oled_write_P(PSTR("     "), false);
            oled_write_P(PSTR("     "), false);
            oled_write_P(PSTR("     "), false);
            break;
        case _VIA2: // layer not used
            oled_write_P(PSTR("???? "), false);
            oled_write_P(PSTR("     "), false);
            oled_write_P(PSTR("     "), false);
            oled_write_P(PSTR("     "), false);
            oled_write_P(PSTR("     "), false);
            break;
        case _VIA3:
            oled_write_P(PSTR("RGB  "), false);
            oled_write_P(PSTR("1H/2S"), false);
            oled_write_P(PSTR("3V/4M"), false);
            oled_write_P(PSTR("9 Spd"), false);
            oled_write_P(PSTR("PsRGB"), false);                        
            break;
    }

    oled_set_cursor(0, 15);
    if (key_timer != 0)
    {
      uint32_t diff0 = timer_elapsed32(key_timer); // 1000=1 sec
      uint32_t diff = diff0 / 1000;
      diff0 = diff % 60;
      uint32_t mins = diff / 60;      
      uint32_t totalHours = mins / 60;
      uint32_t totalMins = mins - (totalHours * 60);
      
      num_to_buf(totalHours, false);
      oled_write(text_buf, false);    
            
      uint32_t seconds = diff % 2;
      if (seconds == 1)
      {
        oled_write_P(PSTR(":"), false);
      }
      else
      {
        oled_write_P(PSTR(" "), false);
      }
      
      num_to_buf(totalMins, false);
      oled_write(text_buf, false);
    }

    if (timer_elapsed32(key_timer_sleep) >= 60000)
    {
       oled_off();
    }
    else
    {
       oled_on();
    }
}

bool oled_task_user(void) {
    print_status_narrow();
    return true;
}
#endif

void keyboard_post_init_user(void) {  
  //debug_enable=true;
  //debug_matrix=true;
  //debug_keyboard=true;
  //debug_mouse=true;
  key_timer = timer_read32();
  set_bitc_LED(LED_OFF);  

  // seems to get skipped sometimes
  //rgblight_mode(rgblight_get_mode());
  //send_syncdata_rgbmode_msg(rgblight_get_mode(), rgblight_is_enabled()); // pass the opposite since it hasn't been set yet?
  //send_syncdata_hsv_msg(rgblight_get_hue(), rgblight_get_sat(), rgblight_get_val());
}

void matrix_init_user(void) { 
    matrix_init_remote_kb();  
}

void matrix_scan_user(void) 
{ 
  matrix_scan_remote_kb();
  // no need to perform below when this is host.
  if (syncdata_state != 2 && !get_is_host())
  {
    uint32_t elapsed = timer_elapsed32(key_timer);
    if(elapsed >= 500 && syncdata_state == 0)
    {
      // sync both - does this work for same mode?  or we change and change back
      rgblight_mode(rgblight_get_mode());
      send_syncdata_rgbmode_msg(rgblight_get_mode(), rgblight_is_enabled()); // pass the opposite since it hasn't been set yet?
      syncdata_state = 1;
    }
    else if (elapsed >= 1000 && syncdata_state == 1)
    {
      send_syncdata_hsv_msg(rgblight_get_hue(), rgblight_get_sat(), rgblight_get_val());
      syncdata_state = 2;
    }
  }
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {

     process_record_remote_kb(keycode, record);
    
    dprintf("tidbit: [%u][%u]\n", keycode, record->event.pressed);

    // Get the current NLCK status & set if not set.
    // Only do this once, in case user has a NLCK key
    // and wants to disable it later on.
    /*
    if (!numlock_set && record->event.pressed) {
        led_t led_state = host_keyboard_led_state();
        if (!led_state.num_lock) {
            register_code(KC_NLCK);
        }
        numlock_set = true;
    }
    */

    if (record->event.pressed)
    {
        key_timer_sleep = timer_read32();
    }

    switch (keycode) {
        case PROG:
            if (record->event.pressed) {
                set_bitc_LED(LED_DIM);
                rgblight_disable_noeeprom();
                oled_off();
                bootloader_jump();  // jump to bootloader
            }
            break;
        case ENCODER_MODE:
          if (record->event.pressed) {
              encoder_mode++;
              if (encoder_mode >= max_encoder_modes)
              {
                encoder_mode = 0;
              }              
          }
          break;
        case DEBUG_MODE:
          if (record->event.pressed) {
              debug_enable=!debug_enable;
          }
          break;  
        case RGB_TOG:
          if (record->event.pressed) {
            if (!get_is_host()) {
              send_syncdata_rgbmode_msg(rgblight_get_mode(), !rgblight_is_enabled()); // pass the opposite since it hasn't been set yet.
            }            
          }
          break;

        default:
            break;
    }
    return true;
}

void tap_remote_kb(uint16_t keycode) {
  keyrecord_t record;
  record.event.pressed = true;
  process_record_remote_kb(keycode, &record);
  record.event.pressed = false;
  process_record_remote_kb(keycode, &record);
}

bool encoder_update_user(uint8_t index, bool clockwise) {        
    key_timer_sleep = timer_read32();    
    
    switch(encoder_mode)
    {
      case 0:
        if (clockwise) {        
            tap_code(KC_VOLU);        // Leave this here for when tidbit is host.
            tap_remote_kb(KC_VOLU);   // globx - This allowing passing VOL controls to the host.
        } else {
            tap_code(KC_VOLD);
            tap_remote_kb(KC_VOLD);
        }
        break;
      case 1:
        if (clockwise) {
            rgblight_step();            
            //tap_remote_kb(RGB_MOD);   // globx - This is the older method of syncing keycodes from remote to host.  Same follows below.
        } else {
            rgblight_step_reverse();
            //tap_remote_kb(RGB_RMOD);
        }
        if (!get_is_host()) { send_syncdata_rgbmode_msg(rgblight_get_mode(), rgblight_is_enabled()); }
        break;
      case 2:
        if (clockwise) {
            rgblight_increase_hue();
            //tap_remote_kb(RGB_HUI);
        } else {
            rgblight_decrease_hue();
            //tap_remote_kb(RGB_HUD);
        }           
        if (!get_is_host()) { send_syncdata_hsv_msg(rgblight_get_hue(), rgblight_get_sat(), rgblight_get_val()); }
        break;
      case 3:
        if (clockwise) {
            rgblight_increase_sat();
            //tap_remote_kb(RGB_SAI);
        } else {
            rgblight_decrease_sat();
            //tap_remote_kb(RGB_SAD);
        }        
        if (!get_is_host()) { send_syncdata_hsv_msg(rgblight_get_hue(), rgblight_get_sat(), rgblight_get_val()); }
        break;
      case 4:
        if (clockwise) {
            rgblight_increase_val();
            //tap_remote_kb(RGB_VAI);
        } else {
            rgblight_decrease_val();            
            //tap_remote_kb(RGB_VAD);
        }             
        if (!get_is_host()) { send_syncdata_hsv_msg(rgblight_get_hue(), rgblight_get_sat(), rgblight_get_val()); }
        break;
    }
    return true;
}

void led_set_kb(uint8_t usb_led) {
  if (get_is_host())
  {
    if (usb_led & (1 << USB_LED_CAPS_LOCK))
        set_bitc_LED(LED_DIM);
    else
        set_bitc_LED(LED_OFF);
  }
  else
  {
    set_bitc_LED(LED_OFF);
  }
}
