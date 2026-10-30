# M5Stack Module-Audio I²C Control Protocol (Firmware Identifier `0x02`)

## 1. Basic Information

| Item | Description |
| :--- | :--- |
| Bus | I²C, slave mode |
| Device | M5Stack Module-Audio |
| Default I²C address (7-bit) | `0x33` |
| Configurable I²C address range (7-bit) | `0x08 ~ 0x77`, excluding `0x10` used by ES8388 |
| Communication speed | `400 kbit/s` (I²C Fast-mode) |
| Register address width | `8 bit` |
| Register data width | `8 bit` |
| Auto-increment | `0x10 ~ 0x11` and `0x40 ~ 0x48` support continuous read/write; `0xE0 ~ 0xEB` supports continuous read |
| Clock stretching | Supported |
| General Call | Not supported |
| Firmware version | Current register `0xFE` returns `0x02`; this may change in later revisions |

### 1.1 Access Permissions

| Permission | Meaning |
| :--- | :--- |
| `R` | Read-only |
| `W` | Write-only |
| `R/W` | Read/write |
| `Reserved` | Reserved; should not be accessed |

### 1.2 Read and Write Transactions

#### Register Write

The host sends the register address followed by the data in a single write transaction:

```text
START → SlaveAddr + W → RegisterAddr → Data[0] ... Data[N] → STOP
```

- Except for the continuous address blocks listed below, ordinary registers accept only `1 byte` of data.
- `0x10 ~ 0x11` support continuous write: a maximum of `2 bytes` may be written starting at `0x10`; only `1 byte` may be written starting at `0x11`.
- `0x40 ~ 0x48` support continuous write from any address in the range. The write length must not exceed the remaining space up to `0x48`.
- Unsupported values are ignored. No separate error code is returned.

#### Register Read

A combined transaction is recommended: write the target register address first, then read the data using a repeated START condition.

```text
START → SlaveAddr + W → RegisterAddr
RESTART → SlaveAddr + R → Data[0] ... Data[N] → STOP
```

- Except for the continuous address blocks listed below, ordinary registers are read as `1 byte`.
- Reading from `0x10` may return `0x10` and `0x11` in address order. Reading from `0x11` returns only `0x11`.
- Reading from `0x40 ~ 0x48` may continue up to `0x48`. The read length must not exceed the remaining data length.
- Reading from `0xE0 ~ 0xEB` may continue up to `0xEB`. The read length must not exceed the remaining UID length.
- The protocol does not provide a unified status code. To confirm a write, read back the corresponding `R/W` register.

---

## 2. Register and Command Overview

| Address / Range | Name | Length | Permission | Default | Power-off retention | Description | Details |
| :--- | :--- | :--: | :--: | :--- | :--- | :--- | :--- |
| `0x00` | Microphone Config | `1` | `R/W` | `0x01` | Yes, after writing `0xF0` | LINE/MIC channel enable | `4.1` |
| `0x10` | Headphone Mode | `1` | `R/W` | `0x00` | Yes, after writing `0xF0` | Headset American/National standard selection | `4.2` |
| `0x11` | Headphone Microphone Enable | `1` | `R/W` | `0x00` | No | Headset microphone channel enable | `4.3` |
| `0x20` | Headphone Insert Status | `1` | `R` | Runtime value | No | Headphone insertion status | `4.4` |
| `0x30` | RGB Brightness | `1` | `R/W` | `0x0A` | Yes, after writing `0xF0` | Global RGB brightness, range `0 ~ 100` | `4.5` |
| `0x40 ~ 0x48` | RGB LED | `9` | `R/W` | All `0x00` | No | Color values for three RGB LEDs | `4.6` |
| `0xE0 ~ 0xEB` | Device UID | `12` | `R` | MCU-specific | MCU-specific | 96-bit device unique identifier | `4.11` |
| `0xF0` | Save Configuration to Flash | `1` | `W` | N/A | N/A | Save persistent configuration | `4.7` |
| `0xFD` | IAP Upgrade | `1` | `W` | N/A | No | Trigger system reset; see `4.8` for IAP entry requirements | `4.8` |
| `0xFE` | Firmware Version | `1` | `R` | `0x02` | Firmware-specific | Read the firmware version identifier | `4.9` |
| `0xFF` | I²C Address | `1` | `R/W` | `0x33` | Yes, automatically saved | Device 7-bit I²C address | `4.10` |

