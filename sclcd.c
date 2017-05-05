/*
 * File:   lcd.c
 * Author: root
 *
 * Created on April 19, 2017, 9:15 PM
 */
#include "sclcd.h"

//uint8_t _addr, _cols, _rows, _backlightval;

uint8_t _addr;
uint8_t _displayfunction;
uint8_t _displaycontrol;
uint8_t _displaymode;
uint8_t _numlines;
uint8_t _cols;
uint8_t _rows;
uint8_t _backlightval;

uint8_t x = 0, y = 0;

void lcd_init(uint8_t lcd_addr, uint8_t lcd_cols, uint8_t lcd_rows) {
    _addr = lcd_addr;
    _cols = lcd_cols;
    _rows = lcd_rows;
    _backlightval = LCD_NOBACKLIGHT;

    init_priv();
}

void init_priv() {
    _displayfunction = LCD_4BITMODE | LCD_1LINE | LCD_5x8DOTS;
    begin(_cols, _rows);
}

void begin(uint8_t cols, uint8_t lines) {
    uint8_t charsize = LCD_5x8DOTS;
    beginc(cols, lines, charsize);
}

void beginc(uint8_t cols, uint8_t lines, uint8_t dotsize) {
    if (lines > 1) {
        _displayfunction |= LCD_2LINE;
    }
    _numlines = lines;

    if ((dotsize != 0) && (lines == 1)) {
        _displayfunction |= LCD_5x10DOTS;
    }

    //LCD_Open();

    DELAY_MS(100);

    noBacklight();
    DELAY_MS(1000);

    write4bits(0x03);
    DELAY_US(4500);

    write4bits(0x03);
    DELAY_US(4500);

    write4bits(0x03);
    DELAY_US(150);

    write4bits(0x02);

    command(LCD_FUNCTIONSET | _displayfunction);

    _displaycontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;
    display();

    clear();

    _displaymode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
    command(LCD_ENTRYMODESET | _displaymode);

    backlight();
    home();
    setCursor(x, y);
    //LCD_Close();
}

void clear() {
    command(LCD_CLEARDISPLAY);
    DELAY_US(2000);
}

void home() {
    command(LCD_RETURNHOME);
    DELAY_US(2000);
}

void setCursor(uint8_t col, uint8_t row) {
    x = col;
    y = row;
    int row_offsets[] = {0x00, 0x40, 0x14, 0x54};
    if (row > _numlines) {
        row = _numlines - 1;
    }
    command(LCD_SETDDRAMADDR | (col + row_offsets[row]));
}

void noDisplay() {
    _displaycontrol &= ~LCD_DISPLAYON;
    command(LCD_DISPLAYCONTROL | _displaycontrol);
}

void display() {
    _displaycontrol |= LCD_DISPLAYON;
    command(LCD_DISPLAYCONTROL | _displaycontrol);
}

void noCursor() {
    _displaycontrol &= ~LCD_CURSORON;
    command(LCD_DISPLAYCONTROL | _displaycontrol);
}

void cursor() {
    _displaycontrol |= LCD_CURSORON;
    command(LCD_DISPLAYCONTROL | _displaycontrol);
}

void noBlink() {
    _displaycontrol &= ~LCD_BLINKON;
    command(LCD_DISPLAYCONTROL | _displaycontrol);
}

void blink() {
    _displaycontrol |= LCD_BLINKON;
    command(LCD_DISPLAYCONTROL | _displaycontrol);
}

void scrollDisplayLeft() {
    command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
}

void scrollDisplayRight() {
    command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
}

void leftToRight() {
    _displaymode |= LCD_ENTRYLEFT;
    command(LCD_ENTRYMODESET | _displaymode);
}

void rightToLeft() {
    _displaymode &= ~LCD_ENTRYLEFT;
    command(LCD_ENTRYMODESET | _displaymode);
}

void autoscroll() {
    _displaymode |= LCD_ENTRYSHIFTINCREMENT;
    command(LCD_ENTRYMODESET | _displaymode);
}

void noAutoscroll() {
    _displaymode &= ~LCD_ENTRYSHIFTINCREMENT;
    command(LCD_ENTRYMODESET | _displaymode);
}

void createChar(uint8_t location, uint8_t charmap[]) {
    location &= 0x7;
    command(LCD_SETCGRAMADDR | (location << 3));
    int i;
    for (i = 0; i < 8; i++) {
        write(charmap[i]);
    }
}

void noBacklight() {
    _backlightval = LCD_NOBACKLIGHT;
    expanderWrite(0);
}

void backlight() {
    _backlightval = LCD_BACKLIGHT;
    expanderWrite(0);
}

void command(uint8_t value) {
    send(value, 0);
}

