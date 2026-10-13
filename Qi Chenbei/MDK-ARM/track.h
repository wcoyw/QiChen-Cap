/**
 * @file    track.h
 * @brief   5 路循迹模块（逻辑对齐 CAR/CAR/track.c 的 7 路加权求差方案）
 *
 * 传感器接线按《引脚定义启辰杯》，车头从左到右：
 *   L2 = PB12, L1 = PB13, M = PB14, R1 = PB15, R2 = PA8
 * 探头引脚已由 MX_GPIO_Init() 配置为上拉输入，本模块只负责读写与差速。
 */
#ifndef __TRACK_H__
#define __TRACK_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stdint.h>

/* ============================== 传感器引脚 ============================== */
#define TRACK_L2_PORT   GPIOB
#define TRACK_L2_PIN    GPIO_PIN_12
#define TRACK_L1_PORT   GPIOB
#define TRACK_L1_PIN    GPIO_PIN_13
#define TRACK_M_PORT    GPIOB
#define TRACK_M_PIN     GPIO_PIN_14
#define TRACK_R1_PORT   GPIOB
#define TRACK_R1_PIN    GPIO_PIN_15
#define TRACK_R2_PORT   GPIOA
#define TRACK_R2_PIN    GPIO_PIN_8

/* ============================== 现场校准项 ==============================
 * 1) 探头“压到黑线”时的电平：0 = 低电平有效（上拉输入 + 常见循迹模块，默认）
 *                              1 = 高电平有效（若 5 路读数全反就改成 1）
 * 2) 左右镜像：默认 0 表示按《引脚定义启辰杯》接线；
 *              若实车左右探头顺序相反（修正方向反了），改成 1
 */
#define TRACK_LINE_ACTIVE_LEVEL   0
#define TRACK_MIRROR              0

/* ================================ 接口 ================================ */

/**
  * @brief  循迹初始化：保证上电停机（探头引脚已由 MX_GPIO_Init 配好）
  * @note   需在 Motor_Init() 之后调用
  */
void Track_Init(void);

/**
  * @brief  循迹一步：读 5 路探头 -> 计算差速 -> Motor_SetSpeed(左, 右)
  * @note   需周期调用（建议 5ms 一次）
  */
void Track_move(void);

/**
  * @brief  任意一路探头压线返回 1，全部丢线返回 0
  */
uint8_t Track_AnyLine(void);

#ifdef __cplusplus
}
#endif

#endif /* __TRACK_H__ */
