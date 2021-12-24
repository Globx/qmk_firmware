/* Copyright 2021 Jose Luis Adelantado Torres
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

__attribute__((weak)) RGB rgb_matrix_hsv_to_rgb(HSV hsv) { return hsv_to_rgb(hsv); }

enum layer_names {
  _BASE,
  _VIA1,
  _VIA2,
  _VIA3
};

// SAFE_RANGE or USER00
enum custom_keycodes {
      PROG = SAFE_RANGE,
      ENCODER_MODE,
      DEBUG_MODE
};

uint8_t encoder_mode = 0; // default vol, 1=RGB
uint8_t max_encoder_modes = 5;
static uint32_t key_timer;
static uint32_t key_timer_sleep;
char text_buf[4];

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
  [_BASE] = LAYOUT_all(
              KC_ESC, KC_1,    KC_2,    KC_3,    KC_4,    KC_5,    KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_MINS, KC_EQL,  KC_BSPC, KC_HOME,
   KC_MUTE,   KC_TAB,  KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,    KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    KC_LBRC, KC_RBRC, KC_BSLS, KC_END,
    ENCODER_MODE,   KC_CAPS, KC_A,    KC_S,    KC_D,    KC_F,    KC_G,    KC_H,    KC_J,    KC_K,    KC_L,    KC_SCLN, KC_QUOT,          KC_ENT,  KC_PGUP,
    KC_F15,   KC_LSFT, KC_NUBS, KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,    KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH, KC_RSFT, KC_UP,   KC_PGDN,
    KC_F16,   KC_LCTL, KC_LGUI, KC_LALT,                            KC_SPC,                    KC_RALT, MO(_VIA1),  KC_RCTL, KC_LEFT, KC_DOWN, KC_RGHT
  ),

  [_VIA1] = LAYOUT_all(
               KC_GRV,   KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,   KC_F6,   KC_F7,   KC_F8,   KC_F9,  KC_F10,  KC_F11,  KC_F12,  KC_DEL, KC_INSERT,
    RGB_TOG,  _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, KC_PSCR,
    _______,  _______, _______, RGB_M_P, DEBUG_MODE, _______, _______, _______, _______, _______, _______, _______, _______,          _______, KC_SLCK,
    _______,  _______, _______, _______, _______, _______, _______,   RESET, _______, _______, _______, _______, _______, _______, _______, KC_PAUSE,
    _______,  _______, _______, _______,                            _______,                   _______, _______, _______, KC_MPRV, KC_MPLY, KC_MNXT
  ),

  [_VIA2] = LAYOUT_all(
              _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,
    _______,  _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,
    _______,  _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,          _______, _______,
    _______,  _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,
    _______,  _______, _______, _______,                            _______,                   _______, _______, _______, _______, _______, _______
  ),

  [_VIA3] = LAYOUT_all(
              _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,
    _______,  _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,
    _______,  _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,          _______, _______,
    _______,  _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,
    _______,  _______, _______, _______,                            _______,                   _______, _______, _______, _______, _______, _______
  ),

};

/*
const rgblight_segment_t PROGMEM my_capslock_layer[] = RGBLIGHT_LAYER_SEGMENTS(
  {0, 9, HSV_RED}
);
*/
/*
const rgblight_segment_t* const PROGMEM my_rgb_layers[] = RGBLIGHT_LAYERS_LIST(
    my_capslock_layer
);
*/
/*
bool led_update_user(led_t led_state) {
    rgblight_set_layer_state(0, led_state.caps_lock);
    return true;
}
*/

/*
void keyboard_post_init_user(void) {
	// Enable the LED layers
	rgblight_set_layer_state(0, led_state.caps_lock);
}
*/

void keyboard_post_init_user(void) {
  // Customise these values to desired behaviour
  //debug_enable=true;
  //debug_matrix=true;
  //debug_keyboard=true;
  //debug_mouse=true;
  key_timer = timer_read32();
}

#ifdef OLED_ENABLE
oled_rotation_t oled_init_user(oled_rotation_t rotation) {    
    return OLED_ROTATION_90;
}

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

