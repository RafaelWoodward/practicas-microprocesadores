#include <xc.h>         // Biblioteca principal del compilador XC8
//=============================================================================
// CONFIGURACION DE BITS DE CONFIGURACION (FUSES)
//=============================================================================
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
#define BTN_PAUSA PORTBbits.RB1 // Boton de pausa en RB1

unsigned char patron[10] = {
    0x3F, // 0
    0x06, // 1
    0x5B, // 2
    0x4F, // 3
    0x66, // 4
    0x6D, // 5
    0x7D, // 6
    0x07, // 7
    0x7F, // 8
    0x6F  // 9
};

unsigned char direccion = 1; // 1 = incrementar, 0 = decrementar
unsigned char pausado   = 0; // 1 = conteo detenido, 0 = conteo activo

void main(void){
    ANSEL  = 0;
    ANSELH = 0;
    OPTION_REG = OPTION_REG & 0b01111111;

    TRISC = 0;
    TRISD = 0;
    TRISB = 0xFF;

    PORTC = 0;
    PORTD = 0; // Estos dos son para evitar que haya un voltaje residual en esos pines

    GIE    = 1; // Activar todas las interrupciones
    INTE   = 1;
    INTEDG = 0; // Interrupciones por flanco de bajada

    int num = 0;

    while(1){
        int mil = num / 1000;        // Extraemos los millares
        int cen = (num / 100) % 10;  // Extraemos las centenas
        int dec = (num / 10)  % 10;  // Extraemos las decenas
        int uni = num % 10;          // Extraemos las unidades

        // Iniciar la multiplexacion (el display siempre refresca, este pausado o no)
        for(int i = 0; i < 10; i++){
            // Mostrar los millares
            PORTC = 0b11110111;
            PORTD = patron[mil];
            __delay_ms(1);

            // Mostrar las centenas
            PORTC = 0b11111011;
            PORTD = patron[cen];
            __delay_ms(1);

            // Mostrar las decenas
            PORTC = 0b11111101;
            PORTD = patron[dec];
            __delay_ms(1);

            // Mostrar las unidades
            PORTC = 0b11111110;
            PORTD = patron[uni];
            __delay_ms(1);
        }

        // Revisa boton de pausa 
        if(BTN_PAUSA == 0){
            pausado = !pausado;          // Alternar entre pausado y activo
            __delay_ms(200);             // Retardo antirebote
            while(BTN_PAUSA == 0);       // Esperar a que suelten el boton
        }

        // Avanzar o retroceder solo si el conteo no esta detenido
        if(pausado == 0){
            if(direccion == 1){
                num++;
                if(num == 10000) num = 0;   // Cambio hacia arriba
            } else {
                num--;
                if(num < 0) num = 9999;     // Cambio hacia abajo
            }
        }
    }
}

void __interrupt() ISR(void){
    if(INTF){   // INTF = Interrupcion externa
        direccion = !direccion; // Cambiar direccion de conteo
        INTF = 0;               // Resetear la bandera de interrupcion
    }
}
