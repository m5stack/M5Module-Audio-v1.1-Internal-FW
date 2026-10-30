/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dma.h"
#include "i2c.h"
#include "iwdg.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "i2c_ex.h"
#include "RGB.h"
#include "myflash.h"
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
// 寄存器地址
#define LIN_MIC_PC_EN_ADDR       (0x00)
#define HP_MODE_ADDR             (0x10)
#define HP_MIC_EN_ADDR           (0x11)
#define HP_DET_ADDR              (0x20)
#define RGB_LIGHT_ADDR           (0x30)
#define RGB1_STATUS_ADDR         (0x40)
#define RGB2_STATUS_ADDR         (0x43)
#define RGB3_STATUS_ADDR         (0x46)
#define UID_REG_ADDR_START       (0xE0)
#define UID_REG_ADDR_END         (0xEB)
#define UID_REG_LENGTH           (UID_REG_ADDR_END - UID_REG_ADDR_START + 1U)
#define FLASH_WRITE_BACK         (0xF0)
#define IAP_UPDATE_ADDR          (0xFD)
#define SW_VER_REG_ADDR          (0xFE)
#define I2C_ADDR_REG_ADDR        (0xFF)
#define HP_DET_POLL_INTERVAL_MS  (5U)
#define HP_DET_STABLE_SAMPLES    (3U)
#define FIRMWARE_VERSION         (0x02)
#define CONTROL_I2C_ADDR_DEFAULT (0x33U)
#define CONTROL_I2C_ADDR_MIN     (0x08U)
#define CONTROL_I2C_ADDR_MAX     (0x77U)
#define ES8388_I2C_ADDR          (0x10U)
#define RGB_REG_LIGHT_BASE       (10)

__IO uint32_t i2c_stop_timeout_delay = 0;

// 寄存器
__IO static uint8_t lin_mic_pc_en_reg  = 0;
__IO static uint8_t hp_mode_reg        = 0;
__IO static uint8_t hp_mic_en_reg      = MICROPHONE_OFF;
__IO static uint8_t hp_det_reg         = 0;
__IO uint8_t rgb_light_reg             = 0;
__IO RGB_Color_TypeDef rgb_reg[3]      = {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}};
__IO static uint8_t sw_ver_reg         = 0;
__IO uint8_t i2c_addr_reg              = 0;
__IO static uint8_t rgb_update_pending = 0;

static uint8_t uid_reg[UID_REG_LENGTH]  = {0};
static uint32_t hp_det_poll_tick        = 0;
static GPIO_PinState hp_det_last_sample = GPIO_PIN_RESET;
static uint8_t hp_det_stable_samples    = 0;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

static uint8_t control_i2c_addr_is_valid(uint8_t addr)
{
    return (addr >= CONTROL_I2C_ADDR_MIN && addr <= CONTROL_I2C_ADDR_MAX && addr != ES8388_I2C_ADDR);
}

static uint8_t microphone_config_is_valid(uint8_t value)
{
    return (value == MICROPHONE_OFF || value == MICROPHONE_ON);
}

static uint8_t headphone_mode_is_valid(uint8_t value)
{
    return (value == AMERICA_STANDARD || value == NATIONAL_STANDARD);
}

static uint8_t rgb_brightness_is_valid(uint8_t value)
{
    return (value <= 100U);
}

static void read_uid(void)
{
    uint32_t uid0 = HAL_GetUIDw0();
    uint32_t uid1 = HAL_GetUIDw1();
    uint32_t uid2 = HAL_GetUIDw2();

    memcpy(&uid_reg[0], &uid0, sizeof(uid0));
    memcpy(&uid_reg[4], &uid1, sizeof(uid1));
    memcpy(&uid_reg[8], &uid2, sizeof(uid2));
}

static void lin_mic_pc_enable(void)
{
    LIN_MIC_PU_EN_GPIO_Port->BSRR = LIN_MIC_PU_EN_Pin;  // 拉高
    lin_mic_pc_en_reg             = MICROPHONE_ON;
}

static void lin_mic_pc_disable(void)
{
    LIN_MIC_PU_EN_GPIO_Port->BSRR = (uint32_t)LIN_MIC_PU_EN_Pin << 16U;  // 拉低
    lin_mic_pc_en_reg             = MICROPHONE_OFF;
}

static void hp_mic_enable(void)
{
    HP_MIC_EN_GPIO_Port->BSRR = HP_MIC_EN_Pin;  // 拉高
    hp_mic_en_reg             = MICROPHONE_ON;
}

