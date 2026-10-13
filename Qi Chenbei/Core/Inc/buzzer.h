/**
  ******************************************************************************
  * @file    buzzer.h
  * @brief   蜂鸣器驱动（标志位方式开关）—— 引脚 PA2
  *
  *   用法（标志位思想：业务逻辑只改变量，硬件输出统一由 Buzzer_Update 完成）：
  *
  *      Buzzer_Init();                 // 初始化，默认静音
  *      Buzzer_SetFlag(1);             // 置标志位 -> 响（内部会立刻更新一次硬件）
  *      Buzzer_SetFlag(0);             // 清标志位 -> 停
  *      Buzzer_ToggleFlag();           // 翻转标志位
  *      if (Buzzer_GetFlag()) { ... }  // 读标志位，别的地方可以据此判断状态
  *
  *      Buzzer_Update();               // 放在主循环 / 定时器中断里，
  *                                     // 保证标志位被任何地方改动后硬件都能跟上
  *
  *   硬件：PA2 低电平 = 响，高电平 = 停（与校赛工程 Buzzer.c 一致）
  ******************************************************************************
  */

#ifndef __BUZZER_H
#define __BUZZER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

/* 蜂鸣器开关标志位：0 = 关，1 = 开
 * volatile：可能在中断里被改，主循环读的时候每次都从内存取 */
extern volatile uint8_t Buzzer_Flag;

/**
  * @brief  蜂鸣器初始化：确保静音、标志位清零
  * @note   PA2 已由 MX_GPIO_Init() 配置为推挽输出，这里只做状态初始化
  */
void Buzzer_Init(void);

/**
  * @brief  设置蜂鸣器标志位（同时立即更新一次硬件输出）
  * @param  on 0 = 关，非 0 = 开
  */
void Buzzer_SetFlag(uint8_t on);

/**
  * @brief  读取当前标志位
  * @retval 0 = 关，1 = 开
  */
uint8_t Buzzer_GetFlag(void);

/**
  * @brief  翻转标志位（响 <-> 停）
  */
void Buzzer_ToggleFlag(void);

/**
  * @brief  按标志位驱动硬件引脚
  * @note   放在主循环或定时器中断中周期调用；重复调用无副作用
  */
void Buzzer_Update(void);

/* ------- 便捷函数（内部也是走标志位，等价于 SetFlag） ------- */
void Buzzer_On(void);
void Buzzer_Off(void);

#ifdef __cplusplus
}
#endif

#endif /* __BUZZER_H */
