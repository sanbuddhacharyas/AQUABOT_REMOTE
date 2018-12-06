/*
 * remote.cpp
 *
 * Created: 11/23/2018 4:38:25 PM
 * Author : laceup216
 */

#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include "lcd.h"
#include "USART_RS232_H_file.h"
#include <stdio.h>
#include <inttypes.h>
#include <avr/interrupt.h>
#include <inttypes.h>
float rotation_angle,sum_,calculated;
uint8_t Lowadc;
uint16_t TenBitResults;
int loop,new_ADC_value,old_ADC_val;
char send_value;

#define Extra_1_Button PC0
#define Extra_2_Button PC1
#define Front_Button PD7
#define Back_Button PD4
#define Left_Button PC6
#define Right_Button PC7
#define Fan_Button PD3
#define Brake_Button PD2
#define Lev_Down_Button PD5
#define Lev_Up_Button PD6
#define Emergency_Button PB2

#define Extra_1_ASCII 'e'
#define Extra_2_ASCII '0'
#define speed_up_ASCII '8'
#define speed_down_ASCII '7'
#define Front_ASCII '1'
#define Back_ASCII '2'
#define Left_ASCII  '3'
#define Right_ASCII '4'
#define Front_Right_ASCII '6'
#define Front_Left_ASCII '5'
#define Lev_Down_ASCII 'd'
#define Lev_Up_ASCII 'u'
#define Brake_ASCII '0'

#define PIND_RELEASED 255
#define PINC_RELEASED 195
#define D_FRONT_PRESSED 127
#define D_BACK_PRESSED 239
#define D_LEV_UP_PRESSED 191
#define D_LEV_DOWN_PRESSED 223
#define C_RIGHT_PRESSED 67
#define C_LEFT_PRESSED 131
#define C_EXTRA1_PRESSED 194
#define C_EXTRA2_PRESSED 193

//Fan means Front_Right
//Brake means Front_Left

#define PRESSED (1)
#define RELEASED (0)

volatile int PinState;
int value;
char buffer[20],float_[10];

void ADC_Init(void);

class Buttons
{
	private:
	int Port;
	int Pin;
	char Ascii;
	
	public:
	Buttons(int _Port,int _PIN,char _ASCII):
	Port(_Port),Pin(_PIN),Ascii(_ASCII){}
		
	int Read_Button_State(void)
	{
		//Mask the pins with appropriate port
		/*if(Port == 'C')
		{
			PinState = (~(PINC) & 1<<Pin);
		}
		if(Port =='D')
		{
			PinState = (~(PIND) & 1<<Pin);
		}
		
		if(Port =='B')
		{
			PinState = ~((PINB) & (1<<Pin));
		}*/
		return PinState;
	}

	void Send_ASCII(void)
	{
		//UART_Transmit
		USART_TxChar(Ascii);
		_delay_ms(5);
		USART_TxChar('.');
	}
};

