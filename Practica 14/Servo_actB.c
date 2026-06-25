#include <xc.h>

#pragma config FOSC = HS
#pragma config WDTE = OFF
#pragma config PWRTE = OFF
#pragma config BOREN = ON
#pragma config LVP = OFF
#pragma config CPD = OFF
#pragma config WRT = OFF
#pragma config CP = OFF

#define _XTAL_FREQ 8000000

#define SERVO PORTDbits.RD0

// Rango de operacion
void p00(void)  { SERVO=1; __delay_us(400);  SERVO=0; __delay_ms(18); }
void p01(void)  { SERVO=1; __delay_us(500);  SERVO=0; __delay_ms(18); }
void p02(void)  { SERVO=1; __delay_us(600);  SERVO=0; __delay_ms(18); }
void p03(void)  { SERVO=1; __delay_us(700);  SERVO=0; __delay_ms(18); }
void p04(void)  { SERVO=1; __delay_us(800);  SERVO=0; __delay_ms(18); }
void p05(void)  { SERVO=1; __delay_us(900);  SERVO=0; __delay_ms(18); }
void p06(void)  { SERVO=1; __delay_us(1000); SERVO=0; __delay_ms(18); }
void p07(void)  { SERVO=1; __delay_us(1100); SERVO=0; __delay_ms(18); }
void p08(void)  { SERVO=1; __delay_us(1200); SERVO=0; __delay_ms(18); }
void p09(void)  { SERVO=1; __delay_us(1300); SERVO=0; __delay_ms(18); }
void p10(void)  { SERVO=1; __delay_us(1400); SERVO=0; __delay_ms(18); }
void p11(void)  { SERVO=1; __delay_us(1500); SERVO=0; __delay_ms(18); }
void p12(void)  { SERVO=1; __delay_us(1600); SERVO=0; __delay_ms(18); }
void p13(void)  { SERVO=1; __delay_us(1700); SERVO=0; __delay_ms(18); }
void p14(void)  { SERVO=1; __delay_us(1800); SERVO=0; __delay_ms(18); }
void p15(void)  { SERVO=1; __delay_us(1900); SERVO=0; __delay_ms(18); }
void p16(void)  { SERVO=1; __delay_us(2000); SERVO=0; __delay_ms(18); }
void p17(void)  { SERVO=1; __delay_us(2100); SERVO=0; __delay_ms(18); }
void p18(void)  { SERVO=1; __delay_us(2200); SERVO=0; __delay_ms(18); }
void p19(void)  { SERVO=1; __delay_us(2300); SERVO=0; __delay_ms(18); }
void p20(void)  { SERVO=1; __delay_us(2400); SERVO=0; __delay_ms(18); }
void p21(void)  { SERVO=1; __delay_us(2500); SERVO=0; __delay_ms(18); }
void p22(void)  { SERVO=1; __delay_us(2600); SERVO=0; __delay_ms(18); }

//=============================================================================
// ADC
//=============================================================================
void ADC_Init() {
    ANSEL  = 0x01;
    ANSELH = 0x00;
    ADCON0 = 0x01;
    ADCON1 = 0x80;
}

unsigned int ADC_Read() {
    __delay_us(5);
    GO_nDONE = 1;
    while(GO_nDONE);
    return (unsigned int)((ADRESH << 8) | ADRESL);
}

//MAIN
void main(void) {
    TRISDbits.TRISD0 = 0;
    ADC_Init();

    // Estabilizar en posicion inicial
    for (unsigned char i = 0; i < 20; i++) p00();

    while(1) {
        unsigned int adc = ADC_Read();

        if      (adc < 45)   p00();
        else if (adc < 89)   p01();
        else if (adc < 134)  p02();
        else if (adc < 178)  p03();
        else if (adc < 223)  p04();
        else if (adc < 267)  p05();
        else if (adc < 312)  p06();
        else if (adc < 356)  p07();
        else if (adc < 400)  p08();
        else if (adc < 445)  p09();
        else if (adc < 489)  p10();
        else if (adc < 534)  p11();
        else if (adc < 578)  p12();
        else if (adc < 623)  p13();
        else if (adc < 667)  p14();
        else if (adc < 712)  p15();
        else if (adc < 756)  p16();
        else if (adc < 800)  p17();
        else if (adc < 845)  p18();
        else if (adc < 889)  p19();
        else if (adc < 934)  p20();
        else if (adc < 978)  p21();
        else                 p22();
    }
}
