/**
  ******************************************************************************
  * @file    oled.c
  * @brief   OLED 驱动（SSD1306，软件 I2C）—— 标准库工程移植到 HAL
  *
  *   移植说明：
  *     1. SPL 的 RCC_APB2PeriphClockCmd / GPIO_Init  ->  HAL 的 __HAL_RCC_GPIOB_CLK_ENABLE / HAL_GPIO_Init
  *     2. SPL 的 GPIO_WriteBit                       ->  HAL_GPIO_WritePin（见 oled.h 的 OLED_W_SCL/SDA）
  *     3. SPL 的上电空循环延时                        ->  HAL_Delay
  *     4. 位操作时序完全保持原样，只额外加了很小的 I2C 延时提高稳定性
  ******************************************************************************
  */

#include "oled.h"
#include "oled_font.h"

/* ============================ 软件 I2C 底层 ============================ */

/**
  * @brief  软件 I2C 时序延时（约 1us）
  * @note   如果屏幕显示不稳定/花屏，把循环次数加大；追求速度可以改成 0
  */
static void OLED_I2C_Delay(void)
{
    volatile uint8_t i;
    for (i = 0; i < 2; i++)
    {
        __NOP();
    }
}

/**
  * @brief  软件 I2C 初始化：PB8/PB9 配成开漏输出 + 内部上拉
  */
static void OLED_I2C_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitStruct.Pin   = OLED_SCL_PIN | OLED_SDA_PIN;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_OD;      /* 开漏，配合外部/内部上拉 */
    GPIO_InitStruct.Pull  = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(OLED_GPIO_PORT, &GPIO_InitStruct);

    OLED_W_SCL(1);
    OLED_W_SDA(1);
}

/**
  * @brief  I2C 起始条件
  */
static void OLED_I2C_Start(void)
{
    OLED_W_SDA(1);
    OLED_W_SCL(1);
    OLED_I2C_Delay();
    OLED_W_SDA(0);
    OLED_I2C_Delay();
    OLED_W_SCL(0);
    OLED_I2C_Delay();
}

/**
  * @brief  I2C 停止条件
  */
static void OLED_I2C_Stop(void)
{
    OLED_W_SDA(0);
    OLED_I2C_Delay();
    OLED_W_SCL(1);
    OLED_I2C_Delay();
    OLED_W_SDA(1);
    OLED_I2C_Delay();
}

/**
  * @brief  I2C 发送一个字节（不处理应答）
  */
static void OLED_I2C_SendByte(uint8_t Byte)
{
    uint8_t i;
    for (i = 0; i < 8; i++)
    {
        OLED_W_SDA(Byte & (0x80 >> i));
        OLED_I2C_Delay();
        OLED_W_SCL(1);
        OLED_I2C_Delay();
        OLED_W_SCL(0);
        OLED_I2C_Delay();
    }
    OLED_W_SCL(1);          /* 额外的一个时钟，不处理应答信号 */
    OLED_I2C_Delay();
    OLED_W_SCL(0);
    OLED_I2C_Delay();
}

/**
  * @brief  OLED 写命令
  */
static void OLED_WriteCommand(uint8_t Command)
{
    OLED_I2C_Start();
    OLED_I2C_SendByte(0x78);        /* 从机地址 */
    OLED_I2C_SendByte(0x00);        /* 写命令 */
    OLED_I2C_SendByte(Command);
    OLED_I2C_Stop();
}

/**
  * @brief  OLED 写数据
  */
static void OLED_WriteData(uint8_t Data)
{
    OLED_I2C_Start();
    OLED_I2C_SendByte(0x78);        /* 从机地址 */
    OLED_I2C_SendByte(0x40);        /* 写数据 */
    OLED_I2C_SendByte(Data);
    OLED_I2C_Stop();
}

/**
  * @brief  设置光标位置
  * @param  Y 页地址 0~7（向下）
  * @param  X 列地址 0~127（向右）
  */
static void OLED_SetCursor(uint8_t Y, uint8_t X)
{
    OLED_WriteCommand(0xB0 | Y);                    /* 设置 Y 位置 */
    OLED_WriteCommand(0x10 | ((X & 0xF0) >> 4));    /* 设置 X 位置高 4 位 */
    OLED_WriteCommand(0x00 | (X & 0x0F));           /* 设置 X 位置低 4 位 */
}

/* ============================== 对外接口 ============================== */

void OLED_Clear(void)
{
    uint8_t i, j;
    for (j = 0; j < 8; j++)
    {
        OLED_SetCursor(j, 0);
        for (i = 0; i < 128; i++)
        {
            OLED_WriteData(0x00);
        }
    }
}

