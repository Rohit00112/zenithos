# Zenith OS — Architecture Overview

## System Layers

```
┌──────────────────────────────────────────────┐
│           User Applications                   │
│   (Flatpak, native packages, containers)     │
├──────────────────────────────────────────────┤
│           Zenith Desktop Shell                │
│   (compositor, panel, dock, launcher)        │
├──────────────────────────────────────────────┤
│           Zenith System Services              │
│   (settings, security, packages, snapshots)  │
│              via D-Bus                        │
├──────────────────────────────────────────────┤
│           Linux System Services               │
│   (systemd, PipeWire, NetworkManager)        │
├──────────────────────────────────────────────┤
│           Security Layer                      │
│   (AppArmor, nftables, polkit, seccomp)      │
├──────────────────────────────────────────────┤
│           Linux Kernel                        │
│   (DRM/KMS, Btrfs, cgroups, netfilter)       │
├──────────────────────────────────────────────┤
│           Hardware                            │
└──────────────────────────────────────────────┘
```

## Component Communication

All system management goes through D-Bus services:

```
GUI App ─── D-Bus ──→ Zenith Service ──→ System
                         ↑
CLI Tool ── D-Bus ───────┘
```

This ensures:
1. GUI and CLI always have the same capabilities
2. Authorization (via polkit) is enforced consistently
3. No direct system file manipulation from applications

## Desktop Shell Architecture

The desktop shell is composed of multiple processes:

| Process | Role | Protocol |
|---------|------|----------|
| zenith-compositor | Wayland compositor, window management | wlroots |
| zenith-panel | Top panel (clock, workspace, indicators) | layer-shell |
| zenith-dock | Bottom dock (app launcher bar) | layer-shell |
| zenith-launcher | Universal search/launcher | layer-shell overlay |
| zenith-notifications | Notification daemon | layer-shell, D-Bus |
| zenith-wallpaper | Wallpaper renderer | layer-shell background |

## Filesystem Layout

### Btrfs Subvolumes
```
@           → /        (root filesystem)
@home       → /home    (user data)
@snapshots  → /.snapshots (system snapshots)
@var-log    → /var/log (logs, excluded from snapshots)
```

### Zenith Files
```
/usr/local/bin/          → Zenith binaries
/usr/share/zenith/       → Shared data (themes, icons, wallpapers)
/etc/zenith/             → System-wide configuration
~/.config/zenith/        → Per-user configuration
```

## Security Model

See [../security/SECURITY.md](../security/SECURITY.md) for detailed security architecture.

- **Principle of least privilege** — No GUI app runs as root
- **AppArmor** — MAC profiles for all Zenith apps
- **polkit** — Authorization for privileged operations
- **nftables** — Firewall with sensible defaults
- **Secure Boot** — Via shim + MOK (Phase 8)
