// Actividad 2
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

volatile unsigned char duty2   = 0; // duty cycle del LED2 (0-255) +
volatile unsigned char pwm_cnt = 0; // contador de ciclo PWM del LED2 +

// PWM HARDWARE con Timer 2 y modulo CCP2
void PWM_Init() {
    TRISCbits.TRISC1 = 0;   // RC1 como salida (pin CCP2) - LED1
    TRISDbits.TRISD0 = 0;   // RD0 como salida - LED2 PWM software +

    PR2 = 99;  // frecuencia = 8,000,000 / (4 * 4 * 100) = 5,000 Hz
    T2CON  = 0b00000101;    // prescaler 1:4, Timer2 ON

    CCP2CON = 0b00001100;   // modo PWM
    CCPR2L  = 0;

    TMR2IE = 1;             // interrupcion de Timer2 para PWM software +
    PEIE   = 1;             // interrupciones de perifericos +
    GIE    = 1;             // interrupciones globales +
}

void PWM_SetDuty(unsigned int duty) {
    CCPR2L  = (unsigned char)(duty >> 2);
    CCP2CON = (CCP2CON & 0b11001111) | ((duty & 0x03) << 4);
}

// ADC
void ADC_Init() {
    ANSEL  = 0x03; // RA0 y RA1 analogicos para los dos potenciometros +
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
    LCD_putrs("L1:");
    LCD_Set_Cursor(1, 0);
    LCD_putrs("L2:");

    while(1) {
        unsigned int adc1 = ADC_Read(0); // potenciometro 1 -> LED1
        unsigned int adc2 = ADC_Read(1); // potenciometro 2 -> LED2 +

        // LED1: PWM hardware, mapeo directo 0-1023
        PWM_SetDuty(adc1);

        // LED2: PWM software, reducir de 10 bits a 8 bits con >> 2 +
        duty2 = (unsigned char)(((unsigned long)adc2 * 100) / 1023);

        // mostrar porcentaje de cada LED en el LCD
        unsigned long pct1 = ((unsigned long)adc1 * 100) / 1023;
        unsigned long pct2 = ((unsigned long)adc2 * 100) / 1023; // +

        LCD_Set_Cursor(0, 3);
        sprintf(buf_volt, "%3u%%   ", (unsigned int)pct1);
        LCD_putrs(buf_volt);

        LCD_Set_Cursor(1, 3);
        sprintf(buf_pct, "%3u%%   ", (unsigned int)pct2); // +
        LCD_putrs(buf_pct);

        __delay_ms(100);
    }
}

// ISR - PWM software para LED2 en RD0 +
// Se dispara cada ciclo de Timer2, compara pwm_cnt con duty2
void __interrupt() ISR(void) {
    if (TMR2IF) {
        pwm_cnt++;
        if (pwm_cnt >= 100) pwm_cnt = 0;  // ciclo de 0 a 99
        PORTDbits.RD0 = (pwm_cnt < duty2) ? 1 : 0;
        TMR2IF = 0;
    }
}
