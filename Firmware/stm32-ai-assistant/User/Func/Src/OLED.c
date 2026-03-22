#include "main.h"
#include "OLED_Font.h"
#include "fft_visualer.h"

#define OLED_SCL_PORT GPIOB
#define OLED_SCL_PIN GPIO_PIN_6
#define OLED_SDA_PORT GPIOB
#define OLED_SDA_PIN GPIO_PIN_7

#define OLED_W_SCL(x)                                                                              \
    HAL_GPIO_WritePin(OLED_SCL_PORT, OLED_SCL_PIN, (x) ? GPIO_PIN_SET : GPIO_PIN_RESET)
#define OLED_W_SDA(x)                                                                              \
    HAL_GPIO_WritePin(OLED_SDA_PORT, OLED_SDA_PIN, (x) ? GPIO_PIN_SET : GPIO_PIN_RESET)

static void OLED_I2C_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitStruct.Pin   = OLED_SCL_PIN | OLED_SDA_PIN;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(OLED_SCL_PORT, &GPIO_InitStruct);

    OLED_W_SCL(1);
    OLED_W_SDA(1);
}

static void OLED_I2C_Start(void)
{
    OLED_W_SDA(1);
    OLED_W_SCL(1);
    OLED_W_SDA(0);
    OLED_W_SCL(0);
}

static void OLED_I2C_Stop(void)
{
    OLED_W_SDA(0);
    OLED_W_SCL(1);
    OLED_W_SDA(1);
}

static void OLED_I2C_SendByte(uint8_t byte)
{
    for (uint8_t i = 0; i < 8; i++) {
        OLED_W_SDA(!!(byte & (0x80U >> i)));
        OLED_W_SCL(1);
        OLED_W_SCL(0);
    }
    OLED_W_SCL(1);
    OLED_W_SCL(0);
}

static void OLED_WriteCommand(uint8_t command)
{
    OLED_I2C_Start();
    OLED_I2C_SendByte(0x78);
    OLED_I2C_SendByte(0x00);
    OLED_I2C_SendByte(command);
    OLED_I2C_Stop();
}

static void OLED_WriteData(uint8_t data)
{
    OLED_I2C_Start();
    OLED_I2C_SendByte(0x78);
    OLED_I2C_SendByte(0x40);
    OLED_I2C_SendByte(data);
    OLED_I2C_Stop();
}

static void OLED_SetCursor(uint8_t y, uint8_t x)
{
    OLED_WriteCommand(0xB0 | y);
    OLED_WriteCommand(0x10 | ((x & 0xF0U) >> 4));
    OLED_WriteCommand(0x00 | (x & 0x0FU));
}

void OLED_Clear(void)
{
    for (uint8_t page = 0; page < 8; page++) {
        OLED_SetCursor(page, 0);
        for (uint8_t col = 0; col < 128; col++) {
            OLED_WriteData(0x00);
        }
    }
}

void OLED_ShowChar(uint8_t Line, uint8_t Column, char Char)
{
    OLED_SetCursor((Line - 1U) * 2U, (Column - 1U) * 8U);
    for (uint8_t i = 0; i < 8; i++) {
        OLED_WriteData(OLED_F8x16[Char - ' '][i]);
    }

    OLED_SetCursor((Line - 1U) * 2U + 1U, (Column - 1U) * 8U);
    for (uint8_t i = 0; i < 8; i++) {
        OLED_WriteData(OLED_F8x16[Char - ' '][i + 8U]);
    }
}

void OLED_ShowString(uint8_t Line, uint8_t Column, char* String)
{
    for (uint8_t i = 0; String[i] != '\0'; i++) {
        OLED_ShowChar(Line, Column + i, String[i]);
    }
}

static uint32_t OLED_Pow(uint32_t X, uint32_t Y)
{
    uint32_t Result = 1;
    while (Y--) {
        Result *= X;
    }
    return Result;
}

void OLED_ShowNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
    for (uint8_t i = 0; i < Length; i++) {
        OLED_ShowChar(Line, Column + i, Number / OLED_Pow(10, Length - i - 1U) % 10U + '0');
    }
}

void OLED_ShowSignedNum(uint8_t Line, uint8_t Column, int32_t Number, uint8_t Length)
{
    uint32_t Number1;
    if (Number >= 0) {
        OLED_ShowChar(Line, Column, '+');
        Number1 = Number;
    } else {
        OLED_ShowChar(Line, Column, '-');
        Number1 = (uint32_t)(-Number);
    }

    for (uint8_t i = 0; i < Length; i++) {
        OLED_ShowChar(Line, Column + i + 1U, Number1 / OLED_Pow(10, Length - i - 1U) % 10U + '0');
    }
}

void OLED_ShowHexNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
    for (uint8_t i = 0; i < Length; i++) {
        uint8_t SingleNumber = Number / OLED_Pow(16, Length - i - 1U) % 16U;
        if (SingleNumber < 10U) {
            OLED_ShowChar(Line, Column + i, SingleNumber + '0');
        } else {
            OLED_ShowChar(Line, Column + i, SingleNumber - 10U + 'A');
        }
    }
}

void OLED_ShowBinNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
    for (uint8_t i = 0; i < Length; i++) {
        OLED_ShowChar(Line, Column + i, Number / OLED_Pow(2, Length - i - 1U) % 2U + '0');
    }
}

void OLED_Init(void)
{
    HAL_Delay(10);
    OLED_I2C_Init();

    OLED_WriteCommand(0xAE);
    OLED_WriteCommand(0xD5);
    OLED_WriteCommand(0x80);
    OLED_WriteCommand(0xA8);
    OLED_WriteCommand(0x3F);
    OLED_WriteCommand(0xD3);
    OLED_WriteCommand(0x00);
    OLED_WriteCommand(0x40);
    OLED_WriteCommand(0xA1);
    OLED_WriteCommand(0xC8);
    OLED_WriteCommand(0xDA);
    OLED_WriteCommand(0x12);
    OLED_WriteCommand(0x81);
    OLED_WriteCommand(0xCF);
    OLED_WriteCommand(0xD9);
    OLED_WriteCommand(0xF1);
    OLED_WriteCommand(0xDB);
    OLED_WriteCommand(0x30);
    OLED_WriteCommand(0xA4);
    OLED_WriteCommand(0xA6);
    OLED_WriteCommand(0x8D);
    OLED_WriteCommand(0x14);
    OLED_WriteCommand(0xAF);
    OLED_Clear();
}

void OLED_DrawSpectrumBars(const uint8_t* heights, uint8_t count)
{
    if (heights == NULL || count == 0) {
        return;
    }

    for (uint8_t page = 2; page < 8; ++page) {
        OLED_SetCursor(page, 0);
        for (uint8_t x = 0; x < 128; ++x) {
            uint8_t band    = (uint8_t)((x * count) / 128U);
            uint8_t height  = heights[band];
            uint8_t local_y = (uint8_t)((page - 2U) * 8U);
            uint8_t top_filled =
                (height >= FFT_VISUAL_GRAPH_HEIGHT) ? 0U : (FFT_VISUAL_GRAPH_HEIGHT - height);
            uint8_t pixels = 0;

            for (uint8_t bit = 0; bit < 8; ++bit) {
                uint8_t y = local_y + bit;
                if (y >= top_filled) {
                    pixels |= (uint8_t)(1U << bit);
                }
            }
            OLED_WriteData(pixels);
        }
    }
}
