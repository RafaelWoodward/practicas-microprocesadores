#include <stdio.h>
#include <stdlib.h>
#include <xc.h>
#include <stdbool.h>
#include "lcd.h"

//=============================================================================
// CONFIGURACIÓN DE BITS DE CONFIGURACIÓN (FUSES)
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
#define _XTAL_FREQ 8000000      // Frecuencia del oscilador (8 MHz)

// VARIABLES GLOBALES
volatile unsigned int tiempo   = 0;  // Segundos transcurridos
volatile unsigned int contador = 0;  // Conteo de interrupciones del Timer0
char buffer_volt[10];                // Buffer para voltaje
char buffer_time[6];                 // Buffer para tiempo MM:SS


// INICIALIZACIÓN DEL TIMER0

void Timer0_Init(){
    OPTION_REG = 0x07;  // Preescaler 1:256 asignado al Timer0
    TMR0 = 178; // Valor de carga (~10 ms con 8 MHz y prescaler 1:256)
    TMR0IE = 1; // Habilitar interrupción del Timer0
    GIE = 1; // Habilitar interrupciones globales
}

// RUTINA DE SERVICIO DE INTERRUPCIÓN (ISR)
void __interrupt() ISR(void){
    if(TMR0IF){
        contador++;
        if(contador > 100){     // ~100 interrupciones = 1 segundo
            tiempo++;
            contador = 0;
        }
        TMR0 = 178;             // Recargar Timer0
        TMR0IF = 0;             // Limpiar bandera
    }
}


// INICIALIZACIÓN DEL ADC
void ADC_Init(){
    ANSEL  = 0x01;  // Solo RA0 como analógico
    ANSELH = 0x00;
    ADCON0 = 0x01;  // Encender ADC, canal 0 seleccionado
    ADCON1 = 0x80;  // Resultado justificado a la derecha, Vref = VDD
}

// LECTURA DEL ADC
unsigned int ADC_Read(unsigned char channel){
    __delay_us(5);
    ADCON0 &= 0x83;             // Limpiar bits de selección de canal
    ADCON0 |= channel << 2;     // Seleccionar canal
    __delay_ms(2);
    GO_nDONE = 1;               // Iniciar conversión
    while(GO_nDONE);            // Esperar a que termine
    return ((ADRESH << 8) | ADRESL);  // Retornar resultado de 10 bits
}

//=============================================================================// PROGRAMA PRINCIPAL
void main(void){
    Timer0_Init();
    ADC_Init();

    LCD lcd = {&PORTC, 2, 3, 4, 5, 6, 7};
    LCD_Init(lcd);

    LCD_Clear();

    while(1){
        // Fila 0: Voltaje del potenciómetro en RA0 
        unsigned int adc_val = ADC_Read(0);
        unsigned long volt   = ((unsigned long)adc_val * 50000UL) / 1023;
        unsigned int v_ent   = volt / 10000; // Parte entera
        unsigned int v_dec   = (volt % 10000) / 100; // Solo 2 decimales

        LCD_Set_Cursor(0, 0);
        LCD_putrs("Voltaje: ");
        sprintf(buffer_volt, "%u.%02u", v_ent, v_dec);
        LCD_putrs(buffer_volt);
        LCD_putrs("V ");// Unidad + espacio para limpiar residuos

        // Fila 1: Cronómetro MM:SS alineado a la derecha ---
        sprintf(buffer_time, "%02u:%02u", tiempo / 60, tiempo % 60);
        LCD_Set_Cursor(1, 11); 
        LCD_putrs(buffer_time);

        __delay_ms(100);
    }
}