static void hp_mic_disable(void)
{
    HP_MIC_EN_GPIO_Port->BSRR = (uint32_t)HP_MIC_EN_Pin << 16U;  // 拉低
    hp_mic_en_reg             = MICROPHONE_OFF;
}

static void set_american_hp_mode(void)
{
    HP_MODE_SET_GPIO_Port->BSRR = (uint32_t)HP_MODE_SET_Pin << 16U;
    hp_mode_reg = AMERICA_STANDARD;
}

static void set_national_hp_mode(void)
{
    HP_MODE_SET_GPIO_Port->BSRR = HP_MODE_SET_Pin;
    hp_mode_reg = NATIONAL_STANDARD;
}

static void hp_det_poll(void)
{
    uint32_t now = HAL_GetTick();
    GPIO_PinState pin_state;

    if ((uint32_t)(now - hp_det_poll_tick) < HP_DET_POLL_INTERVAL_MS) {
        return;
    }
    hp_det_poll_tick = now;
    pin_state        = HAL_GPIO_ReadPin(HP_DET_GPIO_Port, HP_DET_Pin);

    if (pin_state != hp_det_last_sample) {
        hp_det_last_sample    = pin_state;
        hp_det_stable_samples = 1U;
        return;
    }

    if (hp_det_stable_samples < HP_DET_STABLE_SAMPLES) {
        hp_det_stable_samples++;
    }
    if (hp_det_stable_samples >= HP_DET_STABLE_SAMPLES) {
        hp_det_reg = (pin_state == GPIO_PIN_SET) ? HEADPHONE_INSERTION : HEADPHONE_UNPLUG;
    }
}

static void reg_init(void)
{
    uint8_t flag = 0;
    uint8_t saved_lin_mic_pc_en;
    uint8_t saved_hp_mode;
    uint8_t saved_rgb_light;
    uint8_t saved_i2c_addr;
    // 麦克风状态初始化
    saved_lin_mic_pc_en = get_lin_mic_pc_en_status();
    if (!microphone_config_is_valid(saved_lin_mic_pc_en)) {
        lin_mic_pc_en_reg = MICROPHONE_ON;
        flag++;
    } else {
        lin_mic_pc_en_reg = saved_lin_mic_pc_en;
    }
    if (lin_mic_pc_en_reg == MICROPHONE_ON) {
        lin_mic_pc_enable();
    } else {
        lin_mic_pc_disable();
    }
    // 耳机麦克风上电默认关闭，不保存到Flash
    hp_mic_disable();
    // 耳机模式初始化
    saved_hp_mode = get_hp_mode_status();
    if (!headphone_mode_is_valid(saved_hp_mode)) {
        hp_mode_reg = AMERICA_STANDARD;
        flag++;
    } else {
        hp_mode_reg = saved_hp_mode;
    }
    if (hp_mode_reg == NATIONAL_STANDARD) {
        set_national_hp_mode();
    } else {
        set_american_hp_mode();
    }
    // 检测是否有耳机插入
    hp_det_last_sample    = HAL_GPIO_ReadPin(HP_DET_GPIO_Port, HP_DET_Pin);
    hp_det_stable_samples = HP_DET_STABLE_SAMPLES;
    hp_det_reg            = (hp_det_last_sample == GPIO_PIN_SET) ? HEADPHONE_INSERTION : HEADPHONE_UNPLUG;
    hp_det_poll_tick      = HAL_GetTick();
    // rgb_reg亮度初始化
    saved_rgb_light = get_rgb_light_status();
    if (!rgb_brightness_is_valid(saved_rgb_light)) {
        rgb_light_reg = RGB_REG_LIGHT_BASE;
        flag++;
    } else {
        rgb_light_reg = saved_rgb_light;
    }
    if (flag != 0) {
        write_flash(lin_mic_pc_en_reg, hp_mode_reg, rgb_light_reg);
    }
    // 软件版本初始化
    sw_ver_reg = FIRMWARE_VERSION;
    // I2C地址初始化
    saved_i2c_addr = get_i2c_addr();
    if (!control_i2c_addr_is_valid(saved_i2c_addr)) {
        i2c_addr_reg = CONTROL_I2C_ADDR_DEFAULT;
        set_i2c_addr(CONTROL_I2C_ADDR_DEFAULT);
    } else {
        i2c_addr_reg = saved_i2c_addr;
    }
    read_uid();
}

