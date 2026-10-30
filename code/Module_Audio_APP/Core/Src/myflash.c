#include "myflash.h"

#define FLASH_WRITE_RETRY_COUNT (3U)

/**
 * @brief  根据地址获得内存的页
 * @param  addr 内存地址
 * @retval 返回页数
 */
static uint32_t GetPage(uint32_t addr) {
	return (addr - STM32G0xx_FLASH_PAGE0_STARTADDR) / FLASH_PAGE_SIZE;
}

/**
 * @brief  修改8byte的某一字节
 * @param  data 要修改的数据
 * @param  byte_index 数据下标
 * @param  new_value 修改值
 * @retval 修改之后的数
 */
static void set_byte_in_uint64(uint64_t *data, uint8_t byte_index, uint8_t new_value) {
	*data &= ~((uint64_t) (0xFF) << (byte_index * 8));
	// 然后设置新的字节值
	*data |= (uint64_t) new_value << (byte_index * 8);
}

/**
 * @brief  读取一字节
 * @param  address 内存地址
 * @retval 返回值
 */
uint8_t my_flash_read_byte(uint32_t address) {
	return *((__IO uint8_t*) (address));
}

/**
 * @brief  读取半字
 * @param  address 内存地址
 * @retval 返回值
 */
uint16_t my_flash_read_half_word(uint32_t address) {
	return *((__IO uint16_t*) (address));
}

/**
 * @brief  读取字
 * @param  address 内存地址
 * @retval 返回值
 */
uint32_t my_flash_read_word(uint32_t address) {
	return *((__IO uint32_t*) (address));
}

/**
 * @brief  读取双字
 * @param  address 内存地址
 * @retval 返回值
 */
uint64_t my_flash_read_double_word(uint32_t address) {
	return *((__IO uint64_t*) (address));
}

/**
 * @brief  页擦除
 * @param  address 内存地址
 * @retval 是否成功
 */
bool my_flash_earse_pages(uint32_t page_address) {
	uint32_t page_error = 0;           // 设置Page_error,如果出现错误这个变量会被设置为出错的FLASH地址
	FLASH_EraseInitTypeDef my_flash;            // 页擦除所在的位置
	my_flash.TypeErase = FLASH_TYPEERASE_PAGES; // 标明Flash执行页面只做擦除操作
	my_flash.Page = GetPage(page_address);
	my_flash.NbPages = 1; // 说明要擦除的页数
	HAL_FLASH_Unlock();
	HAL_StatusTypeDef status = HAL_FLASHEx_Erase(&my_flash, &page_error);
	HAL_FLASH_Lock();
	if (status == HAL_OK) {
		return true;
	}
	return false;
}

/**
 * @brief  写半字
 * @param  address 内存地址
 * @param  data 要写入的数据
 * @retval 是否成功
 */
bool my_flash_write_half_word(uint32_t address, uint16_t data) {
	HAL_FLASH_Unlock();
	HAL_StatusTypeDef status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD,
			address, data);
	HAL_FLASH_Lock();
	if (status == HAL_OK) {
		return true;
	}
	return false;
}

/**
 * @brief  写双字
 * @param  address 内存地址
 * @param  data 要写入的数据
 * @retval 是否成功
 */
static bool my_flash_write_double_word(uint32_t address, uint64_t data) {
	HAL_FLASH_Unlock();
	HAL_StatusTypeDef status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD,
			address, data);
	HAL_FLASH_Lock();
	if (status == HAL_OK) {
		return true;
	}
	return false;
}

static bool write_flash_page(uint64_t data) {
	for (uint32_t attempt = 0; attempt < FLASH_WRITE_RETRY_COUNT; attempt++) {
		if (!my_flash_earse_pages(STM32G0xx_FLASH_PAGE15_STARTADDR)) {
			continue;
		}
		if (my_flash_write_double_word(STM32G0xx_FLASH_PAGE15_STARTADDR, data)) {
			return true;
		}
	}
	return false;
}

bool set_lin_mic_pc_en_status(uint8_t data) {
	uint64_t temp = my_flash_read_double_word(STM32G0xx_FLASH_PAGE15_STARTADDR);
	set_byte_in_uint64(&temp, 0, data);
	if (!write_flash_page(temp)) {
		return false;
	}
	uint8_t dat = get_lin_mic_pc_en_status();
	if (dat == data) {
		return true;
	}
	return false;
}

bool set_hp_mode_status(uint8_t data) {
	uint64_t temp = my_flash_read_double_word(STM32G0xx_FLASH_PAGE15_STARTADDR);
	set_byte_in_uint64(&temp, 1, data);
	if (!write_flash_page(temp)) {
		return false;
	}
	uint8_t dat = get_hp_mode_status();
	if (dat == data) {
		return true;
	}
	return false;
}

bool set_rgb_light_status(uint8_t data) {
	uint64_t temp = my_flash_read_double_word(STM32G0xx_FLASH_PAGE15_STARTADDR);
	set_byte_in_uint64(&temp, 2, data);
	if (!write_flash_page(temp)) {
		return false;
	}
	uint8_t dat = get_rgb_light_status();
	if (dat == data) {
		return true;
	}
	return false;
}

bool set_i2c_addr(uint8_t data) {
	uint64_t temp = my_flash_read_double_word(STM32G0xx_FLASH_PAGE15_STARTADDR);
	set_byte_in_uint64(&temp, 3, data);
	if (!write_flash_page(temp)) {
		return false;
	}
	uint8_t dat = get_i2c_addr();
	if (dat == data) {
		return true;
	}
	return false;
}

uint8_t get_lin_mic_pc_en_status(void) {
	return *((__IO uint8_t*) (LIN_MIC_PC_EN_STATUS_ADDR));
}

uint8_t get_hp_mode_status(void) {
	return *((__IO uint8_t*) (HP_MODE_STATUS_ADDR));
}

uint8_t get_rgb_light_status(void) {
	return *((__IO uint8_t*) (RGB_LIGHT_STATUS_ADDR));
}

uint8_t get_i2c_addr(void) {
	return *((__IO uint8_t*) (I2C_ADDR));
}

bool write_flash(uint8_t pc_en,uint8_t hp_mode,uint8_t rgb_light){
	__disable_irq();
	uint64_t temp = my_flash_read_double_word(STM32G0xx_FLASH_PAGE15_STARTADDR);
	set_byte_in_uint64(&temp, 0, pc_en);
	set_byte_in_uint64(&temp, 1, hp_mode);
	set_byte_in_uint64(&temp, 2, rgb_light);
	bool write_ok = write_flash_page(temp);
	__enable_irq();
	if (!write_ok) {
		return false;
	}
	if (pc_en == get_lin_mic_pc_en_status() && hp_mode == get_hp_mode_status() && rgb_light == get_rgb_light_status()) {
		return true;
	}
	return false;
}

