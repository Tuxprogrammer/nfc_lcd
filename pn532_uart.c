/*
 * File:   pn532_uart.c
 * Author: root
 *
 * Created on April 26, 2017, 9:24 PM
 */

#include <pic24_all.h>
#include <stdio.h>

//#pragma config FNOSC = FRCPLL // Oscillator Select (Fast RC Oscillator with PLL 4x) 32mhz
//#pragma config FWDTEN = OFF //Watchdog disabled

#define DELAY_RANDOM 3000000
#define RXbuffLen 12

int tempRX, tempRX2;
char buf[10];
char uartRXbuffer[RXbuffLen];

char received;
unsigned int stopwatch;
unsigned int i = 0;



// PN532 Commands
#define PN532_COMMAND_DIAGNOSE (0x00)
#define PN532_COMMAND_GETFIRMWAREVERSION (0x02)
#define PN532_COMMAND_GETGENERALSTATUS (0x04)
#define PN532_COMMAND_READREGISTER (0x06)
#define PN532_COMMAND_WRITEREGISTER (0x08)
#define PN532_COMMAND_READGPIO (0x0C)
#define PN532_COMMAND_WRITEGPIO (0x0E)
#define PN532_COMMAND_SETSERIALBAUDRATE (0x10)
#define PN532_COMMAND_SETPARAMETERS (0x12)
#define PN532_COMMAND_SAMCONFIGURATION (0x14)
#define PN532_COMMAND_POWERDOWN (0x16)
#define PN532_COMMAND_RFCONFIGURATION (0x32)
#define PN532_COMMAND_RFREGULATIONTEST (0x58)
#define PN532_COMMAND_INJUMPFORDEP (0x56)
#define PN532_COMMAND_INJUMPFORPSL (0x46)
#define PN532_COMMAND_INLISTPASSIVETARGET (0x4A)
#define PN532_COMMAND_INATR (0x50)
#define PN532_COMMAND_INPSL (0x4E)
#define PN532_COMMAND_INDATAEXCHANGE (0x40)
#define PN532_COMMAND_INCOMMUNICATETHRU (0x42)
#define PN532_COMMAND_INDESELECT (0x44)
#define PN532_COMMAND_INRELEASE (0x52)
#define PN532_COMMAND_INSELECT (0x54)
#define PN532_COMMAND_INAUTOPOLL (0x60)
#define PN532_COMMAND_TGINITASTARGET (0x8C)
#define PN532_COMMAND_TGSETGENERALBYTES (0x92)
#define PN532_COMMAND_TGGETDATA (0x86)
#define PN532_COMMAND_TGSETDATA (0x8E)
#define PN532_COMMAND_TGSETMETADATA (0x94)
#define PN532_COMMAND_TGGETINITIATORCOMMAND (0x88)
#define PN532_COMMAND_TGRESPONSETOINITIATOR (0x90)
#define PN532_COMMAND_TGGETTARGETSTATUS (0x8A)

#define PN532_RESPONSE_INDATAEXCHANGE (0x41)
#define PN532_RESPONSE_INLISTPASSIVETARGET (0x4B)


#define PN532_MIFARE_ISO14443A (0x00)



char busyUART2(void) {
    /* flag for transission */
    if (!IFS0bits.U2TXIF)
        return 1;
    else {
        IFS0bits.U2TXIF = 0; /* clear flag */
        return 0;
    }

}

void printStringUART2(unsigned char *String) {
    while (*String) {
        WriteCharToUART1(*String++);
    }
}

void WriteCharToUART1(unsigned char Data) {
    U1TXREG = Data;
    while (BusyUART1());
}

void __attribute__((__interrupt__)) _U1RXInterrupt(void) {
    IFS0bits.U1RXIF = 0;


    for (i = 1; i < RXbuffLen; i++)
        uartRXbuffer[i] = U1RXREG;


}

void __attribute__((__interrupt__)) _T1Interrupt(void) {

    TMR1 = 0; //Stop Timer
    stopwatch++; //for Delay_ms() function

    IFS0bits.T1IF = 0; //Clear timer flag
}

void Delay_ms(unsigned int time) {
    stopwatch = 0;
    while (stopwatch < time);
}

void ClearBuffer() {
    unsigned char priv;
    for (priv = 0; priv < RXbuffLen; priv++) {
        uartRXbuffer[priv] = '0';
    }
}

int main(void) {
    configBasic(HELLO_MSG);
    configUART2(DEFAULT_BAUDRATE);

    Init_T1();
    received = 0;
    ClearBuffer();


    ANSBbits.ANSB14 = 0;
    ANSBbits.ANSB15 = 0;
    TRISBbits.TRISB9 = 0;
    LATBbits.LATB14 = 0;




    while (1) {
        doHeartbeat();

        WriteUART1(0x4A);
        Delay_ms(1);
        WriteUART1(0x01);
        Delay_ms(1);
        WriteUART1(0x00);
        Delay_ms(1);
        if (uartRXbuffer[1] == 0x4B) {
            LATBbits.LATB14 = 1;
        } else {
            LATBbits.LATB14 = 0;
        }

    }
    return 0;
}