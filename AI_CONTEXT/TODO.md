# TODO

## Phase 1 — Bootable System (Priority: HIGH)

### Must Have
- [ ] Get zenith-compositor compiling with meson
- [ ] Implement basic window management (xdg-shell)
- [ ] Implement output management (display detection)
- [ ] Implement input handling (keyboard, pointer)
- [ ] Implement 4 virtual workspaces
- [ ] Implement keybinding system (Super+Enter → terminal)
- [ ] Get zenith-panel compiling with meson
- [ ] Panel: clock widget
- [ ] Panel: workspace indicator widget
- [ ] Create zenith-session desktop entry for display manager
- [ ] Write build-rootfs.sh (debootstrap + customization)
- [ ] Write build-iso.sh (squashfs + grub)
- [ ] Write run-vm.sh (QEMU launch script)
- [ ] Implement `osctl system status`
- [ ] Create GRUB theme
- [ ] Create Plymouth theme

### Nice to Have
- [ ] Server-side window decorations
- [ ] Window maximize/unmaximize
- [ ] Drag to tile (left/right half)
- [ ] Panel: network status indicator
- [ ] Panel: battery indicator

## Phase 2 — Desktop Shell (Priority: MEDIUM)
- [ ] Application launcher
- [ ] Application dock
- [ ] Notification daemon
- [ ] Named workspaces
- [ ] Dynamic modes
- [ ] GTK4 theme

## Future Phases
See ROADMAP.md
