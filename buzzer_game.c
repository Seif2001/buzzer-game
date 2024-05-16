#include "TM4C123GH6PM.h"
#include <math.h> 
unsigned int outA = 1; // controls ouput pins
unsigned int outB = 0;
uint8_t lock = 0x00;
uint8_t leds = 0x00;
unsigned int new_round_flag = 0;
unsigned int continue_round_flag = 0;
unsigned int off_flag =0;
unsigned int pause_flag =0;

int main(void)
{
 SYSCTL->RCGCGPIO |= 0x07; /* enable clock to PORT A, B and C, A and B are for the push buttons, C is for the LEDs*/ 
 SYSCTL->RCGCGPIO |= 0x0F; // port D for buzzer
 SYSCTL->RCGCGPIO |= 0x10; // port F for controller buttons
 /* configure PORTF for switch input and LED output */
 GPIOA->DIR &= ~0x0F; /* make Pins 0-3 input for port A */  
 
 GPIOB->DIR |= 0x03; /* make pins 0 and 1 output for port B */
 
 GPIOC->DIR |=0xFF; // all pins output for Port c
 
 GPIOD->DIR |= 0xFF; // all ports output for buzzer

 GPIOF->DIR &= ~0x0F; // pins 0-3 input for controls

 GPIOA->DEN |= 0x0F; 
 GPIOB->DEN |= 0x03;
 GPIOC->DEN |= 0xFF; // make all pins digital
 GPIOD->DEN |= 0xFF;
 GPIOF->DEN |= 0x0F;

 GPIOA->PUR |= 0x0F; /* enable pull up for pins 0-3 for port A*/
 GPIOF->PUR |= 0x0F;

	/* configure PORTA 3-0 for falling edge trigger interrupt */
 GPIOA->IS &= ~0x0F; /* make bits 0-3 edge sensitive */ 
 GPIOA->IBE &= ~0x0F; /* trigger is controlled by IEV */ 
 GPIOA->IEV &= ~0x0F; /* falling edge trigger */  //
 GPIOA->ICR |= 0x0F; /* clear any prior interrupt */ 
 GPIOA->IM |= 0x0F; /* unmask interrupt for PA 0-3 */ 

 /* configure PORTF 3-0 for falling edge trigger interrupt */
 GPIOF->IS &= ~0x0F; /* make bits 0-3 edge sensitive */ 
 GPIOF->IBE &= ~0x0F; /* trigger is controlled by IEV */ 
 GPIOF->IEV &= ~0x0F; /* falling edge trigger */  //
 GPIOF->ICR |= 0x0F; /* clear any prior interrupt */ 
 GPIOF->IM |= 0x0F; /* unmask interrupt for PA 0-3 */ 


/* enable interrupt in NVIC and set priority to 3 */
	
 NVIC->IP[0] &=  3 << 5; /* set priority to 3 for port A */
 NVIC->ISER[0] |= 0x00000001; /* enable IRQ0 (D0 of ISER[0]) */ 

 /* enable interrupt in NVIC and set priority to 3 */
	
 NVIC->IP[30] &=  2 << 5; /* set priority to for port F */
 NVIC->ISER[0] |= 0x10000000; /* enable IRQ30 (D0 of ISER[0]) */ 
 
/* Configure SysTick */
 SysTick->LOAD = 160000-1; /* reload with 10ms */ // frequancy is 1/20ms

 SysTick->CTRL = 7; /* enable SysTick interrupt, use system clock */
 
 __enable_irq(); /* global enable IRQs */

 /* toggle the green/violet LED colors continuously */
 while(1)
 {
     if(off_flag){
        break;
    }
    if(new_round_flag){
        // displayed winner ie buzzed_player
        buzzed = 0x00; // reset buzzed array
        LEDs = 0x00; //reset leds array
    }
    if(continue_round_flag){
        lock[buzzed_player] = 1;
        GPIOD->data = lock;
        LEDs = lock;
        // all buzzed players, ie the indexes of the on bits stored in buzzed are not allowed to play
    }
    if(pause_round){
        // check who pressed and make the LED on for that player only
        LEDs[buzzed_player] = 1;
        GPIOD->data[buzzed_player] = 1;
        GPIOC->data = LEDs;
    }
    if(mute_flag){
        // turn off GPIOD in hardware
        GPIOD->data = 0x00;
    }
   
    
 }
}

void GPIOA_Handler(void)
{
    if(outA == 0){
        buzzed_player = log2(GPIOA->RIS);
    }
    if(outB == 0){
        buzzed_player = log2(GPIOA->RIS) + 4;
    }
    // check if buzzed player is already 1 then we don't allow him to play ie don't stop the game
    if(!buzzed[buzzed_player]){
        pause_round = 1;
    }

}

void GPIOF_Handler(void)
{
    mute_flag = 0;
    new_round_flag = 0;
    continue_round_flag = 0;
    off_flag =0;
    if(GPIOA->RIS == 0x01){
        off_flag = 1;
    }
    if(GPIOA->RIS == 0x02){
        new_round_flag = 1;
    }
    if(GPIOA->RIS == 0x04){
        continue_round_flag = 1;
    }
    if(GPIOA->RIS == 0x08){
        mute_flag = 1;
    }

}


void SysTick_Handler(void)
{
	outA = ~outA;
    outB = ~outB;
    GPIOB->data = ((0b000000 | outA) << 7) | outB; // push data to pins 0 and 1 on port B
    SysTick->LOAD = 160000-1; /* reload with 10ms */
}
