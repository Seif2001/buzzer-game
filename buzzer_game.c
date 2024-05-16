#include "TM4C123GH6PM.h"

unsigned int outA = 1; // controls ouput pins
unsigned int putB = 10;
uint8_t winner = 0x00;
unsigned int winner_flag = 0;
int main(void)
{
 SYSCTL->RCGCGPIO |= 0x03; /* enable clock to PORT A and B*/ 

 /* configure PORTF for switch input and LED output */
 GPIOA->DIR &= ~0x0F; /* make Pins 0-3 input for port A */  

 GPIOB->DIR |= 0x03; /* make pins 0 and 1 output for port B */

 GPIOA->DEN |= 0x0F; 
 GPIOB->DEN |= 0x03; // make all pins digital

 GPIOA->PUR |= 0x0F; /* enable pull up for pins 0-3 for port A*/
	
	/* configure PORTF4, 0 for falling edge trigger interrupt */
 GPIOA->IS &= ~0x0F; /* make bits 0-3 edge sensitive */ 
 GPIOA->IBE &= ~0x0F; /* trigger is controlled by IEV */ 
 GPIOA->IEV &= ~0x0F; /* falling edge trigger */  //
 GPIOA->ICR |= 0x0F; /* clear any prior interrupt */ 
 GPIOA->IM |= 0x0F; /* unmask interrupt for PA 0-3 */ 

/* enable interrupt in NVIC and set priority to 3 */
	
 NVIC->IP[0] &=  3 << 5; /* set priority to 3 for port A */
 NVIC->ISER[0] |= 0x00000001; /* enable IRQ0 (D0 of ISER[0]) */ 
 
/* Configure SysTick */
 SysTick->LOAD = 160000-1; /* reload with 10ms */ // frequancy is 1/20ms

 SysTick->CTRL = 7; /* enable SysTick interrupt, use system clock */
 
 __enable_irq(); /* global enable IRQs */

 /* toggle the green/violet LED colors continuously */
 while(1)
 {
    if(winner_flag){
        // display winner for master

        NVIC->ISER &= 0x00000000; // disable all interupts on port A
    }
    if(reset_flag){
        // remove displayed winner
        winner_flag = 0;
        winner = 0x00;
        NVIC->ISER[0] |= 0x00000001; /* enable IRQ0 (D0 of ISER[0]) */ 

    }
 }
}
/* SW1 is connected to PF4 pin, SW2 is connected to PF0. */
/* Both of them trigger PORTF interrupt */
void GPIOA_Handler(void)
{
    if(outA == 0){
        winner = (0b0000 << 4) | (GPIOA->MIS);
    }
    if(outB == 0){
        winner = winner = (GPIOA->MIS << 4) | (0b0000);
    }
}


void SysTick_Handler(void)
{
	outA = ~outA;
    outB = ~outB;
    GPIOB->data = ((0b000000 | outA) << 7) | outB;
    SysTick->LOAD = 160000-1; /* reload with 10ms */
}
