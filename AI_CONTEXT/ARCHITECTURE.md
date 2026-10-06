# Architecture

## System Architecture

```
┌──────────────────────────────────────────────────────┐
│                    User Space                         │
│                                                      │
│  ┌──────────────────────────────────────────────┐    │
│  │              Zenith Desktop Shell              │    │
│  │  ┌────────┐ ┌────────┐ ┌──────┐ ┌─────────┐  │    │
│  │  │ Panel  │ │Launcher│ │ Dock │ │Notifier │  │    │
│  │  └────────┘ └────────┘ └──────┘ └─────────┘  │    │
│  │  ┌──────────────────────────────────────────┐ │    │
│  │  │      zenith-compositor (wlroots)          │ │    │
│  │  └──────────────────────────────────────────┘ │    │
│  └──────────────────────────────────────────────┘    │
│                                                      │
│  ┌────────────┐ ┌────────────┐ ┌────────────────┐    │
│  │  GTK4 Apps │ │  Flatpak   │ │  XWayland      │    │
│  └────────────┘ └────────────┘ └────────────────┘    │
│                                                      │
│  ┌──────────────────────────────────────────────┐    │
│  │        Zenith System Services (D-Bus)         │    │
│  │  zenith-settings-daemon                       │    │
│  │  zenith-power                                 │    │
│  │  zenith-snapshot                              │    │
│  │  zenith-network-service                       │    │
│  │  zenith-packages-service                      │    │
│  └──────────────────────────────────────────────┘    │
│                                                      │
│  ┌──────────────────────────────────────────────┐    │
│  │              osctl CLI (Rust)                  │    │
│  └──────────────────────────────────────────────┘    │
│                                                      │
│  ┌──────────────────────────────────────────────┐    │
│  │  systemd │ PipeWire │ NetworkManager │ polkit │    │
│  └──────────────────────────────────────────────┘    │
├──────────────────────────────────────────────────────┤
│  Linux Kernel │ DRM/KMS │ Btrfs │ cgroups │ netfilt │
├──────────────────────────────────────────────────────┤
│  Hardware                                            │
└──────────────────────────────────────────────────────┘
```

## Component Communication

All privileged operations go through D-Bus services with polkit authorization.

```
┌───────────┐     D-Bus      ┌─────────────────┐     syscalls     ┌────────┐
│ GTK4 App  │ ──────────────→│ Zenith Service  │ ───────────────→│ Kernel │
└───────────┘                └─────────────────┘                  └────────┘
                                    ↑
┌───────────┐     D-Bus            │
│  osctl    │ ─────────────────────┘
└───────────┘
```

## Desktop Shell Architecture

The desktop shell consists of multiple processes communicating via Wayland protocols:

```
zenith-compositor (PID 1 of session)
    │
    ├── Wayland protocol ──→ zenith-panel    (layer-shell top)
    ├── Wayland protocol ──→ zenith-dock     (layer-shell bottom)
    ├── Wayland protocol ──→ zenith-launcher (layer-shell overlay)
    ├── Wayland protocol ──→ zenith-notify   (layer-shell top-right)
    ├── Wayland protocol ──→ zenith-wallpaper(layer-shell background)
    │
    ├── Wayland protocol ──→ user applications (xdg-shell)
    └── Wayland protocol ──→ XWayland → X11 apps
```

## Filesystem Layout

Btrfs subvolume layout:

```
/                    → subvolume @
/home                → subvolume @home
/.snapshots          → subvolume @snapshots
/var/log             → subvolume @var-log
```

System files:

```
/usr/lib/zenith/     → Zenith binaries and libraries
/usr/share/zenith/   → Zenith data (themes, icons, wallpapers)
/etc/zenith/         → System-wide configuration
~/.config/zenith/    → Per-user configuration
```

## Build System

```
Makefile (top-level orchestration)
    │
    ├── meson (C components)
    │   ├── desktop/zenith-compositor/
    │   ├── desktop/zenith-panel/
    │   ├── desktop/zenith-dock/
    │   ├── desktop/zenith-launcher/
    │   ├── core/zenith-session/
    │   └── apps/*/
    │
    ├── cargo (Rust components)
    │   └── cli/
    │
    └── shell scripts (ISO generation)
        ├── build/rootfs/build-rootfs.sh
        └── build/iso/build-iso.sh
```
