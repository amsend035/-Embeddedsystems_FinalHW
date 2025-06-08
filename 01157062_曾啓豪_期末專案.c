#include <reg52.h>
#include <string.h>

#define KeyPort P1
#define DataPort P0
sbit LATCH1 = P2^2;
sbit LATCH2 = P2^3;

unsigned char code seg_code[11] = {
    0x3f, 0x06, 0x5b, 0x4f, 0x66,
    0x6d, 0x7d, 0x07, 0x7f, 0x6f, 0x00
}; // 0~9 + ¿Õ°×

unsigned char code alpha_code[] = {
    0x77,0x7C,0x39,0x5E,0x79,0x71,0x3D,0x76,0x06,0x1E,
    0x00,0x38,0x00,0x54,0x5C,0x73,0x67,0x50,0x6D,0x78,
    0x3E,0x1C,0x00,0x00,0x6E,0x5B
}; // A~Z

unsigned char code pos_code[] = {
    0xfe, 0xfd, 0xfb, 0xf7, 0xef, 0xdf, 0xbf, 0x7f
};

unsigned char TempData[8] = {0};
unsigned char buf[5];
unsigned char head = 0;

unsigned char currentCity[4] = "";
unsigned char currentTime[5] = "";
bit hasCity = 0;
bit hasTime = 0;
bit showCity = 1;
unsigned int displayCounter = 0;

void DelayMs(unsigned int ms);
void Display(unsigned char first, unsigned char num);
void SendStr(unsigned char *s);
void InitUART(void);
void Init_Timer0(void);
unsigned char KeyScan(void);

void showLetters(char a, char b, char c) {
    unsigned char i;
    TempData[0] = alpha_code[a - 'A'];
    TempData[1] = alpha_code[b - 'A'];
    TempData[2] = alpha_code[c - 'A'];
    for (i = 3; i < 8; i++) TempData[i] = seg_code[10];
}

void showDigits(char *s) {
    unsigned char i;
    for (i = 0; i < 4; i++) {
        if (s[i] >= '0' && s[i] <= '9')
            TempData[i] = seg_code[s[i] - '0'];
        else
            TempData[i] = seg_code[10];
    }
    for (i = 4; i < 8; i++) TempData[i] = seg_code[10];
}

void main(void) {
    unsigned char num;
    InitUART();
    Init_Timer0();

    while (1) {
        num = KeyScan();
        if (num) {
            head = 0;
            hasCity = 1;
            hasTime = 0;
            showCity = 1;
            displayCounter = 0;

            switch (num) {
                case 1: SendStr("TPE"); strcpy(currentCity, "TPE"); showLetters('T','P','E'); break;
                case 2: SendStr("TYO"); strcpy(currentCity, "TYO"); showLetters('T','Y','O'); break;
                case 3: SendStr("PAR"); strcpy(currentCity, "PAR"); showLetters('P','A','R'); break;
                case 4: SendStr("NYC"); strcpy(currentCity, "NYC"); showLetters('N','Y','C'); break;
                case 5: SendStr("SYD"); strcpy(currentCity, "SYD"); showLetters('S','Y','D'); break;
                case 6: SendStr("LON"); strcpy(currentCity, "LON"); showLetters('L','O','N'); break;
                case 7: SendStr("BER"); strcpy(currentCity, "BER"); showLetters('B','E','R'); break;
                case 8: SendStr("SFO"); strcpy(currentCity, "SFO"); showLetters('S','F','O'); break;
            }
        }

        if (head == 4) {
            buf[head] = '\0';
            head = 0;
            strcpy(currentTime, buf);
            hasTime = 1;
            showCity = 0;
            displayCounter = 0;
        }

        if (hasCity && hasTime) {
            displayCounter++;
            if (displayCounter >= 20000) {
                displayCounter = 0;
                showCity = !showCity;
                if (showCity)
                    showLetters(currentCity[0], currentCity[1], currentCity[2]);
                else
                    showDigits(currentTime);
            }
        }
    }
}

void InitUART(void) {
    SCON = 0x50;
    TMOD |= 0x20;
    TH1 = 0xFD;
    TR1 = 1;
    EA = 1;
    ES = 1;
}

void Init_Timer0(void) {
    TMOD |= 0x01;
    EA = 1;
    ET0 = 1;
    TR0 = 1;
}

void UART_SER(void) interrupt 4 {
    if (RI) {
        RI = 0;
        buf[head++] = SBUF;
        if (head > 4) head = 4;
    }
}

void SendByte(unsigned char dat) {
    SBUF = dat;
    while (!TI);
    TI = 0;
}

void SendStr(unsigned char *s) {
    while (*s) {
        SendByte(*s);
        s++;
    }
}

void Display(unsigned char first, unsigned char num) {
    static unsigned char i = 0;
    DataPort = 0;
    LATCH1 = 1; LATCH1 = 0;

    DataPort = pos_code[i + first];
    LATCH2 = 1; LATCH2 = 0;

    DataPort = TempData[i];
    LATCH1 = 1; LATCH1 = 0;

    i++;
    if (i == num) i = 0;
}

void Timer0_isr(void) interrupt 1 {
    TH0 = (65536 - 2000) / 256;
    TL0 = (65536 - 2000) % 256;
    Display(0, 8);
}

void DelayMs(unsigned int t) {
    unsigned char i;
    while (t--) {
        for (i = 0; i < 120; i++);
    }
}

unsigned char KeyScan(void) {
    unsigned char keyvalue;
    if (KeyPort != 0xFF) {
        DelayMs(10);
        if (KeyPort != 0xFF) {
            keyvalue = KeyPort;
            while (KeyPort != 0xFF);
            switch (keyvalue) {
                case 0xfe: return 1;
                case 0xfd: return 2;
                case 0xfb: return 3;
                case 0xf7: return 4;
                case 0xef: return 5;
                case 0xdf: return 6;
                case 0xbf: return 7;
                case 0x7f: return 8;
                default: return 0;
            }
        }
    }
    return 0;
}

