/*
* SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
*
* SPDX-License-Identifier: MIT
*/

#ifndef __MYFLASH_H
#define __MYFLASH_H

#ifdef __cplusplus

extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include <stdbool.h>
#include "stm32g0xx_hal_flash_ex.h"

#define STM32G0xx_PAGE_SIZE (0x800)
#define STM32G0xx_FLASH_PAGE0_STARTADDR (0x8000000)
#define STM32G0xx_FLASH_PAGE15_STARTADDR (STM32G0xx_FLASH_PAGE0_STARTADDR+15*STM32G0xx_PAGE_SIZE)
#define LIN_MIC_PC_EN_STATUS_ADDR (STM32G0xx_FLASH_PAGE15_STARTADDR+0)
#define HP_MODE_STATUS_ADDR (STM32G0xx_FLASH_PAGE15_STARTADDR+1)
#define RGB_LIGHT_STATUS_ADDR (STM32G0xx_FLASH_PAGE15_STARTADDR+2)
#define I2C_ADDR (STM32G0xx_FLASH_PAGE15_STARTADDR+3)

bool set_lin_mic_pc_en_status(uint8_t data);
bool set_hp_mode_status(uint8_t data);
bool set_rgb_light_status(uint8_t data);
bool set_i2c_addr(uint8_t data);

uint8_t get_lin_mic_pc_en_status(void);
uint8_t get_hp_mode_status(void);
uint8_t get_rgb_light_status(void);
uint8_t get_i2c_addr(void);

bool write_flash(uint8_t pc_en,uint8_t hp_mode,uint8_t rgb_light);


#ifdef __cplusplus
}
#endif

#endif /* __MYFLASH_H */
