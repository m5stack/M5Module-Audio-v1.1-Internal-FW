/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef __RGB_H
#define __RGB_H

#ifdef __cplusplus

extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

// 根据系统时钟计算
#define CODE_1 (52) // 1码定时器计数次数
#define CODE_0 (25) // 0码定时器计数次数
#define RGB_NUM (3)	 // LED数量宏定义

/*建立一个定义单个LED三原色值大小的结构体*/
typedef struct {
	uint8_t R;
	uint8_t G;
	uint8_t B;
} RGB_Color_TypeDef;

extern __IO uint8_t rgb_light_reg;
extern __IO RGB_Color_TypeDef rgb_reg[3];
extern __IO uint8_t rgb_dma_busy;

void rgb_init(void); // RGB初始化
void activate_tim17_dma(void);                         // DMA初始化
void set_rgb_color(uint8_t led_id, RGB_Color_TypeDef color); // 给一个LED装载24个颜色数据码（0码和1码）
void rgb_update(void);    // rgb状态更新
void reset_load(void);				 // 该函数用于将数组最后24个数据变为0，代表RESET_code
void rgb_send_array(void);				 // 发送最终数组
void rgb_red(rgb_led_t rgb_led);			 // 显示红灯
void rgb_green(rgb_led_t rgb_led);			 // 显示绿灯
void rgb_blue(rgb_led_t rgb_led);			 // 显示蓝灯
void rgb_white(rgb_led_t rgb_led);			 // 显示白灯
void rgb_close(rgb_led_t rgb_led);			 // 关闭rgb灯
void rgb_close_all(void);				 // 关闭所有RGB灯

#ifdef __cplusplus
}
#endif

#endif /* __RGB_H */
