# TODO

## Phase 1 — Bootable System (Priority: HIGH)

### Must Have
- [x] Get zenith-compositor compiling with meson
- [x] Implement basic window management (xdg-shell)
- [x] Implement output management (display detection)
- [x] Implement input handling (keyboard, pointer)
- [x] Implement 4 virtual workspaces
- [x] Implement keybinding system (Super+Enter → terminal)
- [x] Get zenith-panel compiling with meson
- [x] Panel: clock widget
- [x] Panel: workspace indicator widget
- [x] Create zenith-session desktop entry for display manager
- [x] Write build-rootfs.sh (debootstrap + customization)
- [x] Write build-iso.sh (squashfs + grub)
- [x] Write run-vm.sh (QEMU launch script)
- [x] Implement `osctl system status`
- [x] Create GRUB theme
- [x] Create Plymouth theme

### Nice to Have
- [ ] Server-side window decorations
- [ ] Window maximize/unmaximize
- [ ] Drag to tile (left/right half)
- [ ] Panel: network status indicator
- [ ] Panel: battery indicator

## Phase 2 — Desktop Shell (Priority: MEDIUM)
- [x] Application launcher
- [x] Application dock
- [x] Notification daemon
- [x] GTK4 theme
- [ ] Named workspaces
- [ ] Dynamic modes
- [ ] Window decorations

## Future Phases
See ROADMAP.md
