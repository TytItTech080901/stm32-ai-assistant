#ifndef __W25QXX_H
#define __W25QXX_H

#include <stdint.h>

/**
 * @brief  初始化 W25Q16（读取芯片 ID 验证通信）
 * @retval 0 成功, -1 失败
 */
int W25Q_Init(void);

/**
 * @brief  从 W25Q16 读取数据
 * @param  addr   W25Q16 内部地址 (0x000000 ~ 0x1FFFFF)
 * @param  buf    目标 RAM 缓冲区
 * @param  len    读取字节数
 */
void W25Q_Read(uint32_t addr, uint8_t* buf, uint32_t len);

/**
 * @brief  读取芯片 JEDEC ID
 * @retval 例如 0xEF4015 (Winbond W25Q16)
 */
uint32_t W25Q_ReadID(void);

/**
 * @brief  等待芯片空闲 (BUSY 位清零)
 */
void W25Q_WaitBusy(void);

/**
 * @brief  擦除一个 4KB 扇区 (地址会自动对齐到 4K 边界)
 */
void W25Q_SectorErase(uint32_t addr);

/**
 * @brief  页编程 (最多写 256 字节, 不可跨页)
 * @param  addr  起始地址 (需页对齐以获得最佳性能)
 * @param  data  待写数据
 * @param  len   字节数 (最大 256)
 */
void W25Q_PageProgram(uint32_t addr, const uint8_t* data, uint16_t len);

#endif
