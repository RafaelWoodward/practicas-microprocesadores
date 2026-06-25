// Clase - timer 0
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
volatile unsigned int tiempo = 0; // volatile guarda una variable que cambia constantemente en el tiempo para un acceso mas facil
volatile unsigned int contador = 0;
char exec[6];

// El pic tiene 3 timers, cada uno con diferentes funciones:
// timer 0 - timer de 8 bits - de 0 a 255, cada ciclo/cambio ocurre en frecuencia de ocilación/4. 
// Al llegar a 255 se desvorda

void Timer0_Init(){
    OPTION_REG = 0x07; // option reg tambien abilita las resistencias pull-up 
    TMR0 = 178; // <-- Es el valor que nos otorga el matlab
    TMR0IE = 1; // abilitamos la interrupcion por timer0
    GIE = 1; // interupciones globales van abilitadas
}

void __interrupt() ISR(void){
    if(T0IF){ // el timer0 tambien tiene su vandera como la de las interrupciones externas
        contador++;
        if (contador > 100){
            tiempo++;
            contador = 0;
        } 
        TMR0 = 178; // hay que cargarle este valor constantemente por que el valor se resetea despues del ciclo.
        TMR0IF = 0;
    }
}
// Este codigo no usa delays porque se detendria mucho.

void main(void){
    Timer0_Init();
    LCD lcd = {&PORTC, 2, 3, 4, 5, 6, 7};
    LCD_Init(lcd);
    
    LCD_Clear();
    LCD_Set_Cursor(0,0);
    LCD_putrs("Tiempo: ");
    LCD_Set_Cursor(1,0);
    LCD_putrs("00:00"); // la variable tiempo no inicia en 0, por eso tenemso que declarar el primer valor como 00:00
    
    while(1){
        LCD_Set_Cursor(1,0);
        sprintf(exec,"%02u:%02u",tiempo/60,tiempo%60);
        LCD_putrs(exec); // actualizamos el valor que ya pusimos de "00:00" en putrs
    }
}
