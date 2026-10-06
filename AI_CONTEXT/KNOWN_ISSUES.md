# Known Issues

## Phase 1

_No issues yet — implementation in progress._

## Notes

- wlroots API varies between versions. Code targets wlroots >= 0.17.
  If building on Debian Bookworm, may need to build wlroots from source.
- gtk4-layer-shell may not be in all distro repos.
  May need to build from source: https://github.com/wmww/gtk4-layer-shell
- ISO build requires root (debootstrap, mount). Use `sudo make iso` or
  build inside Docker container with `make docker-build`.
