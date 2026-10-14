#!/bin/bash
set -euo pipefail
module_path=$1
source_path=$2
prepared_path=$3
kernel_path="$module_path/_kernel"
mkdir -p "$kernel_path/arch/arm64"
cp -a "$prepared_path/.config" "$prepared_path/Module.symvers" "$kernel_path/"
cp -a "$prepared_path/include" "$kernel_path/"
cp -a "$prepared_path/scripts" "$kernel_path/"
cp -a "$prepared_path/arch/arm64/include" "$kernel_path/arch/arm64/"
make -C "$source_path" O="$kernel_path" M="$module_path" ARCH=arm64 \
    CROSS_COMPILE=aarch64-linux-gnu- -j4 modules
