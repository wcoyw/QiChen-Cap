/**
  ******************************************************************************
  * @file    motor.h
  * @brief   电机驱动（TB6612 + TIM4 PWM）—— 设定速度与方向
  *          引脚按《引脚定义启辰杯》：
  *            A桥：AIN1 = PA15, AIN2 = PB3,  PWMA = PB6 (TIM4_CH1)
  *            B桥：BIN1 = PB4,  BIN2 = PB5,  PWMB = PB7 (TIM4_CH2)
  ******************************************************************************
  */

#ifndef __MOTOR_H
#define __MOTOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "tim.h"

/* ============================== 电机编号 ============================== */
#define MOTOR_A   0U   /* A桥：AIN1=PA15, AIN2=PB3, 调速 PWMA=PB6 (TIM4_CH1) */
#define MOTOR_B   1U   /* B桥：BIN1=PB4,  BIN2=PB5, 调速 PWMB=PB7 (TIM4_CH2) */

/* ========================= 速度（带符号）范围 =========================
 * TIM4 的 ARR = 99，所以速度值 0~100 就等于占空比 0~100%
 * 正数 = 正转，负数 = 反转，0 = 停转
 */
#define MOTOR_SPEED_MAX   100
#define MOTOR_SPEED_MIN  (-100)

/* ==================== 实际接线方向与代码相反时改成 1 ==================== */
#define MOTOR_A_REVERSE   0
#define MOTOR_B_REVERSE   0

/* =============================== 运动方向 =============================== */
typedef enum
{
    MOTOR_DIR_STOP = 0,     /* 停止：两轮都停转                              */
    MOTOR_DIR_FORWARD,      /* 前进：两轮正转                                */
    MOTOR_DIR_BACKWARD,     /* 后退：两轮反转                                */
    MOTOR_DIR_SPIN_LEFT,    /* 原地左转：A轮反转、B轮正转                    */
    MOTOR_DIR_SPIN_RIGHT,   /* 原地右转：A轮正转、B轮反转                    */
    MOTOR_DIR_TURN_LEFT,    /* 差速左转：A轮停转、B轮正转                    */
    MOTOR_DIR_TURN_RIGHT    /* 差速右转：A轮正转、B轮停转                    */
} Motor_Dir_t;

/* =============================== 接口函数 =============================== */

/**
  * @brief  电机初始化：停止电机并启动 TIM4 两路 PWM 输出
  * @note   必须在 MX_TIM4_Init() 之后调用
  */
void Motor_Init(void);

/**
  * @brief  设定单个电机的速度（带方向）
  * @param  motor MOTOR_A 或 MOTOR_B
  * @param  speed -100 ~ 100，正数正转、负数反转、0 停转
  */
void Motor_SetOne(uint8_t motor, int16_t speed);

/**
  * @brief  同时设定两个电机的速度（带方向）—— 最常用的调速函数
  * @param  speedA A轮速度 -100 ~ 100
  * @param  speedB B轮速度 -100 ~ 100
  * @note   例如 Motor_SetSpeed(60, 60) 前进；Motor_SetSpeed(-60, 60) 原地左转
  */
void Motor_SetSpeed(int16_t speedA, int16_t speedB);

/**
  * @brief  按“方向 + 速度”驱动小车
  * @param  dir   方向，见 Motor_Dir_t
  * @param  speed 速度大小 0 ~ 100（取绝对值使用）
  */
void Motor_SetDirection(Motor_Dir_t dir, int16_t speed);

/**
  * @brief  停止（两轮断电滑行）
  */
void Motor_Stop(void);

/**
  * @brief  刹车（两轮短接制动，比 Stop 停得更干脆）
  */
void Motor_Brake(void);

#ifdef __cplusplus
}
#endif

#endif /* __MOTOR_H */
