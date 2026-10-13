/**
  ******************************************************************************
  * @file    buzzer.c
  * @brief   蜂鸣器驱动（标志位方式开关）—— 引脚 PA2，低电平响
  ******************************************************************************
  */

#include "buzzer.h"


volatile uint8_t Buzzer_Flag = 0;      //1开，0关

void Buzzer_Init(void)
{
    Buzzer_Flag = 0;

   
    HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_SET);
}

void Buzzer_SetFlag(uint8_t on)
{
    Buzzer_Flag = (on != 0) ? 1U : 0U;
    Buzzer_Update();                   /* 立刻生效，不用等下一次 Update */
}

uint8_t Buzzer_GetFlag(void)
{
    return Buzzer_Flag;
}

void Buzzer_ToggleFlag(void)
{
    Buzzer_Flag = (Buzzer_Flag != 0) ? 0U : 1U;
    Buzzer_Update();
}

void Buzzer_Update(void)
{
   
    if (Buzzer_Flag != 0)
    {
        HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_RESET);
    }
    else
    {
        HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_SET);
    }
}

void Buzzer_On(void)
{
    Buzzer_SetFlag(1);
}

void Buzzer_Off(void)
{
    Buzzer_SetFlag(0);
}
