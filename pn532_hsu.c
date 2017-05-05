/*
 * File:   pn532_hsu.c
 * Author: root
 *
 * Created on April 26, 2017, 10:42 PM
 */

#include "pn532_hsu.h"

uint8_t nfccommand;
uint8_t _uid[7]; // ISO14443A uid
uint8_t _uidLen; // uid len
uint8_t _key[6]; // Mifare Classic key
uint8_t inListedTag; // Tg number of inlisted tag.

uint8_t pn532_packetbuffer[64];

uint16_t millis() {
    return 0;
}

int8_t writeCommandNoBody(uint8_t *header, uint8_t hlen) {
    return writeCommand(header, hlen, 0, 0);
}

int8_t writeCommand(uint8_t *header, uint8_t hlen, uint8_t *body, uint8_t blen) {
    if (isCharReady2()) {
        outString("Dump serial buffer: ");
    }
    while (isCharReady2()) {
        uint8_t ret = inChar2();
        outUint8(ret);
        //lcd_outUint8(ret);
        outChar(' ');
        //lcd_outChar(' ');
    }

    nfccommand = header[0];

    outChar2(PN532_PREAMBLE);
    outChar2(PN532_STARTCODE1);
    outChar2(PN532_STARTCODE2);

    uint8_t len = hlen + blen + 1;
    outChar2(len);
    outChar2(~len + 1);

    outChar2(PN532_HOSTTOPN532);
    uint8_t sum = PN532_HOSTTOPN532;

    outString("\nWrite: ");
    //lcd_outString("\nWrite: ");
    uint8_t i;
    for (i = 0; i < hlen; i++) {
        outChar2(header[i]);
        sum += header[i];
        outUint8(header[i]);
        //lcd_outUint8(header[i]);
        outChar(' ');
        //lcd_outChar(' ');
    }

    for (i = 0; i < blen; i++) {
        outChar2(body[i]);
        sum += body[i];
        outUint8(body[i]);
        //lcd_outUint8(body[i]);
        outChar(' ');
        //lcd_outChar(' ');
    }
    
    uint8_t checksum = ~sum + 1;
    outChar2(checksum);
    outChar2(PN532_POSTAMBLE);

    return readAckFrame();
}

int16_t readResponse(uint8_t buf[], uint8_t len) {
    uint8_t tmp[3];
    DELAY_MS(100);
    outString("\nRead:  ");
    //lcd_outString("\nRead:  ");
    if (receive(tmp, 3) <=0) {
        return PN532_TIMEOUT;
    }
    if (0 != tmp[0] || 0!= tmp[1] || 0xFF != tmp[2]) {
        outString("Preamble error");
        //lcd_outString("Preamble error");
        return PN532_INVALID_FRAME;
    }
    
    uint8_t length[2];
    if(receive(length,2) <= 0) {
        return PN532_TIMEOUT;
    }
    if(0!=(uint8_t)(length[0]+length[1])){
        outString("Length error");
        //lcd_outString("Length error");
        return PN532_INVALID_FRAME;
    }
    length[0] -=2;
    if (length[0] > len) {
        return PN532_NO_SPACE;
    }
    
    uint8_t cmd = nfccommand +1;
    if (receive(tmp, 2) <= 0) {
        return PN532_TIMEOUT;
    }
    if (PN532_PN532TOHOST != tmp[0] || cmd != tmp[1]) {
        outString("Command error");
        //lcd_outString("Command error");
        return PN532_INVALID_FRAME;
    }
    
    if(receive(buf, length[0]) != length[0]) {
        return PN532_TIMEOUT;
    }
    
    uint8_t sum = PN532_PN532TOHOST + cmd;
    uint8_t i;
    for(i=0; i<length[0]; i++){
        sum += buf[i];
    }
    
    /** checksum and postamble */
    if(receive(tmp, 2) <= 0){
        return PN532_TIMEOUT;
    }
    if( 0 != (uint8_t)(sum + tmp[0]) || 0 != tmp[1] ){
        outString("Checksum error");
        //lcd_outString("Checksum error");
        return PN532_INVALID_FRAME;
    }
    
    outChar('\n');
    return length[0];
}

int8_t readAckFrame() {
    const uint8_t PN532_ACK[] = {0, 0, 0xFF, 0, 0xFF, 0};
    uint8_t ackBuf[sizeof (PN532_ACK)];

    DELAY_MS(100);

    outString("\nAck: ");
    //lcd_outString("\nAck: ");
    
    if (receive(ackBuf, sizeof (PN532_ACK)) <= 0) {
        outString("Timeout\n");
        //lcd_outString("Timeout\n");
        return PN532_TIMEOUT;
    }

    if (memcmp(ackBuf, PN532_ACK, sizeof (PN532_ACK))) {
        outString("Invalid\n");
        //lcd_outString("Invalid\n");
        return PN532_INVALID_ACK;
    }
    return 0;
}