int write(uint8_t value) {
    send(value, Rs);
    return 1;
}

void send(uint8_t value, uint8_t mode) {
    uint8_t highnib = value & 0xf0;
    uint8_t lownib = (value << 4)&0xf0;
    write4bits((highnib) | mode);
    write4bits((lownib) | mode);
}

void write4bits(uint8_t value) {
    expanderWrite(value);
    pulseEnable(value);
}

void expanderWrite(uint8_t _data) {
    //printf("putI2C1((int) _data | _backlightval)\n");
    uint8_t data = (int) _data | _backlightval;

    startI2C1();
    //    uint8_t res, res2;
    //    res = putNoAckCheckI2C1(_addr);
    //    res2 = putNoAckCheckI2C1(data);
    putI2C1(_addr);
    putI2C1(data);
    stopI2C1();
}

void pulseEnable(uint8_t _data) {
    expanderWrite(_data | En);
    DELAY_US(1);

    expanderWrite(_data & ~En);
    DELAY_US(50);
}

void cursor_on() {
    cursor();
}

void cursor_off() {
    noCursor();
}

void blink_on() {
    blink();
}

void blink_off() {
    noBlink();
}

void load_custom_character(uint8_t char_num, uint8_t* rows) {
    createChar(char_num, rows);
}

void setBacklight(uint8_t new_val) {
    if (new_val) {
        backlight();
    } else {
        noBacklight();
    }
}

void lcd_outChar(uint8_t u8_c) {
    if (u8_c == '\n' || u8_c == '\r') {
        x = 0;
        y++;
        if (y == 4) {
            y = 0;
        }
    } else {
        write(u8_c);
        x++;
        if (x == 20) {
            x = 0;
            y++;
            if (y == 4) {
                y = 0;
            }
        }
    }


    setCursor(x, y);
    DELAY_MS(5);
}

void lcd_outUint8(uint8_t u8_x) {
    //lcd_outString("0x");
    lcd_outUint8NoLeader(u8_x);
}

void lcd_outUint16(uint16_t u16_x) {
    uint8_t u8_c;

    //lcd_outString("0x");
    u8_c = (u16_x >> 8);
    lcd_outUint8NoLeader(u8_c);
    u8_c = (uint8_t) u16_x;
    lcd_outUint8NoLeader(u8_c);
}

void lcd_outUint32(uint32_t u32_x) {
    uint8_t u8_c;
    //lcd_outString("0x");
    u8_c = (u32_x >> 24);
    lcd_outUint8NoLeader(u8_c);
    u8_c = (u32_x >> 16);
    lcd_outUint8NoLeader(u8_c);
    u8_c = (u32_x >> 8);
    lcd_outUint8NoLeader(u8_c);
    u8_c = u32_x;
    lcd_outUint8NoLeader(u8_c);
}

void lcd_outUint8NoLeader(uint8_t u8_x) {
    uint8_t u8_c;
    u8_c = (u8_x >> 4)& 0xf;
    if (u8_c > 9) lcd_outChar('A' + u8_c - 10);
    else lcd_outChar('0' + u8_c);


    //LSDigit
    u8_c = u8_x & 0xf;
    if (u8_c > 9) lcd_outChar('A' + u8_c - 10);
    else lcd_outChar('0' + u8_c);
}

void lcd_outString(const char c[]) {
    writestr(c);
}

size_t writestr(const char *str) {
    if (str == NULL) return 0;
    return writestr3((const uint8_t *) str, strlen(str));
}

size_t writestr2(const char *buffer, size_t size) {
    return writestr3((const uint8_t *) buffer, size);
}

size_t writestr3(const uint8_t *buffer, size_t size) {
    size_t n = 0;
    while (size--) {
        uint8_t u8_c = *buffer++;
        lcd_outChar(u8_c);
        n++;

    }
    return n;
}

void lcd_printHexArray(uint8_t* tmp, uint8_t n) {
    uint8_t i;
    for (i = 0; i < n; i++) {
        lcd_outUint8(tmp[i]);
        if (x != 0) {
            lcd_outChar(' ');
        }
    }
}

//Unsupported

void off() {
}

void on() {
}

void setDelay(int cmdDelay, int charDelay) {
}

uint8_t status() {
    return 0;
}

uint8_t keypad() {
    return 0;
}

uint8_t init_bargraph(uint8_t graphtype) {
    return 0;
}

void draw_horizontal_graph(uint8_t row, uint8_t column, uint8_t len, uint8_t pixel_col_end) {
}

void draw_vertical_graph(uint8_t row, uint8_t column, uint8_t len, uint8_t pixel_row_end) {
}

void setContrast(uint8_t new_val) {
}
