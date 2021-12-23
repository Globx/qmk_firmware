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

/*
Remote keyboard is an experimental feature that allows for connecting another
keyboard, macropad, numpad, or accessory without requiring an additional USB connection.
The "remote keyboard" forwards its keystrokes using UART serial over TRRS. Dynamic VUSB 
detect allows the keyboard automatically switch to host or remote mode depending on
which is connected to the USB port.

Possible functionality includes the ability to send data from the host to the remote using
a reverse link, allowing for LED sync, configuration, and more data sharing between devices.
This will require a new communication protocol, as the current one is limited.
*/

#include "remote_kb.h"
#include "uart.h"

uint8_t
 msg[UART_MSG_LEN],
 msg_idx = 0;

bool
 is_host = true;

// globx syncdata vars
uint8_t syncdata_rgbmode = 0;
bool syncdata_rgbmode_enabled = false;
bool syncdata_rgbmode_set = false; // sync on start?
bool syncdata_hsv_set = false; // sync on start?
uint8_t syncdata_hsv_h = 0;
uint8_t syncdata_hsv_s = 0;
uint8_t syncdata_hsv_v = 0;

// Private functions
static bool vbus_detect(void) {
  #if defined(__AVR_ATmega32U4__)
    //returns true if VBUS is present, false otherwise.
    USBCON |= (1 << OTGPADE); //enables VBUS pad
    _delay_us(10);
    return (USBSTA & (1<<VBUS));  //checks state of VBUS
  #else
    #error vbus_detect is not implemented for this architecure!
  #endif
}

static uint8_t chksum8(const unsigned char *buf, size_t len) {
  unsigned int sum;
  for (sum = 0 ; len != 0 ; len--)
    sum += *(buf++);
  return (uint8_t)sum;
}

static void send_msg(uint16_t keycode, bool pressed) {
  msg[IDX_PREAMBLE] = UART_PREAMBLE;
  msg[IDX_KCLSB] = (keycode & 0xFF);
  msg[IDX_KCMSB] = (keycode >> 8) & 0xFF;
  msg[IDX_PRESSED] = pressed;
  msg[IDX_CHECKSUM] = chksum8(msg, UART_MSG_LEN-1);

  uart_transmit(msg, UART_MSG_LEN);
}

// globx - remote calls this when changing rgbmore or rbglight enabled/disabled
void send_syncdata_rgbmode_msg(uint8_t mode, bool enabled) {  
  msg[IDX_PREAMBLE] = UART_SYNCDATA_RGBMODE_PREAMBLE;
  msg[IDX_SYNCDATA_RGBMODE] = mode;
  msg[IDX_SYNCDATA_RGBMODE_ENABLED] = (uint8_t)enabled; // only need 1 bit for this.
  msg[IDX_SYNCDATA_RGBMODE_RESERVED2] = 0;  
  msg[IDX_CHECKSUM] = chksum8(msg, UART_MSG_LEN-1);

  uart_transmit(msg, UART_MSG_LEN);
}

// globx - remote calls this when changing HSV
void send_syncdata_hsv_msg(uint8_t h, uint8_t s, uint8_t v) {
  msg[IDX_PREAMBLE] = UART_SYNCDATA_HSV_PREAMBLE;  
  msg[IDX_SYNCDATA_HSV_H] = h;
  msg[IDX_SYNCDATA_HSV_S] = s;
  msg[IDX_SYNCDATA_HSV_V] = v;
  msg[IDX_CHECKSUM] = chksum8(msg, UART_MSG_LEN-1);

  uart_transmit(msg, UART_MSG_LEN);
}

static void print_message_buffer(void) {
  for (int i=0; i<UART_MSG_LEN; i++) {
    dprintf("msg[%u]: %u\n", i, msg[i]);
  }
}

