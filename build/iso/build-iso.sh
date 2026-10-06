#!/bin/bash
# Zenith OS — ISO Builder
#
# Creates a bootable ISO from the root filesystem.
# Uses squashfs for the live filesystem and GRUB for booting.
#
# Must be run as root.
#
# SPDX-License-Identifier: GPL-3.0-or-later

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/../.." && pwd)"
BUILD_DIR="$PROJECT_DIR/build"
ROOTFS_DIR="$BUILD_DIR/rootfs/rootfs"
DIST_DIR="$PROJECT_DIR/dist"
ISO_WORK="$BUILD_DIR/iso/work"
ISO_NAME="${ISO_NAME:-zenith-os-0.1.0-x86_64.iso}"

RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
NC='\033[0m'

log() { echo -e "${BLUE}[ISO]${NC} $*"; }
ok()  { echo -e "${GREEN}[OK]${NC} $*"; }
err() { echo -e "${RED}[ERROR]${NC} $*" >&2; }

if [[ $EUID -ne 0 ]]; then
    err "This script must be run as root"
    exit 1
fi

# Check root filesystem exists
if [[ ! -d "$ROOTFS_DIR" ]]; then
    err "Root filesystem not found at $ROOTFS_DIR"
    err "Run 'make rootfs' first"
    exit 1
fi

# Check dependencies
for cmd in mksquashfs grub-mkrescue xorriso; do
    if ! command -v "$cmd" &>/dev/null; then
        err "Required command not found: $cmd"
        exit 1
    fi
done

# ============================================================================
# Prepare ISO workspace
# ============================================================================
log "Preparing ISO workspace..."
rm -rf "$ISO_WORK"
mkdir -p "$ISO_WORK"/{boot/grub,live,EFI/BOOT}

# ============================================================================
# Create squashfs
# ============================================================================
log "Creating squashfs (this may take a while)..."
mksquashfs "$ROOTFS_DIR" "$ISO_WORK/live/filesystem.squashfs" \
    -comp xz \
    -b 1M \
    -Xdict-size 100% \
    -noappend \
    -no-progress 2>/dev/null || \
mksquashfs "$ROOTFS_DIR" "$ISO_WORK/live/filesystem.squashfs" \
    -comp gzip \
    -noappend

ok "Squashfs created: $(du -sh "$ISO_WORK/live/filesystem.squashfs" | cut -f1)"

# ============================================================================
# Copy kernel and initramfs
# ============================================================================
log "Copying kernel and initramfs..."

# Find the kernel and initramfs in the rootfs
VMLINUZ=$(ls "$ROOTFS_DIR"/boot/vmlinuz-* 2>/dev/null | head -1)
INITRD=$(ls "$ROOTFS_DIR"/boot/initrd.img-* 2>/dev/null | head -1)

if [[ -z "$VMLINUZ" ]]; then
    err "No kernel found in rootfs /boot"
    exit 1
fi

cp "$VMLINUZ" "$ISO_WORK/boot/vmlinuz"
if [[ -n "$INITRD" ]]; then
    cp "$INITRD" "$ISO_WORK/boot/initrd.img"
fi

ok "Kernel: $(basename "$VMLINUZ")"

# ============================================================================
# Create GRUB configuration
# ============================================================================
log "Creating GRUB configuration..."

cat > "$ISO_WORK/boot/grub/grub.cfg" << 'GRUBEOF'
# Zenith OS GRUB Configuration

set timeout=5
set default=0

# Theme
# loadfont /boot/grub/fonts/unicode.pf2
# set gfxmode=auto
# set gfxpayload=keep
# terminal_output gfxterm

# Colors
set color_normal=light-gray/black
set color_highlight=white/blue
set menu_color_normal=light-gray/black
set menu_color_highlight=white/blue

menuentry "Zenith OS" --class zenith --class os {
    linux /boot/vmlinuz boot=live quiet splash
    initrd /boot/initrd.img
}

menuentry "Zenith OS (Safe Mode)" --class zenith --class os {
    linux /boot/vmlinuz boot=live nomodeset single
    initrd /boot/initrd.img
}

menuentry "Zenith OS (Debug)" --class zenith --class os {
    linux /boot/vmlinuz boot=live debug loglevel=7
    initrd /boot/initrd.img
}

menuentry "Memory Test" --class memtest {
    linux /boot/memtest86+.bin
}
GRUBEOF

ok "GRUB configuration created"

# ============================================================================
# Build ISO with grub-mkrescue
# ============================================================================
log "Building ISO image..."
mkdir -p "$DIST_DIR"

grub-mkrescue \
    -o "$DIST_DIR/$ISO_NAME" \
    "$ISO_WORK" \
    -- \
    -volid "ZENITH_OS" \
    2>/dev/null || \
grub-mkrescue \
    -o "$DIST_DIR/$ISO_NAME" \
    "$ISO_WORK"

ok "ISO created: $DIST_DIR/$ISO_NAME"
log "Size: $(du -sh "$DIST_DIR/$ISO_NAME" | cut -f1)"

# Cleanup
rm -rf "$ISO_WORK"

echo ""
echo -e "${GREEN}=========================================${NC}"
echo -e "${GREEN} Zenith OS ISO built successfully!${NC}"
echo -e "${GREEN} ${NC}"
echo -e "${GREEN} File: $DIST_DIR/$ISO_NAME${NC}"
echo -e "${GREEN} Size: $(du -sh "$DIST_DIR/$ISO_NAME" | cut -f1)${NC}"
echo -e "${GREEN} ${NC}"
echo -e "${GREEN} Test with: make vm${NC}"
echo -e "${GREEN}=========================================${NC}"
