// ACTIVIDAD
#include <stdio.h>
#include <stdlib.h>
#include <xc.h>
#include <stdbool.h>
#include "lcd.h"

//=============================================================================
// CONFIGURACION DE FUSES
//=============================================================================
#pragma config FOSC = HS
#pragma config WDTE = OFF
#pragma config PWRTE = OFF
#pragma config BOREN = ON
#pragma config LVP = OFF
#pragma config CPD = OFF
#pragma config WRT = OFF
#pragma config CP = OFF

//=============================================================================
// DEFINICIONES
//=============================================================================
#define _XTAL_FREQ 8000000

volatile unsigned int tiempo   = 0; 
volatile unsigned int contador = 0; 

char buf_volt[12]; // buffer para el voltaje
char buf_time[6];  // buffer para el contador

//timer1
void Timer1_Init() {
    T1CON  = 0b00110001; // prescaler 1:8, reloj interno, Timer1 ON
    TMR1H  = 0xF6;
    TMR1L  = 0x3C;
    
    TMR1IE = 1; // interrupcion de Timer1
    PEIE   = 1; // interrupciones de perifericos
    GIE    = 1; // interrupciones globales
}

void __interrupt() ISR(void) {
    if (TMR1IF) {
        contador++;
        if (contador >= 100) { // 100 x 10ms = 1 segundo
            tiempo++;
            contador = 0;
        }
        TMR1H  = 0xF6; // recargar valor inicial
        TMR1L  = 0x3C;
        TMR1IF = 0;    // limpiar bandera
    }
}

// ADC
void ADC_Init() {
    ANSEL  = 0x01; // solo RA0 analogico
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
    Timer1_Init();
    ADC_Init();

    LCD lcd = {&PORTC, 2, 3, 4, 5, 6, 7};
    LCD_Init(lcd);

    LCD_Clear();
    LCD_Set_Cursor(0, 0);
    LCD_putrs("Voltaje:");

    while(1) {
        unsigned int adc = ADC_Read(0);

        // unsigned long para evitar overflow: 1023*50000 = 51,150,000 > 65535
        unsigned long volt    = ((unsigned long)adc * 50000) / 1023;
        unsigned int part_int = (unsigned int)(volt / 10000);
        unsigned int part_dec = (unsigned int)(volt % 10000);

        // Actualizaa voltaje 
        LCD_Set_Cursor(0, 8);
        sprintf(buf_volt, "%u.%04u V", part_int, part_dec);
        LCD_putrs(buf_volt);

        // Actualizar timer en fila 1 esquina derecha
        // El tiempo ocupa "MM:SS" = 5 caracteres
        // En un LCD de 16 columnas, columna 11 lo deja en la esquina inferior derecha
        LCD_Set_Cursor(1, 11);
        sprintf(buf_time, "%02u:%02u", tiempo / 60, tiempo % 60);
        LCD_putrs(buf_time);

        __delay_ms(200);
    }
}
