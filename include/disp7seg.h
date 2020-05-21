#ifndef DISP7SEG_H
#define DISP7SEG_H
//-----------------------------------------------------------------------------
class Display7Seg
{
public:
    Display7Seg(BYTE segCount);
    void print(BYTE value);
    void print(const char* str);
    void refresh();
    void showDot();
    void hideDot();
private:
    BYTE* m_data;
    BYTE m_segCount;
    WORD m_value;
    BYTE m_dot;
    BYTE m_position;
};
//-----------------------------------------------------------------------------
#endif