---

## 3. Address Definitions

### 3.1 Fixed System Addresses

| Address | Name | Description |
| :--- | :--- | :--- |
| `0xF0` | Save Configuration to Flash | Write `0x01` to save the current persistent configuration |
| `0xFD` | IAP Upgrade | Write a non-zero value to trigger a system reset; see `4.8` for IAP entry requirements |
| `0xFE` | Firmware Version | Read the firmware version |
| `0xFF` | I²C Address | Read or change the device 7-bit I²C address |

### 3.2 Reserved Addresses

| Address / Range | Type | Access Rule |
| :--- | :--- | :--- |
| `0x01 ~ 0x0F` | `Reserved` | Must not be accessed |
| `0x12 ~ 0x1F` | `Reserved` | Must not be accessed |
| `0x21 ~ 0x2F` | `Reserved` | Must not be accessed |
| `0x31 ~ 0x3F` | `Reserved` | Must not be accessed |
| `0x49 ~ 0xDF` | `Reserved` | Must not be accessed |
| `0xEC ~ 0xEF` | `Reserved` | Must not be accessed |
| `0xF1 ~ 0xFC` | `Reserved` | Must not be accessed |

Undefined registers do not return valid protocol data. Future firmware may use reserved addresses; host software must not depend on the current behavior of undefined addresses.

---

## 4. Register Details

### 4.1 Microphone Config

| Address | Length | Permission | Description |
| :--- | :--: | :--: | :--- |
| `0x00` | `1 byte` | `R/W` | Control the LINE/MIC channel |

| Value | Name | Default | Description |
| :--- | :--- | :--- | :--- |
| `0x00` | `OFF` | No | Disable the LINE/MIC channel |
| `0x01` | `ON` | Yes | Enable the LINE/MIC channel |

The new value takes effect immediately. It is saved to Flash only after `0x01` is written to `0xF0`. This register does not automatically modify `0x11`; to prevent both microphone paths from being connected to LINE IN at the same time, the host should first write `0x00` to `0x11` before enabling LINE/MIC.

### 4.2 Headphone Mode

| Address | Length | Permission | Description |
| :--- | :--: | :--: | :--- |
| `0x10` | `1 byte` | `R/W` | Select the headset interface mode |

| Value | Name | Default | PA2 `HP_MODE_SET` | Description |
| :--- | :--- | :--- | :--- | :--- |
| `0x00` | `AMERICA_STANDARD` | Yes | Low | American standard (CTIA) mode |
| `0x01` | `NATIONAL_STANDARD` | No | High | National standard (OMTP) mode |

The new value takes effect immediately. It is saved to Flash only after `0x01` is written to `0xF0`.

### 4.3 Headphone Microphone Enable

| Address | Length | Permission | Description |
| :--- | :--: | :--: | :--- |
| `0x11` | `1 byte` | `R/W` | Control the headset microphone channel |

| Value | Name | Default | Description |
| :--- | :--- | :--- | :--- |
| `0x00` | `OFF` | Yes | Disable the headset microphone channel |
| `0x01` | `ON` | No | Set PA0 high and enable the internal solid-state relay, connecting the headset microphone signal to LINE IN |

Notes:

1. This register is not saved to Flash and returns to `0x00` after power-up.
2. When `0x01` is written, the firmware does not check whether a headset is inserted. The host should read `0x20` and confirm headset insertion before enabling it.
3. This register does not automatically disable the `0x00` LINE/MIC channel. The host should write `0x00` to `0x00` before enabling the headset microphone.
4. When the headset microphone is enabled, no external microphone should be connected to the LINE/MIC interface.
5. `0x10` and `0x11` are consecutive addresses and can be read or written together in one continuous transaction starting at `0x10`.

Recommended sequence for switching to the headset microphone:

