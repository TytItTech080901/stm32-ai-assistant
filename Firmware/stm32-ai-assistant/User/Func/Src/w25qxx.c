/**
 ******************************************************************************
 * @file    w25qxx.c
 * @brief   W25Qxx
 ******************************************************************************
 */

#include "w25qxx.h"
#include "main.h"
#include "spi.h"

#define W25Q_CS_PORT GPIOA
#define W25Q_CS_PIN GPIO_PIN_15

#define W25Q_CS_LOW() HAL_GPIO_WritePin(W25Q_CS_PORT, W25Q_CS_PIN, GPIO_PIN_RESET)
#define W25Q_CS_HIGH() HAL_GPIO_WritePin(W25Q_CS_PORT, W25Q_CS_PIN, GPIO_PIN_SET)

#define W25Q_CMD_WRITE_ENABLE 0x06
#define W25Q_CMD_READ_STATUS1 0x05
#define W25Q_CMD_READ_DATA 0x03
#define W25Q_CMD_PAGE_PROGRAM 0x02
#define W25Q_CMD_SECTOR_ERASE 0x20 /* 4KB */
#define W25Q_CMD_JEDEC_ID 0x9F

#define W25Q_STATUS_BUSY 0x01

extern SPI_HandleTypeDef hspi1;

static void W25Q_SPI_TransmitReceive(uint8_t* tx, uint8_t* rx, uint16_t len)
{
  HAL_SPI_TransmitReceive(&hspi1, tx, rx, len, HAL_MAX_DELAY);
}

uint32_t W25Q_ReadID(void)
{
  uint8_t tx[4] = {W25Q_CMD_JEDEC_ID, 0, 0, 0};
  uint8_t rx[4];

  W25Q_CS_LOW();
  W25Q_SPI_TransmitReceive(tx, rx, 4);
  W25Q_CS_HIGH();

  return ((uint32_t)rx[1] << 16) | ((uint32_t)rx[2] << 8) | rx[3];
}

int W25Q_Init(void)
{
  W25Q_CS_HIGH();

  uint32_t id = W25Q_ReadID();
  /* W25Q16 JEDEC ID = 0xEF4015 */
  if ((id & 0xFFFF00) != 0xEF4000) {
    return -1;
  }
  return 0;
}

void W25Q_Read(uint32_t addr, uint8_t* buf, uint32_t len)
{
  uint8_t cmd[4];
  cmd[0] = W25Q_CMD_READ_DATA;
  cmd[1] = (uint8_t)(addr >> 16);
  cmd[2] = (uint8_t)(addr >> 8);
  cmd[3] = (uint8_t)(addr);

  W25Q_CS_LOW();
  HAL_SPI_Transmit(&hspi1, cmd, 4, HAL_MAX_DELAY);
  HAL_SPI_Receive(&hspi1, buf, len, HAL_MAX_DELAY);
  W25Q_CS_HIGH();
}

/* ---- 写入 / 擦除 ---------------------------------------------------- */

static void W25Q_WriteEnable(void)
{
  uint8_t cmd = W25Q_CMD_WRITE_ENABLE;
  W25Q_CS_LOW();
  HAL_SPI_Transmit(&hspi1, &cmd, 1, HAL_MAX_DELAY);
  W25Q_CS_HIGH();
}

static uint8_t W25Q_ReadStatus(void)
{
  uint8_t tx[2] = {W25Q_CMD_READ_STATUS1, 0xFF};
  uint8_t rx[2];
  W25Q_CS_LOW();
  W25Q_SPI_TransmitReceive(tx, rx, 2);
  W25Q_CS_HIGH();
  return rx[1];
}

void W25Q_WaitBusy(void)
{
  while (W25Q_ReadStatus() & W25Q_STATUS_BUSY) {
    /* 擦除期间让出 CPU（典型 50-400ms） */
  }
}

void W25Q_SectorErase(uint32_t addr)
{
  W25Q_WriteEnable();
  uint8_t cmd[4] = {W25Q_CMD_SECTOR_ERASE, (uint8_t)(addr >> 16), (uint8_t)(addr >> 8),
                    (uint8_t)(addr)};
  W25Q_CS_LOW();
  HAL_SPI_Transmit(&hspi1, cmd, 4, HAL_MAX_DELAY);
  W25Q_CS_HIGH();
  W25Q_WaitBusy();
}

void W25Q_PageProgram(uint32_t addr, const uint8_t* data, uint16_t len)
{
  if (len > 256)
    len = 256;
  W25Q_WriteEnable();
  uint8_t cmd[4] = {W25Q_CMD_PAGE_PROGRAM, (uint8_t)(addr >> 16), (uint8_t)(addr >> 8),
                    (uint8_t)(addr)};
  W25Q_CS_LOW();
  HAL_SPI_Transmit(&hspi1, cmd, 4, HAL_MAX_DELAY);
  HAL_SPI_Transmit(&hspi1, (uint8_t*)data, len, HAL_MAX_DELAY);
  W25Q_CS_HIGH();
  W25Q_WaitBusy();
}
