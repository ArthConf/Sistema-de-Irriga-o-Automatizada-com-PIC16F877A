#include <16F877A.h>
#fuses NOWDT, XT, NOPUT, NOPROTECT, NODEBUG, NOBROWNOUT, NOLVP, NOCPD, NOWRT
#use delay(clock=4000000)

#use I2C(master, sda=PIN_C4, scl=PIN_C3, slow=100000)
#include "i2c_Flex_LCD.c"

// Calibração do sensor
#define ADC_SECO       255   // Solo totalmente seco
#define ADC_UMIDO      219   // Solo idealmente úmido
#define ADC_ENCHARCADO  79   // Solo totalmente encharcado

#define SENSOR_ANALOG_CHANNEL  0    // AN0 = RA0
#define RELE_PIN               PIN_B0
#define LIMITE_SECO            230  // Aciona irrigação se valor >= 230 (seco)

void limpa_linha_lcd(int linha) {
    lcd_gotoxy(1, linha);
    printf(lcd_putc, "                ");
    lcd_gotoxy(1, linha);
}

void main() {
    set_tris_a(0b00000001);         // RA0 como entrada analógica
    set_tris_b(0b11111110);         // RB0 como saída (relé)

    output_high(RELE_PIN);          // Relé desligado no início

    setup_adc(ADC_CLOCK_INTERNAL);
    setup_adc_ports(AN0);           // Apenas AN0 analógico

    lcd_init(0x4E, 16, 2);
    lcd_backlight_led(ON);
    
    // Mensagem inicial
    lcd_gotoxy(1,1); printf(lcd_putc, "IRRIGACAO");
    lcd_gotoxy(1,2); printf(lcd_putc, "CONFESSOR");
    delay_ms(3000);
    limpa_linha_lcd(1);
    limpa_linha_lcd(2);
    char estado_anterior = -1;

    while(TRUE) {
        set_adc_channel(SENSOR_ANALOG_CHANNEL);
        delay_us(20);
        int16 adc_val = read_adc();
        int16 umidade_percent = 0;
        if (adc_val <= ADC_ENCHARCADO) {
            umidade_percent = 100;
        } else if (adc_val >= ADC_SECO) {
            umidade_percent = 0;
        } else {
            umidade_percent = 100 * (ADC_SECO - adc_val) / (ADC_SECO - ADC_ENCHARCADO);
        }
        char estado_atual;
        if (adc_val >= LIMITE_SECO) {
            estado_atual = 1;  // seco
            output_low(RELE_PIN);
        } else {
            estado_atual = 0;  // úmido
            output_high(RELE_PIN);
        }

        if (estado_atual != estado_anterior) {
            limpa_linha_lcd(1);
            if (estado_atual == 1)
                printf(lcd_putc, "Solo seco Irrig.");
            else
                printf(lcd_putc, "Solo umido");
            estado_anterior = estado_atual;
        }
        limpa_linha_lcd(2);
        printf(lcd_putc, "%3lu%% Umidade", umidade_percent);
        delay_ms(1000);
    }
}

