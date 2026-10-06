#!/bin/bash
# Zenith OS — Root Filesystem Builder
#
# Creates a minimal Debian-based root filesystem using debootstrap,
# then customizes it with Zenith OS components.
#
# Must be run as root (or via sudo).
#
# SPDX-License-Identifier: GPL-3.0-or-later

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/../.." && pwd)"
ROOTFS_DIR="$SCRIPT_DIR/rootfs"
SUITE="${DEBIAN_SUITE:-noble}"
MIRROR="${DEBIAN_MIRROR:-http://archive.ubuntu.com/ubuntu}"
ARCH="amd64"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
NC='\033[0m'

log() { echo -e "${BLUE}[ROOTFS]${NC} $*"; }
ok()  { echo -e "${GREEN}[OK]${NC} $*"; }
err() { echo -e "${RED}[ERROR]${NC} $*" >&2; }

# Check root
if [[ $EUID -ne 0 ]]; then
    err "This script must be run as root"
    exit 1
fi

# Check dependencies
for cmd in debootstrap chroot mount; do
    if ! command -v "$cmd" &>/dev/null; then
        err "Required command not found: $cmd"
        exit 1
    fi
done

# Clean previous build
if [[ -d "$ROOTFS_DIR" ]]; then
    log "Cleaning previous rootfs..."
    # Unmount any leftover mounts
    umount -lf "$ROOTFS_DIR/dev/pts" 2>/dev/null || true
    umount -lf "$ROOTFS_DIR/dev" 2>/dev/null || true
    umount -lf "$ROOTFS_DIR/proc" 2>/dev/null || true
    umount -lf "$ROOTFS_DIR/sys" 2>/dev/null || true
    rm -rf "$ROOTFS_DIR"
fi

# ============================================================================
# Step 1: Debootstrap base system
# ============================================================================
log "Running debootstrap (suite: $SUITE, arch: $ARCH)..."
log "This may take several minutes..."

PACKAGES=$(cat "$SCRIPT_DIR/packages.list" | grep -v '^#' | grep -v '^$' | tr '\n' ',')

debootstrap --components=main,universe,restricted,multiverse \\
    --arch="$ARCH" \
    --include="$PACKAGES" \
    "$SUITE" \
    "$ROOTFS_DIR" \
    "$MIRROR"

ok "Base system installed"

# ============================================================================
# Step 2: Mount virtual filesystems for chroot
# ============================================================================
log "Mounting virtual filesystems..."
mount --bind /dev "$ROOTFS_DIR/dev"
mount --bind /dev/pts "$ROOTFS_DIR/dev/pts"
mount -t proc proc "$ROOTFS_DIR/proc"
mount -t sysfs sys "$ROOTFS_DIR/sys"

# ============================================================================
# Step 3: Customize inside chroot
# ============================================================================
log "Customizing root filesystem..."

# Copy customization script into chroot
cp "$SCRIPT_DIR/customize-rootfs.sh" "$ROOTFS_DIR/tmp/"

# Copy desktop packages list
cp "$SCRIPT_DIR/packages-desktop.list" "$ROOTFS_DIR/tmp/"

# Copy Zenith binaries (if built)
ZENITH_BIN_DIR="$ROOTFS_DIR/usr/local/bin"
mkdir -p "$ZENITH_BIN_DIR"

# Copy gtk4-layer-shell built SO from host if present
if ls /usr/local/lib/x86_64-linux-gnu/libgtk4-layer-shell.so* 1> /dev/null 2>&1; then
    mkdir -p "$ROOTFS_DIR/usr/local/lib/x86_64-linux-gnu/"
    cp -a /usr/local/lib/x86_64-linux-gnu/libgtk4-layer-shell.so* "$ROOTFS_DIR/usr/local/lib/x86_64-linux-gnu/"
    chroot "$ROOTFS_DIR" ldconfig
fi
if ls /usr/lib/x86_64-linux-gnu/libgtk4-layer-shell.so* 1> /dev/null 2>&1; then
    mkdir -p "$ROOTFS_DIR/usr/lib/x86_64-linux-gnu/"
    cp -a /usr/lib/x86_64-linux-gnu/libgtk4-layer-shell.so* "$ROOTFS_DIR/usr/lib/x86_64-linux-gnu/"
    chroot "$ROOTFS_DIR" ldconfig