```text
1. Read 0x20 and confirm that it returns 0x01.
2. Write 0x00 to 0x00 to disable LINE/MIC.
3. Write 0x01 to 0x11 to enable the headset microphone.
```

Recommended sequence for switching back to LINE/MIC:

```text
1. Write 0x00 to 0x11 to disable the headset microphone.
2. Write 0x01 to 0x00 to enable LINE/MIC.
```

### 4.4 Headphone Insert Status

| Address | Length | Permission | Description |
| :--- | :--: | :--: | :--- |
| `0x20` | `1 byte` | `R` | Read the headset insertion detection status |

| Value | Name | Description |
| :--- | :--- | :--- |
| `0x00` | `HEADPHONE_UNPLUG` | No headset inserted |
| `0x01` | `HEADPHONE_INSERTION` | Headset inserted |

A high level on the detection pin indicates that a headset is inserted; a low level indicates that it is unplugged. The firmware samples the pin every `5 ms` and updates the status after `3` consecutive stable samples. Therefore, the usual delay between the physical insertion/removal and the update of `0x20` is approximately `10 ~ 15 ms`.

### 4.5 RGB Brightness

| Address | Length | Permission | Description |
| :--- | :--: | :--: | :--- |
| `0x30` | `1 byte` | `R/W` | Set the global brightness of the three RGB LEDs |

| Range | Default | Unit | Description |
| :--- | :--- | :--- | :--- |
| `0 ~ 100` | `10` (`0x0A`) | Percentage level | `0` disables output and `100` is the maximum brightness |

Writing a new value sets the RGB update flag. The main loop then updates the LED output. The new value is saved to Flash only after `0x01` is written to `0xF0`. Values outside the valid range are ignored.

### 4.6 RGB LED

| Address Range | Length | Permission | Description |
| :--- | :--: | :--: | :--- |
| `0x40 ~ 0x48` | `9 bytes` | `R/W` | Set or read the color components of the three RGB LEDs |

| Address | Field | Range | Default |
| :--- | :--- | :--- | :--- |
| `0x40` | `LED1_R` | `0 ~ 255` | `0x00` |
| `0x41` | `LED1_G` | `0 ~ 255` | `0x00` |
| `0x42` | `LED1_B` | `0 ~ 255` | `0x00` |
| `0x43` | `LED2_R` | `0 ~ 255` | `0x00` |
| `0x44` | `LED2_G` | `0 ~ 255` | `0x00` |
| `0x45` | `LED2_B` | `0 ~ 255` | `0x00` |
| `0x46` | `LED3_R` | `0 ~ 255` | `0x00` |
| `0x47` | `LED3_G` | `0 ~ 255` | `0x00` |
| `0x48` | `LED3_B` | `0 ~ 255` | `0x00` |

Example: continuous write for all three LEDs:

```text
RegisterAddr: 40
Data:         FF 00 00 00 FF 00 00 00 FF
Result:       LED1=red, LED2=green, LED3=blue
```

Example: continuous write from a middle address:

```text
RegisterAddr: 44
Data:         80 40 FF
Result:       LED2_G=0x80, LED2_B=0x40, LED3_R=0xFF
```

Notes:

1. Continuous read/write may start at any address from `0x40` to `0x48`, but must not cross `0x48`.
2. Writing the registers sets the RGB update flag. The main loop updates the LED output.
3. RGB color values are not saved to Flash and return to `0x00` after power-up.
4. The actual output brightness is also affected by the global brightness register at `0x30`.

### 4.7 Save Configuration to Flash

| Address | Length | Permission | Trigger Value | Description |
| :--- | :--: | :--: | :--- | :--- |
| `0xF0` | `1 byte` | `W` | `0x01` | Save the current configuration to internal Flash |

Saved registers:

- `0x00` Microphone Config
- `0x10` Headphone Mode
- `0x30` RGB Brightness

Notes:

1. The Flash page must be erased before writing. The operation takes approximately `20 ms`; the device may be temporarily unavailable during this time.
2. Avoid frequent writes to extend Flash life.
3. If the current values are identical to the values already stored in Flash, the erase/write operation is skipped.
4. `0x11` and `0x40 ~ 0x48` are not included in the saved configuration.