static void process_uart(void) {
  uint8_t chksum = chksum8(msg, UART_MSG_LEN-1);
  // globx - added syncdata preamble checks
  if ((msg[IDX_PREAMBLE] != UART_PREAMBLE && msg[IDX_PREAMBLE] != UART_SYNCDATA_RGBMODE_PREAMBLE && msg[IDX_PREAMBLE] != UART_SYNCDATA_HSV_PREAMBLE) || msg[IDX_CHECKSUM] != chksum) {
     dprintf("UART checksum mismatch!\n");
     print_message_buffer();
     dprintf("calc checksum: %u\n", chksum);
  } else {

    uint8_t preamble = msg[IDX_PREAMBLE];
    if (preamble == UART_PREAMBLE) {
        uint16_t keycode = (uint16_t)msg[IDX_KCLSB] | ((uint16_t)msg[IDX_KCMSB] << 8);
        bool pressed = (bool)msg[IDX_PRESSED];
        // globx - added syncing keycodes for RGB_MOD and up. Skipped RGB_TOG since that is handled by syncdata.  If you want to use polling instead, then you should add RGB_TOG instead of RGB_MOD.
        if (IS_RM_KC(keycode) || (keycode >= RGB_MOD && keycode  <= RGB_MODE_RGBTEST)) {
          keyrecord_t record;
          record.event.time = 0;  // globx - identify message comes from remote.
          record.event.pressed = pressed;
          if (pressed) dprintf("Remote macro: press [%u]\n", keycode);
          else dprintf("Remote macro: release [%u]\n", keycode);
          process_record_user(keycode, &record);
        } else {
          if (pressed) {
            dprintf("Remote: press [%u]\n", keycode);
            register_code(keycode);
        } else {
            dprintf("Remote: release [%u]\n", keycode);
            unregister_code(keycode);
          }
        }
    }
    else if (preamble == UART_SYNCDATA_RGBMODE_PREAMBLE) {  // globx - handle syncdata rgbmode
        syncdata_rgbmode = msg[IDX_SYNCDATA_RGBMODE];
        syncdata_rgbmode_enabled = msg[IDX_SYNCDATA_RGBMODE_ENABLED] == 1;
        syncdata_rgbmode_set = true;
        dprintf("Remote SyncData RGBMODE [%u] enabled[%u]\n", syncdata_rgbmode, syncdata_rgbmode_enabled);     

        keyrecord_t record;
        record.event.pressed = true;
        process_record_user(RM_SD_RGBMODE, &record);
    }
    else if (preamble == UART_SYNCDATA_HSV_PREAMBLE) {  // globx - handle syncdata HSV
        syncdata_hsv_h = msg[IDX_SYNCDATA_HSV_H];
        syncdata_hsv_s = msg[IDX_SYNCDATA_HSV_S];
        syncdata_hsv_v = msg[IDX_SYNCDATA_HSV_V];
        syncdata_hsv_set = true;
        dprintf("Remote SyncData HSV h[%u] s[%u] v[%u]\n", syncdata_hsv_h, syncdata_hsv_s, syncdata_hsv_v);     

        keyrecord_t record;        
        record.event.pressed = true;
        process_record_user(RM_SD_HSV, &record);
    }
  }
}

static void get_msg(void) {
  while (uart_available()) {
    msg[msg_idx] = uart_read();
    dprintf("idx: %u, recv: %u\n", msg_idx, msg[msg_idx]);
    // globx - added syncdata preamble.
    if (msg_idx == 0 && (msg[msg_idx] != UART_PREAMBLE && msg[msg_idx] != UART_SYNCDATA_RGBMODE_PREAMBLE && msg[msg_idx] != UART_SYNCDATA_HSV_PREAMBLE)) {
      dprintf("Byte sync error!\n");
      msg_idx = 0;
    } else if (msg_idx == (UART_MSG_LEN-1)) {
      process_uart();
      msg_idx = 0;
    } else {
      msg_idx++;
    }
  }
}

static void handle_host_incoming(void) {
  get_msg();
}

static void handle_host_outgoing(void) {
  // for future reverse link use
}

static void handle_remote_incoming(void) {
  // for future reverse link use
}

static void handle_remote_outgoing(uint16_t keycode, keyrecord_t *record) {
  // globx - these should match from process_uart.
  if (IS_HID_KC(keycode) || IS_RM_KC(keycode)  || (keycode >= RGB_MOD && keycode  <= RGB_MODE_RGBTEST)) {
    dprintf("Remote: send [%u]\n", keycode);
    send_msg(keycode, record->event.pressed);
  }
}

// Public functions

void matrix_init_remote_kb(void) {
  uart_init(SERIAL_UART_BAUD);
  is_host = vbus_detect();
}

void process_record_remote_kb(uint16_t keycode, keyrecord_t *record) {
  #if defined (KEYBOARD_HOST)
  handle_host_outgoing();

  #elif defined(KEYBOARD_REMOTE)
  handle_remote_outgoing(keycode, record);

  #else //auto check with VBUS
  if (is_host) {
    handle_host_outgoing();
  }
  else {
    handle_remote_outgoing(keycode, record);
  }
  #endif
}

void matrix_scan_remote_kb(void) {
  #if defined(KEYBOARD_HOST)
  handle_host_incoming();

  #elif defined (KEYBOARD_REMOTE)
  handle_remote_incoming();

  #else //auto check with VBUS
  if (is_host) {
    handle_host_incoming();
  }
  else {
    handle_remote_incoming();
  }
  #endif
}

// globx - return host/remote flag
bool get_is_host(void) {
 return is_host;
}

// globx - Older polling code
/*

bool get_is_rgbmode_set(void) {  
  return syncdata_rgbmode_set;
}

bool get_is_hsv_set(void) {  
  return syncdata_hsv_set;
}
*/

// globx - get syncdata rgbmode
uint8_t get_syncdata_rgbmode(void) {
  syncdata_rgbmode_set = false;
  return syncdata_rgbmode;
}

// globx - get syncdata for rgblight_is_enabled()
bool get_syncdata_rgbmode_enabled(void) {
  syncdata_rgbmode_set = false;
  return syncdata_rgbmode_enabled;
}

// globx - get syncdata for current HSV
HSV get_syncdata_hsv(void) {
  syncdata_hsv_set = false;
  return (HSV) { syncdata_hsv_h, syncdata_hsv_s, syncdata_hsv_v };
}

