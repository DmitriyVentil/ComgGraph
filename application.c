/**
  ******************************************************************************
  * File Name          : application.c
  ******************************************************************************
  */

#include "application.h"



#include "main.h"



#include "usart.h"
#include "mb.h"
#include "mbporttimer.h"
#include "adc.h"
#include "pwm.h"
#include "eeprom.h"


volatile uint8_t application_led_period = 100; /* norm */
static volatile uint8_t application_led_period_cntr = 0;

volatile uint16_t application_main_loop_cntr = 0;


volatile uint8_t need_soft_reset = 0;

static volatile uint8_t application_test_period_cntr = 0;




/* parameters */

volatile uint16_t on_i_threshold = 0;
volatile uint8_t  pusk_t_on = 0;
volatile uint16_t off_vin_threshold = 0;
volatile uint8_t  off_t_mask    = 0;    
volatile uint8_t  off_timeout    = 0;   
volatile uint8_t  ready_t_mask    = 0;  
volatile uint8_t  ready_timeout_sleep    = 0;





volatile uint8_t timer_pusk_t_on = 0;
volatile uint8_t timer_off_t_mask    = 0;    
volatile uint8_t timer_off_timeout    = 0;   
volatile uint8_t timer_ready_t_mask    = 0;  
volatile uint8_t timer_ready_timeout_sleep    = 0;

volatile uint8_t timer_adc6_7_protect   = 70; // 700 мс //  Моя доработка
volatile uint8_t timer_adc6_7_protect_count    = 0; //  Моя доработка

bool powerbank_is_good = false;
bool input_voltage_is_good = false;







volatile uint8_t application_pwm_on = 0;




typedef enum appl_state_e {
	appl_state_off = 0,
	appl_state_startup,
	appl_state_on,
	appl_state_shutdown,
	appl_state_sleep
	
} appl_state_t;

static appl_state_t appl_state = appl_state_off;





void init() {
	
		adc_raw_data_buff[0] = adc_FRAME_START_MRKR;
	
	
  cli();
  	
	application_main_loop_cntr = 0;


	/* gpio in SIGN PORTC.2*/
	DDRC &= ~(1 << DDC2);



	/* ENA */
	DDRB |= (1 << DDB0);
	PORTB &= ~(1 << PB0); /* off ena */
	//// PORTB |= (1 << PB0); /* on ena */

	

	/* LED */
	DDRB |= (1 << DDB2); 
	PORTB |= (1 << PB2); /* LED OFF */
	/*PORTB &= ~(1 << PB2);*/ /* LED ON */






	/* debug_trace pins */

	DDRD |= (1 << DDD3);
	PORTD &= ~(1 << PD3);

	DDRD |= (1 << DDD4);
	PORTD &= ~(1 << PD4);





	eeprom_init();



//// param init section


	mb_address =						eeprom_cache[16]; /* mb_addr_new */
	
	pwm_cntr_on_stage_timeout = 		eeprom_cache[17]; /* pusk_period_sinus */





	on_i_threshold 	    = ((uint16_t)(eeprom_cache[18])) << 8;
	pusk_t_on 			= eeprom_cache[19]; if (0==pusk_t_on) pusk_t_on = 1; if (250<pusk_t_on) pusk_t_on = 250;
	
	off_vin_threshold 	= ((uint16_t)(eeprom_cache[20])) << 8;
	off_t_mask    		= eeprom_cache[21]; if (0==off_t_mask) off_t_mask = 1;   if (250<off_t_mask) off_t_mask = 250;
	off_timeout    		= eeprom_cache[22]; if (0==off_timeout) off_timeout = 1;   if (250<off_timeout) off_timeout = 250;
	
	ready_t_mask    	= eeprom_cache[23]; if (0==ready_t_mask) ready_t_mask = 1;  if (250<ready_t_mask) ready_t_mask = 250;
	ready_timeout_sleep = eeprom_cache[24]; if (0==ready_timeout_sleep) ready_timeout_sleep = 1; if (250<ready_timeout_sleep) ready_timeout_sleep = 250;

//// periph init section

	usart_init_rs485();

	mbporttimer_init();

	adc_init();
	
	pwm_init();

  sei();
  





#if 0

usart_flag_send_msg = usart_flag_send_msg_EEP;

#endif
	





}

volatile uint16_t adc_v_0 = 0;
volatile uint16_t adc_v_1 = 0;
volatile uint16_t adc_v_6 = 0;
volatile uint16_t adc_v_7 = 0;


