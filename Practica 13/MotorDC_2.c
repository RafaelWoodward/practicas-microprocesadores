// ACT_B
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

// Pines de direccion del puente H
#define IN1 PORTDbits.RD1
#define IN2 PORTDbits.RD2

// El centro del ADC (512) es el punto muerto del potenciometro
// Por debajo = reversa, por encima = adelante
// Zona muerta alrededor del centro para evitar que el motor vibre en reposo
#define CENTRO      512
#define ZONA_MUERTA 30  // +/- 30 cuentas alrededor del centro = motor frenado


// PWM HARDWARE en RC1 por CCP2
// Fosc=8MHz, prescaler 1:4, PR2=255
// Frecuencia PWM = 8,000,000 / (4 * 4 * 256) = ~2kHz
// ENA del L298N recibe esta señal para controlar la velocidad

void PWM_Init() {
    TRISCbits.TRISC1 = 0;   // RC1 como salida (CCP2) -> ENA del L298N
    PR2     = 255;
    T2CON   = 0b00000101;   // prescaler 1:4, Timer2 ON
    CCP2CON = 0b00001100;   // modo PWM
    CCPR2L  = 0;            // velocidad inicial en 0
}

void PWM_SetDuty(unsigned int duty) {
    // duty recibe 0-1023 igual que el ADC, igual que en practica 11
    CCPR2L  = (unsigned char)(duty >> 2);
    CCP2CON = (CCP2CON & 0b11001111) | ((duty & 0x03) << 4);
}

//ADC
void ADC_Init() {
    ANSEL  = 0x01;  // RA0 analogico para el potenciometro
    ANSELH = 0x00;
    ADCON0 = 0x01;  // canal AN0, ADC habilitado
    ADCON1 = 0x80;  // right-justified
}

unsigned int ADC_Read() {
    __delay_us(5);
    GO_nDONE = 1;
    while(GO_nDONE);
    return (unsigned int)((ADRESH << 8) | ADRESL);
}

//MAIN
void main(void) {
    // Pines de direccion como salidas
    TRISDbits.TRISD1 = 0;   // IN1
    TRISDbits.TRISD2 = 0;   // IN2

    // Motor frenado al inicio
    IN1 = 0;
    IN2 = 0;

    ADC_Init();
    PWM_Init();

    while(1) {
        unsigned int adc = ADC_Read();

        if (adc > CENTRO + ZONA_MUERTA) {
            // GIRO
            // La velocidad se mapea desde el centro hasta el maximo (1023)
            // unsigned long para evitar overflow en la multiplicacion
            IN1 = 1;
            IN2 = 0;

            unsigned int velocidad = adc - (CENTRO + ZONA_MUERTA);
            unsigned int rango     = 1023 - (CENTRO + ZONA_MUERTA);
            unsigned int duty      = (unsigned int)(((unsigned long)velocidad * 1023) / rango);
            PWM_SetDuty(duty);

        } else if (adc < CENTRO - ZONA_MUERTA) {
            // REVERSA
            // La velocidad se mapea desde el centro hasta el minimo (0)
            IN1 = 0;
            IN2 = 1;

            unsigned int velocidad = (CENTRO - ZONA_MUERTA) - adc;
            unsigned int rango     = CENTRO - ZONA_MUERTA;
            unsigned int duty      = (unsigned int)(((unsigned long)velocidad * 1023) / rango);
            PWM_SetDuty(duty);

        } else {
            // ZONA MUERTA: motor frenado, pot en el centro
            IN1 = 0;
            IN2 = 0;
            PWM_SetDuty(0);
        }

        __delay_ms(20);
    }
}
