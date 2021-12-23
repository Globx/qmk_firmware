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
#pragma once

#include "quantum.h"

#define SERIAL_UART_BAUD 153600 //low error rate for 32u4 @ 16MHz

#define UART_PREAMBLE 0x69
#define UART_MSG_LEN  5
#define UART_NULL     0

#define IDX_PREAMBLE  0
#define IDX_KCLSB     1
#define IDX_KCMSB     2
#define IDX_PRESSED   3
#define IDX_CHECKSUM  4

// globx
#define UART_SYNCDATA_RGBMODE_PREAMBLE 0x70
#define IDX_SYNCDATA_RGBMODE   1
#define IDX_SYNCDATA_RGBMODE_ENABLED 2
#define IDX_SYNCDATA_RGBMODE_RESERVED2 3

#define UART_SYNCDATA_HSV_PREAMBLE 0x71
#define IDX_SYNCDATA_HSV_H    1
#define IDX_SYNCDATA_HSV_S    2
#define IDX_SYNCDATA_HSV_V    3


#define IS_HID_KC(x) ((x > 0) && (x < 0xFF))
#define IS_RM_KC(x) ((x >= RM_BASE) && (x <= 0xFFFF))

// globx - added RM_SD_xxx into remote macros enums.  todo: new enum?
#define RM_BASE 0xFFFF-18
enum remote_macros {
  RM_1 = RM_BASE,
  RM_2,  RM_3,
  RM_4,  RM_5,
  RM_6,  RM_7,
  RM_8,  RM_9,
  RM_10, RM_11,
  RM_12, RM_13,
  RM_14, RM_15,
  RM_SD_RGBMODE,
  RM_SD_HSV
};

// Public functions
void
 matrix_init_remote_kb(void),
 process_record_remote_kb(uint16_t keycode, keyrecord_t *record),
 matrix_scan_remote_kb(void);

// globx
bool get_is_host(void);
// Older polling code
/*
bool get_is_rgbmode_set(void);  // polling
bool get_is_hsv_set(void);      // polling
*/
// remote kb can send syncdata to host
void send_syncdata_rgbmode_msg(uint8_t mode, bool enabled);
void send_syncdata_hsv_msg(uint8_t h, uint8_t s, uint8_t v);

// host can read sync data that was sent from remote kb
uint8_t get_syncdata_rgbmode(void);
bool get_syncdata_rgbmode_enabled(void);
HSV get_syncdata_hsv(void);
