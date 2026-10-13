/**
  ******************************************************************************
  * @file    oled.h
  * @brief   OLED 驱动（SSD1306，软件 I2C）—— 由标准库工程移植到 HAL
  *
  *   引脚按《引脚定义启辰杯》：
  *     PB8 = OLED_SCL
  *     PB9 = OLED_SDA
  *
  *   注意：软件 I2C 不需要 CubeMX 里的硬件 I2C 外设。
  *         本工程 .ioc 里配置的 I2C2(PB10/PB11) 与本驱动无关，
  *         如果实际接线用的是 PB10/PB11 硬件 I2C，需要改用 HAL_I2C_Master_Transmit 版本。
  *
  *   显示坐标：Line 1~4（页），Column 1~16（列，8像素宽）
  ******************************************************************************
  */

#ifndef __OLED_H
#define __OLED_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

/* ============================== 引脚配置 ============================== */
#define OLED_GPIO_PORT   GPIOB
#define OLED_SCL_PIN     GPIO_PIN_8
#define OLED_SDA_PIN     GPIO_PIN_9

/* 位操作宏：写 SCL / SDA 电平（1 = 高，0 = 低） */
#define OLED_W_SCL(x)    HAL_GPIO_WritePin(OLED_GPIO_PORT, OLED_SCL_PIN, (x) ? GPIO_PIN_SET : GPIO_PIN_RESET)
#define OLED_W_SDA(x)    HAL_GPIO_WritePin(OLED_GPIO_PORT, OLED_SDA_PIN, (x) ? GPIO_PIN_SET : GPIO_PIN_RESET)

/* ============================== 接口函数 ============================== */

/**
  * @brief  OLED 初始化（含上电延时、端口初始化、SSD1306 寄存器配置、清屏）
  * @note   必须在 HAL_Init() 和 SystemClock_Config() 之后调用（内部用了 HAL_Delay）
  */
void OLED_Init(void);

/**
  * @brief  清屏
  */
void OLED_Clear(void);

/**
  * @brief  显示一个 ASCII 字符（8x16）
  * @param  Line   行位置 1~4
  * @param  Column 列位置 1~16
  * @param  Char   ASCII 可见字符（0x20 ~ 0x7E）
  */
void OLED_ShowChar(uint8_t Line, uint8_t Column, char Char);

/**
  * @brief  显示字符串
  * @param  Line   起始行 1~4
  * @param  Column 起始列 1~16
  * @param  String 字符串（以 '\0' 结尾）
  */
void OLED_ShowString(uint8_t Line, uint8_t Column, char *String);

/**
  * @brief  显示无符号十进制数
  * @param  Number 0 ~ 4294967295
  * @param  Length 显示位数 1~10
  */
void OLED_ShowNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);

/**
  * @brief  显示带符号十进制数（会显示 +/- 号）
  */
void OLED_ShowSignedNum(uint8_t Line, uint8_t Column, int32_t Number, uint8_t Length);

/**
  * @brief  显示十六进制数
  */
void OLED_ShowHexNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);

/**
  * @brief  显示二进制数
  */
void OLED_ShowBinNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);

/**
  * @brief  显示汉字（16x16，字库见 oled_font.h 的 Hzk1）
  * @param  num 汉字在 Hzk1 里的序号，从 0 开始
  */
void OLED_ShowChinese(uint8_t Line, uint8_t Column, uint8_t num);

#ifdef __cplusplus
}
#endif

#endif /* __OLED_H */