void Slave_Complete_Callback(uint8_t* rx_data, uint16_t len)
{
    uint8_t tx_buf[32];
    if (len > 1) {
        if (rx_data[0] == LIN_MIC_PC_EN_ADDR && len == 2) {
            if (rx_data[1] == MICROPHONE_OFF || rx_data[1] == MICROPHONE_ON) {
                if (lin_mic_pc_en_reg != rx_data[1]) {
                    if (rx_data[1] == MICROPHONE_ON) {
                        lin_mic_pc_enable();
                    } else {
                        lin_mic_pc_disable();
                    }
                }
            }
        } else if (rx_data[0] >= HP_MODE_ADDR && rx_data[0] <= HP_MIC_EN_ADDR &&
                   len <= (uint16_t)(HP_MIC_EN_ADDR - rx_data[0] + 2U)) {
            for (uint16_t i = 1; i < len; i++) {
                uint8_t reg_addr  = rx_data[0] + i - 1U;
                uint8_t reg_value = rx_data[i];

                if (reg_addr == HP_MODE_ADDR) {
                    if ((reg_value == NATIONAL_STANDARD || reg_value == AMERICA_STANDARD) && hp_mode_reg != reg_value) {
                        if (reg_value == NATIONAL_STANDARD) {
                            set_national_hp_mode();
                        } else {
                            set_american_hp_mode();
                        }
                    }
                } else if (reg_addr == HP_MIC_EN_ADDR) {
                    if ((reg_value == MICROPHONE_OFF || reg_value == MICROPHONE_ON) && hp_mic_en_reg != reg_value) {
                        if (reg_value == MICROPHONE_ON) {
                            hp_mic_enable();
                        } else {
                            hp_mic_disable();
                        }
                    }
                }
            }
        } else if (rx_data[0] == RGB_LIGHT_ADDR && len == 2) {
            if (rx_data[1] >= 0 && rx_data[1] <= 100) {
                if (rgb_light_reg != rx_data[1]) {
                    rgb_light_reg      = rx_data[1];
                    rgb_update_pending = 1U;
                }
            }
        } else if (rx_data[0] >= RGB1_STATUS_ADDR && rx_data[0] <= (RGB3_STATUS_ADDR + 2) &&
                   len <= (uint16_t)((RGB3_STATUS_ADDR + 2) - rx_data[0] + 2U)) {
            uint8_t rgb_offset = rx_data[0] - RGB1_STATUS_ADDR;
            for (uint16_t i = 1; i < len; i++) {
                ((uint8_t*)rgb_reg)[rgb_offset + i - 1U] = rx_data[i];
            }
            rgb_update_pending = 1U;
        } else if (rx_data[0] == I2C_ADDR_REG_ADDR && len == 2) {
            if (control_i2c_addr_is_valid(rx_data[1])) {
                if (i2c_addr_reg != rx_data[1]) {
                    if (set_i2c_addr(rx_data[1])) {
                        i2c_addr_reg = rx_data[1];
                        user_i2c_init();
                    }
                }
            }
        } else if (rx_data[0] == IAP_UPDATE_ADDR && len == 2) {
            if (rx_data[1] > 0) {
                NVIC_SystemReset();
            }
        } else if (rx_data[0] == FLASH_WRITE_BACK && len == 2) {
            if (rx_data[1] == 1) {
                if (lin_mic_pc_en_reg != get_lin_mic_pc_en_status() || hp_mode_reg != get_hp_mode_status() ||
                    rgb_light_reg != get_rgb_light_status()) {
                    write_flash(lin_mic_pc_en_reg, hp_mode_reg, rgb_light_reg);
                }
            }
        }
    }
    if (len == 1) {
        if (rx_data[0] == LIN_MIC_PC_EN_ADDR) {
            tx_buf[0] = lin_mic_pc_en_reg;
            i2c2_set_send_data(tx_buf, 1);
        } else if (rx_data[0] >= HP_MODE_ADDR && rx_data[0] <= HP_MIC_EN_ADDR) {
            uint8_t hp_offset = rx_data[0] - HP_MODE_ADDR;
            tx_buf[0]         = hp_mode_reg;
            tx_buf[1]         = hp_mic_en_reg;
            i2c2_set_send_data(tx_buf + hp_offset, 2U - hp_offset);
        } else if (rx_data[0] == HP_DET_ADDR) {
            tx_buf[0] = hp_det_reg;
            i2c2_set_send_data(tx_buf, 1);
        } else if (rx_data[0] == RGB_LIGHT_ADDR) {
            tx_buf[0] = rgb_light_reg;
            i2c2_set_send_data(tx_buf, 1);
        } else if (rx_data[0] >= RGB1_STATUS_ADDR && rx_data[0] <= (RGB3_STATUS_ADDR + 2)) {
            uint8_t rgb_offset = rx_data[0] - RGB1_STATUS_ADDR;
            i2c2_set_send_data(((uint8_t*)rgb_reg) + rgb_offset, 9U - rgb_offset);
        } else if (rx_data[0] >= UID_REG_ADDR_START && rx_data[0] <= UID_REG_ADDR_END) {
            uint8_t uid_offset = rx_data[0] - UID_REG_ADDR_START;
            i2c2_set_send_data(uid_reg + uid_offset, UID_REG_LENGTH - uid_offset);
        } else if (rx_data[0] == SW_VER_REG_ADDR) {
            tx_buf[0] = sw_ver_reg;
            i2c2_set_send_data(tx_buf, 1);
        } else if (rx_data[0] == I2C_ADDR_REG_ADDR) {
            tx_buf[0] = i2c_addr_reg;
            i2c2_set_send_data(tx_buf, 1);
        }
    }
}

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{
    /* USER CODE BEGIN 1 */

    /* USER CODE END 1 */

    /* MCU Configuration--------------------------------------------------------*/

    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();

    /* USER CODE BEGIN Init */

    /* USER CODE END Init */

    /* Configure the system clock */
    SystemClock_Config();

    /* USER CODE BEGIN SysInit */
    /* USER CODE END SysInit */

    /* Initialize all configured peripherals */
    MX_GPIO_Init();
    MX_DMA_Init();
    //	MX_I2C2_Init();
    MX_IWDG_Init();
    MX_TIM17_Init();
    /* USER CODE BEGIN 2 */
    reg_init();
    user_i2c_init();
    i2c2_it_enable();
    rgb_init();
    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1) {
        i2c_timeout_counter = 0;
        if (i2c_stop_timeout_flag) {
            uint32_t current_tick = HAL_GetTick();
            if ((uint32_t)(current_tick - i2c_stop_timeout_delay) >= 10U) {
                i2c_stop_timeout_counter++;
                i2c_stop_timeout_delay = current_tick;
            }
        }
        if (i2c_stop_timeout_counter > 50) {
            i2c_stop_timeout_flag    = 0;
            i2c_stop_timeout_counter = 0;
            LL_I2C_DeInit(I2C2);
            LL_I2C_DisableAutoEndMode(I2C2);
            LL_I2C_Disable(I2C2);
            LL_I2C_DisableIT_ADDR(I2C2);
            user_i2c_init();
            i2c2_it_enable();
            HAL_Delay(10);
        }
        if (rgb_update_pending != 0U && rgb_dma_busy == 0U) {
            rgb_update_pending = 0U;
            rgb_update();
        }
        hp_det_poll();

        /* USER CODE END WHILE */

        /* USER CODE BEGIN 3 */
        LL_IWDG_ReloadCounter(IWDG);  // 复位看门狗
    }
    /* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /** Configure the main internal regulator output voltage
     */
    HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

    /** Initializes the RCC Oscillators according to the specified parameters
     * in the RCC_OscInitTypeDef structure.
     */
    RCC_OscInitStruct.OscillatorType      = RCC_OSCILLATORTYPE_HSI | RCC_OSCILLATORTYPE_LSI;
    RCC_OscInitStruct.HSIState            = RCC_HSI_ON;
    RCC_OscInitStruct.HSIDiv              = RCC_HSI_DIV1;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.LSIState            = RCC_LSI_ON;
    RCC_OscInitStruct.PLL.PLLState        = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource       = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL.PLLM            = RCC_PLLM_DIV1;
    RCC_OscInitStruct.PLL.PLLN            = 8;
    RCC_OscInitStruct.PLL.PLLP            = RCC_PLLP_DIV4;
    RCC_OscInitStruct.PLL.PLLR            = RCC_PLLR_DIV2;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }

    /** Initializes the CPU, AHB and APB buses clocks
     */
    RCC_ClkInitStruct.ClockType      = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1;
    RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
        Error_Handler();
    }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void)
{
    /* USER CODE BEGIN Error_Handler_Debug */
    /* User can add his own implementation to report the HAL error return state */
    __disable_irq();
    while (1) {
    }
    /* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t* file, uint32_t line)
{
    /* USER CODE BEGIN 6 */
    /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
    /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