int main(void)
{
	sei();
	int PIND_New_Value,PINC_New_Value,PIND_Previous_Value,PINC_Previous_Value;
	int NULL_previous,NULL_new;
	uint16_t TWO_Buttons_Previous_Value,TWO_Buttons_New_Value;
	bool check = false ,haha = false;
	
	Buttons Front(4,Front_Button,Front_ASCII);
	Buttons Back(4,Back_Button,Back_ASCII);
	Buttons Right(3,Right_Button,Right_ASCII);
	Buttons Left(3,Left_Button,Left_ASCII);
	Buttons Lev_Up(4,Lev_Up_Button,Lev_Up_ASCII);
	Buttons Lev_Down(4,Lev_Down_Button,Lev_Down_ASCII);
	Buttons Brake(4,Brake_Button,Brake_ASCII);
	Buttons speed_up(2,Extra_1_Button,speed_up_ASCII);
	Buttons speed_down(2,Extra_2_Button,speed_down_ASCII);
	Buttons Front_Right(2,Front_Button,Front_Right_ASCII);
	Buttons Front_Left(2,Front_Button,Front_Left_ASCII);
	Buttons Extra1(4,Extra_1_Button,Extra_1_ASCII);
	Buttons Extra2(4,Extra_2_Button,Extra_2_ASCII);
	
	DDRB |= ~(1<<Emergency_Button);			//Making Input Ports
	PORTB = 0x00;
	DDRD |= ~(1<<Front_Button|1<<Back_Button|1<<Fan_Button|1<<Brake_Button|1<<Lev_Up_Button|1<<Lev_Down_Button);
	PORTD = 0x00;
	DDRC |= ~(1<<Left_Button|1<<Right_Button|1<<Extra_1_Button|1<<Extra_2_Button);
	PORTC = 0x00;
	
	/**************Optional: Also make internal Pull-Ups*********************/
	ADC_Init(); 
	USART_Init(38400);
	
	USART_SendString("USART Tested!");
	
		while(1)
		{
			
		
				TWO_Buttons_New_Value = PINC + PIND;
		
				if(TWO_Buttons_New_Value!=TWO_Buttons_Previous_Value)
				{
					switch(TWO_Buttons_New_Value)
					{
						case (D_FRONT_PRESSED+C_RIGHT_PRESSED):
						Front_Right.Send_ASCII();
						check = true;
						haha = false;
						break;
				
						case (D_FRONT_PRESSED+C_LEFT_PRESSED):
						Front_Left.Send_ASCII();
						check = true;
						haha = false;
						break;
				
						default:
							if(PIND == D_FRONT_PRESSED )
							{
								Front.Send_ASCII();
								check = true;
								break;
					
							}
							else if(PIND ==D_BACK_PRESSED )
							{
								Back.Send_ASCII();
								check = true;
								break;
							}
					
							else if(PINC == C_RIGHT_PRESSED )
							{
								Right.Send_ASCII();
								check = true;
								break;
							}
							else if(PINC ==C_LEFT_PRESSED )
							{
						
								Left.Send_ASCII();
								check = true;
								break;
							}
							else if(PIND == D_LEV_DOWN_PRESSED )
							{
								Lev_Down.Send_ASCII();
								check = true;
								break;
							}
					
							else if(PIND == D_LEV_UP_PRESSED )
							{
								Lev_Up.Send_ASCII();
								check = true;
								break;
							}
							
							break;
					
					}
					TWO_Buttons_Previous_Value = TWO_Buttons_New_Value;
					_delay_ms(10);
				}
			  
			
				if((PINC == PINC_RELEASED)&&(PIND == PIND_RELEASED))
				{
					if(check == true)
					{
						Brake.Send_ASCII();
						check = false;
					}
				
				}
			
		  
				 if((abs(new_ADC_value - old_ADC_val))>=20)
				  {
					  send_value =  3 + ((0.085)*new_ADC_value);
					  dtostrf(send_value,3,2,float_);
					  sprintf(buffer,"%d",send_value);
					   USART_SendString(buffer);
					   _delay_ms(5);
					   USART_TxChar('s');
			   
					  old_ADC_val = new_ADC_value;
				  }
		  
				  ADCSRA|=1<<ADSC;
				 
		
		}


}

void ADC_Init(void)
{	
	ADCSRA|=1<<ADPS2|1<<ADPS1|1<<ADPS0;			//The value of ADC frequency must be less than 200Khz so ADPS1:0 must be selected correctly
	ADMUX|=1<<REFS0|1<<MUX2;				//|1<<REFS1		Here,Using external 5V as reference
	ADCSRA|=1<<ADIE;
	ADCSRA|=1<<ADEN;
	//ADCSRA|=1<<ADSC;
}

ISR(ADC_vect)
{
	Lowadc= ADCL;
	TenBitResults=ADCH<<8|Lowadc;
	calculated=(0.1953125*TenBitResults);
	new_ADC_value=(int)calculated;
}


