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
#define BOTON_180 PORTBbits.RB0
#define BOTON_0   PORTBbits.RB1

// Rango de operacion
void p0(void)  { SERVO=1; __delay_us(400);  SERVO=0; __delay_ms(18); }
void p180(void)  { SERVO=1; __delay_us(2600); SERVO=0; __delay_ms(18); }

//MAIN
void main(void) {
    TRISDbits.TRISD0 = 0; // Salida hacia el servo
    TRISBbits.TRISB0 = 1; // Boton 180 grados (entrada)
    TRISBbits.TRISB1 = 1; // Boton 0 grados (entrada)

    ANSEL  = 0x00;
    ANSELH = 0x00;

    OPTION_REGbits.nRBPU = 0; // Habilita pull-ups globales de PORTB
    WPUBbits.WPUB0 = 1; // Pull-up interno en RB0
    WPUBbits.WPUB1 = 1; // Pull-up interno en RB1

    unsigned char angulo_actual = 0; // 0 = p0 (0 grados), 1 = p22 (180 grados)

    // Estabilizar en posicion inicial (0 grados)
    for (unsigned char i = 0; i < 20; i++) p0();

    while(1) {
        if (BOTON_180 == 0) {
            angulo_actual = 1;
        }
        else if (BOTON_0 == 0) {
            angulo_actual = 0;
        }

        if (angulo_actual == 1) p180();
        else                    p0();
    }
}