### 4.8 IAP Upgrade

| Address | Length | Permission | Trigger Value | Description |
| :--- | :--: | :--: | :--- | :--- |
| `0xFD` | `1 byte` | `W` | `0x01 ~ 0xFF` | Trigger an MCU system reset |

Writing `0x00` performs no operation. After a non-zero value is written, the current I²C session terminates immediately.

IAP entry sequence:

1. Write a non-zero value to `0xFD` at the application address to trigger a reset.
2. Keep SCL and SDA low during the reset period.
3. Approximately `30 ms` after bootloader startup, the bootloader checks both lines. If both are low, it enters an initial IAP waiting window of approximately `500 ms`; otherwise, it jumps back to the application.
4. After entering IAP, use the fixed 7-bit I²C address `0x54`. IAP firmware transfer commands are outside this control-register protocol and must follow the separate IAP protocol.

### 4.9 Firmware Version

| Address | Length | Permission | Current Value | Description |
| :--- | :--: | :--: | :--- | :--- |
| `0xFE` | `1 byte` | `R` | `0x02` | Read the current firmware version identifier |

`0xFE` distinguishes firmware revisions. The host may enable version-specific features based on this value, but should not reject basic registers that are compatible with older versions solely because the value is different from the older version `0x01`.

### 4.10 I²C Address

| Address | Length | Permission | Valid Range | Default | Description |
| :--- | :--: | :--: | :--- | :--- | :--- |
| `0xFF` | `1 byte` | `R/W` | `0x08 ~ 0x77`, excluding `0x10` | `0x33` | Read or change the device 7-bit I²C address |

Notes:

1. After a valid new address is written, the firmware saves it to internal Flash automatically; writing `0xF0` is not required.
2. The new address takes effect immediately. After the write transaction completes, the host must use the new address for subsequent accesses.
3. Values outside `0x08 ~ 0x77` and the value `0x10` are ignored. `0x10` is used by ES8388 on the same bus.
4. The device may be temporarily unavailable while the Flash is written and the I²C peripheral is reinitialized.
5. At startup, if the address stored in Flash is invalid or equals `0x10`, the firmware automatically restores the default address `0x33`.

### 4.11 Device UID

| Address Range | Length | Permission | Description |
| :--- | :---: | :---: | :--- |
| `0xE0 ~ 0xEB` | `12 bytes` | `R` | Read the STM32 MCU 96-bit device unique identifier |

The UID is read using `HAL_GetUIDw0()`, `HAL_GetUIDw1()`, and `HAL_GetUIDw2()`. The three 32-bit words map to `0xE0 ~ 0xE3`, `0xE4 ~ 0xE7`, and `0xE8 ~ 0xEB`, respectively. Each word is returned in little-endian byte order.

Continuous read may start at any address from `0xE0` to `0xEB`, but must not cross `0xEB`. This register block is read-only; writes do not modify the UID.

---

## 5. Invalid Access and Error Handling

| Condition | Device Behavior | Host Recommendation |
| :--- | :--- | :--- |
| Write to an undefined register | No valid operation is performed | Do not access reserved addresses |
| Read from an undefined register | No valid return data is guaranteed | Access only the `R` or `R/W` addresses listed in the overview |
| Write to a read-only register | Ignored | Do not write to `0x20`, `0xE0 ~ 0xEB`, or `0xFE` |
| Read from a write-only register | No valid return data is guaranteed | Do not read `0xF0` or `0xFD` |
| Write an out-of-range value | The setting is ignored | Read back the corresponding register to confirm the result |
| Continuous RGB write crosses `0x48` | The entire RGB write is ignored | Limit the length to the remaining RGB data |
| Continuous RGB read crosses `0x48` | Values beyond the valid range are not guaranteed | Limit the read length to the remaining RGB data |

The protocol does not provide an ACK status byte or a unified error code. An I²C bus-level ACK only indicates that the slave received the byte; it does not indicate that the register value was accepted.

---

## 6. Revision History

| Document Version | Firmware Version | Date | Update Description |
| :--- | :--- | :--- | :--- |
| `V1.0` | `V1` | `2026-10-30` | Release version |
