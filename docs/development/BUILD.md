# Building Zenith OS

## Prerequisites

You need a **Debian-based Linux system** (Debian 12+, Ubuntu 22.04+) to build Zenith OS.

### Quick Setup

```bash
./scripts/setup-dev.sh
```

This installs all dependencies automatically on Debian, Ubuntu, Fedora, or Arch.

### Manual Dependencies

**Build tools:**
- `build-essential`, `meson`, `ninja-build`, `pkg-config`, `cmake`, `git`

**Wayland/wlroots:**
- `libwlroots-dev` (>= 0.17), `libwayland-dev`, `wayland-protocols`
- `libxkbcommon-dev`, `libpixman-1-dev`, `libdrm-dev`, `libgbm-dev`
- `libseat-dev`, `libudev-dev`, `libinput-dev`, `libvulkan-dev`

**GTK4:**
- `libgtk-4-dev`, `libgtk4-layer-shell-dev`

**Rust:**
- Install via [rustup.rs](https://rustup.rs/)

**ISO generation:**
- `debootstrap`, `squashfs-tools`, `grub-pc-bin`, `grub-efi-amd64-bin`
- `xorriso`, `mtools`, `dosfstools`

**Testing:**
- `qemu-system-x86`, `qemu-utils`, `ovmf`

## Build Targets

```bash
# Build all components (compositor, panel, session, CLI)
make all

# Build individual components
make compositor        # Wayland compositor
make panel             # Top panel
make session           # Session manager
make cli               # osctl CLI tool

# Build ISO (requires sudo for debootstrap)
sudo make rootfs       # Generate root filesystem
sudo make iso          # Build bootable ISO

# Test in QEMU
make vm                # Boot ISO in QEMU (BIOS mode)
make vm-uefi           # Boot ISO in QEMU (UEFI mode)

# Docker-based build (no sudo needed for the build itself)
make docker-build
```

## Build Output

```
dist/
├── zenith-os-0.1.0-x86_64.iso    # Bootable ISO
├── zenith-vm-disk.qcow2          # VM disk (created on first VM run)
└── zenith-vm-disk-uefi.qcow2     # UEFI VM disk
```

## Reproducible Builds

For reproducible builds, use the Docker container:

```bash
make docker-build
```

This runs the entire build inside a container with pinned dependencies.

## Troubleshooting

### wlroots version too old

Debian Bookworm ships wlroots 0.16. Zenith requires >= 0.17.
Build wlroots from source:

```bash
git clone https://gitlab.freedesktop.org/wlroots/wlroots.git
cd wlroots && git checkout 0.17.4
meson setup build --prefix=/usr
ninja -C build
sudo ninja -C build install
```

### gtk4-layer-shell not found

Build from source:

```bash
git clone https://github.com/wmww/gtk4-layer-shell.git
cd gtk4-layer-shell
meson setup build --prefix=/usr
ninja -C build
sudo ninja -C build install
```
