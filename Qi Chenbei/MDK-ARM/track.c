/**
 * @file    track.c
 * @brief   5 路循迹 —— 逻辑按 CAR/CAR/track.c（7 路加权求差）移植到启辰杯 STM32F103。
 *
 * 与 CAR 版一致的策略：
 *   1. 加权求差 err -> 乘 TRACK_ERR_SCALE 再除以压线数 -> 限幅得到差速量 turn；
 *   2. 只剩最外侧探头时先降到 EDGE 速度再差速，避免差速打满冲出赛道；
 *   3. 丢线时沿上一次的偏差方向以 LOST 速度继续找线（限幅 LOST_TURN_LIMIT）；
 *   4. 多路同时压线（宽线/十字）低速直行，并清掉找线记忆。
 * 输出经 Motor_SetSpeed(左轮, 右轮) 下发，速度 0~99 对应 PWM 0~100%。
 */
#include "track.h"
#include "motor.h"

/* ===================== 循迹参数（沿用 CAR/track.c 的整定值） ===================== */
#define TRACK_BASE_SPEED        14  /* 正常循迹基础速度 */
#define TRACK_LOST_SPEED        10  /* 丢线找线速度 */
#define TRACK_CROSS_SPEED       12  /* 宽线/十字直行速度 */
#define TRACK_TURN_LIMIT         9  /* 差速量限幅 */
#define TRACK_ERR_SCALE          4  /* 偏差放大倍数 */
#define TRACK_EDGE_SPEED        11  /* 只剩最外侧探头时的基础速度 */
#define TRACK_LOST_TURN_LIMIT    7  /* 丢线找线的差速限幅 */
#define TRACK_CROSS_COUNT        3  /* 5 路里 >=3 路同时压线视为宽线/十字 */

static int16_t s_last_track_turn = 0;   /* 上一次有效偏差，供丢线找线使用 */

static int16_t track_limit(int16_t x, int16_t min, int16_t max)
{
    if (x > max) return max;
    if (x < min) return min;
    return x;
}

/* 判读单个探头：返回 1 = 压到黑线（有效电平由 TRACK_LINE_ACTIVE_LEVEL 决定） */
static uint8_t track_read(GPIO_TypeDef *port, uint16_t pin)
{
    if (HAL_GPIO_ReadPin(port, pin) == (GPIO_PinState)TRACK_LINE_ACTIVE_LEVEL)
        return 1;
    return 0;
}

void Track_Init(void)
{
    Motor_Stop();   /* 上电先确保电机不动 */
}

uint8_t Track_AnyLine(void)
{
    if (track_read(TRACK_L2_PORT, TRACK_L2_PIN)) return 1;
    if (track_read(TRACK_L1_PORT, TRACK_L1_PIN)) return 1;
    if (track_read(TRACK_M_PORT,  TRACK_M_PIN))  return 1;
    if (track_read(TRACK_R1_PORT, TRACK_R1_PIN)) return 1;
    if (track_read(TRACK_R2_PORT, TRACK_R2_PIN)) return 1;
    return 0;
}

void Track_move(void)
{
    uint8_t l2 = track_read(TRACK_L2_PORT, TRACK_L2_PIN);
    uint8_t l1 = track_read(TRACK_L1_PORT, TRACK_L1_PIN);
    uint8_t m  = track_read(TRACK_M_PORT,  TRACK_M_PIN);
    uint8_t r1 = track_read(TRACK_R1_PORT, TRACK_R1_PIN);
    uint8_t r2 = track_read(TRACK_R2_PORT, TRACK_R2_PIN);
    uint8_t count = (uint8_t)(l2 + l1 + m + r1 + r2);

    int16_t err;
    int16_t turn = 0;
    int16_t l = TRACK_BASE_SPEED;
    int16_t r = TRACK_BASE_SPEED;

    /* 丢线：不直冲，沿上一次的偏差方向低速找线 */
    if (count == 0)
    {
        turn = track_limit(s_last_track_turn, -TRACK_LOST_TURN_LIMIT, TRACK_LOST_TURN_LIMIT);
        Motor_SetSpeed(track_limit((int16_t)(TRACK_LOST_SPEED + turn), 0, 99),
                       track_limit((int16_t)(TRACK_LOST_SPEED - turn), 0, 99));
        return;
    }

    /* 宽线/十字：低速直行，并清掉找线记忆 */
    if (count >= TRACK_CROSS_COUNT)
    {
        s_last_track_turn = 0;
        Motor_SetSpeed(TRACK_CROSS_SPEED, TRACK_CROSS_SPEED);
        return;
    }

    /* 加权求差：右侧探头为正 -> 左轮快、右轮慢 -> 向右修正，与 CAR 版方向一致
     * （L2/L1/M/R1/R2 权值即 7 路版去掉次外侧后对称取 6/3/0/-3/-6） */
    err = (int16_t)(6 * r2 + 3 * r1 + 0 * m - 3 * l1 - 6 * l2);
#if TRACK_MIRROR
    err = (int16_t)(-err);
#endif
    turn = track_limit((int16_t)((err * TRACK_ERR_SCALE) / (int16_t)count),
                       -TRACK_TURN_LIMIT, TRACK_TURN_LIMIT);
    s_last_track_turn = turn;

    /* 只剩最外侧探头：说明车已贴到赛道边缘，先降速再差速 */
    if (count == 1 && (l2 || r2))
    {
        l = track_limit((int16_t)(TRACK_EDGE_SPEED + turn), 0, 99);
        r = track_limit((int16_t)(TRACK_EDGE_SPEED - turn), 0, 99);
    }
    else
    {
        l = track_limit((int16_t)(TRACK_BASE_SPEED + turn), 0, 99);
        r = track_limit((int16_t)(TRACK_BASE_SPEED - turn), 0, 99);
    }

    Motor_SetSpeed(l, r);
}
