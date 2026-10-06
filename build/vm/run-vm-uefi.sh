#!/bin/bash
# Zenith OS — QEMU VM Launcher (UEFI)
#
# Same as run-vm.sh but boots in UEFI mode using OVMF.
#
# SPDX-License-Identifier: GPL-3.0-or-later

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/../.." && pwd)"
DIST_DIR="$PROJECT_DIR/dist"

RAM="${QEMU_RAM:-2G}"
CPUS="${QEMU_CPUS:-2}"
ISO=""
DISK_FILE="$DIST_DIR/zenith-vm-disk-uefi.qcow2"
OVMF_CODE="${OVMF_PATH:-/usr/share/OVMF/OVMF_CODE.fd}"
OVMF_VARS="$DIST_DIR/OVMF_VARS.fd"

# Find ISO
ISO=$(ls "$DIST_DIR"/zenith-os-*.iso 2>/dev/null | sort -V | tail -1)

if [[ -z "$ISO" || ! -f "$ISO" ]]; then
    echo "Error: No ISO found. Build one first with 'make iso'"
    exit 1
fi

if [[ ! -f "$OVMF_CODE" ]]; then
    echo "Error: OVMF firmware not found at $OVMF_CODE"
    echo "Install it with: sudo apt install ovmf"
    exit 1
fi

# Create OVMF vars copy if needed
if [[ ! -f "$OVMF_VARS" ]]; then
    cp /usr/share/OVMF/OVMF_VARS.fd "$OVMF_VARS" 2>/dev/null || \
    cp "${OVMF_CODE%CODE*}VARS.fd" "$OVMF_VARS" 2>/dev/null || \
    qemu-img create -f raw "$OVMF_VARS" 256K
fi

# Create disk
if [[ ! -f "$DISK_FILE" ]]; then
    qemu-img create -f qcow2 "$DISK_FILE" 20G
fi

echo "========================================="
echo " Zenith OS — QEMU VM (UEFI)"
echo " ISO:  $ISO"
echo " RAM:  $RAM"
echo " CPUs: $CPUS"
echo "========================================="

exec qemu-system-x86_64 \
    -enable-kvm \
    -m "$RAM" \
    -smp "$CPUS" \
    -cpu host \
    -machine type=q35,accel=kvm \
    -drive if=pflash,format=raw,readonly=on,file="$OVMF_CODE" \
    -drive if=pflash,format=raw,file="$OVMF_VARS" \
    -device virtio-vga-gl \
    -display sdl,gl=on \
    -device virtio-net-pci,netdev=net0 \
    -netdev user,id=net0,hostfwd=tcp::2222-:22 \
    -drive file="$DISK_FILE",if=virtio,format=qcow2 \
    -cdrom "$ISO" \
    -boot d \
    -device virtio-keyboard-pci \
    -device virtio-mouse-pci \
    -device intel-hda \
    -device hda-duplex \
    -usb \
    -device usb-tablet \
    "$@"
