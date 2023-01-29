

/**
 * main.c
 */
#include <stdint.h>
#include <float.h>
#define SYSCTL_RCGC2_R          (*((volatile unsigned long *)0x400FE108))
#define GPIO_PORTB_DATA_R       (*((volatile unsigned long *)0x400053FC))
#define GPIO_PORTB_DIR_R        (*((volatile unsigned long *)0x40005400))
#define GPIO_PORTB_AFSEL_R      (*((volatile unsigned long *)0x40005420))
#define GPIO_PORTB_DEN_R        (*((volatile unsigned long *)0x4000551C))
#define GPIO_PORTB_AMSEL_R      (*((volatile unsigned long *)0x40005528))
#define GPIO_PORTB_PCTL_R       (*((volatile unsigned long *)0x4000552C))
#define SYSCTL_RCGCPWM_R		(*((volatile unsigned long *)0x400FE640))
#define SYSCTL_PRPWM_R			(*((volatile unsigned long *)0x400FEA40))
#define GPIO_PORTB_ODR_R 		(*((volatile unsigned long *)0x4000550C))
#define SYSCTL_RCGCGPIO_R 		(*((volatile unsigned long *)0x400FE608))
#define SYSCTL_RCC_R 			(*((volatile unsigned long *)0x400FE060))
#define SYSCTL_PRGPIO_R			(*((volatile unsigned long *)0x400FEA08))
#define PWMCTL_R 				(*((volatile unsigned long *)0x40028000))
#define PWM_GENA_R 				(*((volatile unsigned long *)0x40028060))
#define PWM_LOAD_R 				(*((volatile unsigned long *)0x40028050))
#define PWM_CMPA_R 				(*((volatile unsigned long *)0x40028058))
#define PWM_CTL_R 				(*((volatile unsigned long *)0x40028040))
#define PWM_ENABLE_R 			(*((volatile unsigned long *)0x40028008))
/*
 * 1) First, we activate the clock for the PWM
module. 2) Second, we activate the output pin as a digital alternate
function. 3) Next, we select the clock to be used for the PWM in RCC
register. If we do not use the PWM divider, then it is clocked from the
bus clock. With the divider we can choose 2, 4, 8, 16, 32, or 64. If the
TM4C123 is running at 50 MHz, this program specifies the PWM
clock to be 25 MHz. 4) We set the PWM to countdown mode. We
specify in the PWM_0_GENA_R register that the comparator action
is to set to one, and the load action is set to zero. 5) We specify the
period in the PWM_0_LOAD_R register. 6) We specify the duty
cycle in the PWM_0_CMPA_R register. 7) Lastly, we start and
enable the PWM.
Period is 16-bit number of PWM clock cycles in one period (3<=period)
Duty is number of PWM clock cycles output is high (2<=duty<=period-1)
PWM clock rate = processor clock rate/SYSCTL_RCC_PWMDIV
= BusClock/2
 */
void Delay_ms(int n);
void init(uint16_t period,uint16_t duty){
	  SYSCTL_RCGCPWM_R |= 0x00000001;			//PWM clock and module 0 activated
	  while((SYSCTL_PRPWM_R&0x00000001) == 0){
		  //wait for the PWM clock to be stabilised
	  };
	  SYSCTL_RCGCGPIO_R |= 0x00000002;           // Port B clock
	  while((SYSCTL_PRGPIO_R&0x00000002) == 0){
		  //wait for Port B clock to be stabilised
	  };
	  GPIO_PORTB_DIR_R |= 0x00000040;         // PB6 output
	  GPIO_PORTB_AFSEL_R |= 0x00000040;      // enable alternate function on PB6
	  GPIO_PORTB_ODR_R &= ~0x00000040;		// disable open drain on PB6?
	  GPIO_PORTB_DEN_R |= 0x00000040;         // enable digital I/O on PB6
	  GPIO_PORTB_AMSEL_R &= ~0x00000040;     // disable analog function on PB6
	  GPIO_PORTB_PCTL_R = (GPIO_PORTB_PCTL_R & 0xF0FFFFFF) + 0x04000000;  // configure PB6 as PWM
	  /*
	  	   * set bit 20 of the register to enable the frequency divider as well as choose /64 configuration
	  	   * to get the clock down to 250 KHz. The XTAL frequency of TM4C is 16 MHz
	  */
	  SYSCTL_RCC_R = 0x00100000 | ((SYSCTL_RCC_R&(~0x000E0000)) + 0x7);
	  PWMCTL_R = 0;  // selection of count down mode
	  PWM_CTL_R = (PWM_CTL_R&0x00000002) + 0x0;  // selection of count down mode
	  PWM_GENA_R = (PWM_GENA_R&0x000000C0) + 0x2;  //drive pwm low when counter reaches COMPARE
	  PWM_GENA_R = (PWM_GENA_R&0x00000006) + 0x3;  //drive pwm high when counter reaches LOAD
	  PWM_LOAD_R = period-1;  //cycles needed to count down to 0; We specify the period in LOAD register
	  PWM_CMPA_R = duty - 1;    //count value when output rises; we specify the duty cycle in CMPA register
	  PWM_CTL_R |= 0x00000001;  //start PWM generator 0
	  PWM_ENABLE_R |= 0x00000001;  //enable PWM generator 0
}

void PWM_Duty(uint16_t duty){
	PWM_CMPA_R = duty - 1;  // count value when output rises
}

void main(void)
{
	init(5000,2500);  // LOAD value to be sent as period, COMPARE going to be sent as duty
	while(1){
	}
}
