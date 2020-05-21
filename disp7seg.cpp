//-----------------------------------------------------------------------------
#include <stdlib.h>
#include <string.h>
#include "lib/avr-misc/avr-misc.h"
#include "lib/avr-debug/debug.h"
#include "disp7seg.h"
#include "config.h"
//-----------------------------------------------------------------------------
// 7-segments codes
const BYTE code7seg[] =
{
//    0     1     2     3     4     5     6     7     8     9
    0x5F, 0x44, 0x3D, 0x75, 0x66, 0x73, 0x7B, 0x45, 0x7F, 0x77,
//    :     ;     <     =     >     ?     @     A     b     C
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x6F, 0x7A, 0x1B,
//    d     E     F     G     H     I     J     K     L     M
    0x7C, 0x3B, 0x2B, 0x5B, 0x6E, 0x0A, 0x00, 0x00, 0x1A, 0x00,
//    n     O     P     Q     r     S     t     U
    0x68, 0x5F, 0x2F, 0x00, 0x28, 0x73, 0x3A, 0x1E
};
//-----------------------------------------------------------------------------
Display7Seg::Display7Seg(BYTE segCount)
{
    m_data = (BYTE*) malloc(segCount);
    m_segCount = segCount;
    m_dot = 0;
    m_position = 0;
}
//-----------------------------------------------------------------------------
void Display7Seg::print(BYTE value)
{
    m_value = value;
    for(BYTE i = 0; i < m_segCount; i++)
    {
        m_data[i] = code7seg[value % 10];
        value /= 10;
    }
}
//-----------------------------------------------------------------------------
void Display7Seg::print(const char* str)
{
    m_value = 0xFFFF;
    memset(m_data, 0, m_segCount);
    register BYTE size = min(strlen(str), m_segCount);
    for(BYTE i = 0; i < size; i++)
    {
        m_data[size - i - 1] = code7seg[(BYTE) str[i] - 48];
    }
}
//-----------------------------------------------------------------------------
void Display7Seg::refresh()
{
    switch(m_position)
    {
    case 0:
        PORTC = (PORTC | (7 << 3)) & ~(1 << 3);
        DISPLAY_PORT = m_data[m_position] | (m_dot << 7);
        break;
    case 1:
        PORTC = (PORTC | (7 << 3)) & ~(1 << 4);
        DISPLAY_PORT = (m_value >= 10) ? m_data[m_position] : 0;
        break;
    case 2:
        PORTC = (PORTC | (7 << 3)) & ~(1 << 5);
        DISPLAY_PORT = (m_value >= 100) ? m_data[m_position] : 0;
        break;
    }
    m_position = (m_position < 2) ? m_position + 1 : 0;
}
//-----------------------------------------------------------------------------
void Display7Seg::showDot()
{
    m_dot = 1;
}
//-----------------------------------------------------------------------------
void Display7Seg::hideDot()
{
    m_dot = 0;
}
//-----------------------------------------------------------------------------