static volatile uint8_t adc_frame_cntr = 0;
volatile uint8_t adc_raw_data_buff[adc_RAW_DATA_BUFF_SZ] = {0};




volatile uint8_t mbporttimer_flag_100hz_local = 0;

volatile uint8_t led_trace = 0;

volatile uint8_t shotdawn_deceted = 0; 



volatile uint8_t startup_timer = 0;
#define startup_timer_START	(100) /*1.0sec*/

void loop() {


////	application_main_loop_cntr ++;
////	if ( 1000 == application_main_loop_cntr ) {
////		application_main_loop_cntr = 0;
////	}
	



	mbporttimer_flag_100hz_local = 0;
	
  cli();
	if ( 0 != mbporttimer_flag_100hz ) {
		mbporttimer_flag_100hz_local = ~0;
		mbporttimer_flag_100hz = 0;
	}
  sei();
	
	
	
		/*  high proiritet */
	
	if ( mbporttimer_flag_100hz_local ) {
		//mbporttimer_flag_100hz_local = 0;
	
	
		if (0==need_soft_reset) 
		{
wdt_reset();
		}
		
		  cli();
			adc_v_0 = adc_val_0;
			adc_v_1 = adc_val_1;
			adc_v_6 = adc_val_6;
			adc_v_7 = adc_val_7;
		  sei();
		
		
		
		
		
		
/*	debug  adc !!!!!  */	
#if 0	
			adc_raw_data_buff[1] = adc_frame_cntr;
			
			adc_frame_cntr ++;
			if ( 100 == adc_frame_cntr ) 
			{
				adc_frame_cntr = 0;
			}
		
		
		
		
			//adc_raw_data_buff[2] = ((adc_v_0>>8)!=adc_FRAME_START_MRKR) ? (adc_v_0>>8) : (adc_FRAME_START_MRKR-1);
			//adc_raw_data_buff[3] = ((adc_v_1>>8)!=adc_FRAME_START_MRKR) ? (adc_v_1>>8) : (adc_FRAME_START_MRKR-1);
		
			adc_raw_data_buff[2] = ((adc_v_6>>8)!=adc_FRAME_START_MRKR) ? (adc_v_6>>8) : (adc_FRAME_START_MRKR-1);
			adc_raw_data_buff[3] = ((adc_v_7>>8)!=adc_FRAME_START_MRKR) ? (adc_v_7>>8) : (adc_FRAME_START_MRKR-1);
		
			
			usart_flag_send_msg = usart_flag_send_msg_ADC;
		
#endif	
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		/* led */
		
		application_led_period_cntr ++;
		if ( application_led_period <= application_led_period_cntr ) 
		{
			application_led_period_cntr = 0;



			if (led_trace==0) {
				application_led_period	 = 200;
			}


			if (led_trace==1) {
				application_led_period	 = 100;
			}
			
			if (led_trace==2) {
				application_led_period	 = 50;
			}
			
			if (led_trace==3) {
				application_led_period	 = 20;
			}	
			
			if (led_trace==4) {
				application_led_period	 = 10;
			}	


			if (led_trace==5) {
				application_led_period	 = 5;
			}	
			

		PORTB &= ~(1 << PB2); // led on


#if 0			
			/* led toggle */
			PORTB ^= (1 << PB2);
			
#endif		
			
			
	

	
		} else 
		{ //application_led_period_cntr
			
			PORTB |= (1 << PB2); // led off
			
		} //application_led_period_cntr
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
	if ( startup_timer_START > startup_timer ) 
	{
		
		
					startup_timer ++;
			
	} else 
	{



					
			if ( (1 << PC2) & PINC ) 
			{
				application_pwm_on = 1;
				
				
				
 // cli();
				
				///------/////pwm_on = 1;
 // sei();
			} else 
			{
				application_pwm_on = 0;
				
				
 // cli();
				
				///------/////pwm_on = 0;
 // sei();
				
			}



	}















		
/* logic automat */

		
	// always do control of voltage
		
		/* operation condition control PFC voltage power banks */
		// todo other operating condition
		if (
				( LIMIT_450V_MAX >= adc_v_6 )
				&&
				( LIMIT_450V_MIN <= adc_v_6 )
				
				&&
				
				( LIMIT_225V_MAX >= adc_v_7 )
				&&
				( LIMIT_225V_MIN <= adc_v_7 )
		
		) 
		{ /* good */
			
			if ( ready_t_mask == timer_ready_t_mask) 
			{
				timer_adc6_7_protect_count=0; //  Моя доработка
				powerbank_is_good = true; // ok
			} else 
			{
				timer_ready_t_mask ++;
				powerbank_is_good = false;
			}
			
		} else 
		{ /* bad */
			
			// !!Моя доработка 
				// Задержка 
				if ( timer_adc6_7_protect <= timer_adc6_7_protect_count)
				{
					timer_ready_t_mask = 0;
					powerbank_is_good = false;
				} else
				{
					timer_adc6_7_protect_count++;
				}
			// !!Конец моей доработки
			//timer_ready_t_mask = 0;
			//powerbank_is_good = false;
			
		}






		/* operation condition input voltage */
		if (
				off_vin_threshold > adc_v_1
		) { /* power_off */
	
			if ( off_t_mask == timer_off_t_mask ) { /* ok */
				input_voltage_is_good = false; /* time window complete */
			} else { /* wait... */
				timer_off_t_mask ++;
				input_voltage_is_good = true;
			}
	
		} else { /* good */
			timer_off_t_mask = 0;
			input_voltage_is_good = true;
		}





////####input_voltage_is_good = true;
////####powerbank_is_good = true;


#define WORK	(1)


	if ( 0!=application_pwm_on ) { /// n
		
#ifndef WORK


			cli();
			pwm_on = 1;// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<  start
			sei(); 

#endif
		
#ifdef WORK	



	
		
		/* state machine */
		switch ( appl_state ) {
			
			case appl_state_off: {
				
				
led_trace = 1;					

				
				if ( /*input_voltage_is_good && */powerbank_is_good /* && ( LIMIT_CURRENT_XX > adc_v_0 ) */) 
				{ /* ток холостого хода мал - только на балансныqй делитель и измериловку */
					

					appl_state = appl_state_startup;
					
					timer_pusk_t_on = 0;
							
			cli();
			pwm_on = 1;// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<  start
			sei(); 
					
				} 
				
				break;
			}

	
			case appl_state_startup: {

led_trace = 2;


				if (timer_pusk_t_on>=pusk_t_on) 
				{
					//pusk complete
					appl_state = appl_state_on;
				} else 
				{
					timer_pusk_t_on ++;
				}
				


				break;
			}
			
			
			case appl_state_on: {

led_trace = 3;

				if ( input_voltage_is_good ) { 
					
					if ( powerbank_is_good && ( on_i_threshold > adc_v_0 ) ) { // good load condition current and PFC_power_bank
						
						
						; // nothing to do
						
						
					} else { // bad load condition --> sleep && restart

						appl_state = appl_state_sleep;
						
		cli();
		pwm_on = 0;
		sei();
						

		
						timer_ready_timeout_sleep = 0;
					}
					
					
					
				} else { // input voltage fail --> shutdown
					appl_state = appl_state_shutdown;
   			
					timer_off_timeout    = 0;   			
	
				}




				break;
			}
			
			
			
			
			
			case appl_state_shutdown: {

led_trace = 4;	

				if (timer_off_timeout >= off_timeout) {
				
					
				
		cli();
		pwm_on = 0; 
		sei();

				appl_state = appl_state_off;
				
led_trace = 2;
				
					//for (;;) {;}  // reboot wdt
				
				
				} else {
					timer_off_timeout ++;
				}



				
				break;
			}
			
			
			case appl_state_sleep: {
				
led_trace = 5;
				
				if (timer_ready_timeout_sleep>=ready_timeout_sleep) {
					appl_state = appl_state_off;

				} else {
					timer_ready_timeout_sleep ++;
				}

				
				break;
			}
			
			
			default: {
				appl_state = appl_state_off;
				
		cli();
		pwm_on = 0;
		sei();


			}
		} /* switch */
		
		
	#endif	
		
		
		
		
	} else { // application_pwm_on off
				
		appl_state = appl_state_off;
		
led_trace = 0;
		
		cli();
		pwm_on = 0;
		sei();

	} // application_pwm_on
		
	






	

		return; /* high prioritet end-->next loop */
		
		
	} // if ( mbporttimer_flag_100hz_local )
	
	


	/*  low prioritet */
	
	mb_poll();
	
	
	
	if (eeprom_need_to_save) {
	
		eeprom_need_to_save = 0;
		
		eeprom_save_from_cache();
		
	}
	
	
}


/*****************************END OF FILE****/
