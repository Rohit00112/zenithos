#!/bin/bash
# Zenith OS — QEMU VM Launcher
#
# Launches the Zenith OS ISO in a QEMU virtual machine for testing.
#
# Usage:
#   ./run-vm.sh                    # Default: 2GB RAM, 2 CPUs
#   ./run-vm.sh --ram 4G --cpus 4  # Custom resources
#
# SPDX-License-Identifier: GPL-3.0-or-later

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/../.." && pwd)"
DIST_DIR="$PROJECT_DIR/dist"

# Defaults
RAM="${QEMU_RAM:-2G}"
CPUS="${QEMU_CPUS:-2}"
ISO=""
DISK_SIZE="20G"
DISK_FILE="$DIST_DIR/zenith-vm-disk.qcow2"

# Parse arguments
while [[ $# -gt 0 ]]; do
    case "$1" in
        --ram)    RAM="$2"; shift 2 ;;
        --cpus)   CPUS="$2"; shift 2 ;;
        --iso)    ISO="$2"; shift 2 ;;
        --disk)   DISK_FILE="$2"; shift 2 ;;
        --help|-h)
            echo "Usage: $0 [--ram SIZE] [--cpus N] [--iso FILE] [--disk FILE]"
            exit 0
            ;;
        *)        echo "Unknown option: $1"; exit 1 ;;
    esac
done

# Find ISO if not specified
if [[ -z "$ISO" ]]; then
    ISO=$(ls "$DIST_DIR"/zenith-os-*.iso 2>/dev/null | sort -V | tail -1)
fi

if [[ -z "$ISO" || ! -f "$ISO" ]]; then
    echo "Error: No ISO found. Build one first with 'make iso'"
    exit 1
fi

echo "========================================="
echo " Zenith OS — QEMU VM"
echo " ISO:  $ISO"
echo " RAM:  $RAM"
echo " CPUs: $CPUS"
echo " Disk: $DISK_FILE"
echo "========================================="

# Create disk image if it doesn't exist
if [[ ! -f "$DISK_FILE" ]]; then
    echo "Creating VM disk: $DISK_FILE ($DISK_SIZE)"
    qemu-img create -f qcow2 "$DISK_FILE" "$DISK_SIZE"
fi

# Determine acceleration based on host OS
if [[ "$(uname -s)" == "Darwin" ]]; then
    ACCEL="-machine type=q35 -cpu qemu64"
else
    ACCEL="-enable-kvm -machine type=q35,accel=kvm -cpu host"
fi

# Launch QEMU
exec qemu-system-x86_64 \
    $ACCEL \
    -m "$RAM" \
    -smp "$CPUS" \
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
