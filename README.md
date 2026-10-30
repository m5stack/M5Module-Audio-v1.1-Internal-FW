# M5Module-Audio v1.1 Internal Firmware

**SKU: M144-V11**

## Overview

This repository contains the STM32 firmware, release images, and I²C control protocol for the M5Stack Module-Audio v1.1. The module uses an STM32G030F6P6 to manage microphone input routing, headset detection, CTIA/OMTP headset mode selection, and RGB LED control.

The current application firmware identifies itself as `0x02` through the firmware-version register. The default control I²C address is `0x33`; the IAP bootloader uses `0x54` during firmware upgrades.

## Repository Contents

- `code/Module_Audio_APP`: application firmware for STM32G030F6P6
- `code/Module_Audio_IAP`: I²C in-application programming (IAP) bootloader for STM32G030F6P6
- `firmware/2026-10-30-0x02`: release firmware package, including the application HEX image, merged BIN image, and generated firmware headers
- `protocol/M5Module-Audio-V1.1-I2C-Protocol-EN_V2.0.md`: I²C control protocol documentation
- `protocol/M5Module-Audio-V1.1-I2C-Protocol-CN_V2.0.md`: Chinese I²C control protocol documentation

## Related Links

- [M144 Module-Audio-V11 Document & Datasheet](https://docs.m5stack.com/en/products/sku/M144-V11)
- [I²C Control Protocol](protocol/M5Module-Audio-V1.1-I2C-Protocol-EN_V2.0.md)
- [I²C 控制协议（中文）](protocol/M5Module-Audio-V1.1-I2C-Protocol-CN_V2.0.md)
- [Firmware v0x02 (Intel HEX)](firmware/2026-10-30-0x02/M144-V11-Module_Audio_v1.1-STM32G030F6P6-TAO-0x02-20261030_0x8000000.hex)
- [Firmware v0x02 (merged BIN)](firmware/2026-10-30-0x02/bootloader_app_merge.bin)

## License

- [MIT](LICENSE)
