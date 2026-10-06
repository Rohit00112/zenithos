# Project Context

## What is this project?

Zenith OS is a next-generation Linux-based operating system. It uses the Linux kernel
and ecosystem underneath but provides a completely original user experience on top.

It is NOT a reskin of GNOME/KDE/XFCE. It is a cohesive product with:
- Custom Wayland compositor (wlroots-based)
- Original desktop shell with dynamic modes
- Suite of GTK4 applications
- Unified CLI tool (`osctl`) in Rust
- D-Bus service architecture for system management
- Btrfs-based atomic updates and snapshots

## Key Terminology

- **Zenith** — The OS brand name
- **zenith-compositor** — The custom Wayland compositor
- **zenith-panel** — The top panel (layer-shell surface)
- **zenith-dock** — The bottom dock (layer-shell surface)
- **zenith-launcher** — The universal application launcher
- **osctl** — The command-line system management tool
- **Dynamic Modes** — Focus, Power, Creator, Developer, Security modes

## Technology Stack

- **Base:** Debian Stable (debootstrap)
- **Kernel:** Linux LTS
- **Init:** systemd
- **Display:** Wayland via wlroots (C)
- **UI Apps:** GTK4 (C)
- **CLI:** Rust (clap)
- **Filesystem:** Btrfs (subvolumes: @, @home, @snapshots, @var-log)
- **Audio:** PipeWire
- **Network:** NetworkManager
- **Firewall:** nftables
- **Sandboxing:** AppArmor + Flatpak portals
- **Containers:** Podman
- **VMs:** KVM/QEMU/libvirt
- **IPC:** D-Bus (session + system bus)
- **Build:** Meson (components) + Make (orchestration)

## Repository Layout

```
myos/
├── build/       → Build system, ISO generation
├── boot/        → GRUB, Plymouth, initramfs
├── kernel/      → Kernel config
├── core/        → D-Bus services (session, settings, power, snapshots)
├── services/    → systemd units, D-Bus activation files
├── security/    → AppArmor, polkit, nftables, audit
├── networking/  → NetworkManager config, network service
├── packages/    → Package management service
├── desktop/     → Compositor, panel, dock, launcher, notifications
├── apps/        → GTK4 applications
├── installer/   → System installer
├── recovery/    → Recovery environment
├── cli/         → osctl (Rust)
├── tests/       → All tests
├── scripts/     → Dev/build scripts
└── configs/     → Default OS configurations, themes
```

## Design Principles

1. **Working software over descriptions** — Every feature must actually work
2. **No fake functionality** — If not implemented, show "Not implemented yet"
3. **GUI and CLI share APIs** — Both use D-Bus services, no separate logic
4. **Modular architecture** — Features are optional, base system is minimal
5. **Security by default** — Least privilege, secure defaults
6. **Offline-first** — Core OS works without internet
7. **VM-first development** — Everything testable in QEMU