int8_t receive(uint8_t *buf, int len) {
    int read_bytes = 0;
    int ret;
    unsigned long start_millis;

    while (read_bytes < len) {
        start_millis = millis();
        do {
            ret = (isCharReady2()) ? inChar2(): -1;
            if (ret >= 0) {
                break;
            }
            DELAY_MS(10);
            start_millis++;
        } while (1);

        if (ret < 0) {
            if (read_bytes) {
                return read_bytes;
            } else {
                return PN532_TIMEOUT;
            }
        }
        buf[read_bytes] = (uint8_t) ret;
        outUint8(ret);
        //lcd_outUint8(ret);
        outChar(' ');
        //lcd_outChar(' ');
        read_bytes++;
    }
    return read_bytes;
}


int SAMConfig() {
    pn532_packetbuffer[0] = PN532_COMMAND_SAMCONFIGURATION;
    pn532_packetbuffer[1] = 0x01;
    pn532_packetbuffer[2] = 0x14;
    pn532_packetbuffer[3] = 0x01;
    
    outString("\n\nSAMConfig\n");
    if(writeCommandNoBody(pn532_packetbuffer, 4))
        return 0;
    
    return (0 < readResponse(pn532_packetbuffer, sizeof(pn532_packetbuffer)));
}

void getFirmwareVersion() {
    uint32_t response;
        pn532_packetbuffer[0] = PN532_COMMAND_GETFIRMWAREVERSION;

        if (writeCommandNoBody(pn532_packetbuffer, 1)) {
            outString("Fail");
        }

        int16_t status = readResponse(pn532_packetbuffer, sizeof (pn532_packetbuffer));

        if (0 > status) {
            outString("Fail");
        }

        response = pn532_packetbuffer[0];
        response <<= 8;
        response |= pn532_packetbuffer[1];
        response <<= 8;
        response |= pn532_packetbuffer[2];
        response <<= 8;
        response |= pn532_packetbuffer[3];
        
        outUint32(response);
}

void wakeup() {
    outChar2(0x55);
    outChar2(0x55);
    outChar2(0);
    outChar2(0);
    outChar2(0);
    
    /** dump serial buffer */
    if(isCharReady2()){
        outString("\nDump serial buffer: ");
    }
    while(isCharReady2()){
        uint8_t ret = inChar2();
        outUint8(ret);
    }
}

int readPassiveTargetIDNoInlist(uint8_t cardbaudrate, uint8_t *uid, uint8_t *uidLength){
    return readPassiveTargetID(cardbaudrate, uid, uidLength, 0);
}

int readPassiveTargetID(uint8_t cardbaudrate, uint8_t *uid, uint8_t *uidLength, int inlist) {
    pn532_packetbuffer[0] = PN532_COMMAND_INLISTPASSIVETARGET;
    pn532_packetbuffer[1] = 1;  // max 1 cards at once (we can set this to 2 later)
    pn532_packetbuffer[2] = cardbaudrate;

    if (writeCommandNoBody(pn532_packetbuffer, 3)) {
        return 0x0;  // command failed
    }

    outString("\n\nreadPassiveTargetID\n");
    // read data packet
    if (readResponse(pn532_packetbuffer, sizeof(pn532_packetbuffer)) < 0) {
        return 0x0;
    }

    // check some basic stuff
    /* ISO14443A card response should be in the following format:
      byte            Description
      -------------   ------------------------------------------
      b0              Tags Found
      b1              Tag Number (only one used in this example)
      b2..3           SENS_RES
      b4              SEL_RES
      b5              NFCID Length
      b6..NFCIDLen    NFCID
    */

    if (pn532_packetbuffer[0] != 1)
        return 0;

    uint16_t sens_res = pn532_packetbuffer[2];
    sens_res <<= 8;
    sens_res |= pn532_packetbuffer[3];

    outString("\nATQA: 0x");  
    outUint16(sens_res);
    outChar(' ');
    outString("SAK: 0x");  outUint8(pn532_packetbuffer[4]);
    outString("\n\n");

    /* Card appears to be Mifare Classic */
    *uidLength = pn532_packetbuffer[5];

    uint8_t i;
    for (i = 0; i < pn532_packetbuffer[5]; i++) {
        uid[i] = pn532_packetbuffer[6 + i];
    }

    if (inlist) {
        inListedTag = pn532_packetbuffer[1];
    }

    return 1;
}

