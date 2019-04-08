/*
 * main.c
 *
 * Created: 05/03/2019 18:00:58
 *  Author: eduardo
 */ 

#include <asf.h>
#include "tfont.h"
#include "sourcecodepro_28.h"
#include "calibri_36.h"
#include "arial_72.h"
#include "math.h"


struct ili9488_opt_t g_ili9488_display_opt;

/************************************************************************/
/* defines                                                              */
/************************************************************************/

#define YEAR        2019
#define MOUNTH      1
#define DAY         1
#define WEEK        1
#define HOUR        0
#define MINUTE      0
#define SECOND      0



#define LED_PIO       PIOC
#define LED_PIO_ID    ID_PIOC
#define LED_IDX       8u
#define LED_IDX_MASK  (1u << LED_IDX)


#define BUT2_PIO           PIOC                 // periferico que controla o LED
#define BUT2_PIO_ID        ID_PIOC                    // ID do periférico PIOC (controla LED)
#define BUT2_PIO_IDX       31u                    // ID do LED no PIO
#define BUT2_PIO_IDX_MASK  (1u << BUT2_PIO_IDX)   // Mascara para CONTROLARMOS o LED


#define BUT3_PIO           PIOA                  // periferico que controla o LED
#define BUT3_PIO_ID        ID_PIOA                    // ID do periférico PIOC (controla LED)
#define BUT3_PIO_IDX       19u                    // ID do LED no PIO
#define BUT3_PIO_IDX_MASK  (1u << BUT3_PIO_IDX)   // Mascara para CONTROLARMOS o LED


/************************************************************************/
/* constants                                                            */
/************************************************************************/
float raio = 0.650/2;
/************************************************************************/
/* variaveis globais                                                    */
/************************************************************************/
volatile Bool f_rtt_alarme = false;
volatile Bool but3_flag = false;
volatile Bool but2_flag = false;
volatile Bool flag_rtc_alarme = true;


/************************************************************************/
/* interrupcoes                                                         */
/************************************************************************/

void RTT_Handler(void)
{
	uint32_t ul_status;

	/* Get RTT status */
	ul_status = rtt_get_status(RTT);

	/* IRQ due to Time has changed */
	if ((ul_status & RTT_SR_RTTINC) == RTT_SR_RTTINC) {
		
	}

	/* IRQ due to Alarm */
	if ((ul_status & RTT_SR_ALMS) == RTT_SR_ALMS) {
		
		//pin_toggle(LED_PIO, LED_IDX_MASK);    // BLINK Led
		f_rtt_alarme = true;                  // flag RTT alarme
	}
}

/**
*  Handle Interrupcao botao 3
*/
static void Button3_Handler(uint32_t id, uint32_t mask)
{
	but3_flag = true;
}
/**
*  Handle Interrupcao botao 2
*/
static void Button2_Handler(uint32_t id, uint32_t mask)
{
	but2_flag = true;
}

void RTC_Handler(void)
{
	uint32_t ul_status = rtc_get_status(RTC);

	/*
	*  Verifica por qual motivo entrou
	*  na interrupcao, se foi por segundo
	*  ou Alarm
	*/
	if ((ul_status & RTC_SR_SEC) == RTC_SR_SEC) {
		rtc_clear_status(RTC, RTC_SCCR_SECCLR);
					flag_rtc_alarme = true;

	}
	
	/* Time or date alarm */
	if ((ul_status & RTC_SR_ALARM) == RTC_SR_ALARM) {
			rtc_clear_status(RTC, RTC_SCCR_ALRCLR);

	}
	
	rtc_clear_status(RTC, RTC_SCCR_ACKCLR);
	rtc_clear_status(RTC, RTC_SCCR_TIMCLR);
	rtc_clear_status(RTC, RTC_SCCR_CALCLR);
	rtc_clear_status(RTC, RTC_SCCR_TDERRCLR);
	
}


/************************************************************************/
/* funcoes                                                              */
/************************************************************************/


/**
* @Brief Inicializa o pino do BUT
*/
void BUT3_init(void){
	/* config. pino botao em modo de entrada */
	pmc_enable_periph_clk(BUT3_PIO_ID);
	pio_set_input(BUT3_PIO, BUT3_PIO_IDX_MASK, PIO_PULLUP | PIO_DEBOUNCE);

	/* config. interrupcao em borda de descida no botao do kit */
	/* indica funcao (but_Handler) a ser chamada quando houver uma interrupção */
	pio_enable_interrupt(BUT3_PIO, BUT3_PIO_IDX_MASK);
	pio_handler_set(BUT3_PIO, BUT3_PIO_ID, BUT3_PIO_IDX_MASK, PIO_IT_FALL_EDGE, Button3_Handler);

	/* habilita interrupçcão do PIO que controla o botao */
	/* e configura sua prioridade                        */
	NVIC_EnableIRQ(BUT3_PIO_ID);
	NVIC_SetPriority(BUT3_PIO_ID, 1);
};

