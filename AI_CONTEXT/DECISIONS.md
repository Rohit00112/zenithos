# Architecture Decisions

## ADR-001: Debian Stable as Base
**Date:** 2024-01-01
**Status:** Accepted

**Context:** Need a stable Linux base for building the OS.
**Decision:** Use Debian Stable with debootstrap to create the root filesystem.
**Rationale:** Most stable, largest package ecosystem, no corporate lock-in, excellent build tooling.
**Alternatives considered:** Fedora (corporate dependency), Arch (rolling release instability), Ubuntu (Canonical dependency).

## ADR-002: wlroots for Compositor
**Date:** 2024-01-01
**Status:** Accepted

**Context:** Need a Wayland compositor for the custom desktop shell.
**Decision:** Build a custom compositor using wlroots library.
**Rationale:** Proven library (used by Sway, Hyprland), gives full control over desktop experience, handles Wayland protocol complexity.
**Alternatives considered:** Smithay/Rust (too immature), GNOME Shell fork (too coupled), KWin fork (Qt dependency).

## ADR-003: GTK4 for Applications
**Date:** 2024-01-01
**Status:** Accepted

**Context:** Need a UI toolkit for system applications.
**Decision:** Use GTK4 with C.
**Rationale:** Best native Linux toolkit, GPU-accelerated rendering, good Wayland support, gtk4-layer-shell for panel/dock integration.
**Alternatives considered:** Qt6 (licensing complexity, C++ verbosity), EFL (small community), custom (unrealistic).

## ADR-004: Rust for CLI
**Date:** 2024-01-01
**Status:** Accepted

**Context:** Need a command-line tool for system management.
**Decision:** Write `osctl` in Rust using clap.
**Rationale:** Memory safety for a tool that may run with elevated privileges, excellent CLI library ecosystem (clap, colored, tabled), fast startup.
**Alternatives considered:** C (manual memory management risk), Python (slow startup, runtime dependency), Go (less Linux ecosystem integration).

## ADR-005: Btrfs as Default Filesystem
**Date:** 2024-01-01
**Status:** Accepted

**Context:** Need atomic updates and snapshot capability.
**Decision:** Use Btrfs with subvolumes (@, @home, @snapshots, @var-log).
**Rationale:** Native snapshot support enables atomic updates and rollback without additional tooling like OSTree.
**Alternatives considered:** OSTree (Fedora-specific, different update model), ZFS (licensing issues), ext4 (no snapshots).

## ADR-006: D-Bus for Service Communication
**Date:** 2024-01-01
**Status:** Accepted

**Context:** Need IPC between GUI apps, CLI, and system services.
**Decision:** Use D-Bus (GDBus for C, zbus for Rust).
**Rationale:** Standard Linux desktop IPC, integrates with polkit for authorization, well-supported by GTK/GLib.
**Alternatives considered:** Unix sockets (more custom work), gRPC (overkill), REST (HTTP overhead).

## ADR-007: TOML for Configuration
**Date:** 2024-01-01
**Status:** Accepted

**Context:** Need a configuration file format for Zenith-specific settings.
**Decision:** Use TOML format.
**Rationale:** Human-readable, well-structured, good library support in both C (toml-c) and Rust (built-in serde support). Better than INI (no nesting), simpler than YAML (no gotchas).