void OLED_ShowChar(uint8_t Line, uint8_t Column, char Char)
{
    uint8_t i;
    OLED_SetCursor((Line - 1) * 2, (Column - 1) * 8);           /* 上半部分 */
    for (i = 0; i < 8; i++)
    {
        OLED_WriteData(OLED_F8x16[Char - ' '][i]);
    }
    OLED_SetCursor((Line - 1) * 2 + 1, (Column - 1) * 8);       /* 下半部分 */
    for (i = 0; i < 8; i++)
    {
        OLED_WriteData(OLED_F8x16[Char - ' '][i + 8]);
    }
}

void OLED_ShowString(uint8_t Line, uint8_t Column, char *String)
{
    uint8_t i;
    for (i = 0; String[i] != '\0'; i++)
    {
        OLED_ShowChar(Line, Column + i, String[i]);
    }
}

void OLED_ShowChinese(uint8_t Line, uint8_t Column, uint8_t num)
{
    uint8_t i;
    OLED_SetCursor((Line - 1) * 2, (Column - 1) * 16);          /* 上半部分 */
    for (i = 0; i < 16; i++)
    {
        OLED_WriteData(Hzk1[num][i]);
    }
    OLED_SetCursor((Line - 1) * 2 + 1, (Column - 1) * 16);      /* 下半部分 */
    for (i = 0; i < 16; i++)
    {
        OLED_WriteData(Hzk1[num][i + 16]);
    }
}

/**
  * @brief  OLED 次方函数
  */
static uint32_t OLED_Pow(uint32_t X, uint32_t Y)
{
    uint32_t Result = 1;
    while (Y--)
    {
        Result *= X;
    }
    return Result;
}

void OLED_ShowNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
    uint8_t i;
    for (i = 0; i < Length; i++)
    {
        OLED_ShowChar(Line, Column + i, Number / OLED_Pow(10, Length - i - 1) % 10 + '0');
    }
}

void OLED_ShowSignedNum(uint8_t Line, uint8_t Column, int32_t Number, uint8_t Length)
{
    uint8_t  i;
    uint32_t Number1;
    if (Number >= 0)
    {
        OLED_ShowChar(Line, Column, '+');
        Number1 = Number;
    }
    else
    {
        OLED_ShowChar(Line, Column, '-');
        Number1 = -Number;
    }
    for (i = 0; i < Length; i++)
    {
        OLED_ShowChar(Line, Column + i + 1, Number1 / OLED_Pow(10, Length - i - 1) % 10 + '0');
    }
}

void OLED_ShowHexNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
    uint8_t i, SingleNumber;
    for (i = 0; i < Length; i++)
    {
        SingleNumber = Number / OLED_Pow(16, Length - i - 1) % 16;
        if (SingleNumber < 10)
        {
            OLED_ShowChar(Line, Column + i, SingleNumber + '0');
        }
        else
        {
            OLED_ShowChar(Line, Column + i, SingleNumber - 10 + 'A');
        }
    }
}

void OLED_ShowBinNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
    uint8_t i;
    for (i = 0; i < Length; i++)
    {
        OLED_ShowChar(Line, Column + i, Number / OLED_Pow(2, Length - i - 1) % 2 + '0');
    }
}

void OLED_Init(void)
{
    HAL_Delay(100);                 /* 上电延时，等 OLED 内部复位完成 */

    OLED_I2C_Init();                /* 端口初始化 */

    OLED_WriteCommand(0xAE);        /* 关闭显示 */

    OLED_WriteCommand(0xD5);        /* 设置显示时钟分频比/振荡器频率 */
    OLED_WriteCommand(0x80);

    OLED_WriteCommand(0xA8);        /* 设置多路复用率 */
    OLED_WriteCommand(0x3F);

    OLED_WriteCommand(0xD3);        /* 设置显示偏移 */
    OLED_WriteCommand(0x00);

    OLED_WriteCommand(0x40);        /* 设置显示开始行 */

    OLED_WriteCommand(0xA1);        /* 设置左右方向：0xA1 正常，0xA0 左右反置 */

    OLED_WriteCommand(0xC8);        /* 设置上下方向：0xC8 正常，0xC0 上下反置 */

    OLED_WriteCommand(0xDA);        /* 设置 COM 引脚硬件配置 */
    OLED_WriteCommand(0x12);

    OLED_WriteCommand(0x81);        /* 设置对比度控制 */
    OLED_WriteCommand(0xCF);

    OLED_WriteCommand(0xD9);        /* 设置预充电周期 */
    OLED_WriteCommand(0xF1);

    OLED_WriteCommand(0xDB);        /* 设置 VCOMH 取消选择级别 */
    OLED_WriteCommand(0x30);

    OLED_WriteCommand(0xA4);        /* 设置整个显示打开/关闭 */

    OLED_WriteCommand(0xA6);        /* 设置正常/倒转显示 */

    OLED_WriteCommand(0x8D);        /* 设置充电泵 */
    OLED_WriteCommand(0x14);

    OLED_WriteCommand(0xAF);        /* 开启显示 */

    OLED_Clear();                   /* OLED 清屏 */
}
