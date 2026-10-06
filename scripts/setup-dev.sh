#!/bin/bash
# Zenith OS — Development Environment Setup
#
# Installs all dependencies needed to build/develop Zenith OS.
# Supports: Debian/Ubuntu, Fedora, Arch, macOS (via Homebrew).
#
# On macOS, only the CLI (Rust) and Docker-based builds are supported.
# The Wayland compositor and GTK4 panel require Linux.
#
# SPDX-License-Identifier: GPL-3.0-or-later

set -euo pipefail

RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[0;33m'
NC='\033[0m'

log()  { echo -e "${BLUE}[SETUP]${NC} $*"; }
ok()   { echo -e "${GREEN}[OK]${NC} $*"; }
warn() { echo -e "${YELLOW}[WARN]${NC} $*"; }
err()  { echo -e "${RED}[ERROR]${NC} $*" >&2; }

# Detect OS — check for macOS first, then Linux distros
UNAME_S="$(uname -s)"
if [[ "$UNAME_S" == "Darwin" ]]; then
    OS_ID="macos"
elif [[ -f /etc/os-release ]]; then
    . /etc/os-release
    OS_ID="${ID:-unknown}"
else
    OS_ID="unknown"
fi

log "Detected OS: $OS_ID"

case "$OS_ID" in
    macos)
        log "Setting up macOS development environment..."
        warn ""
        warn "macOS Limitations:"
        warn "  • The Wayland compositor (zenith-compositor) cannot be built on macOS"
        warn "    (wlroots and Wayland are Linux-only)"
        warn "  • The GTK4 panel (zenith-panel) cannot be built on macOS"
        warn "    (gtk4-layer-shell requires Wayland)"
        warn "  • ISO generation (debootstrap, squashfs) requires Linux"
        warn ""
        warn "What WORKS on macOS:"
        warn "  • osctl CLI tool (Rust) — full build and test"
        warn "  • Docker-based full builds — 'make docker-build'"
        warn "  • Code editing, documentation, planning"
        warn ""

        # Check for Homebrew
        if ! command -v brew &>/dev/null; then
            err "Homebrew is required. Install it from https://brew.sh"
            exit 1
        fi

        log "Installing dependencies via Homebrew..."
        brew install \
            meson \
            ninja \
            pkg-config \
            cmake \
            git \
            qemu \
            docker \
            2>/dev/null || true

        ok "Homebrew dependencies installed"
        ;;

    debian|ubuntu|linuxmint|pop)
        log "Installing Debian/Ubuntu build dependencies..."
        sudo apt-get update
        sudo apt-get install -y \
            build-essential \
            meson \
            ninja-build \
            pkg-config \
            cmake \
            git \
            \
            debootstrap \
            squashfs-tools \
            grub-pc-bin \
            grub-efi-amd64-bin \
            grub-common \
            xorriso \
            mtools \
            dosfstools \
            \
            qemu-system-x86 \
            qemu-utils \
            ovmf \
            \
            libwlroots-dev \
            libwayland-dev \
            wayland-protocols \
            libxkbcommon-dev \
            libpixman-1-dev \
            libdrm-dev \
            libgbm-dev \
            libseat-dev \
            libudev-dev \
            libinput-dev \
            libvulkan-dev \
            \
            libgtk-4-dev \
            libgtk4-layer-shell-dev \
            \
            clang-format \
            clang-tidy
        ok "Debian/Ubuntu dependencies installed"
        ;;

    fedora)
        log "Installing Fedora build dependencies..."
        sudo dnf install -y \
            gcc gcc-c++ make meson ninja-build pkg-config cmake git \
            debootstrap squashfs-tools grub2-tools grub2-efi-x64 xorriso mtools \
            qemu-system-x86 qemu-img edk2-ovmf \
            wlroots-devel wayland-devel wayland-protocols-devel \
            libxkbcommon-devel pixman-devel libdrm-devel mesa-libgbm-devel \
            libseat-devel systemd-devel libinput-devel vulkan-loader-devel \
            gtk4-devel gtk4-layer-shell-devel \
            clang-tools-extra
        ok "Fedora dependencies installed"
        ;;

    arch|manjaro)
        log "Installing Arch build dependencies..."
        sudo pacman -Syu --needed --noconfirm \
            base-devel meson ninja pkg-config cmake git \
            debootstrap squashfs-tools grub xorriso mtools dosfstools \
            qemu-full edk2-ovmf \
            wlroots wayland wayland-protocols \
            libxkbcommon pixman libdrm mesa libseat libinput vulkan-icd-loader \
            gtk4 gtk4-layer-shell \
            clang
        ok "Arch dependencies installed"
        ;;

    *)
        err "Unsupported OS: $OS_ID"
        err "Please install dependencies manually. See docs/development/BUILD.md"
        exit 1
        ;;
esac

# Install Rust if not present
if ! command -v cargo &>/dev/null; then
    log "Installing Rust toolchain..."
    curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y
    source "$HOME/.cargo/env"
    ok "Rust installed"
else
    ok "Rust already installed: $(rustc --version)"
fi

# Install Rust components
rustup component add clippy rustfmt 2>/dev/null || true

echo ""
echo -e "${GREEN}=========================================${NC}"
echo -e "${GREEN} Development environment ready!${NC}"
echo -e "${GREEN}=========================================${NC}"
echo ""
if [[ "$OS_ID" == "macos" ]]; then
    echo "macOS — Available commands:"
    echo "  make cli           # Build osctl CLI tool"
    echo "  cd cli && cargo run -- system status"
    echo "  cd cli && cargo run -- hardware info"
    echo "  cd cli && cargo test"
    echo ""
    echo "For full OS builds (compositor, ISO), use Docker:"
    echo "  make docker-build  # Build everything in a Linux container"
    echo ""
    echo "Or use a Linux VM/remote machine for full development."
else
    echo "Next steps:"
    echo "  make all           # Build all components"
    echo "  make iso           # Build bootable ISO (requires sudo)"
    echo "  make vm            # Test in QEMU"
fi
echo ""

