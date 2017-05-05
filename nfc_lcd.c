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
#include "pic24_all.h"
#include <stdio.h>
#include "sclcd.h"
#include "pn532_hsu.h"

#define LCD_ADDR 0x4E

void printHexArray(uint8_t* tmp, uint8_t n) {
    uint8_t i;
    for (i = 0; i < n; i++) {
        outUint8(tmp[i]);
        outChar(' ');
    }
}

int main(void) {
    configBasic(HELLO_MSG);
    configUART2(115200);
    configI2C1(100);
    outString("Starting...\n");
    outString("lcd_init\n");
    lcd_init(LCD_ADDR, 20, 4);
    outString("noBacklight\n");
    noBacklight();
    outString("clear\n");
    clear();
    outString("cursor\n");
    cursor();
    outString("blink\n");
    blink();
    outString("noAutoScroll\n");
    noAutoscroll();
    setCursor(0, 0);
    lcd_outString("Scan a tag...");

    uint8_t success;
    uint8_t uid[] = {0, 0, 0, 0, 0, 0, 0};
    uint8_t uidLength;

    wakeup();
    wakeup();
    getFirmwareVersion();
    wakeup();
    SAMConfig();

    int i;
    for (i = 0; i < 2; i++) {
        clear();
        setCursor(0, 0);
        lcd_outString("                    ");
    }
    outString("Backlight\n");
    backlight();
    while (1) {
        doHeartbeat();

        clear();
        setCursor(0, 0);
        lcd_outString("Scan a tag...");
        success = readPassiveTargetIDNoInlist(PN532_MIFARE_ISO14443A, uid, &uidLength);

        outString("Found an ISO14443A card\n");
        outString("  UID Length:");
        outUint8Decimal(uidLength);
        outString(" bytes\n");
        outString("  UID Value:");
        printHexArray(uid, uidLength);
        clear();
        setCursor(0, 0);
        lcd_outString("UID: ");
        lcd_printHexArray(uid, uidLength);
        outChar('\n');
        lcd_outChar('\n');


        if (uidLength == 4) {
            // We probably have a Mifare Classic card ... 
            outString("Seems to be a Mifare Classic card (4 byte UID)\n");

            // Now we need to try to authenticate it for read/write access
            // Try with the factory default KeyA: 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF
            outString("Trying to authenticate block 1-63 with default KEYA value\n");
            uint8_t keya[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

            // Start with block 4 (the first block of sector 1) since sector 0
            // contains the manufacturer data and it's probably better just
            // to leave it alone unless you know what you're doing
            uint8_t data[16];
            uint8_t successData;
                successData = mifareclassic_AuthenticateBlock(uid, uidLength, 4, 0, keya);
                if (successData) {
                    outString("\nSector has been authenticated\n");

                    successData = mifareclassic_ReadDataBlock(4, data);
                } else {
                    outString("Ooops ... authentication failed: Try another key?\n");
                }
        
            if (successData) {
                    // Data seems to have been read ... spit it out
                    lcd_outString("Data: ");
                    printHexArray(data, 16);
                    lcd_printHexArray(data, 16);
                    lcd_outChar('\n');

                    DELAY_MS(1000);

                    clear();
                    lcd_outString(data);
                    lcd_outChar('\n');

                    DELAY_MS(2000);
                    // Wait a bit before reading the card again
                } else {
                    outString("Ooops ... unable to read the requested block.  Try another key?\n");
                }

            DELAY_MS(1000);
        }

    }
}//end main
