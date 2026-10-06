#!/bin/bash
# Zenith OS — QEMU VM Launcher (UEFI)
#
# Launches the Zenith OS ISO in a QEMU virtual machine using UEFI firmware.
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
DISK_SIZE="20G"

# Locate OVMF firmware
OVMF_CODE=""
OVMF_VARS_TEMPLATE=""

if [[ -f /usr/share/OVMF/OVMF_CODE.fd ]]; then
    OVMF_CODE="/usr/share/OVMF/OVMF_CODE.fd"
    OVMF_VARS_TEMPLATE="/usr/share/OVMF/OVMF_VARS.fd"
elif [[ -f /opt/homebrew/share/qemu/edk2-x86_64-code.fd ]]; then
    OVMF_CODE="/opt/homebrew/share/qemu/edk2-x86_64-code.fd"
    OVMF_VARS_TEMPLATE="/opt/homebrew/share/qemu/edk2-i386-vars.fd"
elif [[ -n "${OVMF_PATH:-}" && -f "$OVMF_PATH" ]]; then
    OVMF_CODE="$OVMF_PATH"
    OVMF_VARS_TEMPLATE="${OVMF_CODE%CODE*}VARS.fd"
fi

if [[ -z "$OVMF_CODE" ]]; then
    echo "Error: OVMF firmware not found."
    echo "Install: sudo apt install ovmf (Linux) or brew install qemu (macOS)"
    exit 1
fi

# Find ISO
ISO=$(ls "$DIST_DIR"/zenith-os-*.iso 2>/dev/null | sort -V | tail -1)

if [[ -z "$ISO" || ! -f "$ISO" ]]; then
    echo "Error: No ISO found. Build one first with 'make iso'"
    exit 1
fi

# Create OVMF vars copy
OVMF_VARS="$DIST_DIR/OVMF_VARS.fd"
if [[ ! -f "$OVMF_VARS" ]]; then
    if [[ -n "$OVMF_VARS_TEMPLATE" && -f "$OVMF_VARS_TEMPLATE" ]]; then
        cp "$OVMF_VARS_TEMPLATE" "$OVMF_VARS"
    else
        qemu-img create -f raw "$OVMF_VARS" 256K
    fi
fi

# Create disk
if [[ ! -f "$DISK_FILE" ]]; then
    echo "Creating VM disk: $DISK_FILE ($DISK_SIZE)"
    qemu-img create -f qcow2 "$DISK_FILE" "$DISK_SIZE"
fi

echo "========================================="
echo " Zenith OS — QEMU VM (UEFI)"
echo " ISO:  $ISO"
echo " RAM:  $RAM"
echo " CPUs: $CPUS"
echo "========================================="

# Detect host acceleration
if [[ "$(uname -s)" == "Darwin" ]]; then
    ACCEL="-machine type=q35 -cpu qemu64"
else
    ACCEL="-enable-kvm -machine type=q35,accel=kvm -cpu host"
fi

exec qemu-system-x86_64 \
    $ACCEL \
    -m "$RAM" \
    -smp "$CPUS" \
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