fi

# Copy compositor
if [[ -f "$PROJECT_DIR/desktop/zenith-compositor/builddir/zenith-compositor" ]]; then
    cp "$PROJECT_DIR/desktop/zenith-compositor/builddir/zenith-compositor" "$ZENITH_BIN_DIR/"
    ok "Installed zenith-compositor"
fi

# Copy panel
if [[ -f "$PROJECT_DIR/desktop/zenith-panel/builddir/zenith-panel" ]]; then
    cp "$PROJECT_DIR/desktop/zenith-panel/builddir/zenith-panel" "$ZENITH_BIN_DIR/"
    ok "Installed zenith-panel"\
fi\
\
# Copy dock\
if [[ -f "$PROJECT_DIR/desktop/zenith-dock/builddir/zenith-dock" ]]; then\
    cp "$PROJECT_DIR/desktop/zenith-dock/builddir/zenith-dock" "$ZENITH_BIN_DIR/"\
    ok "Installed zenith-dock"\
fi\
\
# Copy launcher\
if [[ -f "$PROJECT_DIR/desktop/zenith-launcher/builddir/zenith-launcher" ]]; then\
    cp "$PROJECT_DIR/desktop/zenith-launcher/builddir/zenith-launcher" "$ZENITH_BIN_DIR/"\
    ok "Installed zenith-launcher"\
fi\
\
# Copy wallpaper\
if [[ -f "$PROJECT_DIR/desktop/zenith-wallpaper/builddir/zenith-wallpaper" ]]; then\
    cp "$PROJECT_DIR/desktop/zenith-wallpaper/builddir/zenith-wallpaper" "$ZENITH_BIN_DIR/"\
    ok "Installed zenith-wallpaper"
fi

# Copy session manager
if [[ -f "$PROJECT_DIR/core/zenith-session/builddir/zenith-session" ]]; then
    cp "$PROJECT_DIR/core/zenith-session/builddir/zenith-session" "$ZENITH_BIN_DIR/"
    ok "Installed zenith-session"
fi

# Copy osctl
if [[ -f "$PROJECT_DIR/cli/target/release/osctl" ]]; then
    cp "$PROJECT_DIR/cli/target/release/osctl" "$ZENITH_BIN_DIR/"
    ok "Installed osctl"
elif [[ -f "$PROJECT_DIR/cli/target/debug/osctl" ]]; then
    cp "$PROJECT_DIR/cli/target/debug/osctl" "$ZENITH_BIN_DIR/"
    ok "Installed osctl (debug)"
fi

# Copy session desktop file
mkdir -p "$ROOTFS_DIR/usr/share/wayland-sessions"
cp "$PROJECT_DIR/core/zenith-session/data/zenith-session.desktop" \
   "$ROOTFS_DIR/usr/share/wayland-sessions/"

# Copy default configs
if [[ -d "$PROJECT_DIR/configs/skel" ]]; then
    cp -r "$PROJECT_DIR/configs/skel/." "$ROOTFS_DIR/etc/skel/"
    ok "Installed default user configuration"
fi

# Run customization inside chroot
chroot "$ROOTFS_DIR" /bin/bash /tmp/customize-rootfs.sh

# ============================================================================
# Step 4: Cleanup
# ============================================================================
log "Cleaning up..."
rm -f "$ROOTFS_DIR/tmp/customize-rootfs.sh"
rm -f "$ROOTFS_DIR/tmp/packages-desktop.list"

# Unmount virtual filesystems
umount -lf "$ROOTFS_DIR/dev/pts" 2>/dev/null || true
umount -lf "$ROOTFS_DIR/dev" 2>/dev/null || true
umount -lf "$ROOTFS_DIR/proc" 2>/dev/null || true
umount -lf "$ROOTFS_DIR/sys" 2>/dev/null || true

ok "Root filesystem created at: $ROOTFS_DIR"
log "Size: $(du -sh "$ROOTFS_DIR" | cut -f1)"
