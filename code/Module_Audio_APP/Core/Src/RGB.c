#include "RGB.h"
#include "tim.h"

/*Some Static Colors------------------------------*/
static const RGB_Color_TypeDef RED   = {255, 0, 0};
static const RGB_Color_TypeDef GREEN = {0, 255, 0};
static const RGB_Color_TypeDef BLUE  = {0, 0, 255};
static const RGB_Color_TypeDef BLACK = {0, 0, 0};
static const RGB_Color_TypeDef WHITE = {255, 255, 255};

/*二维数组存放最终PWM输出数组，每一行24个
 数据代表一个LED，最后一行24个0代表RESET码*/
static uint8_t rgb_buf[RGB_NUM + 1][24];
__IO uint8_t rgb_dma_busy = 0U;

void rgb_init(void)
{
    activate_tim17_dma();
    rgb_close_all();
}

// DMA初始化
void activate_tim17_dma(void)
{
    LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_5, (RGB_NUM + 1) * 24);           // 设置dma数据传输个数/长度
    LL_DMA_SetMemoryAddress(DMA1, LL_DMA_CHANNEL_5, (uint32_t)rgb_buf);         // 设置内存地址，也就是设置buffer地址
    LL_DMA_SetPeriphAddress(DMA1, LL_DMA_CHANNEL_5, (uint32_t)&(TIM17->CCR1));  // 设置外设地址
    LL_DMA_ClearFlag_GI5(DMA1);                                                 // 清除中断标志
    LL_DMA_ClearFlag_TC5(DMA1);                                                 // 清除中断标志
    LL_DMA_EnableIT_TC(DMA1, LL_DMA_CHANNEL_5);                                 // 使能传送完成中断
    LL_DMA_EnableIT_TE(DMA1, LL_DMA_CHANNEL_5);                                 // 使能传送错误中断
    LL_TIM_EnableDMAReq_CC1(TIM17);
    LL_TIM_EnableAllOutputs(TIM17);                            // 使能TIM的输出
    LL_TIM_CC_SetDMAReqTrigger(TIM17, LL_TIM_CCDMAREQUEST_CC);  // 设置TIM17 DMA请求触发器
    LL_TIM_CC_EnableChannel(TIM17, LL_TIM_CHANNEL_CH1);        // 使能TIM17的通道1
}

/**
 * @brief  设定单个RGB LED的颜色，把结构体中RGB的24BIT转换为0码和1码
 * @param  led_id 为LED序号
 * @param  color 要设置的颜色
 * @retval 无
 */
void set_rgb_color(uint8_t led_id, RGB_Color_TypeDef color)
{
    if (rgb_dma_busy != 0U) {
        return;
    }

    uint8_t i;
    color.R = color.R * (rgb_light_reg / 100.0);  // 设置亮度
    color.G = color.G * (rgb_light_reg / 100.0);
    color.B = color.B * (rgb_light_reg / 100.0);
    for (i = 0; i < 8; i++) {
        rgb_buf[led_id][i] = ((color.G & (1 << (7 - i))) ? (CODE_1) : CODE_0);  // 数组某一行0~7转化存放G
    }
    for (i = 8; i < 16; i++) {
        rgb_buf[led_id][i] = ((color.R & (1 << (15 - i))) ? (CODE_1) : CODE_0);  // 数组某一行8~15转化存放R
    }
    for (i = 16; i < 24; i++) {
        rgb_buf[led_id][i] = ((color.B & (1 << (23 - i))) ? (CODE_1) : CODE_0);  // 数组某一行16~23转化存放B
    }
}

/**
 * @brief  rgb状态更新
 * @param  无
 * @retval 无
 */
void rgb_update(void)
{
    if (rgb_dma_busy != 0U) {
        return;
    }

    set_rgb_color(RGB_LED_1, rgb_reg[0]);
    set_rgb_color(RGB_LED_2, rgb_reg[1]);
    set_rgb_color(RGB_LED_3, rgb_reg[2]);
    reset_load();
    rgb_send_array();
}

/**
 * @brief  最后一行装在24个0，输出24个周期占空比为0的PWM波，作为最后reset延时，这里总时长为24*1.2=30us >
 * 24us(要求大于24us)
 * @param  无
 * @retval 无
 */
void reset_load(void)
{
    if (rgb_dma_busy != 0U) {
        return;
    }

    uint8_t i;
    for (i = 0; i < 24; i++) {
        rgb_buf[RGB_NUM][i] = 0;
    }
}

/**
 * @brief  发送数组
 * @param  无
 * @retval 无
 */
void rgb_send_array(void)
{
    if (rgb_dma_busy != 0U) {
        return;
    }

    LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_5, (RGB_NUM + 1) * 24);
    rgb_dma_busy = 1U;
    LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_5);
    LL_TIM_EnableCounter(TIM17);
}

/**
 * @brief  显示红色
 * @param  rgb_led 要设置的RGB灯
 * @retval 无
 */
void rgb_red(rgb_led_t rgb_led)
{
    set_rgb_color(rgb_led, RED);
    reset_load();
    rgb_send_array();
}

/**
 * @brief  显示绿色
 * @param  rgb_led 要设置的RGB灯
 * @retval 无
 */
void rgb_green(rgb_led_t rgb_led)
{
    set_rgb_color(rgb_led, GREEN);
    reset_load();
    rgb_send_array();
}

/**
 * @brief  显示蓝色
 * @param  rgb_led 要设置的RGB灯
 * @retval 无
 */
void rgb_blue(rgb_led_t rgb_led)
{
    set_rgb_color(rgb_led, BLUE);
    reset_load();
    rgb_send_array();
}

/**
 * @brief  显示白色
 * @param  rgb_led 要设置的RGB灯
 * @retval 无
 */
void rgb_white(rgb_led_t rgb_led)
{
    set_rgb_color(rgb_led, WHITE);
    reset_load();
    rgb_send_array();
}

/**
 * @brief  关闭RGB
 * @param  rgb_led 要设置的RGB灯
 * @retval 无
 */
void rgb_close(rgb_led_t rgb_led)
{
    set_rgb_color(rgb_led, BLACK);
    reset_load();
    rgb_send_array();
}

/**
 * @brief  关闭所有RGB灯
 * @param  无
 * @retval 无
 */
void rgb_close_all(void)
{
    for (uint8_t i = 0; i < RGB_NUM; i++) {
        set_rgb_color(i, BLACK);
    }
    reset_load();
    rgb_send_array();
    HAL_Delay(0);
}
