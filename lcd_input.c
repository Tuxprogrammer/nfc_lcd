/*
 * "Copyright (c) 2008 Robert B. Reese, Bryan A. Jones, J. W. Bruce ("AUTHORS")"
 * All rights reserved.
 * (R. Reese, reese_AT_ece.msstate.edu, Mississippi State University)
 * (B. A. Jones, bjones_AT_ece.msstate.edu, Mississippi State University)
 * (J. W. Bruce, jwbruce_AT_ece.msstate.edu, Mississippi State University)
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without written agreement is
 * hereby granted, provided that the above copyright notice, the following
 * two paragraphs and the authors appear in all copies of this software.
 *
 * IN NO EVENT SHALL THE "AUTHORS" BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
 * OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF THE "AUTHORS"
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * THE "AUTHORS" SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS
 * ON AN "AS IS" BASIS, AND THE "AUTHORS" HAS NO OBLIGATION TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS."
 *
 * Please maintain this header in its entirety when copying/modifying
 * these files.
 *
 *
 */
// #include "i2c_device.h"
#include <pic24_all.h>
#include "pic24_i2c.h"
#include <stdio.h>
#include "sclcd.h"

#define LCD_ADDR 0x4E

int main(void) {
    configBasic(HELLO_MSG);
    configI2C1(100);
    outString("Starting...\n");
    outString("lcd_init\n");
    lcd_init(LCD_ADDR, 20, 4);
    outString("clear\n");
    clear();
    outString("cursor\n");
    cursor();
    outString("blink\n");
    blink();
    outString("noAutoScroll\n");
    noAutoscroll();

    uint8_t x = 0, y = 0;
    setCursor(x, y);

    outString("Type a sentence:\n");

    while (1) {
        doHeartbeat();

        uint8_t u8_c = inChar();
        outChar(u8_c);
        if (u8_c == '\n' || u8_c == '\r') {
            x=0;
            y++;
            if (y == 4) {
                y = 0;
                outString("--------------------\n");
            }
        } else {
            write(u8_c);

            x++;
            if (x == 20) {
                outChar('\n');
                x = 0;
                y++;
                if (y == 4) {
                    y = 0;
                    outString("--------------------\n");
                }
            }
        }

        setCursor(x, y);
        DELAY_MS(10);
    }
}
