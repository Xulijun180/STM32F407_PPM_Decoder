# PWM输入信号捕获程序说明

## 简介
该程序适用于STM32F4系列单片机，实现了通过定时器捕获PWM输入信号，并通过串口输出PPM信号数据的功能。

## 参考文章
[用stm32读取遥控器接收机PPM信号各通道值](https://blog.csdn.net/qq_44164303/article/details/108866838)

## 接线说明
- PWM输入信号接口：PB14
- 定时器定时器配置：
  - 定时器：TIM12
  - 频率：84MHz
  - 通道：CH1
  - 
  - [接线如图](bf18ac600a1aad377676b9f25b36bed.jpg)
我这里焊接的时候把3.3v的焊盘弄掉了，所以飞了个线，正常接线就跟蓝线一样就行

下方3pin插座自左而右依次为GND、VCC、Signal_in。
![](bf18ac600a1aad377676b9f25b36bed.jpg)


# PPM 信号解析

PPM 信号是将多个通道的 PWM 信号整合到一个通道传输的信号。其优点是占用硬件资源最少。仅需一根信号线即可解析信号，常用于微型航模或者小型无人机。PPM 的数据帧周期是 20ms，理论上最多支持 9 个通道数据传输。其信号图如下所示，以下包含 1 个起始信号，紧接着跟随 k1-k8 这几个通道信号，从而形成一个完整的周期 PPM 信号。

![PPM 信号图示](https://pic2.zhimg.com/80/v2-7bc3fce62bff1f57f3665434727fdaed_720w.webp)

PPM 信号与 PWM 信号的对应关系如下图：

![PPM 与 PWM 信号对应关系](https://pic2.zhimg.com/80/v2-3a1e7fc8a2a6263d22516dee36350b81_720w.webp)

PPM 信号可作为一帧数据进行处理，它包含了 8 个通道的信息。每个上升沿间隔时间刚好等于 PWM 信号的高电平持续时间，也就是在 1000us 到 2000us 之间。

以航模接收机为例，在发射机摇杆归中状态下，接收机实际输出的信号如下。

![实际输出](https://pic3.zhimg.com/80/v2-f0943702b13cc97391f9c816e370257e_720w.webp)

## PPM 信号解析步骤

PPM 信号解析可以采用外部中断和定时器完成。总体思路为：外部中断检测跳变沿，然后读取定时器计数值，每 1us 增加一个计数。其具体步骤如下：

1. **检测并判断起始信号**：起始信号时长不固定，故需要求其上下限，用于信号判别。
   
   - 根据图示，低电平持续时间固定为 0.5s，高电平持续时间为 0.5s-1.5s。
   - 计算出起始信号高电平持续时间范围如下：
     - $T_{max} = 20 - (9 \times 0.5 + 8 \times 0.5) = 11.5$
     - $T_{min} = 20 - (9 \times 0.5 + 8 \times 1.5) = 3.5$

2. **连续读取 8 个跳边沿**（上升下降均可），并存储数据。

3. **一帧数据读取完成后进行标记，并准备下一帧读取**，即跳转至步骤 1。



## 程序功能及实现
1. 初始化PWM输入引脚PB14，并配置为定时器TIM12的输入捕获通道。
   代码如下：
```c
void Pwm_PB14_InputInit(void)
{
    GPIO_InitTypeDef            GPIO_InitStructure;                // 定义GPIO初始化结构体
    TIM_TimeBaseInitTypeDef     TIM_TimeBaseStructure;            // 定义定时器基本结构体
    NVIC_InitTypeDef            NVIC_InitStructure;                // 定义中断优先级结构体
    TIM_ICInitTypeDef           TIM12_ICInitStructure;            // 定义定时器12输入捕获结构体

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM12, ENABLE);        // 使能TIM12时钟
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);        // 使能GPIOB时钟

    GPIO_InitStructure.GPIO_Pin     = GPIO_Pin_14;               // GPIOB14
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_AF;              // 备用功能
    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_100MHz;         // 100MHz
    GPIO_InitStructure.GPIO_OType   = GPIO_OType_PP;             // 推挽输出
    GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_NOPULL;          // 无上拉或下拉
    GPIO_Init(GPIOB, &GPIO_InitStructure);                        // 初始化GPIOB

    // 定时器TIM12备用功能配置
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource14, GPIO_AF_TIM12);

    TIM_TimeBaseStructure.TIM_Prescaler        = 83;             // 定时器预分频
    TIM_TimeBaseStructure.TIM_CounterMode      = TIM_CounterMode_Up;  // 向上计数模式
    TIM_TimeBaseStructure.TIM_Period           = 49999;          // 自动重装载值
    TIM_TimeBaseStructure.TIM_ClockDivision    = TIM_CKD_DIV1;   // 时钟分频因子
    TIM_TimeBaseInit(TIM12, &TIM_TimeBaseStructure);

    // 初始化TIM12输入捕获参数
    TIM12_ICInitStructure.TIM_Channel      = TIM_Channel_1;       // 通道1
    TIM12_ICInitStructure.TIM_ICPolarity   = TIM_ICPolarity_Falling;  // 下降沿捕获
    TIM12_ICInitStructure.TIM_ICSelection  = TIM_ICSelection_DirectTI; // 直接映射到TI1
    TIM12_ICInitStructure.TIM_ICPrescaler  = TIM_ICPSC_DIV1;      // 输入预分频，无预分频
    TIM12_ICInitStructure.TIM_ICFilter     = 0x05;                // ICxF=0000输入滤波配置，无滤波
    // 初始化PWM输入模式
    TIM_PWMIConfig(TIM12, &TIM12_ICInitStructure);

    // 当工作在PWM输入模式时，只需要设置该通道的触发信号（用于测量周期）
    // 另一个通道（用于测量占空比）将由硬件配置，无需再次配置

    // 选择输入捕获触发信号
    TIM_SelectInputTrigger(TIM12, TIM_TS_TI1FP1);

    // 选择从模式：复位模式
    // 在PWM输入模式下，从模式必须工作在复位模式，在捕获开始时，计数器CNT将被重置
    TIM_SelectSlaveMode(TIM12, TIM_SlaveMode_Reset);
    TIM_SelectMasterSlaveMode(TIM12, TIM_MasterSlaveMode_Enable);

    // 使能捕获中断，此中断用于主捕获通道（测量周期）
    TIM_ITConfig(TIM12, TIM_IT_CC1, ENABLE);
    TIM_ClearITPendingBit(TIM12, TIM_IT_CC1);

    NVIC_InitStructure.NVIC_IRQChannel = TIM8_BRK_TIM12_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;    // 抢占优先级2
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;           // 子优先级0
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;              // 使能IRQ通道
    NVIC_Init(&NVIC_InitStructure);                              // 根据指定的参数初始化VIC寄存器

    TIM_Cmd(TIM12, ENABLE);    // 使能定时器12
}
```

2. 定义定时器TIM12中断服务函数，实现对PWM输入信号的捕获，并存储PPM信号数据。
```c
void TIM8_BRK_TIM12_IRQHandler(void)
{
    /* 清除定时器捕获/比较 1 中断标志位 */
    TIM_ClearITPendingBit(TIM12, TIM_IT_CC1);

    /* 获取输入捕获值 */
    IC1Value = TIM_GetCapture1(TIM12);
    IC2Value = TIM_GetCapture2(TIM12);

    // 计算高电平时间，即PPM信号的脉冲宽度
    uint16_t pulseWidth = IC1Value;

    // 根据通道号存储PPM信号数据



switch (channel)//Í¨µÀ¼ÇÂ¼
	{
		case 0:
			if(IC1Value>=5000)
			{
			channel+=1;//Í¨µÀÊý
			TIM_SetCounter(TIM12, 0);
			}
			else
			{
			TIM_SetCounter(TIM12, 0);
			}
			break;                                                                          
		case 1:
			if(IC1Value<=3000)
			{
			channel+=1;//Í¨µÀÊý
		    PPM[0]=(IC1Value-400);
			TIM_SetCounter(TIM12, 0);
			}
			break;
		
		case 2:
			if(IC1Value<=3000)
			{
			channel+=1;
			PPM[1]=(IC1Value-400);
			TIM_SetCounter(TIM12, 0);
			}
			break;
		
		case 3:
			if(IC1Value<=3000)
			{
			channel+=1;
			PPM[2]=(IC1Value-400);
			TIM_SetCounter(TIM12, 0);
			}
			break;
		
		case 4:
			if(IC1Value<=3000)
			{
			channel+=1;
			PPM[3]=(IC1Value-400);
			TIM_SetCounter(TIM12, 0);
			}
			break;
	}
	if(channel>4)
	{
		channel=0;
		memcpy(PPM_data,PPM,sizeof(PPM));//×ªÒÆÍ¨µÀÊý¾Ý
		memset(PPM,0,sizeof(PPM));//Êý¾ÝÇå³ý
		TIM_SetCounter(TIM12, 0);
	}

}
```

3. 主函数中初始化串口1，配置中断优先级，初始化PWM输入引脚，并进入主循环。
4. 主循环中输出捕获到的PPM信号数据，并通过延时函数控制输出速率。

## 串口输出数据解释

串口输出的数据格式为四个数值，以逗号分隔：

- `PPM_data[0]`: 第一个通道的PPM信号数据，表示通道1的脉冲宽度，单位为微秒（μs）。
- `PPM_data[1]`: 第二个通道的PPM信号数据，表示通道2的脉冲宽度，单位为微秒（μs）。
- `PPM_data[2]`: 第三个通道的PPM信号数据，表示通道3的脉冲宽度，单位为微秒（μs）。
- `PPM_data[3]`: 第四个通道的PPM信号数据，表示通道4的脉冲宽度，单位为微秒（μs）。

这些数据反映了每个通道的控制信号，通常用于遥控器或舵机控制。通过监测和分析这些数据，可以实现相应设备的运动控制或状态监测。

## 调用方法
1. 引入头文件：#include "PPM_Decoder.h"
2. 在主函数中调用`PPM_Decoder_Init();`函数初始化PPM解析程序。
3. 在主循环中调用`PPM_Decoder_Get_Data(PPM_data);`函数获取PPM信号数据。
4. 在主循环中调用`printf`函数输出PPM信号数据。

## 完整代码
```c
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


```

## 注意事项
- 程序中的延时函数`Delay_ms()`需根据实际情况进行实现或替换。

- 若需修改通道数或通道数据处理逻辑，可在`TIM8_BRK_TIM12_IRQHandler()`函数中进行修改。


