/*
Copyright 2021 Globx

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

// You can comment out process_record_user if you'd like to use
// VIA instead and program your key layout and macros there.
// NOTE: If you were to use jump to a layer, then probably
// better to read layer vs maintain in currentLayer with is
// mainly for encoder scolling.

#include QMK_KEYBOARD_H

// Define the number of layers to be used by the encoder.
#define LAYERS   4
uint8_t _currentLayer = 0;
bool _encoderVolumeEnabled = false;

enum layer_names {
    _BASE,
    _VIA1,
    _VIA2,
    _VIA3
};

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {

// Note: KC_F13 - KC_F24 can be used on Windows
[_BASE] = LAYOUT(
    _______, KC_F14, RESET,
    _______, KC_F13, KC_F15
),

[_VIA1] = LAYOUT(
    _______, KC_F17, KC_NO,
    _______, KC_F16, KC_F18
),

[_VIA2] = LAYOUT(
    _______, KC_NO, KC_MUTE,
    _______, KC_F19, KC_MUTE
),

[_VIA3] = LAYOUT(
    _______, KC_NO, KC_NO,
    _______, KC_PSCR, RESET
)

};

#ifdef OLED_DRIVER_ENABLE
// All writes using fixed 21 character spacing to clear previous layer text
void oled_task_user(void) {
    oled_write_P(PSTR("LAYER > "), false);
    switch (get_highest_layer(layer_state)) {
        case _BASE:
            oled_write_P(PSTR("QMK          "), false);
            oled_write_P(PSTR("1 3KEY     2 5KEY    "), false);
            oled_write_P(PSTR("3 SWITCHER           "), false);
            oled_write_P(PSTR("C DFU                "), false);
            break;
        case _VIA1:
            oled_write_P(PSTR("REDDIT       "), false);
            oled_write_P(PSTR("1 NULLBITS 2 XBOX    "), false);
            oled_write_P(PSTR("3 MECHBEYB           "), false);
            oled_write_P(PSTR("                     "), false);
            break;
        case _VIA2:
            oled_write_P(PSTR("AUDIO        "), false);
            if (!_encoderVolumeEnabled)
            {
                oled_write_P(PSTR("1 ENABLE VOL ENCODER "), false);
            }
            else
            {
                oled_write_P(PSTR("1 DISABLE VOL ENCODER"), false);

            }
            oled_write_P(PSTR("3 MUTE               "), false);
            oled_write_P(PSTR("C MUTE               "), false);
            break;
        case _VIA3:
            oled_write_P(PSTR("ADMIN/DFU    "), false);
            oled_write_P(PSTR("1 PRINTSCR           "), false);
            oled_write_P(PSTR("3 DFU                "), false);
            oled_write_P(PSTR("                     "), false);
            break;
    }
}
#endif

void matrix_init_user(void) {
  set_scramble_LED(LED_OFF);
}

// Encoder set to change layers
bool encoder_update_user(uint8_t index, bool clockwise) {

    if (_encoderVolumeEnabled)
    {
        if (clockwise) {
            tap_code(KC_VOLU);
        } else {
            tap_code(KC_VOLD);
        }
    }
    else
    {
        if (clockwise) {
            _currentLayer++;
            if (_currentLayer >= LAYERS) _currentLayer=0;
            layer_move(_currentLayer);
        } else {
            if (_currentLayer > 0)
            {
                _currentLayer--;
            }
            else
            {
                _currentLayer=LAYERS-1;
            }
            layer_move(_currentLayer);
        }
    }
    return true;
}

// Example macros for sending a string and opening Edge with a url.
// You may need to adjust the delays
// If you enable VIA, you can comment out this entire function
// and let VIA handle the key layout and macros.
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case KC_F13:
            if (record->event.pressed) {                
                SEND_STRING("qmk compile -kb nullbitsco/scramble -km 3key" SS_TAP(X_ENTER));
                return false;
            }            
            break;
        case KC_F14:
            if (record->event.pressed) {                
                SEND_STRING("qmk compile -kb nullbitsco/scramble -km 5key" SS_TAP(X_ENTER));
                return true;
            }            
            break;
        case KC_F15:
            if (record->event.pressed) {                
                SEND_STRING("qmk compile -kb nullbitsco/scramble -km switcher" SS_TAP(X_ENTER));
                return true;
            }            
            break;
        case KC_F16:
            if (record->event.pressed) {
                SEND_STRING(SS_LGUI("r") SS_DELAY(500) "msedge www.reddit.com/r/nullbits/" SS_TAP(X_ENTER));
                return false;
            }
            break;
        case KC_F17:
            if (record->event.pressed) {
                SEND_STRING(SS_LGUI("r") SS_DELAY(500) "msedge www.reddit.com/r/xbox/" SS_TAP(X_ENTER));
                return false;
            }
            break;
        case KC_F18:
            if (record->event.pressed) {
                SEND_STRING(SS_LGUI("r") SS_DELAY(500) "msedge www.reddit.com/r/MechanicalKeyboards/" SS_TAP(X_ENTER));
                return false;
            }
            break;
        case KC_F19:
            if (record->event.pressed) {
                _encoderVolumeEnabled = !_encoderVolumeEnabled;
                return false;
            }
            break;

   }
    return true;
}
