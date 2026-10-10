# Roadmap

## Phase 1: Bootable System (Current)
**Status:** Complete

- [x] Directory structure
- [x] Build system (Makefile)
- [x] Wayland compositor (zenith-compositor)
- [x] Panel (zenith-panel)
- [x] Session management (zenith-session)
- [x] osctl CLI (basic system status)
- [x] GRUB theme
- [x] Plymouth boot splash
- [x] Root filesystem generation
- [x] ISO generation
- [x] QEMU test script

**Deliverable:** Bootable ISO → GRUB → Plymouth → Wayland → Panel → Terminal

## Phase 2: Desktop Shell
**Status:** In Progress

- [ ] Application launcher
- [ ] Application dock
- [ ] Notification daemon
- [ ] Workspace system with named workspaces
- [ ] Dynamic modes (Focus, Power, Developer)
- [ ] Wallpaper manager
- [ ] GTK4 theme (Zenith Dark)
- [ ] Window decorations

## Phase 3: Core Applications
**Status:** Not Started

- [ ] File manager (Zenith Files)
- [ ] Terminal (Zenith Terminal)
- [ ] System monitor (Zenith Monitor)
- [ ] Settings (Zenith Settings)
- [ ] Network center (Zenith Network)
- [ ] osctl expanded commands

## Phase 4: Package & Update System
**Status:** Not Started

- [ ] Unified package manager GUI
- [ ] Btrfs snapshot automation
- [ ] Transactional updates
- [ ] Rollback support
- [ ] GRUB boot menu with snapshots

## Phase 5: Security
**Status:** Not Started

- [ ] Security center with real scoring
- [ ] Firewall GUI
- [ ] AppArmor integration
- [ ] Polkit policies
- [ ] Audit logging

## Phase 6: Developer Platform
**Status:** Not Started

- [ ] Developer center
- [ ] Container center (Podman)
- [ ] Virtualization center (libvirt)

## Phase 7: Advanced Features
**Status:** Not Started

- [ ] Automation engine
- [ ] System troubleshooter
- [ ] AI integration (optional, local only)
- [ ] Gaming mode
- [ ] Hardware center
- [ ] Logs explorer
- [ ] Privacy center
- [ ] Backup system

## Phase 8: Polish
**Status:** Not Started

- [ ] Graphical installer
- [ ] Recovery environment
- [ ] First-boot wizard
- [ ] Accessibility
- [ ] Performance optimization
- [ ] Complete documentation
- [ ] Theme import/export
- [ ] Full test suite
