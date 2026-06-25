// Actividad 1
#include <stdio.h>
#include <stdlib.h>
#include <xc.h>
#include <stdbool.h>
#include "lcd.h"

#pragma config FOSC = HS
#pragma config WDTE = OFF
#pragma config PWRTE = OFF
#pragma config BOREN = ON
#pragma config LVP = OFF
#pragma config CPD = OFF
#pragma config WRT = OFF
#pragma config CP = OFF

#define _XTAL_FREQ 8000000

char buf_volt[12];
char buf_pct[8];

// PWM HARDWARE con Timer 2 y modulo CCP1
void PWM_Init() {
    TRISCbits.TRISC1 = 0;   // RC1 como salida (pin CCP2)

    PR2    = 255;
    T2CON  = 0b00000101;    // prescaler 1:4, Timer2 ON

    // CCP2CON en lugar de CCP1CON para usar el puerto RC1
    CCP2CON = 0b00001100;   // modo PWM

    // CCPR2L en lugar de CCPR1L
    CCPR2L = 0;
}

void PWM_SetDuty(unsigned int duty) {
    CCPR2L  = (unsigned char)(duty >> 2);
    CCP2CON = (CCP2CON & 0b11001111) | ((duty & 0x03) << 4);
}

//ADC
void ADC_Init() {
    ANSEL  = 0x01; // RA0 analogico para el potenciometro
    ANSELH = 0x00;
    ADCON0 = 0x01; // canal AN0, ADC habilitado
    ADCON1 = 0x80; // right-justified
}

unsigned int ADC_Read(unsigned char channel) {
    __delay_us(5);
    ADCON0 &= 0x83;
    ADCON0 |= channel << 2;
    __delay_ms(2);
    GO_nDONE = 1;
    while(GO_nDONE);
    return (unsigned int)((ADRESH << 8) | ADRESL);
}

void main(void) {
    ADC_Init();
    PWM_Init();

    LCD lcd = {&PORTC, 2, 3, 4, 5, 6, 7};
    LCD_Init(lcd);

    // Etiquetas fijas
    LCD_Clear();
    LCD_Set_Cursor(0, 0);
    LCD_putrs("Volt:");
    LCD_Set_Cursor(1, 0);
    LCD_putrs("PWM: ");

    while(1) {
        unsigned int adc = ADC_Read(0);

        // Actualizar PWM directamente con la lectura ADC 
        // ADC da 0-1023, PWM espera 0-1023, mapeo directo sin conversion
        PWM_SetDuty(adc);

        // mostrar voltaje en fila 0 
        unsigned long volt    = ((unsigned long)adc * 50000) / 1023;
        unsigned int part_int = (unsigned int)(volt / 10000);
        unsigned int part_dec = (unsigned int)(volt % 10000);

        LCD_Set_Cursor(0, 5);
        sprintf(buf_volt, "%u.%04u V", part_int, part_dec);
        LCD_putrs(buf_volt);

        // mostrar porcentaje de duty cycle en fila 1
        // unsigned long para evitar overflow: 1023*10000 > 65535
        unsigned long pct      = ((unsigned long)adc * 10000) / 1023;
        unsigned int pct_int   = (unsigned int)(pct / 100);
        unsigned int pct_dec   = (unsigned int)(pct % 100);

        LCD_Set_Cursor(1, 5);
        sprintf(buf_pct, "%3u.%02u%%", pct_int, pct_dec);
        LCD_putrs(buf_pct);

        __delay_ms(100);
    }
}
