# STM32F407ZGT6 TF Reader with FAT32 / STM32F407ZGT6 FAT32 TF 读卡器

[中文](#中文) | [English](#english)

## 中文

这个工程面向 STM32F407ZGT6 + 板载 TF 卡座，默认使用 SDIO 4-bit 读取 TF 卡，并用 FatFs 挂载 FAT32。启动后会先通过 USART1 打印卡容量、文件系统类型、根目录，并写入/读回 `0:/STM32F4_TF_TEST.TXT` 验证 FAT32；随后启动 USB Mass Storage，让电脑把这块板识别成 TF 读卡器。

### 默认硬件

- MCU: STM32F407ZGT6, 168 MHz
- HSE: 默认 8 MHz，若你的板子是 25 MHz，改 `include/stm32f4xx_hal_conf.h` 里的 `HSE_VALUE` 为 `25000000U`
- UART 日志: USART1, PA9 TX, PA10 RX, 115200 8N1
- SDIO TF: PC8 D0, PC9 D1, PC10 D2, PC11 D3, PC12 CLK, PD2 CMD
- USB FS Device: PA11 USB_DM, PA12 USB_DP
- 下载器: PWLINK2lite，按 CMSIS-DAP 使用
- 卡检测: 默认关闭。若有 CD 引脚，打开 `APP_SD_DETECT_ENABLE` 并修改 `include/app_config.h`

### FAT32 要求

工程会严格检查 `sd_fs.fs_type == FS_FAT32`。如果卡是 FAT16、exFAT 或没有文件系统，串口会提示重新格式化。USB MSC 仍会启动，方便你在电脑上检查或格式化 TF 卡。建议在电脑上将 TF 卡格式化为 FAT32，扇区大小保持 512 字节。

USB MSC 运行时，MCU 不再挂载 FatFs；电脑是唯一文件系统写入方，这样能避免 FAT 表被两边同时修改。

### 编译和烧录

```powershell
python -m platformio run
python -m platformio run -t upload
python -m platformio device monitor -b 115200
```

工程的 `platformio.ini` 已配置 `upload_protocol = cmsis-dap`，可直接配合 PWLINK2lite 烧录。如果 `python -m platformio` 不可用，但 VS Code 安装了 PlatformIO，也可以直接用 PlatformIO 的 Build/Upload 按钮。

### 切换到 SPI TF 模块

如果你的“板载 TF 模块”只接了 SPI，把 `include/app_config.h` 中的：

```c
#define APP_SD_USE_SDIO 1
```

改为：

```c
#define APP_SD_USE_SDIO 0
```

默认 SPI 引脚是 SPI1: PA5 SCK, PA6 MISO, PA7 MOSI, PA4 CS。需要换引脚时只改 `APP_SD_SPI_*` 这些宏。

### 常用开关

- `APP_ENABLE_USB_MSC`: 默认 `1`，自检后启动 USB TF 读卡器；改为 `0` 时只做串口 FAT32 自检。
- `APP_USB_MSC_READONLY`: 默认 `0`，电脑可读写 TF 卡；改为 `1` 时 USB 只读。
- `APP_REQUIRE_FAT32`: 默认 `1`，上电自检必须识别为 FAT32 才算通过。
- `APP_WRITE_TEST_FILE`: 默认 `1`，上电自检会写入/读回测试文件。

### 外挂 1 MB SRAM

FAT32 挂载和基础读写不依赖外部 SRAM。本工程先保证 TF/FAT32 主链路稳定；后续要做 USB Mass Storage 高速缓存、文件索引或大块读写缓冲时，再把 FSMC SRAM 加入内存池更合适。

### 开源许可证

本项目以 MIT License 开源发布，许可证全文见 [LICENSE](LICENSE)。SPDX 标识: `MIT`。

## English

This project targets an STM32F407ZGT6 board with an onboard TF card socket. By default it reads the card through 4-bit SDIO and mounts FAT32 with FatFs. On boot, it prints the card capacity, filesystem type, and root directory over USART1, then writes and reads back `0:/STM32F4_TF_TEST.TXT` to verify FAT32. After the self-test, it starts USB Mass Storage so the PC can use the board as a TF card reader.

### Default Hardware

- MCU: STM32F407ZGT6, 168 MHz
- HSE: 8 MHz by default. If your board uses a 25 MHz crystal, change `HSE_VALUE` in `include/stm32f4xx_hal_conf.h` to `25000000U`
- UART log: USART1, PA9 TX, PA10 RX, 115200 8N1
- SDIO TF card: PC8 D0, PC9 D1, PC10 D2, PC11 D3, PC12 CLK, PD2 CMD
- USB FS Device: PA11 USB_DM, PA12 USB_DP
- Programmer: PWLINK2lite, used as CMSIS-DAP
- Card detect: disabled by default. If your board has a CD pin, enable `APP_SD_DETECT_ENABLE` and update `include/app_config.h`

### FAT32 Requirement

The firmware strictly checks `sd_fs.fs_type == FS_FAT32`. If the card is FAT16, exFAT, or unformatted, the UART log will ask you to reformat it. USB MSC still starts so you can inspect or format the TF card from the PC. FAT32 with 512-byte sectors is recommended.

While USB MSC is running, the MCU no longer mounts FatFs. The PC becomes the only filesystem writer, which avoids simultaneous FAT table updates from both sides.

### Build and Upload

```powershell
python -m platformio run
python -m platformio run -t upload
python -m platformio device monitor -b 115200
```

`platformio.ini` is already configured with `upload_protocol = cmsis-dap`, so it works directly with PWLINK2lite. If `python -m platformio` is not available but PlatformIO is installed in VS Code, you can also use PlatformIO's Build/Upload buttons.

### Switch to an SPI TF Module

If your onboard TF module is wired only through SPI, change this line in `include/app_config.h`:

```c
#define APP_SD_USE_SDIO 1
```

to:

```c
#define APP_SD_USE_SDIO 0
```

The default SPI pins are SPI1: PA5 SCK, PA6 MISO, PA7 MOSI, PA4 CS. To use different pins, update only the `APP_SD_SPI_*` macros.

### Common Options

- `APP_ENABLE_USB_MSC`: default `1`; starts the USB TF reader after the self-test. Set to `0` to run only the UART FAT32 self-test.
- `APP_USB_MSC_READONLY`: default `0`; the PC can read and write the TF card. Set to `1` for USB read-only mode.
- `APP_REQUIRE_FAT32`: default `1`; the boot self-test must detect FAT32 to pass.
- `APP_WRITE_TEST_FILE`: default `1`; the boot self-test writes and reads back a test file.

### External 1 MB SRAM

FAT32 mounting and basic read/write do not depend on the external SRAM. This project first keeps the TF/FAT32 path stable. The FSMC SRAM can be added later for USB Mass Storage caching, file indexing, or large block buffers.

### License

This project is released under the MIT License. See [LICENSE](LICENSE) for the full license text. SPDX identifier: `MIT`.