bool oled_task_user(void) {    
    oled_set_cursor(0, 0);

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
      case 5:
        oled_write_P(PSTR("D RND"), false);
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
    oled_write_P(PSTR("H INS"), false);
    oled_write_P(PSTR("E PSC"), false);
    oled_write_P(PSTR("U SCL"), false);
    oled_write_P(PSTR("D PAU"), false);
    oled_write_P(PSTR("! FnB"), false);

    // Host Keyboard LED Status
    led_t led_state = host_keyboard_led_state();
    oled_write_P(led_state.caps_lock ? PSTR("C/") : PSTR("c/"), false);
    oled_write_P(led_state.num_lock ? PSTR("N/") : PSTR("n/"), false);    
    oled_write_P(led_state.scroll_lock ? PSTR("S") : PSTR("s"), false);  

    oled_set_cursor(0, 15);
    if (key_timer != 0)
    {
      uint32_t diff0 = timer_elapsed32(key_timer); // / 1000;  // seconds 
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
    return false;
}
#endif

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    // Send keystrokes to host keyboard, if connected (see readme)
    process_record_remote_kb(keycode, record);

    if (record->event.pressed)
    {
        key_timer_sleep = timer_read32();
    }

    dprintf("nibble: [%u][%u]\n", keycode, record->event.pressed);

    switch (keycode) {
        case ENCODER_MODE:
          if (record->event.pressed) {
              encoder_mode++;
              if (encoder_mode >= max_encoder_modes)
              {
                encoder_mode = 0;
              }              
          }
          break;

      // globx - custom keycode is called to report syncdata for rgbmode and enabled from remote kn
      case RM_SD_RGBMODE:
        if (record->event.pressed) {
          uint8_t mode = get_syncdata_rgbmode();
          if(mode != 0)
          {
            rgblight_mode(mode);
            dprintf("Mode changed to [%u]\n", mode);
          }
          else
          {
            dprintf("Ignored Mode changed to [%u]\n", mode);      
          }
          bool enabled = get_syncdata_rgbmode_enabled();
          if (enabled && ! rgblight_is_enabled())
          {
            rgblight_enable();
          }
          else if (!enabled && rgblight_is_enabled())
          {
            rgblight_disable();
          }          
        }
        return false;
        break;

      // globx - custom keycode is called to report syncdata for HSV from remote kn
      case RM_SD_HSV:
        if (record->event.pressed) {      
          HSV hsv = get_syncdata_hsv();
          rgblight_sethsv(hsv.h, hsv.s, hsv.v);
          dprintf("hsv updated to h[%u] s[%u] v[%u]", hsv.h, hsv.s, hsv.v);
        }
        return false;
        break;

      case DEBUG_MODE:
        if (record->event.pressed) {
            debug_enable=!debug_enable;
        }
        return false;
        break;       
        case RM_1:
          if (record->event.pressed) {              
              SEND_STRING(SS_LGUI("r") SS_DELAY(500) "msedge twitch.tv" SS_TAP(X_ENTER));              
          }
          break;
        case RM_2:
          if (record->event.pressed) {              
              SEND_STRING(SS_LGUI("r") SS_DELAY(500) "msedge.exe docs.qmk.fm/#/keycodes" SS_TAP(X_ENTER));
          }
          break;
        case RM_3:
          if (record->event.pressed) {              
              SEND_STRING(SS_LGUI("r") SS_DELAY(500) "msedge www.reddit.com/r/nullbits/" SS_TAP(X_ENTER));              
          }
          break;
        case RM_4:
          if (record->event.pressed) {              
              SEND_STRING(SS_LGUI("r") SS_DELAY(500) "msedge.exe www.reddit.com/r/MechanicalKeyboards/" SS_TAP(X_ENTER));
          }
          break;
    }
    return true;
}

bool encoder_update_user(uint8_t index, bool clockwise) {    
    key_timer_sleep = timer_read32();
    switch(encoder_mode)
    {
      case 0:
        if (clockwise) {
            tap_code(KC_VOLU);
        } else {
            tap_code(KC_VOLD);
        }
        break;
      case 1:
        if (clockwise) {
            rgblight_step();
        } else {
            rgblight_step_reverse();
        }         
        break;
      case 2:
        if (clockwise) {
            rgblight_increase_hue();
        } else {
            rgblight_decrease_hue();
        }         
        break;
      case 3:
        if (clockwise) {
            rgblight_increase_sat();
        } else {
            rgblight_decrease_sat();
        }         
        break;
      case 4:
        if (clockwise) {
            rgblight_increase_val();
        } else {
            rgblight_decrease_val();
        }         
        break;
    }
    return true;
}

void matrix_init_user(void) {  
    // Initialize remote keyboard, if connected (see readme)
    matrix_init_remote_kb();
}

void matrix_scan_user(void) {
    // Scan and parse keystrokes from remote keyboard, if connected (see readme)
    matrix_scan_remote_kb();

// polling method - when not using the msging method, poll to see when the syncdata has arrived.
/*
    bool is_rgbmode = get_is_rgbmode_set();        
    if (is_rgbmode)
    {
      uint8_t mode = get_syncdata_rgbmode();
      if(mode != 0)
      {
        rgblight_mode(mode);
        dprintf("Mode changed to [%u]\n", mode);
      }
      else
      {
        dprintf("Ignored Mode changed to [%u]\n", mode);      
      }
    }

    bool is_hsv = get_is_hsv_set();
    if (is_hsv)
    {
      HSV hsv = get_syncdata_hsv();
      rgblight_sethsv(hsv.h, hsv.s, hsv.v);
      dprintf("hsv updated to h[%u] s[%u] v[%u]", hsv.h, hsv.s, hsv.v);
    }
    */
}
