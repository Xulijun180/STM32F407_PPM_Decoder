#include "stm32f4xx.h"
#include "USART.h"
#include <stdio.h>
#include "string.h"
#include "stdbool.h"
#include "delay.h"
#include "PPM_Decoder.h"

uint16_t PPM_data[4];
uint16_t PPM[4];
uint8_t channel=0;

int main(void)
{   
    HW_USART1_Init(); //初始化串口1
    printf("HW_USART1_Init Success!\r\n");
    // Delay_ms(3000); //延时3s
	unsigned int count = 0;
	printf("SystemInit Start!\r\n");
    // Delay_ms(3000); //延时3s
	
	PPM_Decoder_Init();
	
	

		uint16_t ppm_values[4];
    while (1)
    {
        
        PPM_Get_Values(ppm_values);

        printf("%d,%d,%d,%d",ppm_values[0],ppm_values[1],ppm_values[2],ppm_values[3]);
		printf("\n");
    }



	return 0;
}

