Import("env")

import os

platform = env.PioPlatform()
framework_dir = platform.get_package_dir("framework-stm32cubef4")
fatfs_dir = os.path.join(
    framework_dir,
    "Middlewares",
    "Third_Party",
    "FatFs",
    "src",
)

env.Append(
    CPPPATH=[
        fatfs_dir,
        os.path.join(fatfs_dir, "option"),
        os.path.join(framework_dir, "Middlewares", "ST", "STM32_USB_Device_Library", "Core", "Inc"),
        os.path.join(framework_dir, "Middlewares", "ST", "STM32_USB_Device_Library", "Class", "MSC", "Inc"),
    ]
)

env.BuildSources(
    os.path.join("$BUILD_DIR", "FatFs"),
    fatfs_dir,
    src_filter=[
        "-<*>",
        "+<ff.c>",
        "+<option/unicode.c>",
    ],
)
