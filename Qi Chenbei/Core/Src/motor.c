

#include "motor.h"


static int16_t Motor_Limit(int16_t speed)
{
    if (speed > MOTOR_SPEED_MAX) return MOTOR_SPEED_MAX;
    if (speed < MOTOR_SPEED_MIN) return MOTOR_SPEED_MIN;
    return speed;
}

/**
  * @brief  把速度换算成方向引脚电平 + PWM 比较值并输出
  */
static void Motor_Output(uint8_t motor, int16_t speed)
{
    uint16_t      compare;
    GPIO_PinState in1;
    GPIO_PinState in2;

    speed = Motor_Limit(speed);

    /* 接线方向与代码相反时的补偿 */
    if ((motor == MOTOR_A && MOTOR_A_REVERSE) ||
        (motor == MOTOR_B && MOTOR_B_REVERSE))
    {
        speed = -speed;
    }

    if (speed > 0)                                  /* 正转 */
    {
        in1 = GPIO_PIN_SET;   in2 = GPIO_PIN_RESET;   compare = (uint16_t)speed;
    }
    else if (speed < 0)                             /* 反转 */
    {
        in1 = GPIO_PIN_RESET; in2 = GPIO_PIN_SET;     compare = (uint16_t)(-speed);
    }
    else                                            /* 停止 */
    {
        in1 = GPIO_PIN_RESET; in2 = GPIO_PIN_RESET;   compare = 0;
    }

    if (motor == MOTOR_A)
    {
        HAL_GPIO_WritePin(AIN1_GPIO_Port, AIN1_Pin, in1);
        HAL_GPIO_WritePin(AIN2_GPIO_Port, AIN2_Pin, in2);
        __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, compare);
    }
    else
    {
        HAL_GPIO_WritePin(BIN1_GPIO_Port, BIN1_Pin, in1);
        HAL_GPIO_WritePin(BIN2_GPIO_Port, BIN2_Pin, in2);
        __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_2, compare);
    }
}

void Motor_Init(void)
{
    /* 方向引脚 AIN1/AIN2/BIN1/BIN2 已由 MX_GPIO_Init() 配置为推挽输出 */

    Motor_Stop();                                   /* 上电先确保电机不动 */

    /* 启动 TIM4 的两路 PWM 输出（CubeMX 生成的 MX_TIM4_Init 不会自动启动） */
    HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_2);
}

void Motor_SetOne(uint8_t motor, int16_t speed)
{
    Motor_Output(motor, speed);
}

void Motor_SetSpeed(int16_t speedA, int16_t speedB)
{
    Motor_Output(MOTOR_A, speedA);
    Motor_Output(MOTOR_B, speedB);
}

void Motor_SetDirection(Motor_Dir_t dir, int16_t speed)
{
    if (speed < 0) speed = -speed;                  /* 只取大小，方向由 dir 决定 */
    if (speed > MOTOR_SPEED_MAX) speed = MOTOR_SPEED_MAX;

    switch (dir)
    {
        case MOTOR_DIR_FORWARD:                     /* 前进 */
            Motor_SetSpeed( speed,  speed);
            break;

        case MOTOR_DIR_BACKWARD:                    /* 后退 */
            Motor_SetSpeed(-speed, -speed);
            break;

        case MOTOR_DIR_SPIN_LEFT:                   /* 原地左转 */
            Motor_SetSpeed(-speed,  speed);
            break;

        case MOTOR_DIR_SPIN_RIGHT:                  /* 原地右转 */
            Motor_SetSpeed( speed, -speed);
            break;

        case MOTOR_DIR_TURN_LEFT:                   /* 差速左转 */
            Motor_SetSpeed(     0,  speed);
            break;

        case MOTOR_DIR_TURN_RIGHT:                  /* 差速右转 */
            Motor_SetSpeed( speed,      0);
            break;

        case MOTOR_DIR_STOP:
        default:
            Motor_Stop();
            break;
    }
}

void Motor_Stop(void)
{
    Motor_Output(MOTOR_A, 0);
    Motor_Output(MOTOR_B, 0);
}

void Motor_Brake(void)
{
    /* IN1 = IN2 = 1，PWM = 0 -> 电机绕组短接制动 */
    HAL_GPIO_WritePin(AIN1_GPIO_Port, AIN1_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(AIN2_GPIO_Port, AIN2_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(BIN1_GPIO_Port, BIN1_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(BIN2_GPIO_Port, BIN2_Pin, GPIO_PIN_SET);
    __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, 0);
    __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_2, 0);
}