uint8_t mifareclassic_AuthenticateBlock(uint8_t *uid, uint8_t uidLen, uint32_t blockNumber, uint8_t keyNumber, uint8_t *keyData) {
    uint8_t i;

    // Hang on to the key and uid data
    memcpy (_key, keyData, 6);
    memcpy (_uid, uid, uidLen);
    _uidLen = uidLen;

    // Prepare the authentication command //
    pn532_packetbuffer[0] = PN532_COMMAND_INDATAEXCHANGE;   /* Data Exchange Header */
    pn532_packetbuffer[1] = 1;                              /* Max card numbers */
    pn532_packetbuffer[2] = (keyNumber) ? MIFARE_CMD_AUTH_B : MIFARE_CMD_AUTH_A;
    pn532_packetbuffer[3] = blockNumber;                    /* Block Number (1K = 0..63, 4K = 0..255 */
    memcpy (pn532_packetbuffer + 4, _key, 6);
    for (i = 0; i < _uidLen; i++) {
        pn532_packetbuffer[10 + i] = _uid[i];              /* 4 bytes card ID */
    }

    if (writeCommandNoBody(pn532_packetbuffer, 10 + _uidLen))
        return 0;

    // Read the response packet
    readResponse(pn532_packetbuffer, sizeof(pn532_packetbuffer));

    // Check if the response is valid and we are authenticated???
    // for an auth success it should be bytes 5-7: 0xD5 0x41 0x00
    // Mifare auth error is technically byte 7: 0x14 but anything other and 0x00 is not good
    if (pn532_packetbuffer[0] != 0x00) {
        outString("Authentification failed\n");
        return 0;
    }

    return 1;
}

uint8_t mifareclassic_ReadDataBlock (uint8_t blockNumber, uint8_t *data)
{
    outString("Trying to read 16 bytes from block ");
    outUint8Decimal(blockNumber);
    outChar('\n');

    /* Prepare the command */
    pn532_packetbuffer[0] = PN532_COMMAND_INDATAEXCHANGE;
    pn532_packetbuffer[1] = 1;                      /* Card number */
    pn532_packetbuffer[2] = MIFARE_CMD_READ;        /* Mifare Read command = 0x30 */
    pn532_packetbuffer[3] = blockNumber;            /* Block Number (0..63 for 1K, 0..255 for 4K) */

    /* Send the command */
    if (writeCommandNoBody(pn532_packetbuffer, 4)) {
        return 0;
    }

    /* Read the response packet */
    readResponse(pn532_packetbuffer, sizeof(pn532_packetbuffer));

    /* If byte 8 isn't 0x00 we probably have an error */
    if (pn532_packetbuffer[0] != 0x00) {
        return 0;
    }

    /* Copy the 16 data bytes to the output buffer        */
    /* Block content starts at byte 9 of a valid response */
    memcpy (data, pn532_packetbuffer + 1, 16);

    return 1;
}

uint8_t mifareultralight_ReadPage (uint8_t page, uint8_t *buffer)
{
    if (page >= 64) {
        outString("Page value out of range\n");
        return 0;
    }

    /* Prepare the command */
    pn532_packetbuffer[0] = PN532_COMMAND_INDATAEXCHANGE;
    pn532_packetbuffer[1] = 1;                   /* Card number */
    pn532_packetbuffer[2] = MIFARE_CMD_READ;     /* Mifare Read command = 0x30 */
    pn532_packetbuffer[3] = page;                /* Page Number (0..63 in most cases) */

    /* Send the command */
    if (writeCommandNoBody(pn532_packetbuffer, 4)) {
        return 0;
    }

    /* Read the response packet */
    readResponse(pn532_packetbuffer, sizeof(pn532_packetbuffer));

    /* If byte 8 isn't 0x00 we probably have an error */
    if (pn532_packetbuffer[0] == 0x00) {
        /* Copy the 4 data bytes to the output buffer         */
        /* Block content starts at byte 9 of a valid response */
        /* Note that the command actually reads 16 bytes or 4  */
        /* pages at a time ... we simply discard the last 12  */
        /* bytes                                              */
        memcpy (buffer, pn532_packetbuffer + 1, 4);
    } else {
        return 0;
    }

    // Return OK signal
    return 1;
}

uint8_t mifareclassic_WriteDataBlock (uint8_t blockNumber, uint8_t *data)
{
    /* Prepare the first command */
    pn532_packetbuffer[0] = PN532_COMMAND_INDATAEXCHANGE;
    pn532_packetbuffer[1] = 1;                      /* Card number */
    pn532_packetbuffer[2] = MIFARE_CMD_WRITE;       /* Mifare Write command = 0xA0 */
    pn532_packetbuffer[3] = blockNumber;            /* Block Number (0..63 for 1K, 0..255 for 4K) */
    memcpy (pn532_packetbuffer + 4, data, 16);        /* Data Payload */

    /* Send the command */
    if (writeCommandNoBody(pn532_packetbuffer, 20)) {
        return 0;
    }

    /* Read the response packet */
    return (0 < readResponse(pn532_packetbuffer, sizeof(pn532_packetbuffer)));
}