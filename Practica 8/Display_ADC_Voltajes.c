#include <stdio.h>
#include <stdlib.h>
#include <xc.h>         // Biblioteca principal del compilador X<C8
#include <stdbool.h>
#include "lcd.h"

//=============================================================================
// CONFIGURACI?N DE BITS DE CONFIGURACI?N (FUSES)
//=============================================================================

// Selecci?n de oscilador (usar XT si est?s usando un cristal de 4 MHz)
#pragma config FOSC = HS        // Oscillator Selection bits (XT oscillator)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config BOREN = ON       // Brown-out Reset Enable bit (enabled)
#pragma config LVP = OFF        // Low-Voltage Programming Enable bit (disabled)
#pragma config CPD = OFF        // Data EEPROM Memory Code Protection (disabled)
#pragma config WRT = OFF        // Flash Program Memory Write Enable (disabled)
#pragma config CP = OFF         // Flash Program Memory Code Protection (disabled)

//=============================================================================
// DEFINICIONES
//=============================================================================

#define _XTAL_FREQ 8000000      // Frecuencia del oscilador (para __delay_ms y __delay_us)

void ADC_Init(){
    ANSEL = 0x03; // declaramos analogico 3,5,6,7 con RA1 y RA0 como analogicos
    ANSELH = 0;
    ADCON0 = 0x01;
    ADCON1 = 0x80; // 10000000; aun trabajamos con bits justificados a la dereha
}

unsigned int ADC_Read(unsigned char channel){ //ahora trabajamos con 2 canales, esto nos sirve a seleccionar canales
    __delay_us(5);
    ADCON0 & = 0x83; // 10000011; basicamente un reset de los canales, si hay un canal con 1, lo volvemos 0
    ADCON0 |= channel << 2; // sustituimos los bits del canal por el seleccionado
    __delay_ms(2);
    GO_nDONE = 1; //iniciamos las lecturas analogicas
    while(GO_nDONE); //al terminar las lecturas se pone un 0
    return((ADRESH<<8)+ADRESL); // unimos la palabra de los bits
}

void main(void){
    char buffer1[10];
    char buffer2[10];
    
    ADC_Init();
    LCD lcd = (&PORTC, 2, 3, 4, 5, 6, 7);
    LCD_Init(lcd);
    
    while(1){
        LCD_Clear();
        LCD_Set_Cursor(0,0);
        LCD_putrs("Voltaje 1:");
        LCD_Set_Cursor(0,11); // Para asegurar, "control freak"
        unsigned int adc_result1 = ADC_Read(0); // Lectura del canal 0
        unsigned int volt1 = (adc_result1 * 50000) / 1023;
        unsigned int part_ent1 = volt1 / 10000; //parte entera
        unsigned int part_dec1 = volt1 & 10000; //parte decimal
        sprintf(buffer1, "%u.%u",part_ent1,part_dec1);
        LCD_putrs(buffer1);
        
        LCD_Set_Cursor(1,0);
        LCD_putrs("Voltaje 2:");
        LCD_Set_Cursor(1,11); // Para asegurar, "control freak"
        unsigned int adc_result2 = ADC_Read(0); // Lectura del canal 0
        unsigned int volt2 = (adc_result1 * 50000) / 1023;
        unsigned int part_ent2 = volt2 / 10000; //parte entera
        unsigned int part_dec2 = volt2 & 10000; //parte decimal
        sprintf(buffer2, "%u.%u",part_ent2,part_dec2);
        LCD_putrs(buffer2);
        
        __delay_ms(100); // debe ser almenos de 50 ms para que no se desvorde la variable   
    }
}
