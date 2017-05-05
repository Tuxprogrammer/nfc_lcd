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
#include "pn532_hsu.h"

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


    uint8_t success;
    uint8_t uid[] = {0, 0, 0, 0, 0, 0, 0};
    uint8_t uidLength;

    DELAY_MS(1000);
    wakeup();
    wakeup();
    getFirmwareVersion();
    wakeup();
    SAMConfig();

    while (1) {
        doHeartbeat();

        success = readPassiveTargetIDNoInlist(PN532_MIFARE_ISO14443A, uid, &uidLength);

        outString("Found an ISO14443A card\n");
        outString("  UID Length:");
        outUint8Decimal(uidLength);
        outString(" bytes\n");
        outString("  UID Value:");
        printHexArray(uid, uidLength);
        outChar('\n');


        if (uidLength == 4) {
            // We probably have a Mifare Classic card ... 
            outString("Seems to be a Mifare Classic card (4 byte UID)\n");

            // Now we need to try to authenticate it for read/write access
            // Try with the factory default KeyA: 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF
            outString("Trying to authenticate block 4 with default KEYA value\n");
            uint8_t keya[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

            // Start with block 4 (the first block of sector 1) since sector 0
            // contains the manufacturer data and it's probably better just
            // to leave it alone unless you know what you're doing
            success = mifareclassic_AuthenticateBlock(uid, uidLength, 4, 0, keya);

            if (success) {
                outString("\nSector 1 (Blocks 4..7) has been authenticated\n");
                uint8_t data[16];

                // If you want to write something to block 4 to test with, uncomment
                // the following line and this text should be read back in a minute
                // data = { 'a', 'd', 'a', 'f', 'r', 'u', 'i', 't', '.', 'c', 'o', 'm', 0, 0, 0, 0};
                // success = nfc.mifareclassic_WriteDataBlock (4, data);

                // Try to read the contents of block 4
                success = mifareclassic_ReadDataBlock(4, data);

                if (success) {
                    // Data seems to have been read ... spit it out
                    outString("\nReading Block 4:\n\n");
                    printHexArray(data, 16);
                    outChar('\n');

                    // Wait a bit before reading the card again
                } else {
                    outString("Ooops ... unable to read the requested block.  Try another key?\n");
                }
            } else {
                outString("Ooops ... authentication failed: Try another key?\n");
            }
        }

        if (uidLength == 7) {
            // We probably have a Mifare Ultralight card ...
            outString("Seems to be a Mifare Ultralight tag (7 byte UID)\n");

            // Try to read the first general-purpose user page (#4)
            outString("Reading page 4\n");
            uint8_t data[32];
            success = mifareultralight_ReadPage(4, data);
            if (success) {
                // Data seems to have been read ... spit it out
                printHexArray(data, 4);
                outChar('\n');
                // Wait a bit before reading the card again
            } else {
                outString("Ooops ... unable to read the requested page!?\n");
            }
        }

        DELAY_MS(5000);

    }
}//end main
