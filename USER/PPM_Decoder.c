#include "PPM_Decoder.h"
#include "stm32f4xx.h"

static uint16_t PPM[4];
static uint8_t channel = 0;

static uint32_t IC1Value, IC2Value;
static float DutyCycle,Frequency;

void PPM_Decoder_Init(void)
{
    GPIO_InitTypeDef            GPIO_InitStructure;                // ??GPIO??????
    TIM_TimeBaseInitTypeDef     TIM_TimeBaseStructure;            // ??????????
    NVIC_InitTypeDef            NVIC_InitStructure;                // ??????????
    TIM_ICInitTypeDef           TIM12_ICInitStructure;            // ?????12???????

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM12, ENABLE);        // ??TIM12??
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);        // ??GPIOB??

    GPIO_InitStructure.GPIO_Pin     = GPIO_Pin_14;               // GPIOB14
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_AF;              // ????
    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_100MHz;         // 100MHz
    GPIO_InitStructure.GPIO_OType   = GPIO_OType_PP;             // ????
    GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_NOPULL;          // ??????
    GPIO_Init(GPIOB, &GPIO_InitStructure);                        // ???GPIOB

    // ???TIM12??????
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource14, GPIO_AF_TIM12);

    TIM_TimeBaseStructure.TIM_Prescaler        = 83;             // ??????
    TIM_TimeBaseStructure.TIM_CounterMode      = TIM_CounterMode_Up;  // ??????
    TIM_TimeBaseStructure.TIM_Period           = 49999;          // ??????
    TIM_TimeBaseStructure.TIM_ClockDivision    = TIM_CKD_DIV1;   // ??????
    TIM_TimeBaseInit(TIM12, &TIM_TimeBaseStructure);

    // ???TIM12??????
    TIM12_ICInitStructure.TIM_Channel      = TIM_Channel_1;       // ??1
    TIM12_ICInitStructure.TIM_ICPolarity   = TIM_ICPolarity_Falling;  // ?????
    TIM12_ICInitStructure.TIM_ICSelection  = TIM_ICSelection_DirectTI; // ?????TI1
    TIM12_ICInitStructure.TIM_ICPrescaler  = TIM_ICPSC_DIV1;      // ?????,????
    TIM12_ICInitStructure.TIM_ICFilter     = 0x05;                // ICxF=0000??????,???
    // ???PWM????
    TIM_PWMIConfig(TIM12, &TIM12_ICInitStructure);

    // ????PWM?????,?????????????(??????)
    // ?????(???????)??????,??????

    // ??????????
    TIM_SelectInputTrigger(TIM12, TIM_TS_TI1FP1);

    // ?????:????
    // ?PWM?????,????????????,??????,???CNT????
    TIM_SelectSlaveMode(TIM12, TIM_SlaveMode_Reset);
    TIM_SelectMasterSlaveMode(TIM12, TIM_MasterSlaveMode_Enable);

    // ??????,??????????(????)
    TIM_ITConfig(TIM12, TIM_IT_CC1, ENABLE);
    TIM_ClearITPendingBit(TIM12, TIM_IT_CC1);

    NVIC_InitStructure.NVIC_IRQChannel = TIM8_BRK_TIM12_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;    // ?????2
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;           // ????0
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;              // ??IRQ??
    NVIC_Init(&NVIC_InitStructure);                              // ??????????VIC???

    TIM_Cmd(TIM12, ENABLE);    // ?????12
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
}

void TIM8_BRK_TIM12_IRQHandler(void)
{
    /* ???????/?? 1 ????? */
    TIM_ClearITPendingBit(TIM12, TIM_IT_CC1);

    /* ??????? */
    IC1Value = TIM_GetCapture1(TIM12);
    IC2Value = TIM_GetCapture2(TIM12);

    // ???????,?PPM???????
    uint16_t pulseWidth = IC1Value;

    // ???????PPM????



switch (channel)//ͨ����¼
	{
		case 0:
			if(IC1Value>=5000)//β֡����һ֡��ͷ֡��Ϊ��ȷ��ͨ��������)
			{
			channel+=1;//ͨ����
			TIM_SetCounter(TIM12, 0);//��ʼ��¼��Ƶʱ��
			}
			else
			{
			TIM_SetCounter(TIM12, 0);
			}
			break;                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                
		
		case 1:
			if(IC1Value<=3000)//һ��ppm�ź����Ϊ2ms����ֹ����������
			{
			channel+=1;//ͨ����
		    PPM[0]=(IC1Value-400);//��ȥ�̶��ĵ͵�ƽʱ�䣨�����õ����½����жϣ����¼��һ���ĸߵ�ƽʱ��͵͵�ƽʱ�䣩
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
		TIM_SetCounter(TIM12, 0);
	}

}

void PPM_Get_Values(uint16_t *values)
{
    // Copy PPM data to provided buffer
    for (int i = 0; i < 4; ++i)
    {
        values[i] = PPM[i];
    }
}