/**
* @Brief Inicializa o pino do BUT
*/
void BUT2_init(void){
	/* config. pino botao em modo de entrada */
	pmc_enable_periph_clk(BUT3_PIO_ID);
	pio_set_input(BUT2_PIO, BUT2_PIO_IDX_MASK, PIO_PULLUP | PIO_DEBOUNCE);

	/* config. interrupcao em borda de descida no botao do kit */
	/* indica funcao (but_Handler) a ser chamada quando houver uma interrupção */
	pio_enable_interrupt(BUT2_PIO, BUT2_PIO_IDX_MASK);
	pio_handler_set(BUT2_PIO, BUT2_PIO_ID, BUT2_PIO_IDX_MASK, PIO_IT_FALL_EDGE, Button2_Handler);

	/* habilita interrupçcão do PIO que controla o botao */
	/* e configura sua prioridade                        */
	NVIC_EnableIRQ(BUT2_PIO_ID);
	NVIC_SetPriority(BUT2_PIO_ID, 1);
};

void pin_toggle(Pio *pio, uint32_t mask){
	if(pio_get_output_data_status(pio, mask))
	pio_clear(pio, mask);
	else
	pio_set(pio,mask);
}

void io_init(void){
	/* led */
	pmc_enable_periph_clk(LED_PIO_ID);
	pio_configure(LED_PIO, PIO_OUTPUT_0, LED_IDX_MASK, PIO_DEFAULT);
}

static float get_time_rtt(){
	uint ul_previous_time = rtt_read_timer_value(RTT);
}

static void RTT_init(uint16_t pllPreScale, uint32_t IrqNPulses)
{
	uint32_t ul_previous_time;

	/* Configure RTT for a 4 second tick interrupt */
	rtt_sel_source(RTT, false);
	rtt_init(RTT, pllPreScale);
	
	ul_previous_time = rtt_read_timer_value(RTT);
	while (ul_previous_time == rtt_read_timer_value(RTT));
	
	rtt_write_alarm_time(RTT, IrqNPulses+ul_previous_time);

	/* Enable RTT interrupt */
	NVIC_DisableIRQ(RTT_IRQn);
	NVIC_ClearPendingIRQ(RTT_IRQn);
	NVIC_SetPriority(RTT_IRQn, 0);
	NVIC_EnableIRQ(RTT_IRQn);
	rtt_enable_interrupt(RTT, RTT_MR_ALMIEN);
}


/**
* Configura o RTC para funcionar com interrupcao de alarme
*/
void RTC_init(){
	/* Configura o PMC */
	pmc_enable_periph_clk(ID_RTC);

	/* Default RTC configuration, 24-hour mode */
	rtc_set_hour_mode(RTC, 0);

	/* Configura data e hora manualmente */
	rtc_set_date(RTC, YEAR, MOUNTH, DAY, WEEK);
	rtc_set_time(RTC, HOUR, MINUTE, SECOND);

	/* Configure RTC interrupts */
	NVIC_DisableIRQ(RTC_IRQn);
	NVIC_ClearPendingIRQ(RTC_IRQn);
	NVIC_SetPriority(RTC_IRQn, 0);
	NVIC_EnableIRQ(RTC_IRQn);

	/* Ativa interrupcao via alarme */
	rtc_enable_interrupt(RTC,  RTC_IER_SECEN);
	


}


void configure_lcd(void){
	/* Initialize display parameter */
	g_ili9488_display_opt.ul_width = ILI9488_LCD_WIDTH;
	g_ili9488_display_opt.ul_height = ILI9488_LCD_HEIGHT;
	g_ili9488_display_opt.foreground_color = COLOR_CONVERT(COLOR_WHITE);
	g_ili9488_display_opt.background_color = COLOR_CONVERT(COLOR_WHITE);

	/* Initialize LCD */
	ili9488_init(&g_ili9488_display_opt);
	ili9488_draw_filled_rectangle(0, 0, ILI9488_LCD_WIDTH-1, ILI9488_LCD_HEIGHT-1);
	
}


