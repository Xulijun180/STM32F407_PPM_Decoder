#include "PpmDecode.h"
#include "stm32f4xx.h"

static uint16_t PPM_data[4];
static uint8_t channel = 0;

void Ppm_Input_Init(void)
{
    // ???PPM???????...
}

void TIM8_BRK_TIM12_IRQHandler(void)
{
    // ?????????...
}

void Ppm_Get_Data(uint16_t *ppm_data)
{
    // ??PPM??
    for (int i = 0; i < 4; ++i) {
        ppm_data[i] = PPM_data[i];
    }
}
