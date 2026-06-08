#include <xc.h>         // Biblioteca principal del compilador XC8

//=============================================================================
// CONFIGURACI N DE BITS DE CONFIGURACI N (FUSES)
//=============================================================================

// Selecci n de oscilador (usar XT si est s usando un cristal de 4 MHz)
#pragma config FOSC = XT        // Oscillator Selection bits (XT oscillator)
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

#include <xc.h>

#pragma config FOSC = XT
#pragma config WDTE = OFF
#pragma config PWRTE = OFF
#pragma config BOREN = ON
#pragma config LVP = OFF
#pragma config CPD = OFF
#pragma config WRT = OFF
#pragma config CP = OFF

#define _XTAL_FREQ 8000000

// Definir el patron de los displays
unsigned char seg7[10] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F};

unsigned char contador = 0;
unsigned char decenas, unidades;

void main(void) {
    ANSEL  = 0;
    ANSELH = 0;
    OPTION_REG = OPTION_REG & 0b01111111; // Pull-ups internos activados

    TRISB = 0xFF; // PORTB = entradas (botones)
    TRISC = 0; // PORTC = salida (display decenas R2)
    TRISD = 0; // PORTD = salida (display unidades R9)

    PORTC = seg7[0];
    PORTD = seg7[0];

    while(1) {

        // Botón SUMA (RB0)
        if (PORTBbits.RB0 == 0) {
            __delay_ms(30);
            if (PORTBbits.RB0 == 0) {
                if (contador >= 99) contador = 0; // Si esta en 99, regresa a 0
                else contador++;
                while (PORTBbits.RB0 == 0);
            }
        }

        // Botón RESTA (RB1) 
        if (PORTBbits.RB1 == 0) {
            __delay_ms(30);
            if (PORTBbits.RB1 == 0) {
                if (contador == 0) contador = 99; // Si esta en 0, salta a 99
                else contador--;
                while (PORTBbits.RB1 == 0);
            }
        }

        // Botón RESET (RB2)
        if (PORTBbits.RB2 == 0) {
            __delay_ms(30);
            if (PORTBbits.RB2 == 0) {
                contador = 0;
                while (PORTBbits.RB2 == 0);
            }
        }

        // Actualizar displays
        decenas  = contador / 10;
        unidades = contador % 10;

        PORTC = seg7[decenas];   // Display izquierdo (R2)
        PORTD = seg7[unidades];  // Display derecho  (R9)
    }
}