void font_draw_text(tFont *font, const char *text, int x, int y, int spacing) {
	char *p = text;
	while(*p != NULL) {
		char letter = *p;
		int letter_offset = letter - font->start_char;
		if(letter <= font->end_char) {
			tChar *current_char = font->chars + letter_offset;
			ili9488_draw_pixmap(x, y, current_char->image->width, current_char->image->height, current_char->image->data);
			x += current_char->image->width + spacing;
		}
		p++;
	}	
}

float calcula_velocidade_angular(int ciclos, int tempo){
	return (float) 2*M_PI*ciclos/tempo;
	
}


int main(void) {
	char b[512];
	int ciclos = 0;
	float d = 0;
	int count_button2 = 0;
	
	BUT3_init();
	BUT2_init();
	//RTT_init();
	board_init();
	sysclk_init();	
	configure_lcd();
	RTC_init();
	rtc_set_time_alarm(RTC, 1, HOUR, 1, MINUTE,1, SECOND+1);

	f_rtt_alarme = true;
	
	//font_draw_text(&sourcecodepro_28, "OIMUNDO", 50, 50, 1);
	//font_draw_text(&calibri_36, "Oi Mundo! #$!@", 50, 100, 1);
	//font_draw_text(&arial_72, "102456", 50, 200, 2);
	ciclos = 0;
	while(1) {
	/* Entrar em modo sleep */
	pmc_sleep(SAM_PM_SMODE_SLEEP_WFI);
	
	if (flag_rtc_alarme){
			rtc_set_date_alarm(RTC, 1 , MOUNTH, 1, DAY);
			int hora, min, sec;
			rtc_get_time(RTC, &hora, &min, &sec);
			rtc_set_time_alarm(RTC, 1, hora, 1, min,1, sec+1);
			font_draw_text(&calibri_36, "tempo:", 50, 200, 2);
			sprintf(b,"%02d : %02d : %02d", hora,min,sec);
			font_draw_text(&calibri_36, b, 50, 250, 2);

			flag_rtc_alarme = false;
		}
		
	if (but3_flag){
		ciclos += 1;
		but3_flag = false;
	}
	if (but2_flag){
		if(count_button2 == 0){
			
			
			count_button2 +=1;
			
		}
		d = 0;
		rtc_set_time(RTC, HOUR, MINUTE, SECOND);
		rtc_set_time_alarm(RTC, 1, HOUR, 1, MINUTE,1, SECOND+1);

		but2_flag = false;
	}
		
	if (f_rtt_alarme){
      
      /*
       * O clock base do RTT é 32678Hz
       * Para gerar outra base de tempo é necessário
       * usar o PLL pre scale, que divide o clock base.
       *
       * Nesse exemplo, estamos operando com um clock base
       * de pllPreScale = 32768/32768/2 = 2Hz
       *
       * Quanto maior a frequência maior a resolução, porém
       * menor o tempo máximo que conseguimos contar.
       *
       * Podemos configurar uma IRQ para acontecer quando 
       * o contador do RTT atingir um determinado valor
       * aqui usamos o irqRTTvalue para isso.
       * 
       * Nesse exemplo o irqRTTvalue = 8, causando uma
       * interrupção a cada 4 segundos (lembre que usamos o 
       * pllPreScale, cada incremento do RTT leva 500ms (2Hz).
       */
      uint16_t pllPreScale = (int) (((float) 32768) / 2.0);
      uint32_t irqRTTvalue  = 8;
      
      // reinicia RTT para gerar um novo IRQ
      RTT_init(pllPreScale, irqRTTvalue);         
      
     /*
      * caso queira ler o valor atual do RTT, basta usar a funcao
      *   rtt_read_timer_value()
      */
	 
	 
	  /*
       * Chama funcao que calcula velocidade angular
       */
	  float w = calcula_velocidade_angular(ciclos, 4);
	  float v = w*raio;
	  d += 2*M_PI*raio*ciclos;
	  
	  sprintf(b,"v: %.2f km/h",v*3.6);
	  font_draw_text(&calibri_36, b, 50, 50, 1);
	  sprintf(b,"d: %.2f m",d);
	  font_draw_text(&calibri_36, b, 50, 100, 1);
      
	  /*
       * Reseta ciclos
       */
	  ciclos = 0;

	  
      /*
       * CLEAR FLAG
       */
      f_rtt_alarme = false;
    }
  }
		
	
}