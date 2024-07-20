/***********************************************************************************
* 文件名:	hw_USART1.c
* 创建日期:	2021年5月10日
* 作者:	zc
* 描述:	串口1硬件驱动文件 - 单一模式 - 使用DMA
************************************************************************************/

#include "USART.h"
#include "stm32f4xx_usart.h"
#include <stdio.h>

//////////////////////////////////////////////////////////////////
//如果需要使用printf等函数，请勾选use MicroLIB	  
#if 1
#pragma import(__use_no_semihosting)             
//需要准备好支持printf等函数的底层实现                 
struct __FILE 
{ 
	int handle; 
}; 

FILE __stdout;       
//重定向_sys_exit()函数，以便在使用半主机模式时正常退出
void _sys_exit(int x) 
{ 
	x = x; 
} 

///重定向c库函数中的printf函数到USART1
int fputc(int ch, FILE *f)
{
	/* 发送一个字符到USART1 */
	USART_SendData(USART1, (uint8_t) ch);

	/* 等待发送缓冲区为空 */
	while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);		

	return (ch);
}
#endif


/* 函数名:	HW_USART1_GPIO_Init
 * 功能:	初始化USART1相关GPIO
 * 参数:	无
 * 返回值:	无
 */
static void HW_USART1_GPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE); //使能GPIOA时钟
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10; //GPIOA9和GPIOA10
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF; //设置为复用模式
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//速度50MHz
	GPIO_Init(GPIOA, &GPIO_InitStructure); //初始化PA9和PA10
	
	//配置USART1对应的引脚复用功能
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1); //GPIOA9设置为USART1
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1); //GPIOA10设置为USART1	
}

/* 函数名:	HW_USART1_USART_Init
 * 功能:	初始化USART1
 * 参数:	无
 * 返回值:	无
 */
static void HW_USART1_USART_Init(void)
{
	USART_InitTypeDef USART_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);//使能USART1时钟
	//USART1 初始化配置
	USART_InitStructure.USART_BaudRate = 115200;//波特率115200
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式
	USART_Init(USART1, &USART_InitStructure); //初始化USART1	
}

/* 函数名:	HW_USART1_Init
 * 功能:	初始化USART1模块
 * 参数:	无
 * 返回值:	无
 */
void HW_USART1_Init(void)
{
	HW_USART1_GPIO_Init(); //初始化GPIO
	HW_USART1_USART_Init(); //初始化USART
	USART_Cmd(USART1, ENABLE);  //使能USART1 
}



/////重定向c库函数中的scanf函数到USART1
//int fgetc(FILE *f)
//{
//	/* 等待USART1接收到数据 */
//	while (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET);

//	return (int)USART_ReceiveData(USART1);
//}
