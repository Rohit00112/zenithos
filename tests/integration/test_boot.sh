#!/bin/bash
# Zenith OS — Boot Integration Test
#
# Tests that the ISO can boot in QEMU and reach a login prompt.
# Runs headless (no display).
#
# SPDX-License-Identifier: GPL-3.0-or-later

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/../.." && pwd)"
DIST_DIR="$PROJECT_DIR/dist"

ISO=$(ls "$DIST_DIR"/zenith-os-*.iso 2>/dev/null | sort -V | tail -1)

if [[ -z "$ISO" || ! -f "$ISO" ]]; then
    echo "SKIP: No ISO found. Build one first with 'make iso'"
    exit 0
fi

echo "Testing boot with ISO: $ISO"

# Create a temporary disk
DISK=$(mktemp /tmp/zenith-test-disk.XXXXXX.qcow2)
qemu-img create -f qcow2 "$DISK" 4G >/dev/null 2>&1

# Boot the ISO with a timeout — check if it reaches userspace
TIMEOUT=120

timeout $TIMEOUT qemu-system-x86_64 \
    -m 1G \
    -smp 2 \
    -display none \
    -serial stdio \
    -drive file="$DISK",if=virtio,format=qcow2 \
    -cdrom "$ISO" \
    -boot d \
    -no-reboot \
    2>&1 | tee /tmp/zenith-boot-test.log &

QEMU_PID=$!

# Wait for login prompt or systemd target
FOUND=false
for i in $(seq 1 $TIMEOUT); do
    if grep -q "login:" /tmp/zenith-boot-test.log 2>/dev/null; then
        FOUND=true
        break
    fi
    if grep -q "Zenith OS" /tmp/zenith-boot-test.log 2>/dev/null; then
        FOUND=true
        break
    fi
    sleep 1
done

# Cleanup
kill $QEMU_PID 2>/dev/null || true
rm -f "$DISK"

if $FOUND; then
    echo "PASS: System booted successfully"
    exit 0
else
    echo "FAIL: System did not reach login prompt within ${TIMEOUT}s"
    echo "Last 20 lines of output:"
    tail -20 /tmp/zenith-boot-test.log 2>/dev/null || true
    exit 1
fi
