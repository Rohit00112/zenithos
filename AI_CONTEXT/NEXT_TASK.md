# Next Task

## Current Focus: zenith-compositor

The immediate next task is to complete the Wayland compositor so it can:

1. Initialize wlroots backend (DRM/KMS or headless)
2. Create outputs (monitors)
3. Handle keyboard and pointer input
4. Support xdg-shell (open, move, resize, close windows)
5. Render using wlr_scene
6. Support 4 virtual workspaces
7. Handle keybindings (Super+Enter = terminal, Super+1-4 = workspace)

### Files to work on:
- `desktop/zenith-compositor/src/main.c`
- `desktop/zenith-compositor/src/compositor.c`
- `desktop/zenith-compositor/src/output.c`
- `desktop/zenith-compositor/src/input.c`
- `desktop/zenith-compositor/src/view.c`
- `desktop/zenith-compositor/src/workspace.c`
- `desktop/zenith-compositor/src/keybindings.c`
- `desktop/zenith-compositor/meson.build`

### After compositor:
1. Build zenith-panel (clock + workspace indicator)
2. Build zenith-session (session startup)
3. Build osctl system status
4. Build rootfs and ISO
5. Test in QEMU
