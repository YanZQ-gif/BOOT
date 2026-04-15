#!/bin/bash
# STM32F767 Bootloader 快速构建脚本
# 使用：./build_bootloader.sh

set -e

echo "========================================"
echo "  STM32F767 Bootloader 构建脚本"
echo "========================================"
echo ""

# 检查是否在项目根目录
if [ ! -f "CMakeLists.txt" ]; then
    echo "错误：请在项目根目录运行此脚本"
    exit 1
fi

# 创建 build 目录
mkdir -p build
cd build

echo "[1/4] 配置 CMake..."
cmake .. -DCMAKE_BUILD_TYPE=Debug

echo "[2/4] 编译 Bootloader..."
make bootloader.elf

echo "[3/4] 生成 BIN 文件..."
arm-none-eabi-objcopy -O binary bootloader.elf bootloader.bin

echo "[4/4] 检查文件大小..."
ls -lh bootloader.*

echo ""
echo "========================================"
echo "  ✓ 构建完成！"
echo "========================================"
echo ""
echo "输出文件："
echo "  - bootloader.elf (调试用)"
echo "  - bootloader.bin (烧录用)"
echo ""
echo "烧录地址：0x08000000"
echo ""
echo "烧录命令示例："
echo "  st-flash write bootloader.bin 0x08000000"
echo "  或"
echo "  openocd -f interface/stlink.cfg -f target/stm32f7x.cfg \\"
echo "    -c \"program bootloader.elf 0x08000000 verify reset exit\""
echo ""
