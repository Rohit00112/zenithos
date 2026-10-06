<p align="center">
  <img src="docs/design/zenith-logo.svg" alt="Zenith OS" width="120">
</p>

<h1 align="center">Zenith OS</h1>

<p align="center">
  <strong>Linux underneath. Completely different experience on top.</strong>
</p>

<p align="center">
  A next-generation Linux-based operating system with an original desktop experience,<br>
  built for developers, security professionals, and power users who demand more.
</p>

---

## What is Zenith OS?

Zenith OS is not another Linux distribution. It is a complete operating system product built on the Linux kernel, with:

- **Original Wayland compositor** — Custom desktop shell built with wlroots
- **Dynamic desktop modes** — Focus, Power, Creator, Developer, Security
- **Unified system management** — GUI and CLI share the same APIs via D-Bus
- **Real security center** — Measurable security scoring, not theater
- **Atomic updates** — Btrfs snapshots with automatic rollback
- **Developer-first tools** — Containers, VMs, environments, all integrated
- **Privacy by design** — No telemetry by default, offline-first

## Architecture

```
Linux Kernel (LTS)
    ↓
Hardware / Drivers (Mesa, NVIDIA, firmware)
    ↓
System Services (systemd, PipeWire, NetworkManager)
    ↓
Security Layer (AppArmor, nftables, polkit)
    ↓
Zenith Services (D-Bus: settings, security, packages, snapshots)
    ↓
Zenith Desktop Shell (wlroots compositor, panel, dock, launcher)
    ↓
Zenith Applications (GTK4: files, terminal, monitor, settings)
    ↓
User Applications (Flatpak, native packages, containers)
```

## Quick Start

### Prerequisites

- Debian-based Linux system (for building)
- `debootstrap`, `squashfs-tools`, `grub-pc-bin`, `grub-efi-amd64-bin`, `xorriso`
- `meson`, `ninja-build`, `gcc`, `pkg-config`
- `libwlroots-dev` (≥ 0.17), `libgtk-4-dev`, `libgtk4-layer-shell-dev`
- `cargo` (Rust toolchain)
- `qemu-system-x86` (for testing)

### Build

```bash
# Setup development environment
./scripts/setup-dev.sh

# Build everything
make all

# Build ISO
make iso

# Test in QEMU
make vm
```

### Development

```bash
# Build only the compositor
make compositor

# Build only the panel
make panel

# Build the CLI
make cli

# Run tests
make test
```

## Project Structure

```
myos/
├── AI_CONTEXT/      # Development context for AI-assisted development
├── docs/            # Documentation
├── build/           # Build system and ISO generation
├── boot/            # GRUB, Plymouth, initramfs
├── kernel/          # Kernel configuration
├── core/            # Core OS services (session, settings, power, snapshots)
├── services/        # systemd units and D-Bus service files
├── security/        # AppArmor, polkit, firewall, audit
├── networking/      # Network configuration and services
├── packages/        # Package management service
├── desktop/         # Desktop shell (compositor, panel, dock, launcher)
├── apps/            # Applications (file manager, terminal, monitor, etc.)
├── installer/       # System installer
├── recovery/        # Recovery environment
├── cli/             # osctl command-line tool (Rust)
├── tests/           # Automated tests
├── scripts/         # Development and build scripts
└── configs/         # Default configurations and themes
```

## Technology Stack

| Component | Technology | Why |
|-----------|-----------|-----|
| Base | Debian Stable | Stability, package ecosystem |
| Init | systemd | Standard, reliable |
| Display | Wayland (wlroots) | Modern, secure, full control |
| UI Toolkit | GTK4 | Best native Linux toolkit |
| CLI | Rust (clap) | Memory safety, modern ergonomics |
| Filesystem | Btrfs | Snapshots, atomic updates |
| Audio | PipeWire | Modern audio/video |
| Networking | NetworkManager | Proven, D-Bus API |
| Firewall | nftables | Modern netfilter |
| Sandboxing | AppArmor + Flatpak | MAC + portal isolation |
| Containers | Podman | Daemonless, rootless |
| Virtualization | KVM/QEMU/libvirt | Standard Linux virt |
| IPC | D-Bus | Standard desktop IPC |

## Current Status

**Phase 1: Bootable System** — In Progress

See [AI_CONTEXT/CURRENT_STATE.md](AI_CONTEXT/CURRENT_STATE.md) for detailed status.

## License

Zenith OS is licensed under the [GNU General Public License v3.0](LICENSE).

## Contributing

See [docs/development/CONTRIBUTING.md](docs/development/CONTRIBUTING.md) for contribution guidelines.
